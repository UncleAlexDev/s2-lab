/**
  \file   server.cpp
  \author Aleksey Lekhtin
  \date   Dec 10, 2024
  \brief  Simple server DGRAM UDP
*/


#include "network.h"

#include "adc.h"
#include "cfg.h"
#include "dbg.h"
#include "dpt.h"
#include "mw.h"
#include "wmsg.h"
#include "wsa.h"

#include <QApplication>

#define S1(x) #x
#define S2(x) S1(x)
#define SLINE_ S2(__LINE__)
static_assert(sizeof(packet_aps_t) == 424, __FILE__ ":" SLINE_);
//static_assert(sizeof(packet_atlas_t) == 144, __FILE__ ":" SLINE_);

using namespace std::chrono_literals;

//---------------------------------------------------------
NETWORK::NETWORK()
{
    srv = this;

    WSA::wsa_startup();

    connect(
        this, &NETWORK::signal_message, this,
        [&](const QString &title, const QString &msg, QMessageBox::Icon icon)
        {
            WMSG::WMsg(MW::mw, title, msg, icon);
        },
        Qt::QueuedConnection);

    connect(
        this, &NETWORK::signal_showstatus, this,
        [](const QString &message)
        {
            MW::mw->statusbar_show(message);
        },
        Qt::QueuedConnection);

    connect(
        this, &NETWORK::signal_server_down, this,
        [&](const QString &message)
        {
            if (ADC::adc) {
                ADC::adc->release_receive_button();
            }
            if (DPT::dpt) {
                DPT::dpt->release_receive_button();
            }
            QString msg = QString("%1\n\n%2").arg(tr("Connection has been down"), message);
            emit signal_showstatus({});
            emit signal_message({}, msg, QMessageBox::Warning);
        },
        Qt::QueuedConnection);
}

//---------------------------------------------------------
NETWORK::~NETWORK()
{
#ifdef D_B_G
    DBG(4, "    ++");
#endif

    stop();
    WSA::wsa_cleanup();

#ifdef D_B_G
    DBG(4, "    --");
#endif
}

//// //---------------------------------------------------------
//// void NETWORK::set_atlas_port(int port)
//// {
////     m_atlas_port = port;
//// }

//---------------------------------------------------------
int NETWORK::port() const
{
    return m_sock.port;
}

//// //---------------------------------------------------------
//// void NETWORK::set_atlas_host(const QString &host)
//// {
////     m_atlas_host = host.toStdString();
//// }

//---------------------------------------------------------
QString NETWORK::host() const
{
    return {m_sock.host.c_str()};
}

//---------------------------------------------------------
const char *NETWORK::strerror() const
{
    return m_strerror.c_str();
}
