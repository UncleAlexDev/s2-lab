/**
  \file   mainwindow.h
  \author Aleksey Lekhtin
  \date   27.11.2024
  \brief  Главный класс данных
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QTabWidget>
#include <QTextEdit>
#include <QTranslator>
#include <QWidget>

#define D_B_G 1

class CFG;
class BBIPoints;
class NETWORK;
class TABLE;
class ADC;
class DPT;
class DPTDUMP_VIEW;

enum class ESZ7_LED_STATUS;

//---------------------------------------------------------
class MW : public QMainWindow
{
    Q_OBJECT;

  public:
    inline static bool Bold = true;

  public:
    explicit MW();
    ~MW() override;

    inline static MW *mw;

    void about();
    NETWORK *server();

    QString statusbar_currentmsg() const;
    void statusbar_show(const QString &msg) const;
    void statusbar_show(const QString &msg, int sec) const;
    void statusbar_show(const QString &msg, bool bold) const;
    void statusbar_show(const QString &msg, bool bold, int ms) const;
    void statusbar_clear() const;

    static QFont findMonospaceFont();

    QWidget *currentTabWidget() const;
    void lock_my_tab(const QWidget *tab);
    void unlock_my_tab(const QWidget *tab);

    inline QTabWidget *tabWidget() const {
        return m_tabwgt;
    }

    inline ADC *adc_page() const {
        return m_adc;
    }

    inline DPT *dpt_page() const {
        return m_dpt;
    }

    inline QAction *data_source_settings_menu() const {
        return m_action_settings_data_source;
    }


  private:
    void create_menu();
    static void setup_fonts();
    void set_mainwindow_font_size();
    void set_fonts();
    QGroupBox *create_incoming_data_box();
    QGroupBox *create_adc_settings_box();
    QGroupBox *create_misc_settings_box();

  protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void signal_message(QString title, QString msg, QMessageBox::Icon icon);

  private slots:
    void slot_settings_data_source();
    void slot_settings_miscellaneous();
    void slot_select_next_tab();
    void slot_select_prev_tab();
    void slot_tabwgt_changed(int index);
    void slot_font_settings();
    void slot_set_menu_font_size(const QString &);
    void slot_set_text_font_size(const QString & text);
    void slot_set_chart_font_size(const QString & text);
    void slot_set_mono_font_size(const QString &);
    void slot_set_button_font_size(const QString &);

  private:
    CFG *m_cfg;
    NETWORK *m_network{};
    BBIPoints *m_data_points{};
    ADC *m_adc{};
    DPT *m_dpt{};
    DPTDUMP_VIEW *m_dpdump_gui{};

    QMenu *m_fileMenu;
    QMenu *m_helpMenu;
    QAction *m_action_about;
    QAction *m_action_exit;
    // QAction *m_action_tab1;
    // QAction *m_action_tab2;
    QAction *m_action_settings_data_source;
    QAction *m_action_settings_misc;
    QAction *m_action_fonts_size;
    QShortcut *m_shortcut_prev_tab{};
    QShortcut *m_shortcut_next_tab{};

    QLineEdit *m_field_atlas_host;
    QSpinBox *m_field_atlas_port;
    QLineEdit *m_field_ads_host;
    QSpinBox *m_field_ads_port;
    QLineEdit *m_field_sureshot_host;
    QSpinBox *m_field_sureshot_port;
    QSpinBox *m_field_debug;
    QLineEdit *m_field_ref_voltage;
    QLineEdit *m_field_4_20Current;
    QLineEdit *m_field_scale;
    QLineEdit *m_field_input_range;
    QLineEdit *m_field_raw_dp_file;
    QDoubleSpinBox *m_field_axisY;
    QComboBox *m_field_deciamal_points;

    QWidget *m_widget_esz7_regs_control;
    QWidget *m_widget_mode_reg;
    QWidget *m_widget_conf_reg;
    QPushButton *m_button_unlock_regs;
    QPushButton *m_button_read_mode_reg;
    QPushButton *m_button_write_mode_reg;
    QPushButton *m_button_read_conf_reg;
    QPushButton *m_button_write_conf_reg;

    QPushButton *m_esz7_status_button;
    ESZ7_LED_STATUS m_esz7_status;
    QIcon m_esz7_red;    // красный индикатор
    QIcon m_esz7_gray;   // серый" индикатор
    QIcon m_esz7_green;  // зеленый индикатор
    QIcon m_esz7_yellow; // желтый индикатор

    QTabWidget *m_tabwgt;

    QGroupBox *m_box_mode_regs;
    QString m_mode_value;
    QCheckBox *m_mode_desc23;
    QCheckBox *m_mode_desc22;
    QCheckBox *m_mode_desc21;
    QCheckBox *m_mode_datsta;
    QCheckBox *m_mode_clk_18;
    QCheckBox *m_mode_clk_19;
    QCheckBox *m_mode_sync_3;
    QCheckBox *m_mode_enpari;
    QCheckBox *m_mode_single;
    QCheckBox *m_mode_rejc60;
    QCheckBox *m_mode_fs0;
    QCheckBox *m_mode_fs1;
    QCheckBox *m_mode_fs2;
    QCheckBox *m_mode_fs3;
    QCheckBox *m_mode_fs4;
    QCheckBox *m_mode_fs5;
    QCheckBox *m_mode_fs6;
    QCheckBox *m_mode_fs7;
    QCheckBox *m_mode_fs8;
    QCheckBox *m_mode_fs9;

    QGroupBox *m_box_conf_regs;
    QString m_conf_value{};
    QCheckBox *m_conf_chop{};
    QCheckBox *m_conf_refsel{};
    QCheckBox *m_conf_ch0{};
    QCheckBox *m_conf_ch1{};
    QCheckBox *m_conf_ch2{};
    QCheckBox *m_conf_ch3{};
    QCheckBox *m_conf_ch4{};
    QCheckBox *m_conf_ch5{};
    QCheckBox *m_conf_ch6{};
    QCheckBox *m_conf_ch7{};
    QCheckBox *m_conf_burn{};
    QCheckBox *m_conf_rfdt{};
    QCheckBox *m_conf_buff{};
    QCheckBox *m_conf_unib{};
    QCheckBox *m_conf_gain0{};
    QCheckBox *m_conf_gain1{};
    QCheckBox *m_conf_gain2{};

  public:
    std::vector<uint8_t> m_incl_data;
    int m_incl_data_bits;
};

#endif // MAINWINDOW_H
