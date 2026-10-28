//
// Created by LAV on 06.05.2026.
//

#ifndef TR_TR_H
#define TR_TR_H

#include <QString>
#include <QTranslator>
class TRANSLATIONS
{
  public:
    explicit TRANSLATIONS();
    ~TRANSLATIONS();

    static TRANSLATIONS *tr();

    const QString &tr_path();

  private:
    void load();

  private:
    QString m_tr_path;
    QTranslator *m_qt_translator;
    QTranslator *m_app_translator;
};

#endif // TR_TR_H
