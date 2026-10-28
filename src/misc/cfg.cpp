/**
   \file   cfg.cpp
   \brief  Начальная конфигурация
   \author Aleksey Lekhtin
   \date   created 18.11.2014
 */

#include "cfg.h"
#include "dbg.h"
#include "mw.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>

// #ifdef _WIN32
// #include <windows.h>
// #endif

// bool CFG::locale_ru = false;

//---------------------------------------------------------
CFG::CFG() :
    app_title()
{
    cfg = this;
    read_cfg();
    read_status();

    // auto name = QLocale::system().name();
    // if (name.contains("ru", Qt::CaseInsensitive)) {
    //     locale_ru = true;
    // }
}

//---------------------------------------------------------
CFG::~CFG()
{
#ifdef D_B_G
    DBG(4, "++");
#endif
    delete m_sett;
    delete m_status;
#ifdef D_B_G
    DBG(4, "--");
#endif
}

//---------------------------------------------------------
void CFG::set_title(const QString &title)
{
    app_title = title;
}

//---------------------------------------------------------
QSettings *CFG::app_status()
{
    return m_status;
}

//---------------------------------------------------------
void CFG::read_cfg()
{
    bool ok;
    if (!m_sett) {
        auto fname =
            QString("%1/%2.cfg").arg(QCoreApplication::applicationDirPath(), QCoreApplication::applicationName());
        m_sett = new QSettings(fname, QSettings::IniFormat, nullptr);
        if (m_sett->status() != QSettings::NoError) {
            printf("\n--- ERROR: Can't open status file\n\n");
            delete m_sett;
            m_sett = nullptr;
            return;
        }
    }

    //---------------------------------
    DBG::debug_level = m_sett->value("DebugLevel", 0).toInt();
    DBGF(1, "debug level: {}", DBG::debug_level);
    decimal_points = m_sett->value("DecimalPoints", 8).toInt();

    //---------------------------------
    // data_raw_fname = m_sett->value("RawFile", "data.raw").toString();
    dp_dump_dir = QDir::toNativeSeparators(m_sett->value("Dump/DPDir").toString());

    QString str = m_sett->value("DataFormat", "Atlas").toString().toLower();
    data_format = DATA_FORMAT::NONE;
    if (str == "atlas") {
        data_format = DATA_FORMAT::ATLAS;
    } else if (str == "ads") {
        data_format = DATA_FORMAT::ADS;
    } else if (str == "sureshot") {
        data_format = DATA_FORMAT::SURESHOT;
    }

    //---------------------------------
    m_sett->beginGroup("ATLAS");
    atlas_host = m_sett->value("Host", "192.168.1.80").toString();
    atlas_port = m_sett->value("Port", 16000).toInt();
    m_sett->endGroup();

    //---------------------------------
    m_sett->beginGroup("ADS");
    ads_host = m_sett->value("Host", "192.168.1.81").toString();
    ads_data_port = m_sett->value("Port", DEF_ADS_PORT_DATA).toInt();
    asPulse = m_sett->value("Inclinometer", false).toBool();
    m_sett->endGroup();

    //---------------------------------
    m_sett->beginGroup("SURESHOT");
    sureshot_host = m_sett->value("Host", "192.168.1.81").toString();
    sureshot_data_port = m_sett->value("Port", DEF_SURESHOT_PORT_DATA).toInt();
    m_sett->endGroup();

    // //---------------------------------
    // m_sett->beginGroup("ESZ7Board");
    // esz7_host = m_sett->value("Ip", "192.168.1.81").toString();
    // esz7_port = m_sett->value("Port", 18000).toInt();
    // m_sett->endGroup();

    //---------------------------------
    m_sett->beginGroup("ADC");
    adc_ref_voltage = m_sett->value("RefV", "2.5").toString().toDouble(&ok);
    if (!ok) {
        adc_ref_voltage = 2.5;
    }
    axisY_range = m_sett->value("AxisYRange", adc_ref_voltage).toString().toDouble(&ok);

    atlas_scale_factor = adc_ref_voltage / ((1 << 23) - 1);
    ads_scale_factor = adc_ref_voltage / ((1 << 23) - 1);
    ss_scale_factor = adc_ref_voltage / ((1 << 23) - 1);

    DBGF(1, "Atlas scale for    {}V - {:0.2}", adc_ref_voltage, atlas_scale_factor);
    DBGF(1, "ADS scale for      {}V - {:0.2}", adc_ref_voltage, ads_scale_factor);
    DBGF(1, "SureShot scale for {}V - {:0.2}", adc_ref_voltage, ss_scale_factor);
    m_sett->endGroup();

    //---------------------------------
    // int_for_4ma = (uint32_t)((1<<24) / 20 * 4.0);
    // int_for_1ma = (uint32_t)((1<<24) / 20 * 1.0);

    // adc_low_voltage = int_for_4ma * adc_scale_factor;
    // no_voltage = int_for_1ma * adc_scale_factor;
    //
    // DBGFN(1, "ADC scale:            %.06fV", adc_scale_factor);
    // DBGFN(1, "1 mA: 0x%06X          %.06fV", int_for_1ma, int_for_1ma * adc_scale_factor);
    // DBGFN(1, "4 mA: 0x%06X          %.06fV", int_for_4ma, int_for_4ma * adc_scale_factor);
    // DBGFN(1, "ADC low voltage:      %.06fV", adc_low_voltage);
    // DBGFN(1, "ADC no  voltage:      %.06fV\n", no_voltage);
}

//---------------------------------------------------------
void CFG::save_cfg()
{
    if (!m_sett) {
        return;
    }
#ifdef _WIN32
    m_sett->setValue(("ConsoleOutputCP"), m_console_output_cp);
#endif
    m_sett->setValue(("DebugLevel"), DBG::debug_level);
    m_sett->setValue("DecimalPoints", decimal_points);

    // m_sett->setValue(("RawFile"), data_raw_fname);
    m_sett->setValue(("Dump/DPDir"), QDir::fromNativeSeparators(dp_dump_dir));

    QString str = "Atlas";
    if (data_format == DATA_FORMAT::SURESHOT) {
        str = "SureShot";
    } else if (data_format == DATA_FORMAT::ADS) {
        str = "ADS";
    }
    m_sett->setValue("DataFormat", str);

    m_sett->beginGroup("ATLAS");
    m_sett->setValue("Host", atlas_host);
    m_sett->setValue("Port", atlas_port);
    m_sett->endGroup();

    m_sett->beginGroup("ADS");
    m_sett->setValue("Host", ads_host);
    m_sett->setValue("Port", ads_data_port);
    m_sett->setValue("Inclinometer", asPulse);
    m_sett->endGroup();

    m_sett->beginGroup("SURESHOT");
    m_sett->setValue("Host", sureshot_host);
    m_sett->setValue("Port", sureshot_data_port);
    m_sett->endGroup();

    // m_sett->beginGroup("ESZ7Board");
    // m_sett->setValue("Ip", esz7_host);
    // m_sett->setValue("Port", esz7_port);
    // m_sett->endGroup();

    m_sett->beginGroup("ADC");
    m_sett->setValue("RefV", QString::number(adc_ref_voltage, 'f', 2));
    m_sett->setValue("AxisYRange", QString::number(axisY_range, 'f', 2));
    m_sett->endGroup();
}

//---------------------------------------------------------
void CFG::read_status()
{
    if (!m_status) {

        QString fname =
            QString("%1/%2.sts").arg(QCoreApplication::applicationDirPath(), QCoreApplication::applicationName());
        DBGF(1, "app status: {}\n", qPrintable(QDir::toNativeSeparators(fname)));
        m_status = new QSettings(fname, QSettings::IniFormat, nullptr);
        if (m_status->status() != QSettings::NoError) {
            DBG(0, "\n--- ERROR: Can't open status file\n");
            delete m_status;
            m_status = nullptr;
            return;
        }
    }

    m_status->beginGroup("Status");
    mainwindow_geometry = m_status->value("MainWindow").toByteArray();
    dialog_settings1_state = m_status->value("SettingsDialog1").toByteArray();
    dialog_settings2_state = m_status->value("SettingsDialog2").toByteArray();
    current_page = m_status->value("CurrentPage", 0).toInt();
    m_status->endGroup();

    // QFont monofnt = MW::findMonospaceFont();
    m_status->beginGroup("Fonts");
    menu_font.fromString(m_status->value("Menu", QGuiApplication::font()).toString());
    text_font.fromString(m_status->value("Text", QGuiApplication::font()).toString());
    button_font.fromString(m_status->value("Buttons", QGuiApplication::font()).toString());
    chart_font.fromString(m_status->value("Charts", QGuiApplication::font()).toString());
    mono_font.fromString(m_status->value("Mono", QGuiApplication::font()).toString());
    m_status->endGroup();

    menu_action_adc_enabled = m_status->value("ADC/Enabled", true).toBool();
    menu_action_dpt_enabled = m_status->value("DPT/Enabled", false).toBool();
}

//---------------------------------------------------------
void CFG::save_status()
{
    DBG(1);
    if (!m_status) {
        return;
    }
    m_status->beginGroup("Status");
    // m_status->setValue("LoadFileDialog", dialog_loadfile_state);
    m_status->setValue("SettingsDialog1", dialog_settings1_state);
    m_status->setValue("SettingsDialog2", dialog_settings2_state);

    //// qDebug() << MW::mw->tabWidject()->currentIndex();
    current_page = MW::mw->tabWidget()->currentIndex();
    if (current_page > 2) {
        current_page = 2;
    }
    m_status->setValue("CurrentPage", current_page);

    // m_status->setValue("Ping", esz7_enable_ping);
    // m_status->setValue("EnableRegsControl", esz7_regs_enabled);
    // m_status->setValue("ShowADCValuesTable", show_table1_enabled);
    m_status->endGroup();

    m_status->beginGroup("Fonts");
    m_status->setValue("Menu", menu_font.toString());
    m_status->setValue("Text", text_font.toString());
    m_status->setValue("Buttons", button_font.toString());
    m_status->setValue("Charts", chart_font.toString());
    m_status->setValue("Mono", mono_font.toString());
    m_status->endGroup();

    m_status->setValue("ADC/Enabled", menu_action_adc_enabled);
    m_status->setValue("DPT/Enabled", menu_action_dpt_enabled);
}

//---------------------------------------------------------
void CFG::save_mainwindow_status()
{
    if (!m_status) {
        return;
    }
    if (MW::mw) {
        m_status->beginGroup("Status");
        m_status->setValue("MainWindow", MW::mw->saveGeometry());
        m_status->endGroup();
    }
}
