/**
  \file   dpt_dump_gui.h
  \brief  To show Dual Pressure Dump file
  \author Aleksey Lekhtin
  \date   06.07.2026
*/

#ifndef DP_DUMP_VIEW_H
#define DP_DUMP_VIEW_H

#include "channels_points.h"

#include <QChartView>
#include <QDialogButtonBox>
#include <QSpinBox>

#ifndef QT6_ONLY
QT_CHARTS_USE_NAMESPACE
#endif

class CHART;

//---------------------------------------------------------
class DPTDUMP_VIEW : public QWidget
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
    explicit DPTDUMP_VIEW();
    ~DPTDUMP_VIEW();

    inline static DPTDUMP_VIEW *gui{};

    void run() const;
    QWidget *gui_page();
    const QString &page_name();
    void set_fonts() const;

    void select_page();
    void load(const QString &dirname);

    //// inline QString &filename() {
    ////     return m_fullname;
    //// }

  private:
    void read_status();
    void save_status() const;

    bool select_dump_file(const QString &dirname);
    void load_dump();

  private:
    void set_points(channels_points points);
    void set_points(CHART *chart, channels_points &points);
    void set_points_two_chs(CHART *chart);
    void set_points_sum(CHART *chart);
    //// void rescale_two_channels(CHART *chart);
    void chart_reset(CHART *chart);

  private slots:
    void slot_set_yscale(bool checked);

  private:
    QWidget *create_page();
    QWidget *create_control_buttons();
    //// QWidget *create_close_button();
    void create_chart1_control();
    void create_chart2_control();
    void create_chart3_control();
    void create_chart4_control();
    QWidget *create_delay_field();
    QWidget *create_plus_minus_buttons();

    void set_mono_font() const;
    void set_button_font() const;
    void set_text_font() const;
    void set_charts_font() const;

  private:
    QString m_page_name;
    QWidget *m_gui_page;
    CHART *m_chart1{};
    CHART *m_chart2{};
    CHART *m_chart3{};
    CHART *m_chart4{};
    QWidget *m_chart1_control{};
    QWidget *m_chart2_control{};
    QWidget *m_chart3_control{};
    QWidget *m_chart4_control{};
    QLineEdit *m_chart1_max_min_values[2]{};
    QLineEdit *m_chart2_max_min_values[2]{};

    int m_delay{0};
    QSpinBox *m_delay_field{};
    QWidget *m_box_channels_delay{};
    QPushButton *m_button_clear_charts{};
    QPushButton *m_button_chart_plus{};
    QPushButton *m_button_chart_minus{};
    QDialogButtonBox *m_button_close{};

    SUM_OR_SUB_MODE m_chart4_mode{SUM_OR_SUB_MODE::NONE};
    double m_scale{};
    QString m_fullname;
    channels_points m_channels_points;

    bool m_yscale_button_status{};
    QPushButton *m_button_set_yscale;

    uint32_t m_total_rows{};
};

#endif // DP_DUMP_VIEW_H
