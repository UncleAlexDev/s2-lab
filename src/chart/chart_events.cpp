/**
  \file   chart_events.cpp
  \brief  События на графиках
  \author Aleksey Lekhtin
  \date   26.12.2024
*/

#include <QDebug>

#include "chart.h"
#include "ok.h"

#ifndef INNER_RUBBERBAND
bool CHARTVIEW::viewportEvent(QEvent *event)
{
    //qDebug() << event;
    if (event->type() == QEvent::TouchBegin) {
        // By default touch events are converted to mouse events. So
        // after this event we will get a mouse event also but we want
        // to handle touch events as gestures only. So we need this safeguard
        // to block mouse events that are actually generated from touch.
        //// m_isTouching = true;

        // Turn off animations when handling gestures they
        // will only slow us down.
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void CHARTVIEW::mousePressEvent(QMouseEvent *event)
{
#ifndef INNER_RUBBERBAND
    // TODO
    qDebug() << event;
    m_rubberBand_origin_pos = event->pos();
    if (!m_rubberBand) {
        m_rubberBand = new QRubberBand(QRubberBand::Line, this);
    }
    m_rubberBand->setGeometry(QRect(m_rubberBand_origin_pos, QSize()));
    m_rubberBand->show();
#else
    QChartView::mousePressEvent(event);
#endif

    // if (m_isTouching)
    //     return;
    // QChartView::mousePressEvent(event);
}

void CHARTVIEW::mouseMoveEvent(QMouseEvent *event)
{
#ifndef INNER_RUBBERBAND
    // TODO
    if (m_rubberBand) {
        qDebug() << event;
        m_rubberBand->setGeometry(QRect(m_rubberBand_origin_pos, event->pos()).normalized());
    } else {
        QChartView::mouseMoveEvent(event);
    }
#else
    QChartView::mousePressEvent(event);
#endif
    // //qDebug() << event;
    // if (m_isTouching)
    //     return;
    // QChartView::mouseMoveEvent(event);
}

void CHARTVIEW::mouseReleaseEvent(QMouseEvent *event)
{
#ifndef INNER_RUBBERBAND
    // TODO
    if (m_rubberBand) {
        qDebug() << event;
        m_rubberBand->hide();
        int x1 = m_rubberBand_origin_pos.rx();
        int y1 = m_axisY->max();
        int x2 = event->pos().rx();
        int y2 = m_axisY->min();
        qDebug() << x1 << y1 << x2 << y2;
        //// QList<QAbstractAxis *> axes_list = chart()->axes(Qt::Vertical, m_series);
        //// const QValueAxis *axY = static_cast<QValueAxis*>(axes_list.at(0));
        //// y2 = axY->min();
        //// qDebug() << x1 << y1 << x2 << y2;

        //QRect selection = QRect(x1, y1, x2, y2).normalized();
        QRect selection = QRect(0, 100, 100, 100).normalized();
        qDebug() << "selection:" << selection;

        // determine selection, for example using QRect::intersects()
        // and QRect::contains().
        QRect rect = this->geometry();
        qDebug() << "geometry:" << rect;
        //// if (rect.contains(selection)) {
        ////     chart()->setPlotArea(selection);
        //// }
        //// chart()->setPlotArea(selection);

    }
    //// else {
    ////     QChartView::mouseMoveEvent(event);
    //// }
    QChartView::mouseMoveEvent(event);
#else
    QChartView::mousePressEvent(event);
#endif

    // //qDebug() << event;
    // if (m_isTouching)
    //     m_isTouching = false;
    //
    // // Because we disabled animations when touch event was detected
    // // we must put them back on.
    // chart()->setAnimationOptions(QChart::SeriesAnimations);
    //
    // QChartView::mouseReleaseEvent(event);
}

void CHARTVIEW::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << event->key();
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Asterisk:
        chart()->zoomReset();
        break;

    case Qt::Key_Left:
        if (event->modifiers() == Qt::CTRL) {
            chart()->scroll(-100, 0);
        } else {
            chart()->scroll(-10, 0);
        }
        break;
    case Qt::Key_Right:
        if (event->modifiers() == Qt::CTRL) {
            chart()->scroll(100, 0);
        } else {
            chart()->scroll(10, 0);
        }
        break;
    case Qt::Key_Up:
        if (event->modifiers() == Qt::CTRL) {
            chart()->scroll(0, 50);
        } else {
            chart()->scroll(0, 10);
        }
        break;
    case Qt::Key_Down:
        if (event->modifiers() == Qt::CTRL) {
            chart()->scroll(0, -50);
        } else {
            chart()->scroll(0, -10);
        }
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}
#endif
