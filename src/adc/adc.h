/**
  \file   adc_tab.h
  \brief  The tab with ADC numbers and charts
  \author Aleksey Lekhtin
  \date   06.05.2026
*/

#ifndef ADC_TAB_H
#define ADC_TAB_H

#include "channels_points.h"

#include <QtWidgets>

class CHART;
class TABLE;

class ADC : public QWidget
{
    Q_OBJECT;

  public:
    explicit ADC();
    ~ADC() override;

    inline static ADC *adc{};

    void run() const;
    QWidget *gui_page();
    const QString &page_name();

    void chart_reset(CHART *chart);
    void set_axisY_format(const QString &format);
    void set_fonts() const;
    void release_receive_button();
    void set_points(BBIPoints &points);

    inline CHART *chart1() const {
        return m_chart1;
    }

    inline CHART *chart2() const {
        return m_chart2;
    }

  private:
    QWidget *create_page();
    QWidget *create_control_buttons();
    void read_status();
    void save_status() const;
    void set_monofont() const;
    void set_buttonfont() const;
    void set_textfont() const;
    void set_charts_font() const;
    void set_points(CHART *chart);
    static QString make_title(CHART *chart);
    void toggle_yscale_button(bool checked);
    void toggle_magnify_button(bool checked);

  public slots:
    void slot_set_points();

  private slots:
    void slot_receive_button(bool checked);
    void slot_show_raw(bool checked);
    void slot_set_yscale(bool checked);
    void slot_set_magnify(bool checked);

  public:
    double scale{};
    inline static int atlas_chart1_x_def_range[2]{0, 1000-2};
    inline static int atlas_chart1_x_tick_count_def{20};
    inline static int atlas_chart1_x_minor_tick_count_def{10};

    inline static int aps_chart1_x_def_range[2]{0, 250};
    inline static int aps_chart1_x_tick_count_def{5};
    inline static int aps_chart1_x_minor_tick_count_def{5};

    inline QPushButton *button_receive()
    {
        return m_button_receive;
    }

  private:
    QString m_page_name;
    QWidget *m_gui_page;
    CHART *m_chart1;
    CHART *m_chart2;

    QGroupBox *m_box_set_ch_chart1{};
    QLabel    *m_chart1_max_min_label[2];
    QLineEdit *m_chart1_max_min_values[2]{};
    QLabel    *m_chart1_label_avrg{};
    QLineEdit *m_chart1_avrg_value{};
    QComboBox *m_chart1_channel_list{};
    QShortcut *shortcut_channel_select1{};

    QGroupBox *m_box_set_ch_chart2{};
    QLabel    *m_chart2_max_min_label[2];
    QLineEdit *m_chart2_max_min_values[2]{};
    QLabel    *m_chart2_label_avrg;
    QLineEdit *m_chart2_avrg_value{};
    QComboBox *m_chart2_channel_list{};
    QShortcut *shortcut_channel_select2{};

    QGroupBox *m_box_encoders{};
    QLineEdit *m_encoder[2]{};

    QPushButton *m_button_receive{};
    QPushButton *m_button_clear_charts{};
    QPushButton *m_button_show_Y_raw{};
    QPushButton *m_button_yscale{};
    QPushButton *m_button_magnify{};
    QShortcut   *m_shortcut{};

    channels_points m_channels_points{};

    // settings
    int m_chart1_selected_channel{};
    int m_chart2_selected_channel{};
    bool m_yscale_button_status{};
    bool m_magnify_button_status{};
    bool m_raw_button_status{};
};

#endif // ADC_TAB_H
