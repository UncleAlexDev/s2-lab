/**
  \file   settings.cpp
  \brief  Настройки
  \author Aleksey Lekhtin
  \date   10.12.2024
*/

#include "adc.h"
#include "mw.h"

#include "cfg.h"
#include "dbg.h"
#include "dpt.h"
#include "lineedit.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>

//---------------------------------------------------------
void MW::slot_settings_data_source()
{
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Settings"));
    dlg->setSizeGripEnabled(true);
    dlg->setFont(CFG::cfg->menu_font);

    auto buttons_group = std::make_unique<QDialogButtonBox>(Qt::Horizontal);
    buttons_group->setStandardButtons(QDialogButtonBox::Apply | QDialogButtonBox::Cancel);

    QPushButton *cancel_btn = buttons_group->button(QDialogButtonBox::Cancel);
    connect(cancel_btn, SIGNAL(clicked()), dlg, SLOT(reject()));
    cancel_btn->setDefault(false);
    cancel_btn->setAutoDefault(false);

    QPushButton *apply_btn = buttons_group->button(QDialogButtonBox::Apply);
    connect(apply_btn, SIGNAL(clicked()), dlg, SLOT(accept()));
    apply_btn->setDefault(true);
    apply_btn->setAutoDefault(true);

    QGroupBox *box_incoming_data = create_incoming_data_box();

    //-------------------------------------------
    QGroupBox *box_adc_setts = create_adc_settings_box();

    //-------------------------------------------
    // auto *layout = new QVBoxLayout(dlg);
    auto *layout = new QVBoxLayout();
    layout->addWidget(box_incoming_data);
    layout->addWidget(box_adc_setts);
    layout->addWidget(buttons_group.release());
    dlg->setLayout(layout);

    apply_btn->setFocus();

    //-------------------------------------------
    dlg->adjustSize();
    dlg->setMinimumWidth(600);
    // dlg->restoreGeometry(CFG::cfg->dialog_settings1_state);

    int rc = dlg->exec();
    // CFG::cfg->dialog_settings1_state = dlg->saveGeometry();
    CFG::cfg->save_status();
    if (rc == QDialog::Accepted) {
        CFG::cfg->atlas_host = m_field_atlas_host->text();
        CFG::cfg->atlas_port = m_field_atlas_port->value();
        //// m_network->set_atlas_host(CFG::cfg->atlas_host);
        //// m_network->set_atlas_port(CFG::cfg->atlas_port);

        CFG::cfg->ads_host = m_field_ads_host->text();
        CFG::cfg->ads_data_port = m_field_ads_port->value();

        CFG::cfg->sureshot_host = m_field_sureshot_host->text();
        CFG::cfg->sureshot_data_port = m_field_sureshot_port->value();

        CFG::cfg->axisY_range = m_field_axisY->value();

        CFG::cfg->save_cfg();

        if (DPT::dpt && DPT::dpt->gui_page()) {
            if (CFG::cfg->data_format != CFG::DATA_FORMAT::ATLAS) {
                auto index = m_tabwgt->indexOf(DPT::dpt->gui_page());
                if (index != -1) {
                    m_tabwgt->removeTab(index);
                }
            } else {
                auto index = m_tabwgt->indexOf(DPT::dpt->gui_page());
                if (index == -1) {
                    m_tabwgt->insertTab(index, DPT::dpt->gui_page(), DPT::dpt->page_name());
                }
            }
        }
    }
    delete dlg;
}

//---------------------------------------------------------
void MW::slot_settings_miscellaneous()
{
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Miscellaneous Settings"));
    dlg->setSizeGripEnabled(true);
    dlg->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    dlg->setFont(CFG::cfg->menu_font);

    auto buttons_group = std::make_unique<QDialogButtonBox>(Qt::Horizontal);
    buttons_group->setStandardButtons(QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    QPushButton *apply_btn = buttons_group->button(QDialogButtonBox::Apply);
    connect(apply_btn, SIGNAL(clicked()), dlg, SLOT(accept()));
    apply_btn->setDefault(true);
    apply_btn->setAutoDefault(true);

    QPushButton *cancle_btn = buttons_group->button(QDialogButtonBox::Cancel);
    connect(cancle_btn, SIGNAL(clicked()), dlg, SLOT(reject()));
    cancle_btn->setDefault(false);
    cancle_btn->setAutoDefault(false);

    //-------------------------------------------
    QGroupBox *box_misc_setts = create_misc_settings_box();

    //-------------------------------------------
    // auto *layout = new QVBoxLayout(dlg);
    auto layout = std::make_unique<QVBoxLayout>();
    layout->addWidget(box_misc_setts);
    layout->addWidget(buttons_group.release());
    dlg->setLayout(layout.release());

    //-------------------------------------------
    auto dbg_level = DBG::debug_level;

    //-------------------------------------------
    QFontMetrics fm(m_field_raw_dp_file->font());
    int w = fm.horizontalAdvance(m_field_raw_dp_file->text());
    m_field_raw_dp_file->setMinimumWidth(w + 20);
    if (m_field_raw_dp_file->layout()) {
        m_field_raw_dp_file->layout()->activate();
    }
    m_field_raw_dp_file->adjustSize();
    if (m_field_raw_dp_file->size().rwidth() > 1000) {
        m_field_raw_dp_file->setMinimumWidth(850);
        if (m_field_raw_dp_file->layout()) {
            m_field_raw_dp_file->layout()->activate();
        }
        m_field_raw_dp_file->adjustSize();
    }

    apply_btn->setFocus();

    //-------------------------------------------
    dlg->adjustSize();
    dlg->setMinimumWidth(600);

    int rc = dlg->exec();
    CFG::cfg->save_status();
    if (rc == QDialog::Accepted) {

        dbg_level = m_field_debug->value();
        CFG::cfg->dp_dump_dir = QDir::fromNativeSeparators(m_field_raw_dp_file->text());

        auto str = m_field_deciamal_points->currentText();
        auto str_list = str.split(' ');
        if (!str_list.empty()) {
            CFG::cfg->decimal_points = str_list.at(0).toInt();
        }
        auto format = "%" + QString("0.%1f").arg(CFG::cfg->decimal_points);
        MW::adc_page()->set_axisY_format(format);
        if (MW::dpt_page()) {
            MW::dpt_page()->set_axisY_format(format);
        }

        if (DPT::dpt) {
            auto tip = QStringLiteral("%1\n%2\n[%3]")
                           .arg(tr("Selected Directory"),
                               CFG::cfg->dp_dump_dir,
                               DPT::dpt->button_browse_dir2()->shortcut().toString());
            DPT::dpt->button_browse_dir2()->setToolTip(tip);
        }

        CFG::cfg->save_cfg();
    }
    DBG::debug_level = dbg_level;
    delete dlg;
}

//---------------------------------------------------------
QGroupBox *MW::create_incoming_data_box()
{
    m_field_atlas_host = new LineEdit();
    // m_field_atlas_host->setReadOnly(true);
    // m_field_atlas_host->setDisabled(true);
    m_field_atlas_host->setText(CFG::cfg->atlas_host);
    m_field_atlas_host->setToolTip(tr("Attlass address"));

    int port_min = 6000, port_max = 20000;
    auto tip = QString("%1 [%2...%3]").arg(tr("Listening UDP port")).arg(port_min).arg(port_max);
    m_field_atlas_port = new QSpinBox();
    m_field_atlas_port->setSingleStep(1);
    m_field_atlas_port->setRange(port_min, port_max);
    m_field_atlas_port->setValue(CFG::cfg->atlas_port);
    m_field_atlas_port->setToolTip(tip);

    int row = 0;
    auto *grid_atlas_setts = new QGridLayout();
    grid_atlas_setts->setSizeConstraint(QLayout::SetMinAndMaxSize);
    grid_atlas_setts->addWidget(new QLabel(tr("Host")), row, 0);
    grid_atlas_setts->addWidget(m_field_atlas_host, row, 1);
    ++row;
    grid_atlas_setts->addWidget(new QLabel(tr("UDP port")), row, 0);
    grid_atlas_setts->addWidget(m_field_atlas_port, row, 1);

    auto *box_atlas_setts = new QGroupBox();
    auto *vlay_atlas = new QVBoxLayout(box_atlas_setts);
    vlay_atlas->addLayout(grid_atlas_setts);
    vlay_atlas->addStretch();

    //-------------------------------------------
    m_field_ads_host = new LineEdit();
    m_field_ads_host->setText(CFG::cfg->ads_host);
    m_field_ads_host->setToolTip(tr("Advanced Decoder System address"));

    port_min = 6000, port_max = 10000;
    tip = QString("%1 [%2...%3]").arg(tr("Listening TCP port")).arg(port_min).arg(port_max);
    m_field_ads_port = new QSpinBox();
    m_field_ads_port->setSingleStep(1);
    m_field_ads_port->setRange(port_min, port_max);
    m_field_ads_port->setValue(CFG::cfg->ads_data_port);
    m_field_ads_port->setToolTip(tip);

    row = 0;
    auto *grid_ads_setts = new QGridLayout();
    grid_ads_setts->setSizeConstraint(QLayout::SetMinAndMaxSize);
    grid_ads_setts->addWidget(new QLabel(tr("Host")), row, 0);
    grid_ads_setts->addWidget(m_field_ads_host, row, 1);
    ++row;
    grid_ads_setts->addWidget(new QLabel(tr("Data port")), row, 0);
    grid_ads_setts->addWidget(m_field_ads_port, row, 1);

    auto *box_ads_setts = new QGroupBox();
    auto *vlay_ads = new QVBoxLayout(box_ads_setts);
    vlay_ads->addLayout(grid_ads_setts);
    vlay_ads->addStretch();

    //-------------------------------------------
    m_field_sureshot_host = new LineEdit();
    m_field_sureshot_host->setText(CFG::cfg->sureshot_host);
    m_field_sureshot_host->setToolTip(tr("SureShot address"));

    port_min = 6000, port_max = 10000;
    tip = QString("%1 [%2...%3]").arg(tr("Listening TCP port")).arg(port_min).arg(port_max);
    m_field_sureshot_port = new QSpinBox();
    m_field_sureshot_port->setSingleStep(1);
    m_field_sureshot_port->setRange(port_min, port_max);
    m_field_sureshot_port->setValue(CFG::cfg->sureshot_data_port);
    m_field_sureshot_port->setToolTip(tip);

    row = 0;
    auto *grid_sureshot_setts = new QGridLayout();
    grid_sureshot_setts->setSizeConstraint(QLayout::SetMinAndMaxSize);
    grid_sureshot_setts->addWidget(new QLabel(tr("Host")), row, 0);
    grid_sureshot_setts->addWidget(m_field_sureshot_host, row, 1);
    ++row;
    grid_sureshot_setts->addWidget(new QLabel(tr("Data port")), row, 0);
    grid_sureshot_setts->addWidget(m_field_sureshot_port, row, 1);

    auto *box_sureshot_setts = new QGroupBox();
    auto *vlay_ss = new QVBoxLayout(box_sureshot_setts);
    vlay_ss->addLayout(grid_sureshot_setts);
    vlay_ss->addStretch();

    //-------------------------------------------
    auto *radio_btn_atlas = new QRadioButton(tr("&Atlas"));
    auto *radio_btn_ads = new QRadioButton(tr("A&DS (Advanced Decoder System)"));
    auto *radio_btn_sureshot = new QRadioButton(tr("&SureShot (Surface Systems && Components)"));

    if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {
        radio_btn_atlas->setChecked(true);
        box_atlas_setts->show();
        box_ads_setts->hide();
        box_sureshot_setts->hide();
    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::ADS) {
        radio_btn_ads->setChecked(true);
        box_atlas_setts->hide();
        box_ads_setts->show();
        box_sureshot_setts->hide();
    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::SURESHOT) {
        radio_btn_sureshot->setChecked(true);
        box_atlas_setts->hide();
        box_ads_setts->hide();
        box_sureshot_setts->show();
    }

    connect(radio_btn_atlas,
        &QRadioButton::toggled,
        this,
        [=](bool checked)
        {
            if (checked) {
                box_atlas_setts->show();
                box_ads_setts->hide();
                box_sureshot_setts->hide();
                m_field_scale->setText(QString::number(CFG::cfg->atlas_scale_factor, 'f', 20) + (tr(" V")));
                m_field_input_range->setText("0 - 7FFFFF");
                CFG::cfg->cutoff_filter = false;
                CFG::cfg->asPulse = false;
                CFG::cfg->data_format = CFG::DATA_FORMAT::ATLAS;
            }
        });

    connect(radio_btn_ads,
        &QRadioButton::toggled,
        this,
        [=](bool checked)
        {
            if (checked) {
                box_atlas_setts->hide();
                box_ads_setts->show();
                box_sureshot_setts->hide();
                m_field_scale->setText(QString::number(CFG::cfg->ads_scale_factor, 'f', 20) + (tr(" V")));
                m_field_input_range->setText("0 - 7FFFFF");
                CFG::cfg->cutoff_filter = false;
                CFG::cfg->asPulse = false;
                CFG::cfg->data_format = CFG::DATA_FORMAT::ADS;
            }
        });

    connect(radio_btn_sureshot,
        &QRadioButton::toggled,
        this,
        [=](bool checked)
        {
            if (checked) {
                box_atlas_setts->hide();
                box_ads_setts->hide();
                box_sureshot_setts->show();
                m_field_scale->setText(QString::number(CFG::cfg->ss_scale_factor, 'f', 20) + (tr(" V")));
                m_field_input_range->setText("0 - 7FFFFF");
                CFG::cfg->cutoff_filter = false;
                CFG::cfg->asPulse = false;
                CFG::cfg->data_format = CFG::DATA_FORMAT::SURESHOT;
            }
        });

    //-------------------------------------------
    auto *box = new QGroupBox(tr("Incoming Data Source"));
    auto *vbox = new QVBoxLayout(box);
    vbox->addWidget(radio_btn_atlas);
    vbox->addWidget(radio_btn_ads);
    vbox->addWidget(radio_btn_sureshot);
    vbox->addWidget(box_atlas_setts);
    vbox->addWidget(box_ads_setts);
    vbox->addWidget(box_sureshot_setts);
    vbox->addStretch();
    return box;
}

//---------------------------------------------------------
QGroupBox *MW::create_adc_settings_box()
{
    int ln = 20;
    QString label_4_20Current = tr("Input signal type");
    m_field_4_20Current = new QLineEdit();
    m_field_4_20Current->setText("4-20ma");
    m_field_4_20Current->setToolTip(tr("The input uses a 4-20 mA current signal") + QString("\n[read-only]"));
    m_field_4_20Current->setReadOnly(true);

    QString label_ref_voltage = tr("Voltage reference");
    m_field_ref_voltage = new QLineEdit();
    m_field_ref_voltage->setText(QString::number(CFG::cfg->adc_ref_voltage, 'f', 2) + (tr(" V")));
    m_field_ref_voltage->setToolTip(tr("Reference voltage for current 4-20ma.") + QString("\n[read-only]"));
    m_field_ref_voltage->setReadOnly(true);

    QString label_input_range = tr("Input Range");
    m_field_input_range = new QLineEdit();
    m_field_input_range->setToolTip(tr("Reference voltage for current 4-20ma.") + QString("\n[read-only]"));
    m_field_input_range->setReadOnly(true);
    if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {
        m_field_input_range->setText("0 - 7FFFFF");
    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::ADS || CFG::cfg->data_format == CFG::DATA_FORMAT::SURESHOT) {
        m_field_input_range->setText("0 - 7FFFFF");
    }

    QString label_scale = tr("Scaling factor");
    m_field_scale = new QLineEdit();
    m_field_scale->setText(QString::number(0, 'f', ln) + (tr(" V")));
    m_field_scale->setToolTip(tr("Scale factor for incoming signals") + QString("\n[read-only]"));
    m_field_scale->setReadOnly(true);

    if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {
        m_field_scale->setText(QString::number(CFG::cfg->atlas_scale_factor, 'f', ln) + (tr(" V")));
    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::ADS) {
        m_field_scale->setText(QString::number(CFG::cfg->ads_scale_factor, 'f', ln) + (tr(" V")));
    } else if (CFG::cfg->data_format == CFG::DATA_FORMAT::SURESHOT) {
        m_field_scale->setText(QString::number(CFG::cfg->ss_scale_factor, 'f', ln) + (tr(" V")));
    }

    //-------------------------------------------
    QString label_axisY_range = tr("Adjust Y axis");
    double range = CFG::cfg->axisY_range;
    m_field_axisY = new QDoubleSpinBox();
    m_field_axisY->setRange(0, range * 2);
    m_field_axisY->setValue(range);
    m_field_axisY->setSingleStep(0.5);

    auto *adjY_def_button = new QPushButton(tr("Default"));
    connect(adjY_def_button,
        &QPushButton::clicked,
        this,
        [field = m_field_axisY]()
        {
            field->setValue(CFG::cfg->adc_ref_voltage);
            field->setRange(0, field->value() * 2);
        });
    auto *layout_adjY = new QHBoxLayout();
    layout_adjY->addWidget(m_field_axisY, 100);
    layout_adjY->addWidget(adjY_def_button, 0);

    //-------------------------------------------
    int row = 0;
    auto *grid_adc_setts = new QGridLayout();
    grid_adc_setts->addWidget(new QLabel(label_4_20Current), row, 0);
    grid_adc_setts->addWidget(m_field_4_20Current, row, 1);
    ++row;
    grid_adc_setts->addWidget(new QLabel(label_ref_voltage), row, 0);
    grid_adc_setts->addWidget(m_field_ref_voltage, row, 1);
    ++row;
    grid_adc_setts->addWidget(new QLabel(label_input_range), row, 0);
    grid_adc_setts->addWidget(m_field_input_range, row, 1);
    ++row;
    grid_adc_setts->addWidget(new QLabel(label_scale), row, 0);
    grid_adc_setts->addWidget(m_field_scale, row, 1);
    ++row;
    grid_adc_setts->addWidget(new QLabel(label_axisY_range), row, 0);
    grid_adc_setts->addLayout(layout_adjY, row, 1, 1, 2);
    ++row;

    auto *adc_settings_box = new QGroupBox(tr("ADC"));
    auto *vbox = new QVBoxLayout(adc_settings_box);
    vbox->addLayout(grid_adc_setts);
    vbox->addStretch();
    return adc_settings_box;
}

//---------------------------------------------------------
QGroupBox *MW::create_misc_settings_box()
{
    QString label_debug = tr("Debug level");
    m_field_debug = new QSpinBox();
    m_field_debug->setPrefix(" ");
    m_field_debug->setSuffix(" ");
    m_field_debug->setSingleStep(1);
    m_field_debug->setRange(0, 9);
    m_field_debug->setValue(DBG::debug_level);
    connect(m_field_debug,
        &QSpinBox::valueChanged,
        [](int num)
        {
            DBG::debug_level = num;
        });

    QString label_decimal = tr("Number of decimal points");
    m_field_deciamal_points = new QComboBox();
    m_field_deciamal_points->setDuplicatesEnabled(false);
    m_field_deciamal_points->setMinimumContentsLength(3);
    m_field_deciamal_points->addItem(tr("3 points"));
    m_field_deciamal_points->addItem(tr("6 points"));
    m_field_deciamal_points->addItem(tr("8 points"));
    m_field_deciamal_points->setMaxVisibleItems(3);
    int index = m_field_deciamal_points->findText(QString::number(CFG::cfg->decimal_points), Qt::MatchStartsWith);
    if (index == -1) {
        index = 0;
    }
    m_field_deciamal_points->setCurrentIndex(index);
    // connect(m_field_deciamal_points,
    //     &QComboBox::activated,
    //     [this]()
    //     {
    //         m_field_deciamal_points->oops();
    //     });

    //-------------------------------------------
    QString label_raw_dp_file = tr("Two pressure dump folder");
    m_field_raw_dp_file = new QLineEdit();
    m_field_raw_dp_file->setText(QDir::toNativeSeparators(CFG::cfg->dp_dump_dir));
    m_field_raw_dp_file->setReadOnly(false);

    auto *button_select_dir2 = new QPushButton("...");
    button_select_dir2->setCheckable(false);
    button_select_dir2->setChecked(false);
    button_select_dir2->setAutoDefault(false);
    button_select_dir2->setShortcut(QKeySequence("Ctrl+S"));
    auto tip = QStringLiteral("%1\n[%2]").arg(tr("Select file"), button_select_dir2->shortcut().toString());
    button_select_dir2->setToolTip(tip);

    int buttonHeight = button_select_dir2->sizeHint().height();
    button_select_dir2->setFixedWidth(buttonHeight);

    connect(button_select_dir2,
        &QPushButton::clicked,
        this,
        [field = m_field_raw_dp_file]()
        {
            MW::mw->statusbar_clear();
            auto fn = QDir::toNativeSeparators(CFG::cfg->dp_dump_dir);
            QString dir = QFileDialog::getExistingDirectory(MW::mw,
                tr("Select Directory"),
                fn,
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            if (dir.isEmpty()) {
                return;
            }
            field->setText(dir);
        });

    auto *field_raw_dp_file_layout = new QHBoxLayout();
    field_raw_dp_file_layout->addWidget(m_field_raw_dp_file);
    field_raw_dp_file_layout->addWidget(button_select_dir2);

    //-------------------------------------------
    int row = 0;
    auto *grid_misc_setts = new QGridLayout();
    grid_misc_setts->addWidget(new QLabel(label_raw_dp_file), row, 0);
    grid_misc_setts->addLayout(field_raw_dp_file_layout, row, 1);
    ++row;
    grid_misc_setts->addWidget(new QLabel(label_decimal), row, 0);
    grid_misc_setts->addWidget(m_field_deciamal_points, row, 1);
    ++row;
    grid_misc_setts->addWidget(new QLabel(label_debug), row, 0);
    grid_misc_setts->addWidget(m_field_debug, row, 1, Qt::AlignLeft);
    ++row;

    // grid_misc_setts->addWidget(new QLabel(label_zipfile), row, 0);
    // grid_misc_setts->addWidget(m_field_zipfile, row, 1);
    // ++row;
    // grid_misc_setts->addWidget(new QLabel(label_pkt_values), row, 0);
    // grid_misc_setts->addWidget(field_pkt_values, row, 1);
    // ++row;

    auto *box_misc_settings = new QGroupBox(/*tr("Miscellaneous")*/);
    auto *vbox2 = new QVBoxLayout(box_misc_settings);
    vbox2->addLayout(grid_misc_setts);
    vbox2->addStretch();
    return box_misc_settings;
}
