/**
  \file   dpt_dump_fonts.cpp
  \brief  Dual Pressure Dump file - fonts operations
  \author Aleksey Lekhtin
  \date   06.07.2026
*/

#include "dp_dump_view.h"

#include "cfg.h"
#include "chart.h"
#include "dbg.h"

#include <QApplication>
#include <QPushbutton.h>

//---------------------------------------------------------
void DPTDUMP_VIEW::set_fonts() const
{
    DBGF(4, "system font: {}", QGuiApplication::font().pointSize());
    set_text_font();
    set_button_font();
    set_charts_font();
    set_mono_font();
}

//---------------------------------------------------------
void DPTDUMP_VIEW::set_text_font() const
{
    QList<QWidget *> list{m_chart1_control, m_chart2_control, m_box_channels_delay};

    for (auto *wgt : list) {
        wgt->setStyleSheet(QString("QGroupBox, QWidget { font: %1pt \"%2\"; }")
                .arg(CFG::cfg->text_font.pointSize())
                .arg(CFG::cfg->text_font.family()));
    }
}

//---------------------------------------------------------
void DPTDUMP_VIEW::set_button_font() const
{
    QFont fnt = CFG::cfg->button_font;
    fnt.setBold(true);
    fnt.setPointSize(fnt.pointSize() + 2);
    m_button_chart_plus->setFont(fnt);
    m_button_chart_minus->setFont(fnt);
    //m_button_close->setFont(fnt);
}

//---------------------------------------------------------
void DPTDUMP_VIEW::set_charts_font() const
{
    DBG(1);
    CHART *chart = nullptr;
    for (int ch : {1, 2, 3, 4}) {
        if (ch == 1) {
            chart = m_chart1;
        } else if (ch == 2) {
            chart = m_chart2;
        } else if (ch == 3) {
            chart = m_chart3;
        } else if (ch == 4) {
            chart = m_chart4;
        }
        //// chart->chart()->setTitleFont(CFG::cfg->chart_font);
        chart->set_title_font(CFG::cfg->chart_font);
        chart->axisX()->setTitleFont(CFG::cfg->chart_font);
        chart->axisX()->setLabelsFont(CFG::cfg->chart_font);
        chart->axisY()->setTitleFont(CFG::cfg->chart_font);
        chart->axisY()->setLabelsFont(CFG::cfg->chart_font);
    }
    //// QApplication::processEvents();
}

//---------------------------------------------------------
void DPTDUMP_VIEW::set_mono_font() const
{
}
