/*!
  \file   wmsg.cpp
  \author Aleksey Lekhtin
  \date   27.11.2024
  \brief  Window with message
*/

#include <QGridLayout>

#include "cfg.h"
//#include "mw.h"
#include "wmsg.h"

void WMSG::WMsg(const QString &msg)
{
    WMsg(Q_NULLPTR, QString(), msg, QMessageBox::Information);
}

void WMSG::WMsg(const QString &title, const QString &msg)
{
    WMsg(Q_NULLPTR, title, msg, QMessageBox::Information);
}

void WMSG::WMsg(const QString &msg, QMessageBox::Icon icon)
{
    WMsg(Q_NULLPTR, QString(), msg, icon);
}

void WMSG::WMsg(const QString &title, const QString &msg, QMessageBox::Icon icon)
{
    WMsg(Q_NULLPTR, title, msg, icon);
}

void WMSG::WMsg(QWidget *parent, const QString &msg, QMessageBox::Icon icon)
{
    WMsg(parent, QString(), msg, icon);
}

void WMSG::WMsg(QWidget *parent, const QString &title, const QString &msg, QMessageBox::Icon icon)
{
    auto *msg_box = new QMessageBox(parent);
    msg_box->setFont(CFG::cfg->menu_font);
    msg_box->setIcon(icon);
    msg_box->setStandardButtons(QMessageBox::Close);
    msg_box->setDefaultButton(QMessageBox::Close);
    if (!title.isEmpty()) msg_box->setWindowTitle(title);

    int pos = msg.indexOf("<html>");
    if (pos == -1) {
        msg_box->setText(msg);
    } else {
        QString msg1 = msg.left(pos);
        QString msg2;
        int pos2 = msg.indexOf("</html>");
        if (pos2 == -1) {
            msg2 = msg.mid(pos) + "</html>";
        } else {
            int num = pos2 - pos + 7;
            msg2 = msg.mid(pos, num);
            const QString remainstr = msg.mid(pos + num);
            if (!remainstr.isEmpty()) {
                msg1 += "\n" + remainstr;
            }
        }
        msg_box->setText(msg1);
        msg_box->setInformativeText(msg2);
    }

    auto *layout = (QGridLayout*)msg_box->layout();
    layout->setColumnMinimumWidth(2, 350);
    msg_box->exec();
    delete msg_box;
}
