/**
\file   ADCTab.cpp
\brief  The tab with ADC numbers and charts
\author Aleksey Lekhtin
\date   06.05.2026
*/

#include "adc.h"

#include "cfg.h"
#include "chart.h"
#include "dbg.h"
#include "mw.h"
#include "network.h"
#include "wmsg.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QHBoxLayout>

//---------------------------------------------------------
QWidget *ADC::gui_page()
{
    return m_gui_page;
}

//---------------------------------------------------------
const QString &ADC::page_name()
{
    return m_page_name;
}

//-----------------------------------------------
void ADC::slot_show_raw(bool checked)
{
    DBGF(3, "{}", (checked ? "on" : "off"));
    m_raw_button_status = checked;
    m_chart1->showAsRaw = checked;
    m_chart2->showAsRaw = checked;
    chart_reset(m_chart1);
    chart_reset(m_chart2);
}

//-----------------------------------------------
void ADC::toggle_yscale_button(bool checked)
{
    DBG(3);
    m_button_yscale->setChecked(checked);
    m_chart1->scale_Y = checked;
    m_chart2->scale_Y = checked;
    chart_reset(m_chart1);
    chart_reset(m_chart2);
    m_yscale_button_status = checked;
    save_status();
}

//-----------------------------------------------
void ADC::toggle_magnify_button(bool checked)
{
    DBG(3);
    m_button_magnify->setChecked(checked);
    m_chart1->magnify_Y = checked;
    m_chart2->magnify_Y = checked;
    chart_reset(m_chart1);
    chart_reset(m_chart2);
    m_magnify_button_status = checked;
    save_status();
}

//-----------------------------------------------
void ADC::slot_set_yscale(bool checked)
{
    DBGF(3, "{}", (checked ? "on" : "off"));
    if (checked && m_button_magnify->isChecked()) {
        toggle_magnify_button(false);
    }
    toggle_yscale_button(checked);
}

//-----------------------------------------------
void ADC::slot_set_magnify(bool checked)
{
    DBGF(3, "{}", (checked ? "on" : "off"));
    if (checked && m_button_yscale->isChecked()) {
        toggle_yscale_button(false);
    }
    toggle_magnify_button(checked);
}

//---------------------------------------------------------
void ADC::release_receive_button()
{
    slot_receive_button(false);
}

//---------------------------------------------------------
void ADC::slot_receive_button(bool checked)
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
        m_chart1->enable_legend(true);
        m_chart2->enable_legend(true);
        m_chart1->update();
        m_chart2->update();

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
    int ret = NETWORK::srv->stop();
    if (ret == -1) {
        DBG(0, "WARNING! server still online");
    } else {
        DBG(1, "server stopped gracefully");
    }
    QApplication::restoreOverrideCursor();
    MW::mw->statusbar_show(tr("Receiving data was stopped"));
}

//---------------------------------------------------------
void ADC::set_fonts() const
{
    DBGF(4, "system font: {}", QGuiApplication::font().pointSize());
    set_textfont();
    set_buttonfont();
    set_charts_font();
    set_monofont();
    m_gui_page->updateGeometry();
}

//---------------------------------------------------------
void ADC::set_buttonfont() const
{
    m_button_receive->setFont(CFG::cfg->button_font);
    m_button_clear_charts->setFont(CFG::cfg->button_font);
    m_button_show_Y_raw->setFont(CFG::cfg->button_font);
    m_button_yscale->setFont(CFG::cfg->button_font);
}

//---------------------------------------------------------
void ADC::set_textfont() const
{
    m_box_set_ch_chart1->setFont(CFG::cfg->text_font);
    for (auto widget : m_box_set_ch_chart1->findChildren<QWidget *>()) {
        widget->setFont(CFG::cfg->text_font);
    }
    m_box_set_ch_chart2->setFont(CFG::cfg->text_font);
    for (auto widget : m_box_set_ch_chart2->findChildren<QWidget *>()) {
        widget->setFont(CFG::cfg->text_font);
    }
}

//---------------------------------------------------------
void ADC::set_charts_font() const
{
    DBG(1);
    m_chart1->chart()->setTitleFont(CFG::cfg->chart_font);
    m_chart2->chart()->setTitleFont(CFG::cfg->chart_font);

    m_chart1->axisY()->setLabelsFont(CFG::cfg->chart_font);
    m_chart2->axisY()->setLabelsFont(CFG::cfg->chart_font);

    m_chart1->axisX()->setLabelsFont(CFG::cfg->chart_font);
    m_chart2->axisX()->setLabelsFont(CFG::cfg->chart_font);

    m_chart1->axisY()->setTitleFont(CFG::cfg->chart_font);
    m_chart2->axisY()->setTitleFont(CFG::cfg->chart_font);

    m_chart1->axisX()->setTitleFont(CFG::cfg->chart_font);
    m_chart2->axisX()->setTitleFont(CFG::cfg->chart_font);

    //// m_chart1->update();
    //// m_chart2->update();
    //// QApplication::processEvents();
}

//---------------------------------------------------------
void ADC::set_monofont() const
{
    auto pointsize = CFG::cfg->text_font.pointSize();
    QFont fnt = CFG::cfg->mono_font;
    fnt.setPointSize(pointsize);
    m_chart1_max_min_label[0]->setFont(fnt);
    m_chart1_max_min_label[1]->setFont(fnt);
    m_chart1_label_avrg->setFont(fnt);

    m_chart2_max_min_label[0]->setFont(fnt);
    m_chart2_max_min_label[1]->setFont(fnt);
    m_chart2_label_avrg->setFont(fnt);

    fnt.setBold(true);
    m_chart1_max_min_values[0]->setFont(fnt);
    m_chart1_max_min_values[1]->setFont(fnt);
    m_chart1_avrg_value->setFont(fnt);

    m_chart2_max_min_values[0]->setFont(fnt);
    m_chart2_max_min_values[1]->setFont(fnt);
    m_chart2_avrg_value->setFont(fnt);


    QFontMetrics fm(fnt);
    int textWidth = fm.horizontalAdvance(m_chart1_max_min_values[0]->text()) + 15;
    m_chart1_max_min_values[0]->setMinimumWidth(textWidth);
    m_chart1_max_min_values[1]->setMinimumWidth(textWidth);
    m_chart1_avrg_value->setMinimumWidth(textWidth);

    m_chart1_max_min_values[0]->updateGeometry();
    m_chart1_max_min_values[1]->updateGeometry();
    m_chart1_avrg_value->updateGeometry();


    m_chart2_max_min_values[0]->setMinimumWidth(textWidth);
    m_chart2_max_min_values[1]->setMinimumWidth(textWidth);
    m_chart2_avrg_value->setMinimumWidth(textWidth);

    m_chart2_max_min_values[0]->updateGeometry();
    m_chart2_max_min_values[1]->updateGeometry();
    m_chart2_avrg_value->updateGeometry();
}

//---------------------------------------------------------
QWidget *ADC::create_control_buttons()
{
    QString tip;
    m_button_receive = new QPushButton(QString("%1\n[%2]").arg(tr("Receive Data"), tr("Space")));
    m_button_receive->setCheckable(true);
    m_button_receive->setChecked(false);
    m_button_receive->setAutoDefault(false);
    m_button_receive->setShortcut(QKeySequence("Space"));
    tip = QStringLiteral("%1 %2\n[%3]")
              .arg(tr("Receive data on port"))
              .arg(CFG::cfg->atlas_port)
              .arg(m_button_receive->shortcut().toString());
    m_button_receive->setToolTip(tip);
    connect(m_button_receive, SIGNAL(clicked(bool)), this, SLOT(slot_receive_button(bool)));

    //---------------------------------
    m_button_clear_charts = new QPushButton(tr("Clear [C]"));
    m_button_clear_charts->setCheckable(false);
    m_button_clear_charts->setChecked(false);
    m_button_clear_charts->setAutoDefault(false);
    m_button_clear_charts->setShortcut(QKeySequence("C"));
    // tip = QStringLiteral("%1\n[%2]").arg(tr("Clear charts"), m_button_clear_charts->shortcut().toString());
    tip = tr("Clear charts");
    m_button_clear_charts->setToolTip(tip);

    connect(m_button_clear_charts,
        &QPushButton::clicked,
        this,
        [&]()
        {
            DBG(3);
            chart_reset(m_chart1);
            chart_reset(m_chart2);
            //m_chart1->set_legend_series_name(make_title(m_chart1));
            //m_chart2->set_legend_series_name(make_title(m_chart2));
            auto zero_string = QString("0.%1 %2").arg(QString(CFG::cfg->decimal_points, QChar('0')), tr("V"));
            if (m_chart1->showAsRaw) {
                zero_string = QString("00000000");
            }
            m_chart1_max_min_values[0]->setText(zero_string);
            m_chart1_max_min_values[1]->setText(zero_string);
            m_chart1_avrg_value->setText(zero_string);

            m_chart2_max_min_values[0]->setText(zero_string);
            m_chart2_max_min_values[1]->setText(zero_string);
            m_chart2_avrg_value->setText(zero_string);
        });

    //---------------------------------
    m_button_show_Y_raw = new QPushButton(tr("Raw [R]"));
    m_button_show_Y_raw->setCheckable(true);
    m_button_show_Y_raw->setChecked(m_raw_button_status);
    m_button_show_Y_raw->setShortcut(QKeySequence("R"));
    tip = QStringLiteral("%1\n[%2]")
              .arg(tr("Display Raw data that is scaling to real voltage"), m_button_show_Y_raw->shortcut().toString());
    m_button_show_Y_raw->setToolTip(tip);
    connect(m_button_show_Y_raw, &QPushButton::clicked, this, &ADC::slot_show_raw);

    //---------------------------------
    m_button_yscale = new QPushButton(tr("Stretch [Y]"));
    m_button_yscale->setCheckable(true);
    m_button_yscale->setChecked(m_yscale_button_status);
    m_button_yscale->setShortcut(QKeySequence("Y"));
    tip = tr("Scale data on Y axis\n(Zoom In)");
    m_button_yscale->setToolTip(tip);
    connect(m_button_yscale, &QPushButton::clicked, this, &ADC::slot_set_yscale);

    //---------------------------------
    m_button_magnify = new QPushButton(tr("Magnify [M]"));
    m_button_magnify->setCheckable(true);
    m_button_magnify->setChecked(m_magnify_button_status);
    m_button_magnify->setShortcut(QKeySequence("M"));
    tip = tr("Adjust the Y-axis scale to fit the data range perfectly, \n"
             "eliminating empty space at the top and bottom");
    m_button_magnify->setToolTip(tip);
    connect(m_button_magnify, &QPushButton::clicked, this, &ADC::slot_set_magnify);

    //---------------------------------
    auto *layout_left_buttons = new QHBoxLayout();
    layout_left_buttons->addStretch();
    layout_left_buttons->addWidget(m_button_receive);
    layout_left_buttons->addStretch();

    auto *layout_right_buttons = new QHBoxLayout();
    //layout_right_buttons->setContentsMargins(0, 0, 0, 0);
    layout_right_buttons->addSpacing(16);
    layout_right_buttons->addStretch();
    layout_right_buttons->addWidget(m_button_yscale);
    layout_right_buttons->addSpacing(16);
    layout_right_buttons->addWidget(m_button_magnify);
    layout_right_buttons->addSpacing(16);
    layout_right_buttons->addWidget(m_button_show_Y_raw);
    layout_right_buttons->addSpacing(16);
    layout_right_buttons->addWidget(m_button_clear_charts);

    auto *wgt = new QWidget();
    auto *layout = new QHBoxLayout(wgt);
    layout->addLayout(layout_left_buttons, 1);
    layout->addLayout(layout_right_buttons, 99);
    return wgt;
}

//---------------------------------------------------------
QWidget *ADC::create_page()
{
    m_chart1_channel_list = new QComboBox();
    m_chart1_channel_list->setDuplicatesEnabled(false);
    m_chart1_channel_list->setMinimumContentsLength(12);
    m_chart1_channel_list->addItem(tr("Nothing"));
    m_chart1_channel_list->addItem(tr("Channel 1"));
    m_chart1_channel_list->addItem(tr("Channel 2"));
    m_chart1_channel_list->addItem(tr("Channel 3"));
    m_chart1_channel_list->addItem(tr("Channel 4"));
    m_chart1_channel_list->setMaxVisibleItems(5);
    m_chart1_channel_list->setCurrentIndex(m_chart1_selected_channel);
    QString tip = tr("Selects the channel from which data will be displayed on chart #1");
    m_chart1_channel_list->setToolTip(tip);

    connect(m_chart1_channel_list,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [&](int index)
        {
            m_chart1_selected_channel = index;
            m_chart1->m_channel = index;
            chart_reset(m_chart1);
            save_status();
        });

    //-------------------------------------------
    m_chart2_channel_list = new QComboBox();
    m_chart2_channel_list->setDuplicatesEnabled(false);
    m_chart2_channel_list->setMinimumContentsLength(12);
    m_chart2_channel_list->addItem(tr("Nothing"));
    m_chart2_channel_list->addItem(tr("Channel 1"));
    m_chart2_channel_list->addItem(tr("Channel 2"));
    m_chart2_channel_list->addItem(tr("Channel 3"));
    m_chart2_channel_list->addItem(tr("Channel 4"));
    m_chart2_channel_list->addItem(tr("Accumulation"));
    m_chart2_channel_list->setMaxVisibleItems(6);
    m_chart2_channel_list->setCurrentIndex(m_chart2_selected_channel);
    tip = tr("Selects the channel from which data will be displayed on chart #2");
    m_chart2_channel_list->setToolTip(tip);

    connect(m_chart2_channel_list,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [&](int index)
        {
            m_chart2_selected_channel = index;
            m_chart2->m_channel = index;
            chart_reset(m_chart2);
            save_status();
        });

    //-------------------------------------------
    auto zero_string = QString("0.%1 %2").arg(QString(CFG::cfg->decimal_points, QChar('0')), tr("V"));

    //-------------------------------------------
    m_box_set_ch_chart1 = new QGroupBox(tr("Chart 1"));

    m_chart1_max_min_label[0] = new QLabel("Min");
    m_chart1_max_min_label[1] = new QLabel("Max");
    m_chart1_label_avrg = new QLabel("Avg");
    m_chart1_avrg_value = new QLineEdit(zero_string);
    m_chart1_max_min_values[0] = new QLineEdit(zero_string);
    m_chart1_max_min_values[1] = new QLineEdit(zero_string);
    m_chart1_max_min_values[0]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_chart1_max_min_values[1]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto *chart1_field_min_layout = new QHBoxLayout();
    chart1_field_min_layout->addWidget(m_chart1_max_min_label[0]);
    chart1_field_min_layout->addWidget(m_chart1_max_min_values[0]);
    chart1_field_min_layout->addStretch();

    auto *chart1_field_max_layout = new QHBoxLayout();
    chart1_field_max_layout->addWidget(m_chart1_max_min_label[1]);
    chart1_field_max_layout->addWidget(m_chart1_max_min_values[1]);
    chart1_field_max_layout->addStretch();

    auto *chart1_field_avrg_layout = new QHBoxLayout();
    chart1_field_avrg_layout->addWidget(m_chart1_label_avrg);
    chart1_field_avrg_layout->addWidget(m_chart1_avrg_value);
    chart1_field_avrg_layout->addStretch();

    auto *chart1_info_box_layout = new QVBoxLayout(m_box_set_ch_chart1);
    chart1_info_box_layout->addWidget(m_chart1_channel_list, Qt::AlignLeft);
    chart1_info_box_layout->addLayout(chart1_field_max_layout);
    chart1_info_box_layout->addLayout(chart1_field_min_layout);
    chart1_info_box_layout->addLayout(chart1_field_avrg_layout);
    chart1_info_box_layout->addStretch();

    //-------------------------------------------
    m_box_set_ch_chart2 = new QGroupBox(tr("Chart 2"));

    m_chart2_max_min_label[0] = new QLabel("Min");
    m_chart2_max_min_label[1] = new QLabel("Max");
    m_chart2_label_avrg = new QLabel("Avg");
    m_chart2_avrg_value = new QLineEdit(zero_string);
    m_chart2_max_min_values[0] = new QLineEdit(zero_string);
    m_chart2_max_min_values[1] = new QLineEdit(zero_string);
    m_chart2_max_min_values[0]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_chart2_max_min_values[1]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto *chart2_field_min_layout = new QHBoxLayout();
    chart2_field_min_layout->addWidget(m_chart2_max_min_label[0]);
    chart2_field_min_layout->addWidget(m_chart2_max_min_values[0]);
    chart2_field_min_layout->addStretch();

    auto *chart2_field_max_layout = new QHBoxLayout();
    chart2_field_max_layout->addWidget(m_chart2_max_min_label[1]);
    chart2_field_max_layout->addWidget(m_chart2_max_min_values[1]);
    chart2_field_max_layout->addStretch();

    auto *chart2_field_avrg_layout = new QHBoxLayout();
    chart2_field_avrg_layout->addWidget(m_chart2_label_avrg);
    chart2_field_avrg_layout->addWidget(m_chart2_avrg_value);
    chart2_field_avrg_layout->addStretch();

    auto *chart2_info_box_layout = new QVBoxLayout(m_box_set_ch_chart2);
    //chart2_info_box_layout->setContentsMargins(2, 6, 2, 6);
    chart2_info_box_layout->addWidget(m_chart2_channel_list);
    chart2_info_box_layout->addLayout(chart2_field_max_layout);
    chart2_info_box_layout->addLayout(chart2_field_min_layout);
    chart2_info_box_layout->addLayout(chart2_field_avrg_layout);
    chart2_info_box_layout->addStretch();

    //-------------------------------------------
    m_encoder[0] = new QLineEdit("0");
    m_encoder[1] = new QLineEdit("0");
    m_encoder[0]->setAlignment(Qt::AlignRight);
    m_encoder[1]->setAlignment(Qt::AlignRight);

    auto *encoder1_layout = new QHBoxLayout();
    encoder1_layout->addWidget(new QLabel("#1"));
    encoder1_layout->addWidget(m_encoder[0]);
    encoder1_layout->addStretch();

    auto *encoder2_layout = new QHBoxLayout();
    encoder2_layout->addWidget(new QLabel("#2"));
    encoder2_layout->addWidget(m_encoder[1]);
    encoder2_layout->addStretch();

    m_box_encoders = new QGroupBox(tr("Encoders"));
    auto *encoders_box_layout = new QVBoxLayout(m_box_encoders);
    //encoders_box_layout->setContentsMargins(2, 6, 2, 6);
    encoders_box_layout->addLayout(encoder1_layout);
    encoders_box_layout->addLayout(encoder2_layout);
    encoders_box_layout->addStretch();

    //-------------------------------------------
    // блок кнопок управления
    auto *chart_buttons = create_control_buttons();

    //-------------------------------------------
    auto *layout_chart1_select_ch = new QVBoxLayout();
    layout_chart1_select_ch->addWidget(m_box_set_ch_chart1);

    auto *layout_chart2_select_ch = new QVBoxLayout();
    layout_chart2_select_ch->addWidget(m_box_set_ch_chart2);
    layout_chart2_select_ch->addStretch();
    layout_chart2_select_ch->addWidget(m_box_encoders);

    //-------------------------------------------
    // финальная раскладка виджетов
    auto *layout_chart1 = new QHBoxLayout();
    layout_chart1->addLayout(layout_chart1_select_ch, 1);
    layout_chart1->addWidget(m_chart1->gui_page(), 99);

    auto *layout_chart2 = new QHBoxLayout();
    layout_chart2->addLayout(layout_chart2_select_ch, 1);
    layout_chart2->addWidget(m_chart2->gui_page(), 99);

    auto *wgt = new QWidget(this);
    auto *layout = new QVBoxLayout(wgt);
    layout->addLayout(layout_chart1);
    layout->addLayout(layout_chart2);
    layout->addWidget(chart_buttons);
    return wgt;
}
