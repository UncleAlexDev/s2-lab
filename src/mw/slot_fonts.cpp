/**
  \file   slot_fonts.cpp
  \brief  Работа с фонтами
  \author Aleksey Lekhtin
  \date   30.03.2010
*/

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QGridLayout>
#include <QGuiApplication>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QStatusBar>

#include "adc.h"
#include "cfg.h"
#include "chart.h"
#include "dbg.h"
#include "dpt.h"
#include "mw.h"

//---------------------------------------------------------
static bool isFixedPitch(const QFont &font)
{
    const QFontInfo fi(font);
    DBGF(3, "looking for fixed font: {} - fixed: {}", qPrintable(fi.family()), (fi.fixedPitch() ? "yes" : "no"));
    return fi.fixedPitch();
}

//---------------------------------------------------------
QFont MW::findMonospaceFont()
{
    QFont font("Courier New");
    if (font.styleHint() == QFont::Monospace)
        return font;
    font = QFont("Lucida Sans");
    if (font.styleHint() == QFont::Monospace)
        return font;
    font = QFont("Typewriter");
    if (font.styleHint() == QFont::Monospace)
        return font;
    font = QFont("Lucida Sans Typewriter");
    if (font.styleHint() == QFont::Monospace)
        return font;
    font = QFont("monospace");
    if (font.styleHint() == QFont::Monospace)
        return font;
    font.setStyleHint(QFont::Monospace);
    if (font.styleHint() == QFont::Monospace)
        return font;
    font.setStyleHint(QFont::TypeWriter);
    if (font.styleHint() == QFont::Monospace)
        return font;
    if (font.styleHint() == QFont::TypeWriter)
        return font;

    ////QFont font("Courier New");
    ////if (isFixedPitch(font)) return font;
    ////font = QFont("Lucida Sans");
    ////if (isFixedPitch(font)) return font;
    ////font = QFont("Typewriter");
    ////if (isFixedPitch(font)) return font;
    ////font = QFont("Lucida Sans Typewriter");
    ////if (isFixedPitch(font)) return font;
    ////font = QFont("monospace");
    ////if (isFixedPitch(font)) return font;
    ////font.setStyleHint(QFont::Monospace);
    ////if (isFixedPitch(font)) return font;
    ////font.setStyleHint(QFont::TypeWriter);
    ////if (isFixedPitch(font)) return font;
    return font;
}

//---------------------------------------------------------
void MW::slot_font_settings()
{
    QFont fnt;
    auto fontdlg = new QDialog(this);
    fontdlg->setWindowTitle(tr("Setting the font size"));

    fnt = fontdlg->font();
    fnt.setPointSize(CFG::cfg->menu_font.pointSize());
    fontdlg->setFont(fnt);

    // 1. ---------------------------------------
    QComboBox *font_menu_sizes = new QComboBox();
    font_menu_sizes->addItem(tr("System"));
    font_menu_sizes->addItem("8");
    font_menu_sizes->addItem("10");
    font_menu_sizes->addItem("12");
    font_menu_sizes->addItem("14");
    font_menu_sizes->addItem("16");

    if (CFG::cfg->menu_font.pointSize() == 8)
        font_menu_sizes->setCurrentIndex(1);
    else if (CFG::cfg->menu_font.pointSize() == 10)
        font_menu_sizes->setCurrentIndex(2);
    else if (CFG::cfg->menu_font.pointSize() == 12)
        font_menu_sizes->setCurrentIndex(3);
    else if (CFG::cfg->menu_font.pointSize() == 14)
        font_menu_sizes->setCurrentIndex(4);
    else if (CFG::cfg->menu_font.pointSize() == 16)
        font_menu_sizes->setCurrentIndex(5);

    fnt = font_menu_sizes->font();
    fnt.setPointSize(CFG::cfg->menu_font.pointSize());
    font_menu_sizes->setFont(fnt);

    connect(font_menu_sizes, SIGNAL(currentTextChanged(const QString &)), this,
        SLOT(slot_set_menu_font_size(const QString &)));

    // 2. ---------------------------------------
    QComboBox *font_text_sizes = new QComboBox();
    font_text_sizes->addItem(tr("System"));
    font_text_sizes->addItem("8");
    font_text_sizes->addItem("10");
    font_text_sizes->addItem("12");
    font_text_sizes->addItem("14");
    font_text_sizes->addItem("16");

    if (CFG::cfg->text_font.pointSize() == 8)
        font_text_sizes->setCurrentIndex(1);
    else if (CFG::cfg->text_font.pointSize() == 10)
        font_text_sizes->setCurrentIndex(2);
    else if (CFG::cfg->text_font.pointSize() == 12)
        font_text_sizes->setCurrentIndex(3);
    else if (CFG::cfg->text_font.pointSize() == 14)
        font_text_sizes->setCurrentIndex(4);
    else if (CFG::cfg->text_font.pointSize() == 16)
        font_text_sizes->setCurrentIndex(5);

    fnt = font_text_sizes->font();
    fnt.setPointSize(CFG::cfg->menu_font.pointSize());
    font_text_sizes->setFont(fnt);

    connect(font_text_sizes, SIGNAL(currentTextChanged(const QString &)), this,
        SLOT(slot_set_text_font_size(const QString &)));

    // 3. ---------------------------------------
    QComboBox *font_chart_sizes = new QComboBox();
    font_chart_sizes->addItem(tr("System"));
    font_chart_sizes->addItem("8");
    font_chart_sizes->addItem("10");
    font_chart_sizes->addItem("12");
    font_chart_sizes->addItem("14");
    font_chart_sizes->addItem("16");

    if (CFG::cfg->chart_font.pointSize() == 8)
        font_chart_sizes->setCurrentIndex(1);
    else if (CFG::cfg->chart_font.pointSize() == 10)
        font_chart_sizes->setCurrentIndex(2);
    else if (CFG::cfg->chart_font.pointSize() == 12)
        font_chart_sizes->setCurrentIndex(3);
    else if (CFG::cfg->chart_font.pointSize() == 14)
        font_chart_sizes->setCurrentIndex(4);
    else if (CFG::cfg->chart_font.pointSize() == 16)
        font_chart_sizes->setCurrentIndex(5);

    fnt = font_chart_sizes->font();
    fnt.setPointSize(CFG::cfg->menu_font.pointSize());
    font_chart_sizes->setFont(fnt);

    connect(font_chart_sizes, SIGNAL(currentTextChanged(const QString &)), this,
        SLOT(slot_set_chart_font_size(const QString &)));

    // 5. ---------------------------------------
    QComboBox *font_button_sizes = new QComboBox();
    font_button_sizes->addItem(tr("System"));
    font_button_sizes->addItem("8");
    font_button_sizes->addItem("10");
    font_button_sizes->addItem("12");
    font_button_sizes->addItem("14");
    font_button_sizes->addItem("16");

    if (CFG::cfg->button_font.pointSize() == 8)
        font_button_sizes->setCurrentIndex(1);
    else if (CFG::cfg->button_font.pointSize() == 10)
        font_button_sizes->setCurrentIndex(2);
    else if (CFG::cfg->button_font.pointSize() == 12)
        font_button_sizes->setCurrentIndex(3);
    else if (CFG::cfg->button_font.pointSize() == 14)
        font_button_sizes->setCurrentIndex(4);
    else if (CFG::cfg->button_font.pointSize() == 16)
        font_button_sizes->setCurrentIndex(5);

    fnt = font_button_sizes->font();
    fnt.setPointSize(CFG::cfg->menu_font.pointSize());
    font_button_sizes->setFont(fnt);

    connect(font_button_sizes, SIGNAL(currentTextChanged(const QString &)), this,
        SLOT(slot_set_button_font_size(const QString &)));

    //-------------------------------------------
    QDialogButtonBox *buttons_group = new QDialogButtonBox(Qt::Horizontal);
    buttons_group->setStandardButtons(QDialogButtonBox::Close);
    buttons_group->setMinimumSize(300, 24);

    QPushButton *btn = buttons_group->button(QDialogButtonBox::Close);
    btn->setDefault(true);
    btn->setAutoDefault(true);

    connect(buttons_group, SIGNAL(accepted()), fontdlg, SLOT(accept()));
    connect(buttons_group, SIGNAL(rejected()), fontdlg, SLOT(reject()));

    //-------------------------------------------
    int nrow = 0;
    auto *grid = new QGridLayout();
    grid->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    grid->addWidget(new QLabel(tr("Menu Font")), nrow, 0);
    grid->addWidget(font_menu_sizes, nrow, 1);
    ++nrow;
    grid->addWidget(new QLabel(tr("Text Font")), nrow, 0);
    grid->addWidget(font_text_sizes, nrow, 1);
    ++nrow;
    grid->addWidget(new QLabel(tr("Charts Font")), nrow, 0);
    grid->addWidget(font_chart_sizes, nrow, 1);
    ++nrow;
    grid->addWidget(new QLabel(tr("Buttons Font")), nrow, 0);
    grid->addWidget(font_button_sizes, nrow, 1);
    ++nrow;

    //-------------------------------------------
    auto *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(grid);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttons_group);

    mainLayout->setAlignment(Qt::AlignHCenter);
    mainLayout->setAlignment(grid, Qt::AlignCenter);
    mainLayout->setAlignment(buttons_group, Qt::AlignCenter);
    fontdlg->setLayout(mainLayout);

    //-------------------------------------------
    fnt = btn->font();
    fnt.setPointSize(CFG::cfg->menu_font.pointSize());
    btn->setFont(fnt);

    fontdlg->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    fontdlg->adjustSize();
    fontdlg->exec();
    delete fontdlg;
}

//---------------------------------------------------------
void MW::setup_fonts()
{
    CFG::cfg->mono_font = QGuiApplication::font();
    auto pointsize = CFG::cfg->mono_font.pointSize();
    DBGF(3, "{}", qPrintable(CFG::cfg->mono_font.toString()));
    if (!isFixedPitch(CFG::cfg->mono_font)) {
        CFG::cfg->mono_font = findMonospaceFont();
        if (isFixedPitch(CFG::cfg->mono_font)) {
            CFG::cfg->mono_font.setPointSize(pointsize);
            CFG::cfg->mono_font.setWeight(QFont::Medium);
        }
    }
    DBGF(1, "monofont: {}", qPrintable(CFG::cfg->mono_font.toString()));
}

//---------------------------------------------------------
void MW::set_fonts()
{
    if (m_adc) {
        m_adc->set_fonts();
    }
    if (m_dpt) {
        m_dpt->set_fonts();
    }
}

//---------------------------------------------------------
void MW::slot_set_menu_font_size(const QString &text)
{
    int sz = text.left(2).toInt();
    if (sz != 8 && sz != 10 && sz != 12 && sz != 14 && sz != 16) {
        CFG::cfg->menu_font = QGuiApplication::font();
    } else {
        CFG::cfg->menu_font.setPointSize(sz);
    }
    MW::set_mainwindow_font_size();
    CFG::cfg->save_status();
}

//---------------------------------------------------------
void MW::slot_set_text_font_size(const QString &text)
{
    int sz = text.left(2).toInt();
    if (sz != 8 && sz != 10 && sz != 12 && sz != 14 && sz != 16) {
        CFG::cfg->text_font = QGuiApplication::font();
    } else {
        CFG::cfg->text_font.setPointSize(sz);
    }
    set_fonts();
    CFG::cfg->save_status();
}

//---------------------------------------------------------
void MW::slot_set_chart_font_size(const QString &text)
{
    int sz = text.left(2).toInt();
    if (sz != 8 && sz != 10 && sz != 12 && sz != 14 && sz != 16) {
        CFG::cfg->chart_font = QGuiApplication::font();
    } else {
        CFG::cfg->chart_font.setPointSize(sz);
    }
    set_fonts();
    CFG::cfg->save_status();
}

//---------------------------------------------------------
void MW::slot_set_mono_font_size(const QString &text)
{
    int sz = text.left(2).toInt();
    if (sz != 8 && sz != 10 && sz != 12 && sz != 14 && sz != 16) {
        CFG::cfg->mono_font = QGuiApplication::font();
    } else {
        CFG::cfg->mono_font.setPointSize(sz);
    }
    auto pointsize = CFG::cfg->mono_font.pointSize();
    qDebug() << CFG::cfg->mono_font << pointsize;
    if (!isFixedPitch(CFG::cfg->mono_font)) {
        CFG::cfg->mono_font = findMonospaceFont();
        CFG::cfg->mono_font.setPointSize(pointsize);
        qDebug() << CFG::cfg->mono_font;
    }
    set_fonts();
    CFG::cfg->save_status();
}

//---------------------------------------------------------
void MW::slot_set_button_font_size(const QString &text)
{
    int sz = text.left(2).toInt();
    if (sz != 8 && sz != 10 && sz != 12 && sz != 14 && sz != 16) {
        CFG::cfg->button_font = QGuiApplication::font();
    } else {
        CFG::cfg->button_font.setPointSize(sz);
    }
    set_fonts();
    CFG::cfg->save_status();
}

//---------------------------------------------------------
void MW::set_mainwindow_font_size()
{
    menuBar()->setFont(CFG::cfg->menu_font);
    m_action_exit->setFont(CFG::cfg->menu_font);
    // m_action_tab1->setFont(CFG::cfg->menu_font);
    // m_action_tab2->setFont(CFG::cfg->menu_font);
    m_fileMenu->setFont(CFG::cfg->menu_font);
    m_helpMenu->setFont(CFG::cfg->menu_font);

    int sz = CFG::cfg->menu_font.pointSize();
    if (sz > 12) {
        sz = 12;
    }
    QFont fnt = statusBar()->font();
    fnt.setPointSize(sz);
    fnt.setStyleHint(QFont::Monospace);
    fnt.setFamily("Fixed");
    fnt.setFixedPitch(true);
    statusBar()->setFont(fnt);
}
