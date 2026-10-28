/**
  \file   dpt.h
  \brief  Dual Pressure Transducer Algorithm
  \author Aleksey Lekhtin
  \date   06.05.2026
*/

#ifndef DPT_H
#define DPT_H

#include "dp_dump.h"
#include "dp_dump_view.h"

#include <QShortcut>
#include <QChartView>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>

#ifndef QT6_ONLY
QT_CHARTS_USE_NAMESPACE
#endif

class CHART;
class DPDUMP;

class DPT : public QWidget
{
    Q_OBJECT;

  public:
    enum class SUM_OR_SUB_MODE
    {
        NONE,
        SUM,
        SUB
    };

  public:
    explicit DPT();
    ~DPT() override;

    inline static DPT *dpt{};
    inline static int delay{0};
    inline static int max_delay{50};

    QWidget *gui_page();
    const QString &page_name();
    void set_fonts() const;
    void chart_reset(CHART *chart);
    void set_axisY_format(const QString &format);
    void release_receive_button();
    void set_points(BBIPoints &points);

    inline QPushButton *button_browse_dir2() const {
        return m_button_set_dump_dir;
    }

    inline QPushButton *button_receive()
    {
        return m_button_receive;
    }

  private:
    void read_status();
    void save_status() const;

    QWidget *create_page();
    QWidget *create_control_buttons();

    void create_chart1_control();
    void create_chart2_control();
    void create_chart3_control();
    void create_chart4_control();

    QWidget *create_control_buttons_2();
    QWidget *create_delay_field();
    QWidget *create_plus_minus_buttons();
    QWidget *create_button_receive_data();
    QWidget *create_dump_control();

    void set_points(CHART *chart);
    void set_points_two_channels(CHART *chart);
    void set_points_sum_channels(CHART *chart);

    void set_mono_font() const;
    void set_button_font() const;
    void set_text_font() const;
    void set_charts_font() const;

    static QWidget *view_dump(const QString &dirname);

public slots:
    void slot_set_points();

  private slots:
    void slot_receive_button(bool checked);
    ////void slot_show_raw(bool checked);
    void slot_set_yscale(bool checked);

  public:
    double m_scale{};
    inline static int atlas_chart1_x_def_range[2]{0, 1000-2};
    inline static int atlas_chart1_x_tick_count_def{20};
    inline static int atlas_chart1_x_minor_tick_count_def{10};

    inline static int aps_chart1_x_def_range[2]{0, 250};
    inline static int aps_chart1_x_tick_count_def{5};
    inline static int aps_chart1_x_minor_tick_count_def{5};

  private:
    QString m_page_name;
    QWidget *m_gui_page;
    CHART *m_chart1;
    CHART *m_chart2;
    CHART *m_chart3;
    CHART *m_chart4;
    SUM_OR_SUB_MODE m_chart4_mode{SUM_OR_SUB_MODE::NONE};
    QLineEdit *m_chart1_max_min_values[2]{};
    QLineEdit *m_chart2_max_min_values[2]{};
    QComboBox *m_chart1_channel_list{};
    QComboBox *m_chart2_channel_list{};
    QShortcut *shortcut_channel_select1{};
    QShortcut *shortcut_channel_select2{};

    QWidget *m_chart1_control{};
    QWidget *m_chart2_control{};
    QWidget *m_chart3_control{};
    QWidget *m_chart4_control{};

    QWidget *m_box_channels_delay{};
    QSpinBox *m_delay_field{};
    QSpinBox *m_delay_field2{};

    QPushButton *m_button_receive{};
    QPushButton *m_button_clear_charts{};
    //// QPushButton *m_button_show_Y_raw{};
    QPushButton *m_button_set_yscale{};
    QPushButton *m_button_chart_plus{};
    QPushButton *m_button_chart_minus{};

    QPushButton *m_button_set_dump_dir{};
    QPushButton *m_button_make_dump{};
    //// QPushButton *m_button_view_dump{};

    bool m_do_dump{};
    DPDUMP *m_dump{};
    DPTDUMP_VIEW *m_dump_gui{};

    channels_points m_channels_points;

    // settings
    int chart1_selected_channel{};
    int chart2_selected_channel{};
    bool m_yscale_button_status{};
    bool m_raw_button_status{};
};

#endif // DPT_H
