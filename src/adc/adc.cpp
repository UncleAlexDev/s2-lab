/**
\file   ADCTab.cpp
\brief  The tab with ADC numbers and charts
\author Aleksey Lekhtin
\date   06.05.2026
*/

#include "adc.h"

#include "cfg.h"
#include "chart.h"
#include "dbg.h"
#include "mw.h"
#include "network.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QHBoxLayout>

ADC::ADC()
{
    DBG(1);
    adc = this;

    read_status();

    m_chart1 = new CHART(1, tr("Chart 1"));
    m_chart1->m_channel = m_chart1_selected_channel;
    m_chart1->showAsRaw = m_raw_button_status;
    m_chart1->magnify_Y = m_magnify_button_status;
    m_chart1->scale_Y = m_yscale_button_status;
    chart_reset(m_chart1);

    m_chart2 = new CHART(2, tr("Chart 2"));
    m_chart2->m_channel = m_chart2_selected_channel;
    m_chart2->scale_Y = m_yscale_button_status;
    m_chart2->showAsRaw = m_raw_button_status;
    m_chart2->magnify_Y = m_magnify_button_status;
    chart_reset(m_chart2);

    connect(NETWORK::srv, &NETWORK::signal_points_updated,
        this, &ADC::slot_set_points, Qt::BlockingQueuedConnection);

    m_page_name = tr("ADC");
    m_gui_page = create_page();
    set_fonts();
}

//---------------------------------------------------------
ADC::~ADC()
{
#ifdef D_B_G
    DBG(7, "    ++");
#endif
    save_status();
    delete m_chart1;
    delete m_chart2;
    delete m_shortcut;
#ifdef D_B_G
    DBG(7, "    --");
#endif
}

//---------------------------------------------------------
void ADC::read_status()
{
    DBG(1);
    auto status_file = CFG::cfg->app_status();
    if (!status_file) {
        return;
    }
    status_file->beginGroup("ADC");
    m_chart1_selected_channel = status_file->value("Chart1ChSelected", 1).toInt();
    m_chart2_selected_channel = status_file->value("Chart2ChSelected", 2).toInt();
    m_yscale_button_status = status_file->value("StretchY", false).toBool();
    m_magnify_button_status = status_file->value("MagnifyY", false).toBool();
    m_raw_button_status = status_file->value("RawY", false).toBool();
    status_file->endGroup();
}

//---------------------------------------------------------
void ADC::save_status() const
{
    DBG(1);
    auto status_file = CFG::cfg->app_status();
    if (!status_file) {
        return;
    }
    status_file->beginGroup("ADC");
    status_file->setValue("Chart1ChSelected", m_chart1_selected_channel);
    status_file->setValue("Chart2ChSelected", m_chart2_selected_channel);
    status_file->setValue("StretchY", m_yscale_button_status);
    status_file->setValue("MagnifyY", m_magnify_button_status);
    status_file->setValue("RawY", m_raw_button_status);
    status_file->endGroup();
}

//---------------------------------------------------------
QString ADC::make_title(CHART *chart)
{
    int prec = CFG::cfg->decimal_points;
    //auto xrange = chart->xrange();
    auto yrange = chart->yrange1();

    QString msg1 =
        QString("%1 %2 ").arg(tr("Points:")).arg(chart->count());

    QString msg2;
    if (chart->magnify_Y) {
        auto dy = yrange.second - yrange.first;
        if (chart->showAsRaw) {
            msg2 = QString(", %1:  0x%2").arg(tr("Peak-to-Peak")).arg((int32_t)dy, 6, 16, QChar('0'));
        } else {
            auto str = QString::number(dy, 'f', prec).toUpper();
            msg2 = QString(", %1:  %2 %3").arg(tr("Peak-to-Peak"), str, "В");
        }
    }
    return msg1 + msg2;
}

//---------------------------------------------------------
void ADC::set_axisY_format(const QString &format)
{
    m_chart1->set_axisY_format(format);
    m_chart2->set_axisY_format(format);
}

//---------------------------------------------------------
void ADC::chart_reset(CHART *chart)
{
    DBGF(3, "chart-{}, channel:{}", chart->chart_num, chart->m_channel);
    chart->clean();

    // X axis
    chart->axisX()->setTitleText("ms");
    chart->axisX()->setLabelFormat("%0.1f");

    if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {
        chart->axisX()->setRange(atlas_chart1_x_def_range[0], atlas_chart1_x_def_range[1]);
        chart->axisX()->setTickCount(atlas_chart1_x_tick_count_def + 1);
        chart->axisX()->setMinorTickCount(atlas_chart1_x_minor_tick_count_def - 1);

        if (chart->m_channel == 5) {
            chart->axisX()->setRange(0, chart->ch5_fix_range());
            chart->axisX()->setTickCount(atlas_chart1_x_tick_count_def + 1);
            chart->axisX()->setMinorTickCount(atlas_chart1_x_minor_tick_count_def - 1);
        }

    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::ADS ||
               CFG::cfg->data_format == CFG::DATA_FORMAT::SURESHOT) {
        chart->axisX()->setRange(aps_chart1_x_def_range[0], aps_chart1_x_def_range[1]);
        chart->axisX()->setTickCount(aps_chart1_x_tick_count_def + 1);
        chart->axisX()->setMinorTickCount(aps_chart1_x_minor_tick_count_def - 1);
    }

    // Y axis
    if (chart->showAsRaw) {
        chart->axisY()->setTitleText("Raw Values");
        chart->axisY()->setLabelFormat("0x%08X");
        chart->axisY()->setTickCount(aps_chart1_x_tick_count_def + 1);
        chart->axisY()->setRange(0, (1 << 23) - 1);
    } else {
        chart->axisY()->setTitleText(tr("Voltage"));
        auto str = "%" + QString("0.%1f").arg(CFG::cfg->decimal_points);
        chart->axisY()->setLabelFormat(str);
        chart->axisY()->setTickCount(aps_chart1_x_tick_count_def + 1);
        if (!chart->scale_Y) {
            chart->axisY()->setRange(0, CFG::cfg->axisY_range);
        } else {
            auto mn = chart->axisY()->min(); // TOFIX
            auto mx = chart->axisY()->max();
            chart->axisY()->setRange(mn, mx);
        }
    }

    if (DBG::debug_level >= 3) {
        scale = 0;
        if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {
            scale = CFG::cfg->atlas_scale_factor;
        } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::ADS ||
                   CFG::cfg->data_format == CFG::DATA_FORMAT::SURESHOT) {
            scale = CFG::cfg->ads_scale_factor;
        }
        DBGF(3, "adc scale: {:0.18f}", scale);
    }
}

//---------------------------------------------------------
void ADC::set_points(CHART *chart)
{
    int chnum = chart->m_channel;
    DBGF(3, "\nchart-{}: channel:{}, scale:{:0.18f}", chart->chart_num, chnum, scale);
    const channel_points *ref_chanl{};
    if (chnum == 1) {
        ref_chanl = &m_channels_points.ch1();
    } else if (chnum == 2) {
        ref_chanl = &m_channels_points.ch2();
    } else if (chnum == 3) {
        ref_chanl = &m_channels_points.ch3();
    } else if (chnum == 4) {
        ref_chanl = &m_channels_points.ch4();
    } else if (chnum == 5) {
        if (m_chart1_selected_channel == 1) {
            ref_chanl = &m_channels_points.ch1();
        } else if (m_chart1_selected_channel == 2) {
            ref_chanl = &m_channels_points.ch2();
        } else if (m_chart1_selected_channel == 3) {
            ref_chanl = &m_channels_points.ch3();
        } else if (m_chart1_selected_channel == 4) {
            ref_chanl = &m_channels_points.ch4();
        } else {
            DBG(0, "500: INTERNAL ERROR: no channel - nothing to accumulate");
            return;
        }
    } else {
        DBGF(0, "500: INTERNAL ERROR: chart-{}: requested wrong channel:{}", chart->chart_num, chnum);
        return;
    }

    bool none = chart->showAsRaw ? ref_chanl->points_raw().empty() : ref_chanl->points().empty();
    if (none) {
        DBGF(3, "chart-{}: channel:{} - no points", chart->chart_num, chnum);
        return;
    }

    if (DBG::debug_level >= 4) {
        NETWORK::dump(ref_chanl->points().vector());
    }

    chart->set_points(*ref_chanl);
    chart->set_title(make_title(chart));
}

//---------------------------------------------------------
void ADC::set_points(BBIPoints &points)
{
    m_channels_points.set_points(1, points.ch1());
    m_channels_points.set_points(2, points.ch2());
    m_channels_points.set_points(3, points.ch3());
    m_channels_points.set_points(4, points.ch4());

    m_channels_points.set_encoder1(points.encoder1());
    m_channels_points.set_encoder2(points.encoder2());

    // clang-format off
    DBGF(3, "channel: 1, {} - x min:{:0.2f} - x max:{:0.2f} y min:{:11.8f} - y max:{:11.8f} avrg:{:11.8f}",
        m_channels_points.ch1().timestamp(),
        m_channels_points.ch1().points().xminmax().first,
        m_channels_points.ch1().points().xminmax().second,
        m_channels_points.ch1().points().yminmax().first,
        m_channels_points.ch1().points().yminmax().second,
        m_channels_points.ch1().points().avrg());

    DBGF(3, "channel: 1, raw           - x min:{:0.2f} - x max:{:0.2f} y min:{:11.2f} - y max:{:11.2f} avrg:{:11.2f}",
        m_channels_points.ch1().points_raw().xminmax().first,
        m_channels_points.ch1().points_raw().xminmax().second,
        m_channels_points.ch1().points_raw().yminmax().first,
        m_channels_points.ch1().points_raw().yminmax().second,
        m_channels_points.ch1().points_raw().avrg());

    DBGF(3, "channel: 2, {} - x min:{:0.2f} - x max:{:0.2f} y min:{:11.8f} - y max:{:11.8f} avrg:{:11.8f}",
        m_channels_points.ch2().timestamp(),
        m_channels_points.ch2().points().xminmax().first,
        m_channels_points.ch2().points().xminmax().second,
        m_channels_points.ch2().points().yminmax().first,
        m_channels_points.ch2().points().yminmax().second,
        m_channels_points.ch2().points().avrg());

    DBGF(3, "channel: 2, raw           - x min:{:0.2f} - x max:{:0.2f} y min:{:11.8f} - y max:{:11.2f} avrg:{:11.2f}",
        m_channels_points.ch2().points_raw().xminmax().first,
        m_channels_points.ch2().points_raw().xminmax().second,
        m_channels_points.ch2().points_raw().yminmax().first,
        m_channels_points.ch2().points_raw().yminmax().second,
        m_channels_points.ch2().points_raw().avrg());

    // clang-format on
}

//---------------------------------------------------------
void ADC::slot_set_points()
{
    if (MW::mw->currentTabWidget() != gui_page()) {
        return;
    }
    DBG(3);
    MW::mw->statusbar_show(tr("Receiving"), 300);
    auto decimal = CFG::cfg->decimal_points;

    // 0 => Nothing to show
    if (m_chart1->m_channel > 0) {
        set_points(m_chart1);
    }

    if (m_chart2->m_channel > 0) {
        set_points(m_chart2);
    }

    auto r1 = m_chart1->yminmax1();
    auto r2 = m_chart2->yminmax1();
    auto avrg1 = m_chart1->avrg1();
    auto avrg2 = m_chart2->avrg1();
    if (m_chart1->showAsRaw) {
        m_chart1_max_min_values[0]->setText(QString("%1").arg((uint32_t)r1.first, 8, 16, QChar('0')).toUpper());
        m_chart1_max_min_values[1]->setText(QString("%1").arg((uint32_t)r1.second, 8, 16, QChar('0')).toUpper());
        m_chart1_avrg_value->setText(QString("%1").arg((uint32_t)avrg1, 8, 16, QChar('0')).toUpper());

        m_chart2_max_min_values[0]->setText(QString("%1").arg((uint32_t)r2.first, 8, 16, QChar('0')).toUpper());
        m_chart2_max_min_values[1]->setText(QString("%1").arg((uint32_t)r2.second, 8, 16, QChar('0')).toUpper());
        m_chart2_avrg_value->setText(QString("%1").arg((uint32_t)avrg2, 8, 16, QChar('0')).toUpper());
    } else {
        m_chart1_max_min_values[0]->setText(QString("%1 %2").arg(r1.first, 0, 'f', decimal, QChar('0')).arg(tr("V")));
        m_chart1_max_min_values[1]->setText(QString("%1 %2").arg(r1.second, 0, 'f', decimal, QChar('0')).arg(tr("V")));
        m_chart1_avrg_value->setText(QString("%1 %2").arg(avrg1, 0, 'f', decimal, QChar('0')).arg(tr("V")));

        m_chart2_max_min_values[0]->setText(QString("%1 %2").arg(r2.first, 0, 'f', decimal, QChar('0')).arg(tr("V")));
        m_chart2_max_min_values[1]->setText(QString("%1 %2").arg(r2.second, 0, 'f', decimal, QChar('0')).arg(tr("V")));
        m_chart2_avrg_value->setText(QString("%1 %2").arg(avrg2, 0, 'f', decimal, QChar('0')).arg(tr("V")));
    }

    m_encoder[0]->setText(QString::number(m_channels_points.encoder1()));
    m_encoder[1]->setText(QString::number(m_channels_points.encoder2()));
}
