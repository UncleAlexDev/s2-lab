/**
  \file   chartview.cpp
  \brief  Создание графиков
  \author Aleksey Lekhtin
  \date   27.11.2024
*/

#include "chart.h"
#include "channels_points.h"
#include "dbg.h"
#include "network.h"

#include <iostream>

#include <QApplication>
#include <QFontDatabase>
#include <QRubberBand>
#include <QVBoxLayout>

CHART::CHART(int num, const QString &title) :
    QChartView(new QChart()),
    opengl_enabled(false),
    chart_num(num)
{
    setRenderHint(QPainter::Antialiasing);
    chart()->setAnimationOptions(QChart::NoAnimation);
#ifdef INNER_RUBBERBAND
    setRubberBand(QChartView::RectangleRubberBand);
#endif

    // setDragMode(QGraphicsView::ScrollHandDrag);
    // setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    // setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    base_title = title;
    // chart()->setTitle(base_title);
    chart()->legend()->hide();
    chart()->setDropShadowEnabled(true);
    chart()->setAcceptHoverEvents(true);
    chart()->setMargins(QMargins(0, 0, 0, 0));
    chart()->setAcceptHoverEvents(true);

    // QLineSeries - #1
    m_series = new QLineSeries(chart());
    m_series->setPointsVisible(false);
    m_series->setPointLabelsVisible(false);
    // m_series->setPointLabelsFormat("(@xPoint, @yPoint)");
    // m_series->setColor(Qt::darkBlue);
    m_series->setUseOpenGL(true);
    opengl_enabled = m_series->useOpenGL();

    chart()->addSeries(m_series);

    m_axisX = new QValueAxis; // m_axisX = new QDateTimeAxis;
    m_axisX->setLabelFormat("%i");
    chart()->addAxis(m_axisX, Qt::AlignBottom);
    m_axisY = new QValueAxis;
    chart()->addAxis(m_axisY, Qt::AlignLeft);

    // last!
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);

    QPen pen = m_series->pen();
    pen.setWidth(1);
    m_pen2.setColor(Qt::darkCyan);
    // pen.setStyle(Qt::DashDotLine);
    m_series->setPen(pen);

    // QLineSeries - #2
    m_series2 = new QLineSeries(chart());
    m_series2->setPointsVisible(false);
    m_series2->setPointLabelsVisible(false);
    m_series2->setUseOpenGL(true);
    chart()->addSeries(m_series2);
    // last!
    m_series2->attachAxis(m_axisX);
    m_series2->attachAxis(m_axisY);
    m_series2->setPen(pen);

    if (DBG::debug_level >= 3) {
        for (QAbstractAxis *axis : chart()->axes()) {
            auto *a = qobject_cast<QValueAxis *>(axis);
            DBGF(3, "min:{}  max:{}", a->min(), a->max());
        }
    }

    //-------------------------------------------
    m_chart_title = new QLabel(base_title);
    m_chart_title->setAlignment(Qt::AlignCenter);
    QFont font = this->font();
    m_chart_title->setFont(font);

    m_gui_page = new QWidget();
    auto *layout = new QVBoxLayout(m_gui_page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chart_title);
    layout->addWidget(this);

    DBGF(2,
        "chart-{} Title: '{}', Y-Title: '{}', OpenGL: '{}'",
        chart_num,
        chart()->title().toStdString(),
        m_axisY->titleText().toStdString(),
        (opengl_enabled ? "enabled" : "disable"));
}

//---------------------------------------------------------
QWidget *CHART::gui_page()
{
    return m_gui_page;
}

//---------------------------------------------------------
void CHART::set_title(const QString &title)
{
    m_chart_title->setText(title);
}

//---------------------------------------------------------
void CHART::set_title_font(const QFont &font)
{
    m_chart_title->setFont(font);
}

//---------------------------------------------------------
////// CHART::~CHART()
////// {
////// #ifdef D_B_G
//////     DBG(4, "++");
////// #endif
////// }

//---------------------------------------------------------
void CHART::enable_legend(bool enabled)
{
    if (series_name_enabled) {
        if (enabled) {
            chart()->legend()->show();
        } else {
            chart()->legend()->hide();
        }
        ////// chart()->update();
        ////// repaint();
    }
}

//---------------------------------------------------------
void CHART::set_axisY_format(const QString &format)
{
    axisY()->setLabelFormat(format);
}

//---------------------------------------------------------
void CHART::set_line_width(int num, int series_num)
{
    if (series_num == 2) {
        m_pen2 = m_series2->pen();
        m_pen2.setWidth(num);
    } else {
        m_pen = m_series->pen();
        m_pen.setWidth(num);
    }
}

//---------------------------------------------------------
void CHART::set_line_color(const QColor &color, int series_num)
{
    if (series_num == 2) {
        m_pen2 = m_series2->pen();
        m_pen2.setColor(color);
    } else {
        m_pen = m_series->pen();
        m_pen.setColor(color);
    }
}

//---------------------------------------------------------
QValueAxis *CHART::axisY()
{
    return m_axisY;
}

//---------------------------------------------------------
QValueAxis *CHART::axisX()
{
    return m_axisX;
}

//---------------------------------------------------------
QLineSeries *CHART::series() const
{
    return m_series;
}

//---------------------------------------------------------
QLineSeries *CHART::series2() const
{
    return m_series2;
}

//---------------------------------------------------------
QString CHART::series_name() const
{
    return m_series->name();
}

//---------------------------------------------------------
void CHART::set_legend_series_name(const QString &text)
{
    if (series_name_enabled) {
        m_series->setName(text);
    }
}

//---------------------------------------------------------
int CHART::count() const
{
    return m_series->count();
}

//---------------------------------------------------------
void CHART::clean()
{
    DBGF(4, "chart-{}", chart_num);
    for (QAbstractSeries *series : chart()->series()) {
        if (auto *s = qobject_cast<QXYSeries *>(series)) {
            QVector<QPointF> empty_vector{};
            s->replace(empty_vector);
            s->clear();
        }
    }
    for (QAbstractAxis *axis : chart()->axes()) {
        axis->setRange(0, 1);
    }

    m_xrange1 = {};
    m_yrange1 = {};
    m_xrange2 = {};
    m_yrange2 = {};
    m_xminmax1 = {};
    m_yminmax1 = {};
    m_xrange = {};
    m_yrange = {};
}

//---------------------------------------------------------
void CHART::refresh()
{
    if (m_yrange2.first == 0.0 && m_yrange2.second == 0.0) {
        // no second line
        m_yrange.first = m_yrange1.first;
        m_yrange.second = m_yrange1.second;
    } else {
        m_yrange.first = std::min(m_yrange1.first, m_yrange2.first);
        m_yrange.second = std::max(m_yrange1.second, m_yrange2.second);
    }

    // clang-format off
    DBGF(3, "chart-{}: y:{:0.8f}|{:0.8f} - {:0.8f}|{:0.8f}",
        chart_num,
        m_yrange1.first,
        m_yrange2.first,
        m_yrange1.second,
        m_yrange2.second);
    // clang-format on

    if (m_channel == 5) {
        DBGF(3, "chart-{}: x max: {:0.2f}, x aisx max: {:0.2f}", chart_num, m_xrange1.second, m_axisX->max());
        if (m_xrange1.second >= (float)m_axisX->max()) {
            int x = (int)(m_axisX->max() * 2 / 3);
            //int x = (int)(m_axisX->max() * 2 / 3) / m_ch5_axisX_fixed_range * m_ch5_axisX_fixed_range;
            m_xrange1.second = (float)m_axisX->max() + (float)x;
            m_axisX->setRange(m_xrange1.first, m_xrange1.second);
        }
    } else {
        m_axisX->setRange(m_xrange1.first, m_xrange1.second);
    }
    DBGF(3, "chart-{}: x min:{:0.2f} - x max:{:0.2f}", chart_num, m_xrange1.first, m_xrange1.second);

    auto interval = (axisY()->max() - axisY()->min()) / (axisY()->tickCount() - 1);
    axisY()->setTickInterval(interval);
    DBGF(4, "Y: tickCount: {}, tickInterval: {}", axisY()->tickCount(), axisY()->tickInterval());

    if (scale_Y || magnify_Y) {
        if (scale_Y) {
            // m_axisY->setRange(ymin, ymax);
            auto ymax = m_yrange.second;
            auto ymin = m_yrange.first;
            if (ymin >= 0.0) {
                m_axisY->setRange(0.0, ymax * 1.05);
            } else if (ymax < 0.0) {
                m_axisY->setRange(ymin * 1.05, 0.0);
            } else {
                m_axisY->setRange(ymin * 1.05, ymax * 1.05);
            }
        } else { // magnify_Y
            m_axisY->setRange(m_yrange.first, m_yrange.second);
        }
    } else {

        auto ay_min = (float)axisY()->min();
        while (m_yrange.first < ay_min) {
            ay_min -= (float)axisY()->tickInterval();
            axisY()->setTickCount(axisY()->tickCount() + 1);
        }

        auto ay_max = (float)axisY()->max();
        while (m_yrange.second > ay_max) {
            ay_max += (float)axisY()->tickInterval();
            axisY()->setTickCount(axisY()->tickCount() + 1);
        }

        m_axisY->setRange(ay_min, ay_max);
    }

    // clang-format off
    if (showAsRaw) {
        DBGF(3, "chart-{}: y:{:08X} - {:08X}, AxisY:{:08X} - {:08X}", chart_num,
            (int32_t)m_yrange.first, (int32_t)m_yrange.second, (int32_t)axisY()->min(), (int32_t)axisY()->max());
    } else {
        DBGF(3, "chart-{}: y:{:0.8f} - {:0.8f}, AxisY:{:0.8f} - {:0.8f}", chart_num,
            m_yrange.first, m_yrange.second, axisY()->min(), axisY()->max());
    }
    // clang-format on
}

//---------------------------------------------------------
void CHART::set_points(const channel_points &ch, int series_num)
{
    const Points &points = showAsRaw ? ch.points_raw() : ch.points();

    if (points.empty()) {
        DBGF(3, "chart-{}[{}]: channel:{} - no points", chart_num, series_num, m_channel);
        return;
    }

    setUpdatesEnabled(false);

    auto process_channel_points = [off = ch.off()](const Points &points,
                                      float dt,
                                      std::pair<float, float> &x_range,
                                      std::pair<float, float> &y_range,
                                      float &avrg,
                                      QVector<QPointF> &chart_points,
                                      size_t max_points = 0)
    {
        if (off < 0 || off >= (int)points.size()) {
            return;
        }

        const auto src_len = points.size() - off;
        const float *src_ptr = points.data() + off;

        double x = 0.0;
        auto current_size = chart_points.size();

        if (current_size > 0) {
            x = chart_points.last().rx() + dt;
        } else {
            x = x_range.first;
        }

        chart_points.resize(current_size + qsizetype(src_len));
        QPointF *dst_ptr = chart_points.data() + current_size;

        avrg = 0;
        float y_min = y_range.first;
        float y_max = y_range.second;
        for (size_t i = 0; i < src_len; ++i) {
            const float y = src_ptr[i];
            dst_ptr[i] = QPointF(x, y);
            x += dt;

            y_min = std::min(y_min, y);
            y_max = std::max(y_max, y);
            avrg += y;
        }
        x -= dt;
        avrg = avrg / (float)src_len;

        // очистка старых точек (cдвиг скользящего окна)
        if (max_points > 0 && (size_t)chart_points.size() > max_points) {
            size_t points_to_remove = (size_t)chart_points.size() - max_points;
            chart_points.remove(0, qsizetype(points_to_remove));
            x_range.first = float(chart_points.first().rx());
            y_min = (float)chart_points.first().ry();
            y_max = (float)chart_points.first().ry();
            for (const auto &p : chart_points) {
                y_min = std::min(y_min, (float)p.y());
                y_max = std::max(y_max, (float)p.y());
            }
        }

        // clang-format off
        DBGF(4, "Y:{:0.8f} - {:0.8f}", y_min, y_max);
        // clang-format on

        y_range.first = y_min;
        y_range.second = y_max;
        x_range.second = (float)x;
    };

    //-------------------------------------------
    QVector<QPointF> chart_points;
    int npoints = 0;

    if (m_channel == 5) {
        // Режим накопления (специфично для 5-го канала)
        chart_points = m_series->points();
        npoints = 100000; // далее - сдвиг
    }

    //-------------------------------------------
    m_xminmax1 = points.xminmax();
    m_yminmax1 = points.yminmax();
    // clang-format off
    if (showAsRaw) {
        DBGF(3, "chart-{}[{}]: x:{:08X} - {:08X} y min:{:08X} - y max:{:08X}", chart_num, series_num,
            (int32_t)m_xminmax1.first, (int32_t)m_xminmax1.second,
            (int32_t)m_yminmax1.first, (int32_t)m_yminmax1.second);
    } else {
        DBGF(3, "chart-{}[{}]: x:{:0.2f} - {:0.2f} y min:{:0.8f} - y max:{:0.8f}", chart_num, series_num,
            m_xminmax1.first, m_xminmax1.second, m_yminmax1.first, m_yminmax1.second);
    }
    // clang-format on

    //-------------------------------------------
    {
        // !!!
        float avrg = 0;
        std::pair<float, float> yrange{};
        std::pair<float, float> xrange{};
        if (scale_Y || magnify_Y) {
            yrange.first = (2 << 23) - 1;
            yrange.second = -yrange.first;
        } else {
            yrange.first = 0;
            yrange.second = 0;
        }

        float dt;
        if (m_channel == 5) {
            dt = (float)ch.dt() / 1000.0f;
        } else {
            dt = (float)ch.dt();
        }

        process_channel_points(points, dt, xrange, yrange, avrg, chart_points, npoints);

        if (series_num == 1) {
            if (m_channel ==5) {
                m_yrange1.first = std::min(yrange.first, m_yrange1.first);
                m_yrange1.second = std::max(yrange.second, m_yrange1.second);
            } else {
                m_yrange1 = yrange;
            }
            m_xrange1 = xrange;
            m_avrg1 = avrg;
        } else {
            m_yrange2 = yrange;
            m_xrange2 = xrange;
            m_avrg2 = avrg;
        }
    }

    auto *target_series = (series_num == 2) ? m_series2 : m_series;
    const auto &target_pen = (series_num == 2) ? m_pen2 : m_pen;
    target_series->replace(chart_points);
    target_series->setPen(target_pen);

    refresh(); // chart refresh for X, Y axis

    setUpdatesEnabled(true);
}
