/**
\file   dpt.cpp
\brief  Dual Pressure Transducer Algorithm
\author Aleksey Lekhtin
\date   06.05.2026
*/

#include "dpt.h"

#include "cfg.h"
#include "chart.h"
#include "dbg.h"
#include "dp_dump.h"
#include "mw.h"
#include "network.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QStorageInfo>

//---------------------------------------------------------
DPT::DPT()
{
    DBG(1, "\n");
    dpt = this;
    m_dump = new DPDUMP();

    read_status();
    m_chart1 = new CHART(1, tr("Main Pressure Sensor"));
    m_chart1->m_channel = chart1_selected_channel;
    chart_reset(m_chart1);
    m_chart1->set_line_color(Qt::darkGreen);

    m_chart2 = new CHART(2, tr("Additional pressure sensor"));
    m_chart2->m_channel = chart2_selected_channel;
    chart_reset(m_chart2);
    m_chart2->set_line_color(Qt::red);

    m_chart3 = new CHART(3, tr("Overlay chart of two channels"));
    m_chart3->m_channel = 100; // special number, just > 4
    chart_reset(m_chart3);
    m_chart3->set_line_color(Qt::darkGreen, 1);
    m_chart3->set_line_color(Qt::red, 2);

    m_chart4 = new CHART(4, tr("Chart of Signal addition"));
    m_chart4->m_channel = 200; // special number, just > 4
    chart_reset(m_chart4);
    m_chart4->set_line_color(Qt::darkCyan);
    m_chart4->set_line_width(1);

    m_chart1->showAsRaw = m_raw_button_status;
    m_chart2->showAsRaw = m_raw_button_status;
    m_chart1->scale_Y = m_yscale_button_status;
    m_chart2->scale_Y = m_yscale_button_status;
    m_chart3->scale_Y = m_yscale_button_status;
    m_chart4->scale_Y = m_yscale_button_status;

    connect(NETWORK::srv, &NETWORK::signal_points_updated, this, &DPT::slot_set_points, Qt::BlockingQueuedConnection);

    m_page_name = tr("Two Pressure Sensors");
    m_gui_page = create_page();
    set_fonts();
}

//---------------------------------------------------------
DPT::~DPT()
{
#ifdef D_B_G
    DBG(4, "++");
#endif

    save_status();
    delete m_chart1;
    delete m_chart2;
    delete m_chart3;
    delete m_chart4;
    delete m_dump;
    delete m_dump_gui;

#ifdef D_B_G
    DBG(4, "--");
#endif
}

//---------------------------------------------------------
const QString &DPT::page_name()
{
    return m_page_name;
}

//---------------------------------------------------------
void DPT::read_status()
{
    DBG(1);
    auto status_file = CFG::cfg->app_status();
    if (!status_file) {
        return;
    }
    status_file->beginGroup("DPT");
    chart1_selected_channel = status_file->value("DPT_Chart1ChSelected", 1).toInt();
    chart2_selected_channel = status_file->value("DPT_Chart2ChSelected", 2).toInt();
    m_yscale_button_status = status_file->value("StretchY", false).toBool();
    m_raw_button_status = status_file->value("RawY", false).toBool();
    status_file->endGroup();
}

//---------------------------------------------------------
void DPT::save_status() const
{
    DBG(1);
    auto status_file = CFG::cfg->app_status();
    if (!status_file) {
        return;
    }
    status_file->beginGroup("DPT");
    status_file->setValue("DPT_Chart1ChSelected", chart1_selected_channel);
    status_file->setValue("DPT_Chart2ChSelected", chart2_selected_channel);
    status_file->setValue("StretchY", m_yscale_button_status);
    status_file->setValue("RawY", m_raw_button_status);
    status_file->endGroup();
}

//---------------------------------------------------------
void DPT::set_axisY_format(const QString &format)
{
    m_chart1->set_axisY_format(format);
    m_chart2->set_axisY_format(format);
    m_chart3->set_axisY_format(format);
    m_chart4->set_axisY_format(format);
}

//---------------------------------------------------------
void DPT::chart_reset(CHART *chart)
{
    DBGF(3, "chart-{}, channel:{}", chart->chart_num, chart->m_channel);
    chart->clean();
    if (chart->m_channel == 0) {
        return;
    }

    if ((chart->chart_num == 3 || chart->chart_num == 4) && m_chart1->showAsRaw) {
        chart->set_title(tr("The chart does not support scaling for this mode."));
    } else {
        chart->set_title(chart->base_title);
    }

    QRectF currentRect = chart->chart()->geometry();
    chart->chart()->setGeometry(currentRect.adjusted(0, 0, 1, 0));  // Увеличили на 1 пиксель
    chart->chart()->setGeometry(currentRect);

    chart->update();
    chart->chart()->update();

    chart->viewport()->update();
    chart->repaint();


    if (chart->chart_num == 1 || chart->chart_num == 2 || chart->chart_num == 3) {
        if (chart->showAsRaw) {
            chart->axisY()->setTitleText("Raw Values");
            chart->axisY()->setLabelFormat("0x%08X");
            chart->set_y_range1({0, (1 << 23) - 1});
            chart->set_y_range2({0, (1 << 23) - 1});
        } else {
            chart->axisY()->setTitleText(tr("Voltage"));
            auto str = "%" + QString("0.%1f").arg(CFG::cfg->decimal_points);
            chart->axisY()->setLabelFormat(str);
            chart->set_y_range1({0, CFG::cfg->axisY_range});
            chart->set_y_range2({0, CFG::cfg->axisY_range});
        }
    } else if (chart->chart_num == 4) {
        chart->axisY()->setTitleText(tr("Voltage"));
        auto str = "%" + QString("0.%1f").arg(CFG::cfg->decimal_points);
        chart->axisY()->setLabelFormat(str);
        chart->set_y_range1({0, CFG::cfg->axisY_range * 2});
        chart->set_y_range2({0, CFG::cfg->axisY_range * 2});
    }
    // chart->axisY()->setTickCount(11);

    chart->axisX()->setTitleText("ms");
    chart->axisX()->setLabelFormat("%i"); // "%0.1f"

    m_scale = 0;
    if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {

        chart->axisX()->setRange(atlas_chart1_x_def_range[0], atlas_chart1_x_def_range[1]);
        chart->axisX()->setTickCount(atlas_chart1_x_tick_count_def + 1);
        chart->axisX()->setMinorTickCount(atlas_chart1_x_minor_tick_count_def - 1);

        m_scale = CFG::cfg->atlas_scale_factor;

        DBGF(3,
            "chart-{}: X range: {:0.2} - {:0.2f}",
            chart->chart_num,
            chart->axisX()->min(),
            (double)chart->axisX()->max());

    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::ADS || CFG::cfg->data_format == CFG::DATA_FORMAT::SURESHOT) {

        chart->axisX()->setRange(aps_chart1_x_def_range[0], aps_chart1_x_def_range[1]);
        chart->axisX()->setTickCount(aps_chart1_x_tick_count_def + 1);
        chart->axisX()->setMinorTickCount(aps_chart1_x_minor_tick_count_def - 1);

        m_scale = CFG::cfg->ads_scale_factor;
    }

    DBGF(3, "adc scale: {:0.18f}", m_scale);
}

//---------------------------------------------------------
void DPT::set_points(CHART *chart)
{
    int chnum = chart->m_channel;
    const channel_points *ref_chanl{};
    if (chnum == 1) {
        ref_chanl = &m_channels_points.ch1();
    } else if (chnum == 2) {
        ref_chanl = &m_channels_points.ch2();
    } else if (chnum == 3) {
        ref_chanl = &m_channels_points.ch3();
    } else if (chnum == 4) {
        ref_chanl = &m_channels_points.ch4();
    } else {
        DBG(0, "500: INTERNAL ERROR: no channel - nothing to accumulate");
        return;
    }

    bool none = chart->showAsRaw ? ref_chanl->points_raw().empty() : ref_chanl->points().empty();
    if (none) {
        DBGF(3, "chart-{}: channel:{} - no points", chart->chart_num, chnum);
        return;
    }
    DBGF(3, "chart-{}: channel:{}", chart->chart_num, chnum);

    //// if (chart->showAsRaw) {
    ////     chart->set_points(ref_pnts->data_raw);
    //// } else {
    ////     chart->set_points(ref_pnts->data);
    //// }

    chart->set_points(*ref_chanl);
}

//---------------------------------------------------------
void DPT::set_points_two_channels(CHART *chart)
{
    DBGF(3, "chart-{}: channel:{}", chart->chart_num, chart->m_channel);

    if (m_chart1->showAsRaw) {
        chart->series()->clear();
        chart->series2()->clear();
        return;
    }

    m_delay_field->setSingleStep((int)m_channels_points.ch1().dt());
    auto tip = QString("%1\n%2 %3 ms\n[%4 - %5]")
                   .arg(tr("Compensation delay between the channels"), tr("Step:"))
                   .arg(m_channels_points.ch1().dt())
                   .arg(m_delay_field->minimum())
                   .arg(m_delay_field->maximum());
    m_delay_field->setToolTip(tip);

    //// chart->set_points(m_channels_points.ch1.data, 1);
    //// chart->set_points(m_channels_points.ch2.data, 2);

    chart->set_points(m_channels_points.ch1(), 1);
    chart->set_points(m_channels_points.ch1(), 2);


    //// if (chart->scale_Y) {
    ////     //// auto yrange1 = chart->yrange1();
    ////     //// auto yrange2 = chart->yrange2();
    ////     //// yrange1.first = std::min(yrange1.first, yrange2.first);
    ////     //// yrange1.second = std::max(yrange1.second, yrange2.second);
    ////     //// chart->set_y_range(yrange1);
    ////     chart->refresh();
    //// }
}

//---------------------------------------------------------
void DPT::set_points_sum_channels(CHART *chart)
{
    DBGF(3,
        "chart-{}: channel:{}  mode:{}{}{}, delay: {} ms",
        chart->chart_num,
        chart->m_channel,
        (int)m_chart4_mode,
        (m_chart4_mode == SUM_OR_SUB_MODE::SUM ? "[+]" : ""),
        (m_chart4_mode == SUM_OR_SUB_MODE::SUB ? "[-]" : ""),
        delay);

    if (m_chart1->showAsRaw) {
        chart->series()->clear();
        return;
    }

    if (m_chart4_mode != SUM_OR_SUB_MODE::SUM && m_chart4_mode != SUM_OR_SUB_MODE::SUB) {
        chart->series()->clear();
        DBGF(3, "    chart-{}: mode is not selected", chart->chart_num);
        return;
    }

    auto *pnts1 = m_channels_points.ch1().points().data();
    auto *pnts2 = m_channels_points.ch2().points().data();
    const size_t sz = m_channels_points.ch1().size();

    channel_points diff{};
    diff.points().resize(sz);
    auto *dst = diff.points().data();

    if (m_chart4_mode == SUM_OR_SUB_MODE::SUM) {
        for (size_t i = 0; i < sz; ++i) {
            dst[i] = pnts1[i] + pnts2[i];
        }
    } else {
        for (size_t i = 0; i < sz; ++i) {
            dst[i] = pnts1[i] - pnts2[i];
        }
    }

    auto [min_it, max_it] = std::minmax_element(diff.points().begin(), diff.points().end());
    diff.points().set_yminmax({*min_it, *max_it});

    diff.set_dt(m_channels_points.ch1().dt());
    auto x_max = m_channels_points.ch1().timerange() - m_channels_points.ch1().dt();
    diff.points().set_xminmax({0, x_max});

    if (m_chart4_mode == SUM_OR_SUB_MODE::SUM) {
        if (chart->axisY()->max() != CFG::cfg->axisY_range * 2) {
            chart->axisY()->setRange(0, CFG::cfg->axisY_range * 2);
        }
    } else {
        // SUM_OR_SUB_MODE::SUB
        if (diff.points().yminmax().first < 0) {
            auto ymin = std::abs(diff.points().yminmax().first);
            auto ymax = std::abs(diff.points().yminmax().second);
            auto value = std::max(ymin, ymax);
            auto yrange_min = std::floor(-value);
            auto yrange_max = std::ceil(value);
            chart->axisY()->setRange(yrange_min, yrange_max);
            auto dy = diff.points().yminmax().second - diff.points().yminmax().first;
            //// auto title = QString("%1<div align='center'>Y: [%2 - %3] Amplitude: %4</dev>")
            auto title = QString("%1 &nbsp; &nbsp; Y: [%2 - %3] &nbsp; Amplitude: <b>%4</b> %5")
                             .arg(chart->base_title)
                             .arg(diff.points().yminmax().first, 0, 'f', 6, QChar('0'))
                             .arg(diff.points().yminmax().second, 0, 'f', 6, QChar('0'))
                             .arg(dy, 0, 'f', 6, QChar('0'))
                             .arg("В");
            DBGF(3,
                "chart-{}: Y: [{} - {}], range: [{} - {}], ampl: {}",
                chart->chart_num,
                diff.points().yminmax().first,
                diff.points().yminmax().second,
                yrange_min,
                yrange_max,
                dy);
            ////chart->chart()->setTitle(title);
            ////chart->update();
            chart->set_title(title);
        }
    }
    m_chart4->set_points(diff);
}

//---------------------------------------------------------
void DPT::set_points(BBIPoints &points)
{
    m_channels_points.ch1() = channel_points(1, points.ch1());
    m_channels_points.ch2() = channel_points(2, points.ch2());
    m_channels_points.ch3() = channel_points(3, points.ch3());
    m_channels_points.ch4() = channel_points(4, points.ch4());
    m_channels_points.set_encoder1(points.encoder1());
    m_channels_points.set_encoder2(points.encoder2());
}

//---------------------------------------------------------
void DPT::slot_set_points()
{
    if (MW::mw->currentTabWidget() != dpt->gui_page()) {
        return;
    }
    DBG(3);
    MW::mw->statusbar_show(tr("Receiving"), 300);
    auto decimal = CFG::cfg->decimal_points;

    if (m_do_dump) {
        m_dump->dump_data(m_channels_points);
    }

    // 0 => Nothing to show
    if (m_chart1->m_channel > 0) {
        set_points(m_chart1);
    }

    // 0 => Nothing to show
    if (m_chart2->m_channel > 0) {
        set_points(m_chart2);
    }

    set_points_two_channels(m_chart3); // draw chart for two pressure
    set_points_sum_channels(m_chart4); // draw chart for dual pressure transducer

    auto r1 = m_chart1->yrange1();
    auto r2 = m_chart2->yrange1();
    if (m_chart1->showAsRaw) {
        m_chart1_max_min_values[0]->setText(QString("HEX: %1").arg((uint32_t)r1.first, 8, 16, QChar('0')).toUpper());
        m_chart1_max_min_values[1]->setText(QString("HEX: %1").arg((uint32_t)r1.second, 8, 16, QChar('0')).toUpper());

        m_chart2_max_min_values[0]->setText(QString("HEX: %1").arg((uint32_t)r2.first, 8, 16, QChar('0')).toUpper());
        m_chart2_max_min_values[1]->setText(QString("HEX: %1").arg((uint32_t)r2.second, 8, 16, QChar('0')).toUpper());
    } else {
        m_chart1_max_min_values[0]->setText(QString("%1 %2").arg(r1.first, 0, 'f', decimal, QChar('0')).arg(tr("V")));
        m_chart1_max_min_values[1]->setText(QString("%1 %2").arg(r1.second, 0, 'f', decimal, QChar('0')).arg(tr("V")));

        m_chart2_max_min_values[0]->setText(QString("%1 %2").arg(r2.first, 0, 'f', decimal, QChar('0')).arg(tr("V")));
        m_chart2_max_min_values[1]->setText(QString("%1 %2").arg(r2.second, 0, 'f', decimal, QChar('0')).arg(tr("V")));
    }
}
