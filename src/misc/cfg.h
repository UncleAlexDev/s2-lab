/**
  \file   cfg.h
  \brief  Настройки
  \author Aleksey Lekhtin
  \date   24.01.2025
*/
#ifndef CFG_H_
#define CFG_H_

#include <QFont>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QString>

#define DEF_ADS_PORT_CMD 8220
#define DEF_ADS_PORT_DATA 8110

#define DEF_SURESHOT_PORT_CMD 7111
#define DEF_SURESHOT_PORT_DATA 8111

class CFG
{
  public:
    explicit CFG();
    virtual ~CFG();

    inline static CFG *cfg;

  public:
    enum class DATA_FORMAT : uint8_t
    {
        NONE = 0,
        ATLAS,
        SURESHOT,
        ADS,
    };

  public:
    void set_title(const QString &title);
    void save_cfg();
    void save_status();
    void save_mainwindow_status();
    QSettings *app_status();

  private:
    void read_cfg();
    void read_status();

  public:
    // static bool locale_ru;
    QString app_title;
    QFont menu_font;
    QFont text_font;
    QFont mono_font;
    QFont button_font;
    QFont chart_font;
    ////// QFont adcvalues_font;
    ////// QFont adcregs_font;

    QString dp_dump_fname;
    QString dp_dump_dir;
    int     decimal_points{};

    DATA_FORMAT data_format{};
    QString atlas_host;
    int atlas_port{};
    QString ads_host{};
    int ads_data_port{};
    QString sureshot_host;
    int sureshot_data_port{};

    double atlas_scale_factor{};
    double ads_scale_factor{};
    double ss_scale_factor{};
    double adc_ref_voltage{};
    double axisY_range{};

    // double      adc_low_voltage;
    // double      adc_no_voltage;

    // double      atlas_scale_factor;
    // double      atlas_voltage_ref;
    // double      atlas_low_voltage;
    // uint32_t    int_for_4ma;
    // uint32_t    int_for_1ma;
    // double      no_voltage;

    bool cutoff_filter{false};
    ///// bool ADC_points_visible;
    bool asPulse{};

    int current_page{};
    bool menu_action_adc_enabled{};
    bool menu_action_dpt_enabled{};
    QByteArray mainwindow_geometry;
    QByteArray table1_status;
    QByteArray dialog_loadfile_state;
    QByteArray dialog_settings1_state;
    QByteArray dialog_settings2_state;
    QString esz7_host;
    int esz7_port{};
    bool esz7_enable_ping{};
    bool esz7_regs_enabled{};

  private:
    QSettings *m_sett{};
    QSettings *m_status{};
    unsigned int m_console_output_cp{};
    QSize m_app_size;
    QPoint m_app_position;
};

#endif // CFG_H_
