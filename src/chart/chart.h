/**
  \file   chart.ch
  \brief  Создание графиков
  \author Aleksey Lekhtin
  \date   27.11.2024
*/

#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include "channels_points.h"

#include <QChart>
#include <QChartView>
#include <QDateTimeAxis>
#include <QLabel>
#include <QLineSeries>
#include <QObject>
#include <QRubberBand>
#include <QValueAxis>
#ifndef QT6_ONLY
QT_CHARTS_USE_NAMESPACE
#endif

#define INNER_RUBBERBAND

class CHART : public QChartView
{
    Q_OBJECT;

  public:
    explicit CHART(int num = 0, const QString &title = {});
    ////// ~CHART() override;

  public:
    QWidget *gui_page();
    void set_title(const QString &title);
    void set_title_font(const QFont &font);
    int count() const;
    void enable_legend(bool enabled);
    QLineSeries *series() const;
    QLineSeries *series2() const;
    void set_legend_series_name(const QString &text);
    QString series_name() const;
    void set_axisY_format(const QString &format);
    void set_line_width(int num, int series_num = 1);
    void set_line_color(const QColor &color, int series_num = 1);

    //// void set_points(const Points &points, int series_num = 1);
    void set_points(const channel_points &ch, int series_num = 1);
    void refresh();
    void clean();

    QValueAxis *axisX();
    QValueAxis *axisY();

    //// inline std::pair<float, float> xrange() const
    //// {
    ////     return m_xrange;
    //// }

    //// inline std::pair<float, float> yrange() const
    //// {
    ////     return m_yrange;
    //// }

    //// inline void set_y_range(const std::pair<float, float> &range)
    //// {
    ////     m_yrange = range;
    //// }

    inline float avrg1() const
    {
        return m_avrg1;
    }

    inline float avrg2() const
    {
        return m_avrg2;
    }

    inline std::pair<float, float> xminmax1() const
    {
        return m_xminmax1;
    }

    inline std::pair<float, float> yminmax1() const
    {
        return m_yminmax1;
    }

    inline std::pair<float, float> yrange1() const
    {
        return m_yrange1;
    }

    inline void set_y_range1(const std::pair<float, float> &range)
    {
        m_yrange1 = range;
    }

    inline std::pair<float, float> yrange2() const
    {
        return m_yrange2;
    }

    inline void set_y_range2(const std::pair<float, float> &range)
    {
        m_yrange2 = range;
    }

    inline int channel() const {
        return m_channel;
    }

    inline float ch5_fix_range() {
        return m_ch5_axisX_fixed_range;
    }

  public:
    QString base_title{};
    bool opengl_enabled{};
    int chart_num{}; // any number, just for identification
    int m_channel{0};
    bool series_name_enabled{false};
    bool scale_Y{false};
    bool magnify_Y{false};
    bool showAsRaw{false};

#ifndef INNER_RUBBERBAND
  private:
    QRubberBand *m_rubberBand{};
    QPoint m_rubberBand_origin_pos{};
#endif

  private:
    QWidget     *m_gui_page;
    QLabel      *m_chart_title;
    QLineSeries *m_series{};
    QPen m_pen;

    QLineSeries *m_series2{};
    QPen m_pen2;

    QValueAxis *m_axisX{}; // QDateTimeAxis
    QValueAxis *m_axisY{};
    int m_ch5_axisX_fixed_range{10}; // sec

    float m_avrg1{};
    float m_avrg2{};
    std::pair<float, float> m_xminmax1{};
    std::pair<float, float> m_yminmax1{};
    std::pair<float, float> m_yrange1{};
    std::pair<float, float> m_xrange1{};
    std::pair<float, float> m_yrange2{};
    std::pair<float, float> m_xrange2{};
    std::pair<float, float> m_yrange{};
    std::pair<float, float> m_xrange{};
};

#endif // CHARTVIEW_H
