/**
  \file   create_layouts.cpp
  \brief  Создание раскладки интерфейсных объектов (кнопки, графики, ...)
  \author Aleksey Lekhtin
  \date   10.12.2024
*/

#include <QAction>
#include <QApplication>
#include <QLayout>
#include <QMenuBar>
#include <QShortcut>

#include "adc.h"
#include "dpt.h"
#include "mw.h"

static QString make_tip(QAction *act)
{
    QString tip = act->text();
    tip.replace("&", "");
    if (!act->shortcut().toString().isEmpty())
        tip = tip + "\n[" + act->shortcut().toString() + "]";
    return tip;
}

void MW::create_menu()
{
    ////action_open = new QAction(tr("&Open"), this);
    ////action_open->setShortcut(QKeySequence("Ctrl+O"));
    ////action_open->setStatusTip(make_tip(action_open) + " or [Ctrl+F]");
    ////connect(action_open, &QAction::triggered, this, &MW::slot_load_file);

    m_action_about = new QAction(tr("&About"), this);
    m_action_about->setShortcut(QKeySequence("F1"));
    connect(m_action_about, &QAction::triggered, this,
        [this]()
        {
            about();
        });

    // m_action_tab1 = new QAction(tr("ADC"), this);
    // m_action_tab1->setCheckable(true);
    // m_action_tab1->setChecked(CFG::cfg->menu_action_adc_enabled);
    // m_action_tab1->setShortcut(QKeySequence("Ctrl+1"));
    // m_action_tab1->setStatusTip(make_tip(m_action_tab1));
    // connect(m_action_tab1, &QAction::toggled, this,
    //     [this](bool checked)
    //     {
    //         CFG::cfg->menu_action_adc_enabled = checked;
    //         int index = 0;
    //         if (checked) {
    //             m_tabwgt->tabBar()->setTabVisible(index, true);
    //             m_tabwgt->setCurrentIndex(index);
    //             m_tabwgt->tabBar()->updateGeometry();
    //             CFG::cfg->current_page = m_tabwgt->currentIndex();
    //             return;
    //         }
    //         if (m_tabwgt->isVisibleTo(DPT::dpt->gui_page())) {
    //             m_tabwgt->setCurrentIndex(1);
    //         }
    //         m_tabwgt->tabBar()->setTabVisible(index, false);
    //         m_tabwgt->tabBar()->updateGeometry();
    //         CFG::cfg->current_page = m_tabwgt->currentIndex();
    //     });

    // m_action_tab2 = new QAction(tr("Two Pressure Sensors"), this);
    // m_action_tab2->setCheckable(true);
    // m_action_tab2->setChecked(CFG::cfg->menu_action_dpt_enabled);
    // m_action_tab2->setShortcut(QKeySequence("Ctrl+2"));
    // m_action_tab2->setStatusTip(make_tip(m_action_tab2));
    // connect(m_action_tab2, &QAction::toggled, this,
    //     [this](bool checked)
    //     {
    //         CFG::cfg->menu_action_dpt_enabled = checked;
    //         int index = 1;
    //         CFG::cfg->current_page = index;
    //         if (checked) {
    //             m_tabwgt->tabBar()->setTabVisible(index, true);
    //             m_tabwgt->setCurrentIndex(index);
    //             m_tabwgt->tabBar()->updateGeometry();
    //             CFG::cfg->current_page = m_tabwgt->currentIndex();
    //             return;
    //         }
    //         if (m_tabwgt->isVisibleTo(ADC::adc->gui_page())) {
    //             m_tabwgt->setCurrentIndex(0);
    //         }
    //         m_tabwgt->tabBar()->setTabVisible(index, false);
    //         m_tabwgt->tabBar()->updateGeometry();
    //         CFG::cfg->current_page = m_tabwgt->currentIndex();
    //     });

    m_action_exit = new QAction(tr("&Exit"), this);
    m_action_exit->setShortcut(QKeySequence("Ctrl+Q"));
    m_action_exit->setStatusTip(make_tip(m_action_exit));
    connect(m_action_exit, &QAction::triggered, this,
        []()
        {
            QApplication::closeAllWindows();
        });

    // TODO
    //    action_settings_esz7 = new QAction(tr("&ESZ7 Board"), this);
    //    action_settings_esz7->setShortcut(QKeySequence("Ctrl+Alt+E"));
    //    action_settings_esz7->setStatusTip(make_tip(action_settings_esz7));
    //    connect(action_settings_esz7, &QAction::triggered, this, &MW::slot_settings_esz7);

    m_action_settings_data_source = new QAction(tr("&Data Source"), this);
    m_action_settings_data_source->setShortcut(QKeySequence("Ctrl+Alt+S"));
    m_action_settings_data_source->setStatusTip(make_tip(m_action_settings_data_source));
    connect(m_action_settings_data_source, &QAction::triggered, this, &MW::slot_settings_data_source);

    m_action_settings_misc = new QAction(tr("&Miscellaneous"), this);
    m_action_settings_misc->setShortcut(QKeySequence("Ctrl+Alt+M"));
    m_action_settings_misc->setStatusTip(make_tip(m_action_settings_misc));
    connect(m_action_settings_misc, &QAction::triggered, this, &MW::slot_settings_miscellaneous);

    m_action_fonts_size = new QAction(tr("&Font Sizes"), this);
    m_action_fonts_size->setToolTip(make_tip(m_action_fonts_size));
    // m_action__fonts_size->setIconVisibleInMenu(false);
    connect(m_action_fonts_size, SIGNAL(triggered()), this, SLOT(slot_font_settings()));

    //-------------------------------------------------
    // QMenuBar *menu_bar = new QMenuBar(this);
    // setMenuBar(menu_bar);
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addSeparator();
    // m_fileMenu->addAction(m_action_tab1);
    // m_fileMenu->addAction(m_action_tab2);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_action_exit);

    m_fileMenu = menuBar()->addMenu(tr("&Settings"));
    m_fileMenu->addAction(m_action_settings_data_source);
    m_fileMenu->addAction(m_action_settings_misc);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_action_fonts_size);

    menuBar()->addSeparator();
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_action_about);

    //-------------------------------------------------
    m_shortcut_prev_tab = new QShortcut(QKeySequence("Ctrl+PgUp"), this);
    connect(m_shortcut_prev_tab, &QShortcut::activated, this, &MW::slot_select_prev_tab);

    m_shortcut_next_tab = new QShortcut(QKeySequence("Ctrl+PgDown"), this);
    connect(m_shortcut_next_tab, &QShortcut::activated, this, &MW::slot_select_next_tab);
}
