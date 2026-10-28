/**
\file   dpt_gui.cpp
\brief  Dual Pressure widget page
\author Aleksey Lekhtin
\date   06.05.2026
*/

#include "dpt.h"

#include "cfg.h"
#include "chart.h"
#include "dbg.h"
#include "dp_dump.h"
#include "mw.h"
#include "network.h"
#include "utils.h"
#include "wmsg.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QStorageInfo>
#include <QTimer>

//---------------------------------------------------------
QWidget *DPT::gui_page()
{
    return m_gui_page;
}

//---------------------------------------------------------
QWidget *DPT::create_page()
{
    create_chart1_control();
    create_chart2_control();
    create_chart3_control();
    create_chart4_control();

    // финальная раскладка виджетов слева
    auto *layout_left = new QVBoxLayout();
    layout_left->setContentsMargins(0, 0, 0, 0);
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
    auto *dpt_page = new QWidget(this);
    auto *lay = new QHBoxLayout(dpt_page);
    lay->addLayout(layout_left, 100);
    lay->addLayout(layout_right, 0);
    return dpt_page;
}

//---------------------------------------------------------
QWidget *DPT::create_control_buttons()
{
    m_button_clear_charts = new QPushButton(tr("Clear [C]"));
    m_button_clear_charts->setCheckable(false);
    m_button_clear_charts->setChecked(false);
    m_button_clear_charts->setAutoDefault(false);
    m_button_clear_charts->setShortcut(QKeySequence("C"));
    //auto tip = QStringLiteral("%1\n[%2]").arg(tr("Clear charts"), m_button_clear_charts->shortcut().toString());
    auto tip = tr("Clear charts");
    m_button_clear_charts->setToolTip(tip);

    connect(m_button_clear_charts,
        &QPushButton::clicked,
        this,
        [&]()
        {
            chart_reset(m_chart1);
            chart_reset(m_chart2);
            chart_reset(m_chart3);
            chart_reset(m_chart4);
            auto zero_string = "0." + QString(CFG::cfg->decimal_points, QChar('0'));
            m_chart1_max_min_values[0]->setText(zero_string);
            m_chart1_max_min_values[1]->setText(zero_string);
            m_chart2_max_min_values[0]->setText(zero_string);
            m_chart2_max_min_values[1]->setText(zero_string);
        });

    //---------------------------------
    ////m_button_show_Y_raw = new QPushButton(tr("Raw [R]"));
    ////m_button_show_Y_raw->setCheckable(true);
    ////m_button_show_Y_raw->setChecked(m_raw_button_status);
    ////m_button_show_Y_raw->setShortcut(QKeySequence("R"));
    ////tip = tr("Display Raw value of data");
    ////m_button_show_Y_raw->setToolTip(tip);
    ////connect(m_button_show_Y_raw, &QPushButton::toggled, this, &DPT::slot_show_raw);

    //---------------------------------
    m_button_set_yscale = new QPushButton(tr("Stretch [Y]"));
    m_button_set_yscale->setCheckable(true);
    m_button_set_yscale->setChecked(m_yscale_button_status);
    m_button_set_yscale->setShortcut(QKeySequence("Y"));
    tip = tr("Scale data on Y axis\n(Zoom In)");
    m_button_set_yscale->setToolTip(tip);
    connect(m_button_set_yscale, &QPushButton::toggled, this, &DPT::slot_set_yscale);

    //-------------------------------------------
    // раскладка кнопок управления графиком
    auto *buttons_layout = new QVBoxLayout();
    buttons_layout->setContentsMargins(0, 0, 0, 0);
    buttons_layout->addWidget(m_button_set_yscale);
    buttons_layout->addSpacing(2);
    //// buttons_layout->addWidget(m_button_show_Y_raw);
    //// buttons_layout->addSpacing(2);
    buttons_layout->addWidget(m_button_clear_charts);
    buttons_layout->addStretch();

    //-------------------------------------------
    auto *wgt = new QWidget();
    wgt->setLayout(buttons_layout);
    return wgt;
}

//---------------------------------------------------------
void DPT::create_chart1_control()
{
    m_chart1_channel_list = new QComboBox();
    m_chart1_channel_list->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_chart1_channel_list->setDuplicatesEnabled(false);
    m_chart1_channel_list->setMinimumContentsLength(12);
    m_chart1_channel_list->addItem(tr("Nothing"));
    m_chart1_channel_list->addItem(tr("Channel 1"));
    m_chart1_channel_list->addItem(tr("Channel 2"));
    m_chart1_channel_list->addItem(tr("Channel 3"));
    m_chart1_channel_list->addItem(tr("Channel 4"));
    m_chart1_channel_list->setMaxVisibleItems(5);
    m_chart1_channel_list->setCurrentIndex(chart1_selected_channel);

    shortcut_channel_select1 = new QShortcut(QKeySequence("Alt+F1"), this);
    QString tip = QStringLiteral("%1 \n[%2]")
                      .arg(tr("Selects the channel from which data will be displayed  on chart #1"),
                          shortcut_channel_select1->key().toString());
    m_chart1_channel_list->setToolTip(tip);

    connect(shortcut_channel_select1,
        &QShortcut::activated,
        [combobox = m_chart1_channel_list]()
        {
            combobox->setFocus(Qt::ShortcutFocusReason);
            combobox->showPopup();
        });

    connect(m_chart1_channel_list,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [&](int index)
        {
            chart1_selected_channel = index;
            m_chart1->m_channel = index;
            chart_reset(m_chart1);
            save_status();
        });

    //-------------------------------------------
    auto zero_string = "0." + QString(CFG::cfg->decimal_points, QChar('0'));
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
    chart1_info_box_layout->addWidget(m_chart1_channel_list, Qt::AlignLeft);
    chart1_info_box_layout->addLayout(chart1_field_max_layout);
    chart1_info_box_layout->addLayout(chart1_field_min_layout);
    chart1_info_box_layout->addStretch();
}

//---------------------------------------------------------
void DPT::create_chart2_control()
{
    m_chart2_channel_list = new QComboBox();
    m_chart2_channel_list->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_chart2_channel_list->setDuplicatesEnabled(false);
    m_chart2_channel_list->setMinimumContentsLength(12);
    m_chart2_channel_list->addItem(tr("Nothing"));
    m_chart2_channel_list->addItem(tr("Channel 1"));
    m_chart2_channel_list->addItem(tr("Channel 2"));
    m_chart2_channel_list->addItem(tr("Channel 3"));
    m_chart2_channel_list->addItem(tr("Channel 4"));
    m_chart2_channel_list->setMaxVisibleItems(5);
    m_chart2_channel_list->setCurrentIndex(chart2_selected_channel);
    shortcut_channel_select2 = new QShortcut(QKeySequence("Alt+F2"), this);
    QString tip = QStringLiteral("%1 \n[%2]")
                      .arg(tr("Selects the channel from which data will be displayed on chart #2"),
                          shortcut_channel_select2->key().toString());
    m_chart2_channel_list->setToolTip(tip);

    connect(shortcut_channel_select2,
        &QShortcut::activated,
        [combobox = m_chart2_channel_list]()
        {
            combobox->setFocus(Qt::ShortcutFocusReason);
            combobox->showPopup();
        });

    connect(m_chart2_channel_list,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [&](int index)
        {
            chart2_selected_channel = index;
            m_chart2->m_channel = index;
            chart_reset(m_chart2);
            save_status();
        });

    //-------------------------------------------
    auto zero_string = "0." + QString(CFG::cfg->decimal_points, QChar('0'));
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
    chart2_info_box_layout->addWidget(m_chart2_channel_list, Qt::AlignLeft);
    chart2_info_box_layout->addLayout(chart2_field_max_layout);
    chart2_info_box_layout->addLayout(chart2_field_min_layout);
    chart2_info_box_layout->addStretch();
}

//---------------------------------------------------------
void DPT::create_chart3_control()
{
    auto *delay_field = create_delay_field();
    m_chart3_control = new QWidget();
    auto *layout = new QVBoxLayout(m_chart3_control);
    layout->addStretch();
    layout->addWidget(delay_field);
    layout->addStretch();
}

//---------------------------------------------------------
void DPT::create_chart4_control()
{
    auto *buttons_make_dump = create_dump_control();
    auto *buttons_plus_minus = create_plus_minus_buttons();
    auto *buttons_ctrl = create_control_buttons();
    auto *button_receive_data = create_button_receive_data();

    auto *buttons1_layout = new QHBoxLayout();
    buttons1_layout->addWidget(buttons_plus_minus, 1, Qt::AlignLeft);
    buttons1_layout->addWidget(buttons_ctrl, 99, Qt::AlignCenter);

    auto *buttons2_layout = new QVBoxLayout();
    buttons2_layout->addWidget(buttons_make_dump);
    buttons2_layout->addLayout(buttons1_layout);
    buttons2_layout->addWidget(button_receive_data);

    m_chart4_control = new QWidget();
    auto *layout = new QHBoxLayout(m_chart4_control);
    layout->addLayout(buttons2_layout, 1);
}

//---------------------------------------------------------
QWidget *DPT::create_button_receive_data()
{
    m_button_receive = new QPushButton(QString("%1\n[%2]").arg(tr("Receive Data"), tr("Space")));
    m_button_receive->setCheckable(true);
    m_button_receive->setChecked(false);
    m_button_receive->setAutoDefault(false);
    m_button_receive->setShortcut(QKeySequence("Space"));
    auto tip = QStringLiteral("%1 %2\n[%3]")
                   .arg(tr("Receive data on port"))
                   .arg(CFG::cfg->atlas_port)
                   .arg(m_button_receive->shortcut().toString());
    m_button_receive->setToolTip(tip);

    connect(m_button_receive, SIGNAL(clicked(bool)), this, SLOT(slot_receive_button(bool)));

    auto *wgt = new QWidget();
    auto *hlay_receive_button = new QHBoxLayout(wgt);
    hlay_receive_button->addStretch();
    hlay_receive_button->addWidget(m_button_receive);
    //hlay_receive_button->addStretch();
    return wgt;
}

//---------------------------------------------------------
QWidget *DPT::create_dump_control()
{
    m_button_make_dump = new QPushButton(tr("Dump [D]"));
    m_button_make_dump->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_button_make_dump->setCheckable(true);
    m_button_make_dump->setChecked(false);
    m_button_make_dump->setAutoDefault(false);
    m_button_make_dump->setShortcut(QKeySequence("D"));
    auto tip = QStringLiteral("%1\n%2\n%3")
                   .arg(tr("Recording data from two sensors to a file (dump)"))
                   .arg(CFG::cfg->dp_dump_dir)
                   .arg(tr("(see Settings)"));
    m_button_make_dump->setToolTip(tip);

    connect(m_button_make_dump,
        &QPushButton::clicked,
        this,
        [this](bool checked)
        {
            m_do_dump = checked;
            MW::mw->statusbar_clear();
            if (checked) {

                if (CFG::cfg->dp_dump_dir.isEmpty()) {
                    m_do_dump = false;
                    m_button_make_dump->setChecked(false);
                    m_button_set_dump_dir->click();
                    return;
                }

                qint64 required = 1024 * 1024 * 1024; // bytes
                auto res = UTILS::storage_space_available(CFG::cfg->dp_dump_dir, required);
                if (!res.isEmpty()) {
                    m_button_make_dump->setChecked(false);
                    m_do_dump = false;
                    WMSG::WMsg(MW::mw, res, QMessageBox::Warning);
                    return;
                }

                if (!m_button_receive->isChecked()) {
                    m_button_make_dump->setChecked(false);
                    m_do_dump = false;
                    auto msg = tr("The data reception is not activated");
                    WMSG::WMsg(MW::mw, msg, QMessageBox::Warning);
                    return;
                }

                bool ok = m_dump->create(CFG::cfg->dp_dump_dir.toStdString());
                if (!ok) {
                    m_button_make_dump->setChecked(false);
                    m_do_dump = false;
                    auto msg = QString("%1\n%2\n\n%3")
                                   .arg(tr("Can't create dump file"), m_dump->filename(), m_dump->strerror());
                    DBGF(1, "{} - {}", m_dump->filename().toStdString(), m_dump->strerror().toStdString());
                    MW::mw->statusbar_show(m_dump->strerror());
                    WMSG::WMsg(MW::mw, msg, QMessageBox::Critical);
                    return;
                }
                auto msg = QString("%1: %2").arg(tr("dump"), m_dump->filename());
                MW::mw->statusbar_show(msg, MW::Bold);

            } else {
                m_dump->close();
            }
        });

    //-------------------------------------------
    m_button_set_dump_dir = new QPushButton("...");
    m_button_set_dump_dir->setCheckable(false);
    m_button_set_dump_dir->setChecked(false);
    m_button_set_dump_dir->setAutoDefault(false);
    m_button_set_dump_dir->setShortcut(QKeySequence("Ctrl+S"));
    tip = QStringLiteral("%1\n%2\n[%3]")
              .arg(tr("Selected Directory"), CFG::cfg->dp_dump_dir, m_button_set_dump_dir->shortcut().toString());
    m_button_set_dump_dir->setToolTip(tip);
    QFont fnt = m_button_set_dump_dir->font();
    fnt.setBold(true);
    m_button_set_dump_dir->setFont(fnt);

    int buttonHeight = m_button_set_dump_dir->sizeHint().height();
    m_button_set_dump_dir->setFixedWidth(buttonHeight);

    connect(m_button_set_dump_dir,
        &QPushButton::clicked,
        this,
        [this]()
        {
            MW::mw->statusbar_clear();
            if (m_button_make_dump->isChecked()) {
                auto msg = tr("Recording dump is in progress\n\n"
                              "Operation is not allowed");
                WMSG::WMsg(MW::mw, msg, QMessageBox::Warning);
                return;
            }

            auto dn = QDir::toNativeSeparators(CFG::cfg->dp_dump_dir);
            QString dir = QFileDialog::getExistingDirectory(MW::mw,
                tr("Select Directory for dump file"),
                dn,
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            if (dir.isEmpty()) {
                return;
            }
            auto tip =
                QStringLiteral("%1\n%2\n[%3]")
                    .arg(tr("Select Directory"), CFG::cfg->dp_dump_dir, m_button_set_dump_dir->shortcut().toString());
            m_button_set_dump_dir->setToolTip(tip);

            CFG::cfg->dp_dump_dir = dir;
            qint64 required = 1024 * 1024 * 1024; // bytes
            auto res = UTILS::storage_space_available(CFG::cfg->dp_dump_dir, required);
            if (!res.isEmpty()) {
                WMSG::WMsg(MW::mw, res, QMessageBox::Warning);
                return;
            }
        });

    //// //-------------------------------------------
    //// //// auto icon = qApp->style()->standardIcon(QStyle::SP_FileIcon);
    //// //// m_button_view_dump = new QPushButton(icon, {});
    //// QIcon icon(":load_raw.jpg");
    //// m_button_view_dump = new QPushButton(icon, {});
    //// m_button_view_dump->setIconSize(QSize(24, 24));
    //// DBGF(3,
    ////     "view dump icon size: {} x {}",
    ////     m_button_view_dump->iconSize().rheight(),
    ////     m_button_view_dump->iconSize().rwidth());

    //// m_button_view_dump->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //// m_button_view_dump->setCheckable(false);
    //// m_button_view_dump->setChecked(false);
    //// m_button_view_dump->setAutoDefault(false);
    //// m_button_view_dump->setShortcut(QKeySequence("Ctrl+L"));
    //// tip = QStringLiteral("%1\n%2\n[%3]")
    ////           .arg(tr("View dump file from"), CFG::cfg->dp_dump_dir, m_button_view_dump->shortcut().toString());
    //// m_button_view_dump->setToolTip(tip);

    //// // buttonHeight = m_button_view_dump->sizeHint().height();
    //// // m_button_view_dump->setFixedWidth(buttonHeight);

    //// connect(m_button_view_dump,
    ////     &QPushButton::clicked,
    ////     this,
    ////     [this]()
    ////     {
    ////         MW::mw->statusbar_clear();

    ////         if (m_button_receive->isChecked()) {
    ////             auto msg = tr("The data reception is enabled\n\n"
    ////                           "Operation is not allowed");
    ////             WMSG::WMsg(MW::mw, msg, QMessageBox::Warning);
    ////             return;
    ////         }

    ////         if (m_button_make_dump->isChecked()) {
    ////             auto msg = tr("Recording dump is in progress\n\n"
    ////                           "Operation is not allowed");
    ////             WMSG::WMsg(MW::mw, msg, QMessageBox::Warning);
    ////             return;
    ////         }

    ////         if (!DPTDUMP_VIEW::gui) {
    ////             auto msg =
    ////                 QString("500: INTERNAL ERROR\n\n%1").arg(tr("It looks like the dump page was not prepared."));
    ////             WMSG::WMsg(MW::mw, msg, QMessageBox::Critical);
    ////             return;
    ////         }

    ////         DPTDUMP_VIEW::gui->load(CFG::cfg->dp_dump_dir);
    ////         // DPDUMP_GUI::gui->select_page();
    ////     });

    //--- блок кнопок записи дампа --------------
    auto *wgt = new QWidget();
    auto *dump_layout = new QHBoxLayout(wgt);
    dump_layout->setContentsMargins(0, 0, 0, 0);
    dump_layout->addWidget(m_button_make_dump, 1);
    dump_layout->addWidget(m_button_set_dump_dir, 0);
    //// dump_layout->addWidget(m_button_view_dump, 0);
    return wgt;
}

//---------------------------------------------------------
QWidget *DPT::create_plus_minus_buttons()
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
                if (!NETWORK::running()) {
                    set_points_sum_channels(m_chart4);
                }
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
                if (!NETWORK::running()) {
                    set_points_sum_channels(m_chart4);
                }
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
QWidget *DPT::create_delay_field()
{
    //--- блок настройки задержки ------
    auto tip = QString("%1\n[%2 - %3]").arg("Compensation delay between the channels").arg(0).arg(DPT::max_delay);
    auto *delay_field_label = new QLabel(tr("Delay"));
    delay_field_label->setToolTip(tip);
    m_delay_field = new QSpinBox();
    m_delay_field->setContentsMargins(0, 0, 0, 0);
    m_delay_field->setSingleStep(2);
    m_delay_field->setRange(0, DPT::max_delay);
    m_delay_field->setValue(0);
    m_delay_field->setSuffix(tr(" ms"));
    m_delay_field->setToolTip(tip);

    connect(m_delay_field,
        qOverload<int>(&QSpinBox::valueChanged),
        [this](int num)
        {
            MW::mw->statusbar_clear();
            DBGF(1, "{} ms", num);
            if ((num % m_delay_field->singleStep()) != 0) {
                num = num & ~1;
                DBGF(1, "rounded to {} ms", num);
                m_delay_field->setValue(num);
                return;
            }
            delay = num;
            if (!NETWORK::running()) {
                set_points_two_channels(m_chart3);
            }

            //QTimer::singleShot(0,
            //    m_delay_field,
            //    [this]()
            //    {
            //        // это дрочиво :(
            //        if (auto *lineEdit = m_delay_field->findChild<QLineEdit *>()) {
            //            lineEdit->deselect();   // уберет выделение
            //            lineEdit->clearFocus(); // вообще убрать все
            //        }
            //    });
        });

    //--- настройки задержки на блоке esz7 ------
    tip = QString("%1\n[%2-%3]").arg(tr("Compensation delay between the channels\nin BBI module")).arg(0).arg(DPT::max_delay);
    auto *delay_field2_label = new QLabel(tr("Set delay in BBI"));
    delay_field2_label->setToolTip(tip);
    m_delay_field2 = new QSpinBox();
    m_delay_field2->setContentsMargins(0, 0, 0, 0);
    m_delay_field2->setSingleStep(2);
    m_delay_field2->setRange(0, DPT::max_delay);
    m_delay_field2->setValue(0);
    m_delay_field2->setToolTip(tip);
    m_delay_field2->setSuffix(tr(" ms"));

    connect(m_delay_field2,
        qOverload<int>(&QSpinBox::valueChanged),
        [this](int num)
        {
            DBGF(1, "{} ms", num);
            if ((num % m_delay_field2->singleStep()) != 0) {
                num = num & ~1;
                DBGF(1, "rounded to {} ms", num);
                m_delay_field2->setValue(num);
                return;
            }
            MW::mw->statusbar_show("Under developing...", 2000);
            // if (ESZ7CLI::running()) {
            //     ESZ7CLI::send_delay_to_esz(num);
            // }

            //QTimer::singleShot(0,
            //    m_delay_field2,
            //    [this]()
            //    {
            //        // это дрочиво :(
            //        if (auto *lineEdit = m_delay_field2->findChild<QLineEdit *>()) {
            //            lineEdit->deselect();   // уберет выделение
            //            lineEdit->clearFocus(); // вообще убрать все
            //        }
            //    });
        });

    //-------------------------------------------
    auto *button_reset_delay = new QPushButton(tr("Reset"));
    button_reset_delay->setCheckable(false);
    button_reset_delay->setChecked(false);
    button_reset_delay->setAutoDefault(false);
    // button_reset_delay->setShortcut(QKeySequence("Alt+C"));
    // auto tip = QStringLiteral("%1\n[%2]").arg(tr("Clear charts"), button_reset_delay->shortcut().toString());
    tip = tr("Reset delay to 0 ms");
    button_reset_delay->setToolTip(tip);

    connect(button_reset_delay,
        &QPushButton::clicked,
        this,
        [&]()
        {
            DBG(1);
            MW::mw->statusbar_clear();
            m_delay_field->setValue(0);
            delay = 0;
            set_points_two_channels(m_chart3);
        });

    //-------------------------------------------
    //// auto *button_auto_align = new QPushButton(tr("Auto Align"));
    //// button_auto_align->setCheckable(false);
    //// button_auto_align->setChecked(false);
    //// button_auto_align->setAutoDefault(false);
    //// //button_auto_align->setShortcut(QKeySequence("Alt+C"));
    //// //auto tip = QStringLiteral("%1\n[%2]").arg(tr("Clear charts"), button_auto_align->shortcut().toString());
    //// tip = tr("Align plots by finding the delay) to achieve perfect overlay");
    //// button_auto_align->setToolTip(tip);

    //// connect(button_auto_align,
    ////     &QPushButton::clicked,
    ////     this,
    ////     [&]()
    ////     {
    ////         DBG(1);
    ////         auto dt = S2Points::points->ch1().buffer.dt;
    ////         auto *pnts1 = S2Points::points->ch1().buffer.R;
    ////         auto *pnts2 = S2Points::points->ch2().buffer.R;
    ////         const size_t sz = pnts1->size();

    ////         float avrg_value;
    ////         float value;
    ////         int num = -1;
    ////         for(int n = 0; n < DPT::max_delay; n += dt ) {
    ////             avrg_value = 0.0f;
    ////             for (size_t i = 0; i < sz; ++i) {
    ////                 avrg_value += pnts1->at(i) - pnts2->at(i);
    ////             }
    ////             avrg_value /= static_cast<float>(sz);
    ////             if (num == -1) {
    ////                 value = avrg_value;
    ////                 num = 0;
    ////             } else if (avrg_value < value) {
    ////                 value = avrg_value;
    ////                 num = n;
    ////             }
    ////             DBGF(1, "avrg_value: {}, delay: {}", avrg_value, delay);
    ////         }
    ////         if (num <= 0) {
    ////             return;
    ////         }
    ////         delay = num;
    ////         //m_button_chart_minus->setChecked(true);

    ////         if (!NETWORK::running()) {
    ////             set_points_two_channels(m_chart3);
    ////             //set_points_sum_channels(m_chart4);
    ////         }

    ////     });

    //--- Раскладка -----------------------------
    auto *delay_field_layout = new QHBoxLayout();
    delay_field_layout->setContentsMargins(0, 0, 0, 0);
    delay_field_layout->addWidget(delay_field_label);
    delay_field_layout->addStretch();
    delay_field_layout->addWidget(m_delay_field);

    auto *delay_field2_layout = new QHBoxLayout();
    delay_field2_layout->setContentsMargins(0, 0, 0, 0);
    delay_field2_layout->addWidget(delay_field2_label);
    delay_field2_layout->addStretch();
    delay_field2_layout->addWidget(m_delay_field2);

    auto *layout = new QVBoxLayout();
    layout->addLayout(delay_field_layout);
    layout->addWidget(button_reset_delay);
    layout->addSpacing(32);
    layout->addLayout(delay_field2_layout);

    m_box_channels_delay = new QWidget();
    m_box_channels_delay->setContentsMargins(0, 0, 0, 0);
    m_box_channels_delay->setLayout(layout);

    // TODO - заблокировано - алгоритм в отладке еще...
    // m_delay_field2->setDisabled(true);
    // delay_field_label->setToolTip("\nNow is lock!!! \n\nUnder developing...\n");
    // m_delay_field2->setToolTip(delay_field_label->toolTip());

    return m_box_channels_delay;
}

//// //-----------------------------------------------
//// void DPT::slot_show_raw(bool checked)
//// {
////     DBGF(3, "{}", (checked ? "on" : "off"));
////     m_raw_button_status = checked;
////     m_chart1->showAsRaw = checked;
////     m_chart2->showAsRaw = checked;
////     chart_reset(m_chart1);
////     chart_reset(m_chart2);
////     chart_reset(m_chart3);
////     chart_reset(m_chart4);
//// }

//-----------------------------------------------
void DPT::slot_set_yscale(bool checked)
{
    DBG(3);
    DBGF(3, "{}", (checked ? "on" : "off"));
    m_yscale_button_status = checked;
    m_chart1->scale_Y = checked;
    m_chart2->scale_Y = checked;
    m_chart3->scale_Y = checked;
    m_chart4->scale_Y = checked;
    chart_reset(m_chart1);
    chart_reset(m_chart2);
    chart_reset(m_chart3);
    chart_reset(m_chart4);
    save_status();
}

//---------------------------------------------------------
void DPT::release_receive_button()
{
    slot_receive_button(false);
}

//---------------------------------------------------------
void DPT::slot_receive_button(bool checked)
{
    static std::chrono::system_clock::time_point last_click_time{};
    auto now = std::chrono::system_clock::now();
    if (now - last_click_time < std::chrono::milliseconds(1000)) {
        m_button_receive->setChecked(!checked);
        last_click_time = now;
        return;
    }
    last_click_time = now;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    DBGF(4, "\nchecked: {}", (checked ? "yes" : "no"));
    // Start
    if (checked) {
        chart_reset(m_chart1);
        chart_reset(m_chart2);
        chart_reset(m_chart3);
        chart_reset(m_chart4);
        m_chart1->enable_legend(true);
        m_chart2->enable_legend(true);
        m_chart3->enable_legend(true);
        m_chart4->enable_legend(true);

               // server start
        bool ok = NETWORK::srv->run();
        if (!ok) {
            MW::mw->statusbar_clear();
            m_button_receive->setChecked(false);
            int ret = MW::mw->server()->stop();
            if (ret == -1) {
                DBG(0, "WARNING! server still online");
            } else {
                DBG(1, "server stopped gracefully");
            }
            QApplication::restoreOverrideCursor();
            auto msg = QString("%1\n").arg(MW::mw->server()->strerror());
            WMSG::WMsg(MW::mw, msg, QMessageBox::Critical);
            return;
        }
        MW::mw->statusbar_show(QString("%1 %2:%3")
                .arg(tr("listening data on port"), MW::mw->server()->host())
                .arg(MW::mw->server()->port()));

        MW::mw->lock_my_tab(m_gui_page);
        MW::mw->data_source_settings_menu()->setEnabled(false);

        QApplication::restoreOverrideCursor();
        return;
    }

           // Stop
    MW::mw->unlock_my_tab(m_gui_page);
    MW::mw->data_source_settings_menu()->setEnabled(true);

    m_button_receive->setChecked(false);
    m_button_make_dump->setChecked(false);
    m_do_dump = false;
    m_dump->close();

    int ret = NETWORK::srv->stop();
    if (ret == -1) {
        DBG(0, "WARNING! server still online");
    } else {
        DBG(1, "server stopped gracefully");
    }
    QApplication::restoreOverrideCursor();
    MW::mw->statusbar_show(tr("Receiving data was stopped"));
}
