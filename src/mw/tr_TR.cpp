//
// Created by LAV on 06.05.2026.
//

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QProcessEnvironment>

#include "dbg.h"
#include "tr_TR.h"

TRANSLATIONS *m_tr;

//---------------------------------------------------------
TRANSLATIONS::TRANSLATIONS() :
    m_qt_translator(new QTranslator()),
    m_app_translator(new QTranslator())
{
    m_tr = this;
    load();
}

//---------------------------------------------------------
TRANSLATIONS::~TRANSLATIONS()
{
    m_qt_translator = new QTranslator();
    m_app_translator = new QTranslator();
}

//---------------------------------------------------------
TRANSLATIONS *TRANSLATIONS::tr()
{
    return m_tr;
}

//---------------------------------------------------------
const QString &TRANSLATIONS::tr_path()
{
    return m_tr_path;
}

//---------------------------------------------------------
/** Search transaltions file
  \param[in] qt_translator  - translator sybsystem for QT
  \param[in] app_translator - translator sybsystem for application
  \param[in] prog_name      - application name (looking for file name)
  \note
  Path for searching:
  - locale/xx_XX/LC_MESSAGES/
  - locale/xx_XX
  - ./
  \result
  - path with transaltion
  */
void TRANSLATIONS::load()
{
#if 0
    auto nm = "qt_" + QLocale::system().name();
    DBGF(0, "translator for system messages: \"{}\"", nm.toStdString());
    auto ok = m_qt_translator->load(nm, QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    if (ok) {
        qApp->installTranslator(m_qt_translator.get());
    } else {
        DBG(0, fmt::format("can't load \"{}\"", nm.toStdString()));
    }

    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = QLocale(locale).name();
        DBGF(0, "translation for {} - ", QLocale(locale).name().toStdString());
        if (m_app_translator->load(baseName, ":/i18n/")) {
            qApp->installTranslator(m_app_translator.get());
            DBG(0, "found");
            break;
        }
        DBG(0, "not found");
    }
    m_tr_path = m_app_translator->filePath();

#else
    //--- locale for QT ----------------------------------------
    DBGF(0, "\nbcp47Name: \"{}\"", qPrintable(QLocale::system().bcp47Name()));
    DBGF(0, "locale system name: \"{}\"", qPrintable(QLocale::system().name()));
    auto nm = "translations/qt_" + QLocale::system().name() + ".qm";
    DBGF(0, "translator for system messages: \"{}\"", qPrintable(nm));

#ifdef QT6_ONLY
    auto ok = m_qt_translator->load(nm, QLibraryInfo::path(QLibraryInfo::TranslationsPath));
#else
    auto ok = m_qt_translator->load(nm, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#endif
    if (!ok) {
        DBGF(0, "can't load \"{}\"", nm.toStdString());
    } else {
        qApp->installTranslator(m_qt_translator);
    }

    //----------------------------------------------------------
    if (!ok) {
        DBG(0, "List of locale names for use in selecting translations:");
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : uiLanguages) {
            const QString baseName = QLocale(locale).name();
            //DBGF(0, "    {:12} - {}", locale.toStdString(), baseName.toStdString());
            nm = "./translations/qt_" + locale + ".qm";

            QFileInfo fi(QDir::cleanPath(nm));
            auto fn = QDir::toNativeSeparators(fi.absoluteFilePath());
            if (!fi.exists()) {
                DBGF(0, "    {} - not found", fn.toStdString());
                continue;
            }
            DBGF(0, "    {} - file found", fn.toStdString());
//#ifdef QT6_ONLY
            //ok = m_qt_translator->load(nm, QLibraryInfo::path(QLibraryInfo::TranslationsPath));
            ok = m_qt_translator->load(fn);
//#else
            //ok = m_qt_translator->load(nm, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
//#endif
            if (!ok) {
                DBG(0, "    can't load");
            } else {
                DBG(0, "    LOADED");
                //if (qApp->installTranslator(m_qt_translator)) {
                break;
                //} else {
                //    DBGF(0, "    Shit! Can't install translator {}", m_qt_translator->filePath().toStdString());
                //}
            }
        }
    }

    //----------------------------------------------------------
    QFileInfo f_info(QCoreApplication::applicationName());
    QString ext = f_info.suffix();
    QString app_name = f_info.fileName();
    if (ext == "exe") {
        app_name = f_info.completeBaseName();
    }
    if (app_name.isEmpty()) {
        return;
    }

    //----------------------------------------------------------
    QLocale loc;
    if (loc.name().isEmpty()) {
        DBG(0, "\nlocale is empty name. skip");
        return;
    }
    auto loc_name = loc.name();

    DBG(0, "\nsearch translation:");
#if 1
    QString translation_path;
    QString qm_file;
    for (int i = 0; i < 5; ++i) {
        if (i == 0) {
            qm_file = "./" + app_name + ".qm";
        } else if (i == 1) {
            qm_file = QString("./%1.%2.qm").arg(app_name).arg(loc_name);
        } else if (i == 2) {
            qm_file = QString("./locale/%1/LC_MESSAGES/%2.qm").arg(loc_name).arg(app_name);
        } else if (i == 3) {
            qm_file = QString("./locale/%1/%2.qm").arg(loc_name).arg(app_name);
        } else if (i == 4) {
            qm_file = QString("../locale/%1/%2.qm").arg(loc_name).arg(app_name);
        }

        QFileInfo fi(QDir::cleanPath(qm_file));
        DBGF(0, "    {}", QDir::toNativeSeparators(fi.filePath()).toStdString());

        auto fn = QDir::toNativeSeparators(fi.absoluteFilePath());
        if (!m_app_translator->load(fn)) {
            DBGF(0, "    {} - not found", fn.toStdString());
        } else {
            DBGF(0, "    {} - FOUND", m_app_translator->filePath().toStdString());
            if (qApp->installTranslator(m_app_translator)) {
                m_tr_path = m_app_translator->filePath();
                break;
            } else {
                DBG(0, "Shit! Can't install translator...");
            }
        }
        DBGNL(1);
    }
#else

    // translator.load(":/i18n/esz7-lab.qm")
    if (!m_app_translator->load(":/i18n/esz7-lab.ru_RU.qm")) {
        DBG(0, "    :/i18n/esz7-lab.ru_RU.qm - not found");
    } else {
        DBG(0, "    :/i18n/esz7-lab.ru_RU.qm - FOUND");
        if (qApp->installTranslator(m_app_translator)) {
            m_tr_path = m_app_translator->filePath();
            DBGF(0, "    translations: {}", m_tr_path.toStdString());
        } else {
            DBG(0, "Shit! Can't install translator...");
        }
    }
#endif

    DBGNL(1);
#endif
}
