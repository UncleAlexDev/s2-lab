/**
  \file   dpt_dump_gui.cpp
  \brief  Dual Pressure Dump -  widget page
  \author Aleksey Lekhtin
  \date   06.07.2026
*/

#include "dp_dump_view.h"

#include "cfg.h"
#include "chart.h"
#include "dbg.h"
#include "dp_dump.h"
#include "dpt.h"
#include "mw.h"
#include "packet_atlas_t.h"
#include "utils.h"
#include "wmsg.h"

#include <fmt/chrono.h>

#include <QApplication.h>
#include <QDir.h>
#include <QFileDialog>
#include <QLineEdit.h>
#include <QTimer>
#include <QVBoxLayout>

//---------------------------------------------------------
QWidget *DPTDUMP_VIEW::gui_page()
{
    return m_gui_page;
}

//---------------------------------------------------------
const QString &DPTDUMP_VIEW::page_name()
{
    return m_page_name;
}

//---------------------------------------------------------
QWidget *DPTDUMP_VIEW::create_page()
{
    create_chart1_control();
    create_chart2_control();
    create_chart3_control();
    create_chart4_control();

    // финальная раскладка виджетов слева
    auto *layout_left = new QVBoxLayout();
    layout_left->addWidget(m_chart1->gui_page(), 50);
    layout_left->addWidget(m_chart2->gui_page(), 50);
    layout_left->addWidget(m_chart3->gui_page(), 50);
    layout_left->addWidget(m_chart4->gui_page(), 50);

    // финальная раскладка виджетов справа
    auto *layout_right = new QVBoxLayout();
    layout_right->addWidget(m_chart1_control, 50);
    layout_right->addStretch();
    layout_right->addWidget(m_chart2_control, 50);
    layout_right->addStretch();
    layout_right->addWidget(m_chart3_control, 50);
    layout_right->addStretch();
    layout_right->addWidget(m_chart4_control, 50);

    // формирование экрана с графиками
    auto *lay = new QHBoxLayout();
    lay->addLayout(layout_left, 100);
    lay->addLayout(layout_right, 0);

    auto *page = new QWidget(this);
    page->setLayout(lay);
    return page;
}

//// //---------------------------------------------------------
//// QWidget *DPTDUMP_VIEW::create_close_button()
//// {
////     m_button_close = new QDialogButtonBox(QDialogButtonBox::Close);
////     QPushButton *button = m_button_close->button(QDialogButtonBox::Close);
////     if (button) {
////         auto *shortcut = new QShortcut(MW::mw);
////         shortcut->setKey(QKeySequence("Ctrl+W"));
////         auto tip = QStringLiteral("%1\n[%2]").arg(tr("Close window and return back"), button->shortcut().toString());
////         m_button_close->setToolTip(tip);
////         connect(shortcut, &QShortcut::activated, button, &QPushButton::click);
////
////         // shortcut->setKeys({QKeySequence("Backspace"), QKeySequence("Esc")});
////         // shortcut->setKeys({QKeySequence("Ctrl+W")});
////         // connect(shortcut, &QShortcut::activated, button, &QPushButton::click);
////         // QString tip = QStringLiteral("%1").arg(tr("Close window and return back"));
////         // auto keys = shortcut->keys();
////         // for (const auto &k : std::as_const(keys)) {
////         //    tip += QString("\n[%1]").arg(k.toString());
////         //}
////         // m_button_close->setToolTip(tip);
////     }
////
////     auto *wgt = new QWidget();
////     auto *hlay_receive_button = new QHBoxLayout(wgt);
////     hlay_receive_button->addStretch();
////     hlay_receive_button->addWidget(m_button_close);
////     hlay_receive_button->addStretch();
////
////     connect(m_button_close,
////         &QDialogButtonBox::rejected,
////         this,
////         [this]()
////         {
////             DBG(1, "close dump");
////             MW::mw->statusbar_clear();
////             chart_reset(m_chart1);
////             chart_reset(m_chart2);
////             chart_reset(m_chart3);
////             chart_reset(m_chart4);
////             auto index = MW::mw->tabWidget()->currentIndex();
////             if (index != -1) {
//// #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
////                 MW::mw->tabWidget()->setTabVisible(index, false);
//// #else
////                 int idx = MW::mw->tabWidget()->indexOf(m_gui_page);
////                 if (idx != -1) {
////                     MW::mw->tabWidget()->removeTab(idx);
////                 }
//// #endif
////                 if (index > 0) {
////                     --index;
////                 }
////             }
////             MW::mw->unlock_my_tab(m_gui_page);
////             if (MW::mw->tabWidget()->widget(index)) {
////                 MW::mw->tabWidget()->setCurrentIndex(index);
////             }
////         });
////
////     return wgt;
//// }

//---------------------------------------------------------
QWidget *DPTDUMP_VIEW::create_control_buttons()
{
    m_button_clear_charts = new QPushButton(tr("Clear [C]"));
    m_button_clear_charts->setCheckable(false);
    m_button_clear_charts->setChecked(false);
    m_button_clear_charts->setAutoDefault(false);
    m_button_clear_charts->setShortcut(QKeySequence("C"));
    auto tip = QStringLiteral("%1\n[%2]").arg(tr("Clear charts"), m_button_clear_charts->shortcut().toString());
    m_button_clear_charts->setToolTip(tip);

    connect(m_button_clear_charts,
        &QPushButton::clicked,
        this,
        [&]()
        {
            DBG(1);
            chart_reset(m_chart1);
            chart_reset(m_chart2);
            chart_reset(m_chart3);
            chart_reset(m_chart4);
            auto zero_string = "0.0000";
            m_chart1_max_min_values[0]->setText(zero_string);
            m_chart1_max_min_values[1]->setText(zero_string);
            m_chart2_max_min_values[0]->setText(zero_string);
            m_chart2_max_min_values[1]->setText(zero_string);
        });

    //---------------------------------
    m_button_set_yscale = new QPushButton(tr("Stretch [Y]"));
    m_button_set_yscale->setCheckable(true);
    m_button_set_yscale->setChecked(m_yscale_button_status);
    m_button_set_yscale->setShortcut(QKeySequence("Y"));
    tip = tr("Scale data on Y axis\n(Zoom In)");
    m_button_set_yscale->setToolTip(tip);
    connect(m_button_set_yscale, &QPushButton::toggled, this, &DPTDUMP_VIEW::slot_set_yscale);

    //-------------------------------------------
    QIcon icon(":load_raw.jpg");
    auto *button_view_dump = new QPushButton(icon, tr("Load [L]"));
    button_view_dump->setIconSize(QSize(24, 24));
    DBGF(3,
        "view dump icon size: {} x {}",
        button_view_dump->iconSize().rheight(),
        button_view_dump->iconSize().rwidth());

    button_view_dump->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    button_view_dump->setCheckable(false);
    button_view_dump->setChecked(false);
    button_view_dump->setAutoDefault(false);
    button_view_dump->setShortcut(QKeySequence("L"));
    //// tip = QStringLiteral("%1\n%2\n[%3]")
    ////           .arg(tr("View dump file"), CFG::cfg->dp_dump_dir, button_view_dump->shortcut().toString());
    tip = QStringLiteral("%1\n%2").arg(tr("View dump file"), CFG::cfg->dp_dump_dir);
    button_view_dump->setToolTip(tip);

    // auto buttonHeight = button_view_dump->sizeHint().height();
    // button_view_dump->setFixedWidth(buttonHeight);

    connect(button_view_dump,
        &QPushButton::clicked,
        this,
        [this]()
        {
            MW::mw->statusbar_clear();
            if (!select_dump_file(CFG::cfg->dp_dump_dir)) {
                return;
            }
            load_dump();
        });

    //-------------------------------------------
    // раскладка кнопок управления
    auto *buttons_layout = new QVBoxLayout();
    buttons_layout->setContentsMargins(0, 0, 0, 0);
    buttons_layout->addStretch();
    buttons_layout->addWidget(button_view_dump);
    buttons_layout->addSpacing(2);
    buttons_layout->addWidget(m_button_set_yscale);
    buttons_layout->addSpacing(2);
    buttons_layout->addWidget(m_button_clear_charts);
    buttons_layout->addStretch();

    //-------------------------------------------
    auto *wgt = new QWidget();
    wgt->setLayout(buttons_layout);
    return wgt;
}

//---------------------------------------------------------
void DPTDUMP_VIEW::create_chart1_control()
{
    auto zero_string = "0.0000";
    m_chart1_max_min_values[0] = new QLineEdit(zero_string);
    m_chart1_max_min_values[0]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_chart1_max_min_values[0]->setContentsMargins(2, 2, 2, 2);
    m_chart1_max_min_values[0]->setReadOnly(true);

    m_chart1_max_min_values[1] = new QLineEdit(zero_string);
    m_chart1_max_min_values[1]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_chart1_max_min_values[1]->setContentsMargins(2, 2, 2, 2);
    m_chart1_max_min_values[1]->setReadOnly(true);

    //-------------------------------------------
    auto *chart1_field_min_layout = new QHBoxLayout();
    chart1_field_min_layout->addWidget(new QLabel("Min"));
    chart1_field_min_layout->addWidget(m_chart1_max_min_values[0]);
    chart1_field_min_layout->addStretch();

    auto *chart1_field_max_layout = new QHBoxLayout();
    chart1_field_max_layout->addWidget(new QLabel("Max"));
    chart1_field_max_layout->addWidget(m_chart1_max_min_values[1]);
    chart1_field_max_layout->addStretch();

    m_chart1_control = new QWidget();
    m_chart1_control->setContentsMargins(0, 0, 0, 0);
    auto *chart1_info_box_layout = new QVBoxLayout(m_chart1_control);
    chart1_info_box_layout->setContentsMargins(2, 6, 2, 6);
    chart1_info_box_layout->addLayout(chart1_field_max_layout);
    chart1_info_box_layout->addLayout(chart1_field_min_layout);
    chart1_info_box_layout->addStretch();
}

//---------------------------------------------------------
void DPTDUMP_VIEW::create_chart2_control()
{
    auto zero_string = "0.0000";
    m_chart2_max_min_values[0] = new QLineEdit(zero_string);
    m_chart2_max_min_values[0]->setContentsMargins(2, 2, 2, 2);
    m_chart2_max_min_values[0]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_chart2_max_min_values[0]->setReadOnly(true);

    m_chart2_max_min_values[1] = new QLineEdit(zero_string);
    m_chart2_max_min_values[1]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_chart2_max_min_values[1]->setContentsMargins(2, 2, 2, 2);
    m_chart2_max_min_values[1]->setReadOnly(true);

    //-------------------------------------------
    auto *chart2_field_min_layout = new QHBoxLayout();
    chart2_field_min_layout->addWidget(new QLabel("Min"));
    chart2_field_min_layout->addWidget(m_chart2_max_min_values[0]);
    chart2_field_min_layout->addStretch();

    auto *chart2_field_max_layout = new QHBoxLayout();
    chart2_field_max_layout->addWidget(new QLabel("Max"));
    chart2_field_max_layout->addWidget(m_chart2_max_min_values[1]);
    chart2_field_max_layout->addStretch();

    //-------------------------------------------
    m_chart2_control = new QWidget;
    m_chart2_control->setContentsMargins(0, 0, 0, 0);
    auto *chart2_info_box_layout = new QVBoxLayout(m_chart2_control);
    chart2_info_box_layout->setContentsMargins(2, 6, 2, 6);
    chart2_info_box_layout->addLayout(chart2_field_max_layout);
    chart2_info_box_layout->addLayout(chart2_field_min_layout);
    chart2_info_box_layout->addStretch();
}

//---------------------------------------------------------
void DPTDUMP_VIEW::create_chart3_control()
{
    auto *delay_field = create_delay_field();
    auto *layout = new QVBoxLayout();
    layout->addStretch();
    layout->addWidget(delay_field);
    layout->addStretch();
    m_chart3_control = new QWidget();
    m_chart3_control->setLayout(layout);
}

//---------------------------------------------------------
void DPTDUMP_VIEW::create_chart4_control()
{
    auto *buttons_plus_minus = create_plus_minus_buttons();
    auto *buttons_ctrl = create_control_buttons();
    //// auto *button_close = create_close_button();

    auto *buttons_plus_minus_layout = new QHBoxLayout();
    buttons_plus_minus_layout->addWidget(buttons_plus_minus, 1, Qt::AlignLeft);
    buttons_plus_minus_layout->addWidget(buttons_ctrl, 99, Qt::AlignCenter);

    auto *control_layout = new QVBoxLayout();
    control_layout->addLayout(buttons_plus_minus_layout);
    //// control_layout->addWidget(button_close);

    m_chart4_control = new QWidget();
    auto *layout = new QHBoxLayout(m_chart4_control);
    // layout->addWidget(m_chart4->gui_page(), 99);
    layout->addLayout(control_layout, 1);
}

//---------------------------------------------------------
QWidget *DPTDUMP_VIEW::create_plus_minus_buttons()
{
    m_button_chart_plus = new QPushButton("+");
    m_button_chart_plus->setFixedWidth(32);
    m_button_chart_plus->setCheckable(true);
    m_button_chart_plus->setChecked(false);
    m_button_chart_plus->setAutoDefault(false);
    m_button_chart_plus->setShortcut(QKeySequence(Qt::Key_Plus));
    QString tip = QString("%1\n[%2]").arg(tr("Summation of two channels"), m_button_chart_plus->shortcut().toString());
    m_button_chart_plus->setToolTip(tip);
    connect(m_button_chart_plus,
        &QPushButton::clicked,
        [&](bool status)
        {
            if (status) {
                m_chart4_mode = SUM_OR_SUB_MODE::SUM;
                m_button_chart_minus->setChecked(false);
                chart_reset(m_chart4);
                set_points_sum(m_chart4);
            } else {
                m_chart4_mode = SUM_OR_SUB_MODE::NONE;
                m_chart4->series()->clear();
            }
        });

    //---------------------------------
    m_button_chart_minus = new QPushButton("-");
    m_button_chart_minus->setFixedWidth(32);
    m_button_chart_minus->setCheckable(true);
    m_button_chart_minus->setChecked(false);
    m_button_chart_minus->setAutoDefault(false);
    m_button_chart_minus->setShortcut(QKeySequence(Qt::Key_Minus));
    tip = QString("%1\n[%2]").arg(tr("Subtraction of two channels"), m_button_chart_minus->shortcut().toString());
    m_button_chart_minus->setToolTip(tip);
    connect(m_button_chart_minus,
        &QPushButton::clicked,
        [&](bool status)
        {
            if (status) {
                m_chart4_mode = SUM_OR_SUB_MODE::SUB;
                m_button_chart_plus->setChecked(false);
                chart_reset(m_chart4);
                set_points_sum(m_chart4);
            } else {
                m_chart4_mode = SUM_OR_SUB_MODE::NONE;
                m_chart4->series()->clear();
            }
        });

    //--- блок плюс/минус кнопок ------
    auto *wgt = new QWidget();
    auto *layout_plus_minus_button = new QVBoxLayout(wgt);
    layout_plus_minus_button->addStretch();
    layout_plus_minus_button->addWidget(m_button_chart_minus);
    layout_plus_minus_button->addSpacing(2);
    layout_plus_minus_button->addWidget(m_button_chart_plus);
    layout_plus_minus_button->addStretch();
    return wgt;
}

//---------------------------------------------------------
QWidget *DPTDUMP_VIEW::create_delay_field()
{
    //--- блок настроки задержки ------
    int max_delay_ms = 50;
    auto *delay_field_label = new QLabel(QString("%1 [%2-%3]").arg(tr("Delay")).arg(0).arg(max_delay_ms));
    delay_field_label->setToolTip(tr("Compensation delay between the channels"));
    m_delay_field = new QSpinBox();
    m_delay_field->setContentsMargins(0, 0, 0, 0);
    m_delay_field->setSingleStep(2);
    m_delay_field->setRange(0, max_delay_ms);
    m_delay_field->setValue(0);
    m_delay_field->setToolTip(delay_field_label->toolTip());
    m_delay_field->setSuffix(tr(" ms"));

    connect(m_delay_field,
        qOverload<int>(&QSpinBox::valueChanged),
        [this](int num)
        {
            DBGF(1, "{} ms", num);
            m_delay = num;
            set_points_two_chs(m_chart3);
            set_points_sum(m_chart4);

            QTimer::singleShot(0,
                m_delay_field,
                [this]()
                {
                    // это дрочиво :(
                    if (auto *lineEdit = m_delay_field->findChild<QLineEdit *>()) {
                        lineEdit->deselect();   // уберет выделение
                        lineEdit->clearFocus(); // вообще убрать все
                    }
                });
        });

    //-------------------------------------------
    auto *button_reset_delay = new QPushButton(tr("Reset"));
    button_reset_delay->setCheckable(false);
    button_reset_delay->setChecked(false);
    button_reset_delay->setAutoDefault(false);
    auto tip = tr("Reset delay to 0 ms");
    button_reset_delay->setToolTip(tip);

    connect(button_reset_delay,
        &QPushButton::clicked,
        this,
        [&]()
        {
            DBG(1);
            m_delay_field->setValue(0);
            m_delay = 0;
            set_points_two_chs(m_chart3);
        });

    //--- Раскладка -----------------------------
    m_box_channels_delay = new QWidget();
    m_box_channels_delay->setContentsMargins(0, 0, 0, 0);

    auto *field_layout = new QHBoxLayout();
    field_layout->setContentsMargins(0, 0, 0, 0);
    field_layout->addWidget(delay_field_label);
    field_layout->addWidget(m_delay_field);
    field_layout->addStretch();

    auto *layout = new QVBoxLayout(m_box_channels_delay);
    layout->addLayout(field_layout);
    layout->addWidget(button_reset_delay);

    // TODO - заблокировано - алгоритм в отладке еще...
    //// m_delay_field->setDisabled(true);
    //// delay_field_label->setToolTip("\nNow is lock!!! \n\nUnder developing...\n");
    m_delay_field->setToolTip(delay_field_label->toolTip());

    return m_box_channels_delay;
}

//-----------------------------------------------
void DPTDUMP_VIEW::slot_set_yscale(bool checked)
{
    DBG(3);
    DBGF(3, "{}", (checked ? "on" : "off"));
    m_yscale_button_status = checked;
    m_chart1->scale_Y = checked;
    m_chart2->scale_Y = checked;
    m_chart3->scale_Y = checked;
    m_chart4->scale_Y = checked;
    save_status();
    chart_reset(m_chart1);
    chart_reset(m_chart2);
    chart_reset(m_chart3);
    chart_reset(m_chart4);
    set_points(m_channels_points);
}

//---------------------------------------------------------
void DPTDUMP_VIEW::select_page()
{
    if (!m_gui_page) {
        return;
    }

    int index = MW::mw->tabWidget()->indexOf(m_gui_page);
    if (index == -1) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    MW::mw->tabWidget()->setTabVisible(index, true);
    MW::mw->tabWidget()->setCurrentIndex(index);
#else
    int idx = MW::mw->tabWidget()->indexOf(m_gui_page);
    if (idx == -1) {
        MW::mw->tabWidget()->insertTab(idx, m_gui_page, m_page_name);
        auto tip = QString("%1\n\n[%2]")
                       .arg(MW::mw->tabWidget()->tabText(index), tr("PgUp or PgDown to select prev/next tab"));
        MW::mw->tabWidget()->setTabToolTip(idx, tip);
    }
#endif
    MW::mw->lock_my_tab(m_gui_page);
}

