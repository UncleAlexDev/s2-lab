/**
  \file   dpt_dump_gui.cpp
  \brief  Dual Pressure Dump -  widget page
  \author Aleksey Lekhtin
  \date   06.07.2026
*/

#include "dp_dump_view.h"

#include "cfg.h"
#include "chart.h"
#include "dbg.h"
#include "dp_dump.h"
#include "dpt.h"
#include "mw.h"
#include "packet_atlas_t.h"
#include "utils.h"
#include "wmsg.h"

#include <fmt/chrono.h>

#include <QApplication.h>
#include <QDir.h>
#include <QFileDialog>
#include <QLineEdit.h>
#include <QTimer>
#include <QVBoxLayout>

//---------------------------------------------------------
DPTDUMP_VIEW::DPTDUMP_VIEW()
{
    DBG(1, "\n");
    gui = this;
    read_status();

    m_chart1 = new CHART(1, tr("Main pressure Sensor"));
    m_chart1->m_channel = 1;
    chart_reset(m_chart1);
    m_chart1->set_line_color(Qt::darkGreen);

    m_chart2 = new CHART(2, tr("Additional pressure sensor"));
    m_chart2->m_channel = 2;
    chart_reset(m_chart2);
    m_chart2->set_line_color(Qt::red);

    m_chart3 = new CHART(3, tr("Overlay chart of two channels"));
    m_chart3->m_channel = 3;
    chart_reset(m_chart3);
    m_chart3->set_line_color(Qt::darkGreen, 1);
    m_chart3->set_line_color(Qt::red, 2);

    m_chart4 = new CHART(4, tr("Signal overlap chart"));
    m_chart4->m_channel = 4;
    chart_reset(m_chart4);
    m_chart4->set_line_color(Qt::darkCyan);
    m_chart4->set_line_width(1);

    m_page_name = tr("Two Pressure Dump");
    m_gui_page = create_page();
    set_fonts();
}

//---------------------------------------------------------
DPTDUMP_VIEW::~DPTDUMP_VIEW()
{
#ifdef D_B_G
    DBG(4, "++");
#endif

    save_status();

#ifdef D_B_G
    DBG(4, "--");
#endif
}

//---------------------------------------------------------
void DPTDUMP_VIEW::read_status()
{
    DBG(1);
}

//---------------------------------------------------------
void DPTDUMP_VIEW::save_status() const
{
    DBG(1);
}

//---------------------------------------------------------
void DPTDUMP_VIEW::chart_reset(CHART *chart)
{
    DBGF(3, "chart-{}, channel:{}", chart->chart_num, chart->m_channel);
    chart->clean();
    if (chart->m_channel == 0) {
        return;
    }

    if ((chart->chart_num == 3 || chart->chart_num == 4) && m_chart1->showAsRaw) {
        ////chart->chart()->setTitle(tr("The chart does not support scaling for this mode."));
        chart->set_title(tr("The chart does not support scaling for this mode."));
    } else {
        ////chart->chart()->setTitle(chart->base_title);
        chart->set_title(chart->base_title);
    }
    ////chart->chart()->update();

    if (chart->chart_num == 1 || chart->chart_num == 2 || chart->chart_num == 3) {
        if (chart->showAsRaw) {
            chart->axisY()->setTitleText("Raw Values");
            chart->axisY()->setLabelFormat("0x%08X");
            chart->axisY()->setRange(0, (1 << 23) - 1);
        } else {
            chart->axisY()->setTitleText(tr("Voltage"));
            auto str = "%" + QString("0.%1f").arg(CFG::cfg->decimal_points);
            chart->axisY()->setLabelFormat(str);
            chart->axisY()->setRange(0, CFG::cfg->axisY_range);
        }
    } else if (chart->chart_num == 4) {
        chart->axisY()->setTitleText(tr("Voltage"));
        auto str = "%" + QString("0.%1f").arg(CFG::cfg->decimal_points);
        chart->axisY()->setLabelFormat(str);
        chart->axisY()->setRange(0, CFG::cfg->axisY_range * 2);
    }

    chart->axisX()->setTitleText("ms");
    chart->axisX()->setLabelFormat("%i"); // "%0.1f"

    double m_scale = 0;
    if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {

        chart->axisX()->setRange(DPT::atlas_chart1_x_def_range[0], DPT::atlas_chart1_x_def_range[1]);
        chart->axisX()->setTickCount(DPT::atlas_chart1_x_tick_count_def + 1);
        chart->axisX()->setMinorTickCount(DPT::atlas_chart1_x_minor_tick_count_def - 1);

        m_scale = CFG::cfg->atlas_scale_factor;

        DBGF(3,
            "chart-{}: X range: {:0.2} - {:0.2f}",
            chart->chart_num,
            chart->axisX()->min(),
            (double)chart->axisX()->max());
    }

    DBGF(3, "adc scale: {:0.18f}", m_scale);
}

//---------------------------------------------------------
bool DPTDUMP_VIEW::select_dump_file(const QString &dirname)
{
    QDir dir;
    if (!dirname.isEmpty()) {
        dir.setPath(dirname);
    } else {
        dir.setPath(QDir::homePath());
    }

    if (!dir.exists()) {
        WMSG::WMsg(MW::mw,
            QString("%1\n\n%2").arg(tr("Directory not found"), QDir::toNativeSeparators(dir.absolutePath())),
            QMessageBox::Warning);
        return false;
    }

    QString fn = QFileDialog::getOpenFileName(MW::mw,
        tr("Select Dump File"),
        dir.absolutePath(),
        tr("Dump (*.raw);; All (*.*)"));
    if (fn.isEmpty()) {
        return false;
    }
    m_fullname = QDir::toNativeSeparators(fn);

    MW::mw->statusbar_show(QString("%1: %2").arg(QObject::tr("dump file"), m_fullname), MW::Bold);
    return true;
}

//---------------------------------------------------------
void DPTDUMP_VIEW::load(const QString &dirname)
{
    MW::mw->statusbar_clear();
    if (!select_dump_file(dirname)) {
        return;
    }

#if 0
    int index = MW::mw->tabWidget()->indexOf(m_gui_page);
    if (index == -1) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    MW::mw->tabWidget()->setTabVisible(index, true);
    MW::mw->tabWidget()->setCurrentIndex(index);
#else
    int idx = MW::mw->tabWidget()->indexOf(m_gui_page);
    if (idx == -1) {
        MW::mw->tabWidget()->insertTab(idx, m_gui_page, m_page_name);
        auto tip = QString("%1\n\n[%2]")
                       .arg(MW::mw->tabWidget()->tabText(index), tr("PgUp or PgDown to select prev/next tab"));
        MW::mw->tabWidget()->setTabToolTip(idx, tip);
    }
#endif
#endif
    MW::mw->lock_my_tab(m_gui_page);
    load_dump();
}

//---------------------------------------------------------
void DPTDUMP_VIEW::load_dump()
{
    DPDUMP::dump_t dump{};

    if (m_fullname.isEmpty()) {
        auto msg = QString("500: INTERNAL ERROR\n\n%1").arg(tr("File name is not specified"));
        WMSG::WMsg(MW::mw, msg, QMessageBox::Critical);
        return;
    }

    QFile file(m_fullname);
    if (!file.open(QIODevice::ReadOnly)) {
        auto errmsg = QString("%1\n(%2)").arg(file.errorString()).arg(file.error());
        WMSG::WMsg(MW::mw,
            QString("%1\n\n%2\n\n%3").arg(tr("Can't open file"), QDir::toNativeSeparators(file.fileName()), errmsg),
            QMessageBox::Warning);
        return;
    }

    auto off = file.pos();
    DBGF(3, "read header - offset: {} (0x{:08x})", off, off);
    if (file.read(reinterpret_cast<char *>(&dump.file_header), sizeof(dump.file_header)) != sizeof(dump.file_header)) {
        auto errmsg = QString("%1\n(%2)").arg(file.errorString()).arg(file.error());
        file.close();
        WMSG::WMsg(MW::mw,
            QString("%1\n\n%2\n\n%3").arg(tr("File too small"), QDir::toNativeSeparators(file.fileName()), errmsg),
            QMessageBox::Critical);
        return;
    }

    // проверки формата дампа
    if (dump.file_header.sign != DPDUMP::DP_SIGN) {
        auto errmsg = QString("%1\n(%2)").arg(file.errorString()).arg(file.error());
        file.close();
        WMSG::WMsg(MW::mw,
            QString("%1\n\n%2\n\n%3").arg(tr("Wrong signature"), QDir::toNativeSeparators(file.fileName()), errmsg),
            QMessageBox::Critical);
        return;
    }

    if (dump.file_header.version != DPDUMP::DP_VERSION1) {
        file.close();
        WMSG::WMsg(MW::mw,
            QString("%1\n\n%2").arg(tr("Wrong version"), QDir::toNativeSeparators(file.fileName())),
            QMessageBox::Critical);
        return;
    }

    DBGF(3, "dump created at {}", UTILS::get_date_str(dump.file_header.date));

    if (file.atEnd()) {
        file.close();
        WMSG::WMsg(MW::mw,
            QString("%1\n\n%2").arg(tr("No records"), QDir::toNativeSeparators(file.fileName())),
            QMessageBox::Information);
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString errmsg;

    //-------------------------------------------
    // чтение дампа
    bool ok = true;
    QString str;
    uint32_t nblock = 0;
    uint64_t timestamp_prev{0}, timerange{0};

    m_total_rows = 0;
    m_channels_points.clear();
    channel_points *chnl{};

    while (!file.atEnd() && ok) {

        ++nblock;

        // заголовок блока данных
        off = file.pos();
        if (file.read(reinterpret_cast<char *>(&dump.data.header), sizeof(dump.data.header)) !=
            sizeof(dump.data.header)) {
            errmsg = QString("%1\n\n(%2)").arg(file.errorString()).arg(file.error());
            DBGF(0, "{}", errmsg.toStdString());
            ok = false;
            break;
        }
        DBGF(3,
            "\noffset: {:>6} (0x{:08x}) read {:4} bytes, block {} (#{:>2} 0x{:08X}), nrecs: {}",
            off,
            off,
            sizeof(dump.data.header),
            nblock,
            (int)dump.data.header.block_number,
            (int)dump.data.header.block_number,
            dump.data.header.npnts);

        if (dump.data.header.ctrlw.check_byte != DPDUMP::DP_DUMP_CHECK_BYTE) {
            errmsg = QString("%1 #%2\n(offset in file: 0x%3)\n\n%4 0x%5 %6 0x%7")
                         .arg("Incorrect control byte in header of block")
                         .arg(nblock)
                         .arg(off, 2, 16, QChar('0'))
                         .arg(tr("Expected"))
                         .arg(DPDUMP::DP_DUMP_CHECK_BYTE, 2, 16, QChar('0'))
                         .arg(tr("but read"))
                         .arg(dump.data.header.ctrlw.check_byte, 2, 16, QChar('0'));
            DBGF(0, "{}", errmsg.toStdString());
            ok = false;
            break;
        }

        if (dump.data.header.block_number != nblock) {
            errmsg = QString("%1 #%2\n(offset in file: 0x%3)\n\n%4: %5 (0x%6) %7: %8 (0x%9)\n")
                         .arg("Incorrect header of block")
                         .arg(nblock)
                         .arg(off, 8, 16, QChar('0'))
                         .arg(tr("Block number"))
                         .arg(dump.data.header.block_number)
                         .arg(dump.data.header.block_number, 2, 16, QChar('0'))
                         .arg(tr("expected"))
                         .arg(nblock)
                         .arg(nblock);
            DBGF(0, "{}", errmsg.toStdString());
            ok = false;
            break;
        }

        if (dump.data.header.npnts != packet_atlas_t::npnts) {
            errmsg = QString("%1 #%2 \n(offset in file: 0x%3)\n\n%4: %5 (0x%6)\n%7: %8")
                         .arg("Wrong number of records of block")
                         .arg(nblock)
                         .arg(off, 8, 16, QChar('0'))
                         .arg(tr("records in a block"))
                         .arg(dump.data.header.npnts)
                         .arg(dump.data.header.npnts, 2, 16, QChar('0'))
                         .arg(tr("expected"))
                         .arg(packet_atlas_t::npnts);
            DBGF(0, "{}", errmsg.toStdString());
            ok = false;
            break;
        }

        //-------------------------------------------------
        if (timestamp_prev == 0) {
            timestamp_prev = dump.data.header.timestamp - 1000; // ms
        }
        timerange = dump.data.header.timestamp - timestamp_prev;
        DBGF(3,
            "{:>14} {}",
            "",
            fmt::format("timestamp: {}, timerange: {}",
                UTILS::get_date_str(dump.data.header.timestamp),
                dump.data.header.timestamp - timestamp_prev));
        timestamp_prev = dump.data.header.timestamp;

        m_channels_points.set_encoder1(dump.data.header.encoder1);
        m_channels_points.set_encoder2(dump.data.header.encoder2);

        //-------------------------------------------------
        auto npnts = dump.data.header.npnts;
        qint64 data_size = (qint64)npnts * (qint64)sizeof(uint32_t);

        // read 3 data block - pressure-1 pressure-2 hookload
        for (int chnum : {1, 2, 3}) {

            chnl = {};
            if (chnum == 1) {
                chnl = &m_channels_points.ch1();
            } else if (chnum == 2) {
                chnl = &m_channels_points.ch2();
            } else if (chnum == 3) {
                off = file.pos();
                DBGF(3, "offset: {:>6} (0x{:08x}) ch:{:>2} skip {:4} bytes", off, off, chnum, data_size);
                file.seek(file.pos() + data_size);
            }

            if (chnl) {
                off = file.pos();
                QByteArray bytes = file.read(data_size);
                if (bytes.size() != data_size) {
                    DBGF(0,
                        "    ERROR: offset: {:>6} (0x{:08x}), block {:>2}, size: {}",
                        off,
                        off,
                        (int)dump.data.header.block_number,
                        bytes.size());
                    errmsg = QString("%1\n\n(%2)").arg(file.errorString()).arg(file.error());
                    DBGF(0, "{}", errmsg.toStdString());
                    ok = false;
                    break;
                }
                DBGF(3, "offset: {:>6} (0x{:08x}) ch:{:>2} read {:4} bytes", off, off, chnum, data_size);

                if (chnl->timestamp() == 0) {
                    chnl->set_timestamp(dump.data.header.timestamp);
                }
                chnl->set_timerange(chnl->timerange() + timerange);

                auto *dst = &chnl->points().vector();
                auto *src = reinterpret_cast<const float *>(bytes.constData());

                dst->reserve(dst->size() + npnts);
                dst->insert(dst->end(), src, src + npnts);
                // std::transform(src, src + npnts, std::back_inserter(*dst),
                //     [](uint32_t val)
                //     {
                //         return static_cast<float>(val);
                //     });
            }
        }

        m_total_rows += npnts;
    }

    file.close();
    DBGF(2, "\ngot {} records", m_total_rows);

    if (ok) {
        for (int chnum : {1, 2}) {

            if (chnum == 1) {
                chnl = &m_channels_points.ch1();
            } else if (chnum == 2) {
                chnl = &m_channels_points.ch2();
            }

            int dt = chnl->dt();
            if (dt != 2) {
                DBGF(0, "dt: {}", dt);
            }
            auto x_max = chnl->timerange() - dt;
            chnl->points().set_xminmax({0, x_max});

            auto [min_it, max_it] = std::minmax_element(chnl->points().begin(), chnl->points().end());
            chnl->points().set_yminmax({*min_it, *max_it});
        }

        m_button_chart_minus->setChecked(true);
        m_chart4_mode = SUM_OR_SUB_MODE::SUB;
        set_points(m_channels_points);
    }

    QApplication::restoreOverrideCursor();
    if (!errmsg.isEmpty()) {
        WMSG::WMsg(MW::mw, errmsg, QMessageBox::Critical);
    }
}

//// //---------------------------------------------------------
//// void DPTDUMP_VIEW::rescale_two_channels(CHART *chart)
//// {
////     if (chart->scale_Y) {
////         auto yrange1 = chart->yrange();
////         auto yrange2 = chart->yrange2();
////         yrange1.first = std::min(yrange1.first, yrange2.first);
////         yrange1.second = std::max(yrange1.second, yrange2.second);
////         chart->set_y_range(yrange1);
////         return;
////     }
////
////     auto yrange1 = chart->yrange();
////     auto yrange2 = chart->yrange2();
////     yrange1.first = std::min(yrange1.first, yrange2.first);
////     yrange1.second = std::max(yrange1.second, yrange2.second);
////     chart->set_y_range(yrange1);
////     chart->refresh();
//// }

//---------------------------------------------------------
void DPTDUMP_VIEW::set_points_two_chs(CHART *chart)
{
    DBGF(3, "chart-{}: channel:{}", chart->chart_num, chart->m_channel);

    if (m_chart1->showAsRaw) {
        chart->series()->clear();
        chart->series2()->clear();
        return;
    }

    m_delay_field->setSingleStep((int)m_channels_points.ch1().dt());
    auto tip = QString("%1\n%2 %3\n[%4 - %5]")
                   .arg(tr("Compensation delay between the channels"), tr("Step:"))
                   .arg(m_channels_points.ch1().dt())
                   .arg(m_delay_field->minimum())
                   .arg(m_delay_field->maximum());
    m_delay_field->setToolTip(tip);

    m_channels_points.ch1().set_off(0);
    if (m_delay > 0) {
        // delay for Dual Pressure Transducer
        int dx = (int)std::abs((m_delay / m_channels_points.ch1().dt()));
        m_channels_points.ch1().set_off((int)dx);
    }

    //// chart->set_points(m_channels_points.ch1.data, 1);
    //// chart->set_points(m_channels_points.ch2.data, 2);

    chart->set_points(m_channels_points.ch1(), 1);
    chart->set_points(m_channels_points.ch2(), 2);

    if (chart->scale_Y) {
        //// auto yrange1 = chart->yrange1();
        //// auto yrange2 = chart->yrange2();
        //// yrange1.first = std::min(yrange1.first, yrange2.first);
        //// yrange1.second = std::max(yrange1.second, yrange2.second);
        //// chart->set_y_range(yrange1);
        chart->refresh();
    }
}

//---------------------------------------------------------
void DPTDUMP_VIEW::set_points_sum(CHART *chart)
{
    DBGF(3,
        "chart-{}: mode:{}{}, delay: {} ms",
        chart->chart_num,
        (m_chart4_mode == SUM_OR_SUB_MODE::SUM ? "[+]" : ""),
        (m_chart4_mode == SUM_OR_SUB_MODE::SUB ? "[-]" : ""),
        m_delay);

    if (m_chart4_mode != SUM_OR_SUB_MODE::SUM && m_chart4_mode != SUM_OR_SUB_MODE::SUB) {
        chart->series()->clear();
        DBGF(3, "mode not selected", chart->chart_num);
        return;
    }

    if (m_delay > 0) {
        int dx = (int)std::abs((m_delay / m_channels_points.ch1().dt()));
        m_channels_points.ch1().set_off(dx);
    }

    const auto pnts_size = m_channels_points.ch1().points().size() - m_channels_points.ch1().off();
    const auto *pnts1_ptr = m_channels_points.ch1().points().data() + m_channels_points.ch1().off();
    const auto *pnts2_ptr = m_channels_points.ch2().points().data() + m_channels_points.ch2().off();

    if (pnts_size == 0) {
        auto msg = QString("505: INTERNAL ERROR\n\n%2").arg(tr("No data for processing"));
        DBGF(0, "{}", msg.toStdString());
        WMSG::WMsg(MW::mw, msg, QMessageBox::Critical);
        return;
    }

    channel_points diff; // точки после применения задержки
    diff.points().resize(pnts_size);
    float *dst = diff.points().data();

    if (m_chart4_mode == SUM_OR_SUB_MODE::SUM) {
        for (size_t i = 0; i < pnts_size; ++i) {
            dst[i] = pnts1_ptr[i] + pnts2_ptr[i];
        }
    } else {
        for (size_t i = 0; i < pnts_size; ++i) {
            dst[i] = pnts1_ptr[i] - pnts2_ptr[i];
        }
    }

    auto [min_it, max_it] = std::minmax_element(diff.points().begin(), diff.points().end());
    if (min_it == diff.points().end()) {
        return;
    }
    diff.points().set_yminmax({*min_it, *max_it});

    diff.set_dt(m_channels_points.ch1().dt());
    auto x_max = m_channels_points.ch1().timerange() - m_channels_points.ch1().dt();
    diff.points().set_xminmax({0, x_max});

    DBGF(3,
        "chart-{}: points: X:{:0.2f} - {:0.2f}, Y:{:0.8f} - {:0.8f}",
        chart->chart_num,
        diff.points().xminmax().first,
        diff.points().xminmax().second,
        diff.points().yminmax().first,
        diff.points().yminmax().second);

    if (m_chart4_mode == SUM_OR_SUB_MODE::SUM) {
        if (chart->axisY()->max() != CFG::cfg->axisY_range * 2) {
            chart->axisY()->setRange(0, CFG::cfg->axisY_range * 2);
        }
    } else {
        // SUM_OR_SUB_MODE::SUB
        if (diff.points().yminmax().first < 0) {
            auto ymin = chart->axisY()->min();
            auto ymax = chart->axisY()->max();
            chart->axisY()->setRange(-ymax, -ymin);
        }
    }
    DBGF(3,
        "chart-{}: axis X: {:0.2f} - {:5.2f}, Y: {:0.8f} - {:0.8f}",
        chart->chart_num,
        chart->axisX()->min(),
        chart->axisX()->max(),
        chart->axisY()->min(),
        chart->axisY()->max());

    m_chart4->set_points(diff);
}

//---------------------------------------------------------
void DPTDUMP_VIEW::set_points(CHART *chart, channels_points &points)
{
    int chnum = chart->m_channel;
    DBGF(3, "chart-{}: channel:{}", chart->chart_num, chnum);
    if (chnum == 1) {
        chart->set_points(points.ch1());
        auto title = QString("%1 (%2 %3)").arg(chart->base_title, tr("Points"), QString::number(m_total_rows));
        chart->set_title(title);
    } else if (chnum == 2) {
        chart->set_points(points.ch2());
    } else {
        DBGF(0, "500: INTERNAL ERROR: chart-{}: requested wrong channel:{}", chart->chart_num, chnum);
        return;
    }
}

//---------------------------------------------------------
void DPTDUMP_VIEW::set_points(channels_points points)
{
    if (MW::mw->currentTabWidget() != gui->gui_page()) {
        return;
    }
    DBG(3);

    set_points(m_chart1, points);
    set_points(m_chart2, points);
    set_points_two_chs(m_chart3); // draw chart for two pressure
    set_points_sum(m_chart4);     // draw chart for dual pressure transducer

    // отобразить мин / макс значения
    auto r1 = m_chart1->yrange1();
    m_chart1_max_min_values[0]->setText(
        QString("%1 %2").arg(r1.first, 0, 'f', CFG::cfg->decimal_points, QChar('0')).arg(tr("V")));
    m_chart1_max_min_values[1]->setText(
        QString("%1 %2").arg(r1.second, 0, 'f', CFG::cfg->decimal_points, QChar('0')).arg(tr("V")));

    auto r2 = m_chart2->yrange1();
    m_chart2_max_min_values[0]->setText(
        QString("%1 %2").arg(r2.first, 0, 'f', CFG::cfg->decimal_points, QChar('0')).arg(tr("V")));
    m_chart2_max_min_values[1]->setText(
        QString("%1 %2").arg(r2.second, 0, 'f', CFG::cfg->decimal_points, QChar('0')).arg(tr("V")));
}
