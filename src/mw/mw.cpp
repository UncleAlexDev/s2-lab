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
#include "dp_dump_view.h"
#include "dpt.h"
#include "network.h"
#include "tr_TR.h"
#include "ver.h"
#include "wmsg.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDir>
#include <QLayout>
#include <QMenu>
#include <QStatusBar>
#include <QTimer>
#include <QWindow>

//// Q_DECLARE_METATYPE(POINTS_DATA)
//// // В main.cpp перед app.exec()
//// qRegisterMetaType<POINTS_DATA>();

//---------------------------------------------------------
MW::MW() :
    QMainWindow()
{
    mw = this;
    m_cfg = new CFG();

    m_data_points = new BBIPoints();
    m_network = new NETWORK();

    if (CFG::cfg->menu_action_adc_enabled) {
        m_adc = new ADC();
    }

    if (CFG::cfg->menu_action_dpt_enabled) {
        m_dpt = new DPT();
        m_dpdump_gui = new DPTDUMP_VIEW();
    }

    this->setAttribute(Qt::WA_QuitOnClose, true);
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus, true);

    QString title = tr("BBI2-Lab");
    setWindowTitle(title);
    CFG::cfg->set_title(title);

    create_menu();

    //-------------------------------------------
    m_tabwgt = new QTabWidget(this);
    m_tabwgt->setStyleSheet("QTabBar::tab {color: rgb(43, 50, 143); font-weight: bold; font-size: 14; width:180;}");
    m_tabwgt->setTabShape(QTabWidget::Triangular);
    m_tabwgt->setElideMode(Qt::ElideRight);

    //-------------------------------------------
    // Tab 1.
    if (m_adc) {
        auto index = m_tabwgt->addTab(m_adc->gui_page(), m_adc->page_name());
        auto tip = QString("%1\n\n[%2]").arg(m_tabwgt->tabText(index), tr("PgUp or PgDown to select prev/next tab"));
        m_tabwgt->setTabToolTip(index, tip);
    }

    // Tab 2.
    if (m_dpt) {
        if (CFG::cfg->data_format == CFG::DATA_FORMAT::ATLAS) {
            auto index = m_tabwgt->addTab(m_dpt->gui_page(), m_dpt->page_name());
            auto tip =
                QString("%1\n\n[%2]").arg(m_tabwgt->tabText(index), tr("PgUp or PgDown to select prev/next tab"));
            m_tabwgt->setTabToolTip(index, tip);
        }
    }

    // Tab 3.
    if (m_dpdump_gui) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        auto index = m_tabwgt->addTab(m_dpdump_gui->gui_page(), m_dpdump_gui->page_name());
        auto tip = QString("%1\n\n[%2]").arg(m_tabwgt->tabText(index), tr("PgUp or PgDown to select prev/next tab"));
        m_tabwgt->setTabToolTip(index, tip);
        //// m_tabwgt->setTabVisible(index, false);
#endif
    }

    //-------------------------------------------
    DBGF(3, "currentIndex: {}", CFG::cfg->current_page);
    if (CFG::cfg->current_page == -1) {
        CFG::cfg->current_page = 0;
    }
    m_tabwgt->setCurrentIndex(CFG::cfg->current_page);
    auto index = m_tabwgt->currentIndex();
    for (;;) {
        DBGF(3, "index: {}, isTabeVisible(): {}", m_tabwgt->currentIndex(), m_tabwgt->isTabVisible(index));
        if (m_tabwgt->isTabVisible(index)) {
            break;
        }
        --index;
        m_tabwgt->setCurrentIndex(index);
    }
    CFG::cfg->current_page = index;
    DBGF(3, "currentIndex: {}, visible: {}", m_tabwgt->currentIndex(), m_tabwgt->isTabVisible(index));

    //-------------------------------------------
#if 1
    // setup width of tab
    int ww = 100;
    m_tabwgt->setFont(CFG::cfg->menu_font);
    QFontMetrics metrics = m_tabwgt->fontMetrics();
    for (int idx = 0; idx < m_tabwgt->count(); ++idx) {
        auto str = m_tabwgt->tabText(idx);
        ww = qMax(ww, metrics.horizontalAdvance(str));
    }
    ww += metrics.maxWidth() * 4;
    DBGF(3, "tab width: {}", ww);
    auto stylesheet =
        QString("QTabBar::tab { font-family: 'Verdana'; font-size: %1 px; font-weight: bold; width: %2 px;} "
                "QTabBar::tab:selected { font-weight: bold; } QTabBar::tab:!selected { font-weight: normal; }")
            .arg(CFG::cfg->menu_font.pointSize())
            .arg(ww);
    DBG(3, stylesheet.toStdString());
    m_tabwgt->setStyleSheet(stylesheet);
#endif

    //-------------------------------------------
    connect(m_tabwgt, &QTabWidget::currentChanged, this, &MW::slot_tabwgt_changed);
    setCentralWidget(m_tabwgt);
    statusbar_show("");

    //-------------------------------------------
    setup_fonts();
    set_mainwindow_font_size();

    if (!CFG::cfg->mainwindow_geometry.isEmpty()) {
        restoreGeometry(CFG::cfg->mainwindow_geometry);
    } else {
        setGeometry(100, 100, 800, 600);
    }
}

//---------------------------------------------------------
MW::~MW()
{
#ifdef D_B_G
    DBG(4, "++");
#endif
    CFG::cfg->save_cfg();
    CFG::cfg->save_status();
    delete m_adc;
    delete m_dpt;
    delete m_dpdump_gui;
    delete m_network;
    delete m_data_points;
    delete m_shortcut_prev_tab;
    delete m_shortcut_next_tab;
    delete m_cfg;
#ifdef D_B_G
    DBG(4, "--");
#endif
}

//---------------------------------------------------------
void MW::closeEvent(QCloseEvent *event)
{
#ifdef D_B_G
    DBG(4, "++");
#endif
    CFG::cfg->save_mainwindow_status();
    QWidget::closeEvent(event);
    // ev->accept();
}

//---------------------------------------------------------
void MW::statusbar_show(const QString &msg) const
{
    statusbar_show(msg, false, 0);
}

//---------------------------------------------------------
void MW::statusbar_show(const QString &msg, int sec) const
{
    statusbar_show(msg, false, sec);
}

//---------------------------------------------------------
void MW::statusbar_show(const QString &msg, bool bold) const
{
    statusbar_show(msg, bold, 0);
}

//---------------------------------------------------------
void MW::statusbar_show(const QString &msg, bool bold, int ms) const
{
    if (!statusBar()) {
        return;
    }
    if (msg.isEmpty()) {
        statusBar()->clearMessage();
    } else {
        statusBar()->showMessage(msg, ms);
    }
    QFont fnt = statusBar()->font();
    fnt.setBold(bold);
    statusBar()->setFont(fnt);
    QApplication::processEvents();
}

//---------------------------------------------------------
void MW::statusbar_clear() const
{
    statusBar()->clearMessage();
    QApplication::processEvents();
}

//---------------------------------------------------------
QString MW::statusbar_currentmsg() const
{
    if (!statusBar())
        return {};
    return statusBar()->currentMessage();
}

//---------------------------------------------------------
QWidget *MW::currentTabWidget() const
{
    return m_tabwgt->currentWidget();
}

//---------------------------------------------------------
NETWORK *MW::server()
{
    return m_network;
}

//---------------------------------------------------------
void MW::about()
{
    auto tr = TRANSLATIONS::tr();
    QString trmsg;
    if (tr) {
        trmsg = tr->tr_path();
    }
    if (trmsg.isEmpty()) {
        trmsg = "Translation not found.\nUsing en_US";
    } else {
        trmsg = QString("%1:\n%2").arg(QObject::tr("Translations"), QDir::toNativeSeparators(trmsg));
    }

    auto version = QString("Ver. %1 %2\n(built %3 %4)\n\nbuilt with Qt %5\nrun-time Qt %6")
                       .arg(VER_NUM, VER_DATE, __DATE__, __TIME__, QT_VERSION_STR)
                       .arg(qVersion());
    // auto versionshort = QString("Ver. %1 %2").arg(VERS, VERS_DATE);

    QStringList argv = QCoreApplication::arguments();
    QFileInfo fn(argv[0]);
#ifdef Q_OS_WIN
    if (!fn.suffix().contains(".exe"))
        fn.setFile("");
#endif
    QString ver = QString("%1%2 %3").arg(CFG::cfg->app_title, fn.fileName(), version);

    QString msg = QString("%1\n%2\n\n%3").arg(CFG::cfg->app_title, version, trmsg);
    WMSG::WMsg(this, QObject::tr("About Application"), msg, QMessageBox::Information);
}
