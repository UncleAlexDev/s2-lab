/*!
  \file   mainwindow.cpp
  \author Aleksey Lekhtin
  \date   27.11.2024
  \brief  Главное окно приложения
*/

#include "mw.h"

#include "adc.h"
#include "cfg.h"
#include "dbg.h"
#include "dpt.h"
#include "network.h"

#include <QApplication>
#include <QDir>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>


//---------------------------------------------------------
void MW::slot_select_next_tab()
{
    int stop = 0;
    int index = m_tabwgt->currentIndex();
    do {
        ++index;
        if (index >= m_tabwgt->count()) {
            index = 0;
            ++stop;
        }
        if (m_tabwgt->isTabVisible(index)) {
            m_tabwgt->setCurrentIndex(index);
            break;
        }
    } while (stop < 2);
}

//---------------------------------------------------------
void MW::slot_select_prev_tab()
{
    int stop = 0;
    int index = m_tabwgt->currentIndex();
    do {
        --index;
        if (index < 0) {
            index = m_tabwgt->count() - 1;
            ++stop;
        }
        if (m_tabwgt->isTabVisible(index)) {
            m_tabwgt->setCurrentIndex(index);
            break;
        }
    } while (stop < 2);
}

//---------------------------------------------------------
void MW::slot_tabwgt_changed(int index)
{
    DBG(1);
    //auto status = NETWORK::running();
    //if (ADC::adc && m_tabwgt->currentWidget() == ADC::adc->gui_page()) {
    //    ADC::adc->button_receive()->setChecked(status);
    //} else if (DPT::dpt && m_tabwgt->currentWidget() == DPT::dpt->gui_page()) {
    //    DPT::dpt->button_receive()->setChecked(status);
    //}

    CFG::cfg->current_page = index;
    CFG::cfg->save_status();
}

//---------------------------------------------------------
void MW::lock_my_tab(const QWidget *tab)
{
    for (int index = 0; index < m_tabwgt->count(); ++index) {
        auto *wgt = m_tabwgt->widget(index);
        if (wgt != tab) {
            m_tabwgt->tabBar()->setTabEnabled(index, false);
        }
    }
}

//---------------------------------------------------------
void MW::unlock_my_tab(const QWidget *tab)
{
    for (int index = 0; index < m_tabwgt->count(); ++index) {
        auto *wgt = m_tabwgt->widget(index);
        if (wgt != tab) {
            m_tabwgt->tabBar()->setTabEnabled(index, true);
        }
    }
}
