/**
  \file   esz7cli.c
  \author Aleksey Lekhtin
  \date   Feb 10, 2025
  \brief  Control client for esz7 board
*/

#include <QApplication>
#include <QTimer>

#include "cfg.h"
#include "dbg.h"
#include "esz7cli.h"
#include "mw.h"
#include "wmsg.h"

#include "esz7_client.h"
#include "esz7_pkt.h"
#include "esz7_sig.h"

ESZ7CLI::ESZ7CLI() :
    ping_period_ms(2000),
    m_ping_timer(new QTimer())
{
    ESZ7CLIENT::esz7_client_set_debug_level(DBG::debug_level);
    m_ping_timer->setSingleShot(true);

    connect(this, &ESZ7CLI::signal_message, this, [](const QString& title, const QString& msg, QMessageBox::Icon icon) {
        WMSG::WMsg(MW::mw(), title, msg, icon);
    }, Qt::QueuedConnection);

    connect(this, &ESZ7CLI::signal_refresh_register, this, [](int regno, uint32_t value) {
        QString msg;
        if (regno == 1) {
            MW::mw()->update_adc_mode_register(value);
            msg = QString("%1 (#%2)").arg(tr("Register read")).arg(regno);
        } else if (regno == 2) {
            MW::mw()->update_adc_conf_register(value);
            msg = QString("%1 (#%2)").arg(tr("Register read")).arg(regno);
        }
        MW::mw()->showstatus(msg, 3);
    }, Qt::QueuedConnection);

    connect(this, &ESZ7CLI::signal_register_updated, this, [](int regno, uint32_t value) {
        (void)value;
        QString msg = QString("%1 (#%2)").arg(tr("Register updated")).arg(regno);
        MW::mw()->showstatus(msg, 3);
    }, Qt::QueuedConnection);


    set_hostname(CFG::cfg()->esz7_host);
    set_port(CFG::cfg()->esz7_port);
    ////if (!run()) {
    ////    printf("%s(): %s\n", __func__, esz7_client_strerror());
    ////}
}

ESZ7CLI::~ESZ7CLI()
{
    m_ping_timer->stop();
    this->disconnect();
    ESZ7CLIENT::esz7_clients_stop();
}

void ESZ7CLI::start_ping()
{
    DBGF(3, "ESZ7CLI::%s(): start ping timer...\n", __func__);
    m_ping_timer->start(500);
}

void ESZ7CLI::stop_ping()
{
    DBG(1);
    m_ping_timer->stop();
}

bool ESZ7CLI::run()
{
    DBG(3, "\nstarting esz7 board client");

    auto host = m_host.toStdString();
    if (host.empty()) {
        host = "localhost";
    }
    int port = m_port;
    if (port == 0) {
        port = 18000;
    }

    m_client = ESZ7CLIENT::esz7_client_create(host.c_str(), port);
    if (!m_client) {
        emit signal_message(tr("System error"),
                            QString(ESZ7CLIENT::esz7_client_strerror()),
                            QMessageBox::Critical);
        return FAIL;
    }

    ESZ7CLIENT::esz7_client_set_response_callback(m_client, this, slot_parse_response);
    ESZ7CLIENT::esz7_client_set_error_callback(m_client, this, slot_show_errors);
    ESZ7_SIGNAL::esz7_set_sig_action();

    if (!ESZ7CLIENT::esz7_client_run(m_client)) {
        emit signal_message(tr("System error"),
                            QString(ESZ7CLIENT::esz7_client_strerror()),
                            QMessageBox::Critical);
        return FAIL;
    }

    connect(m_ping_timer, &QTimer::timeout, this, [this]() {
        ESZ7_LED_STATUS status = MW::mw()->esz7_led_icon_status();
        DBGF(3, "-- ping timer status: {}", MW::mw()->esz7_led_icon_status_string(status));
        set_led_icon_ping();
        esz7_pkt_t *packet = ESZ7PKT::esz7_pkt_make_cmd(PKT_CMD_PING);
        send_packet(&packet);
    });

    qRegisterMetaType<ESZ7_LED_STATUS>("ESZ7_LED_STATUS");
    connect(this, &ESZ7CLI::signal_start_ping_timer, this, [this](ESZ7_LED_STATUS status) {
        if (!m_client) {
            set_led_icon_gray();
            return;
        }
        if (status == ESZ7_LED_STATUS::INACTIVE) {
            set_led_icon_inactive();
        } else if (status == ESZ7_LED_STATUS::ACTIVE) {
            set_led_icon_active();
        } else {
            set_led_icon_gray();
        }
        DBGF(3, "restart ping timer at %dms ...\n\n", ping_period_ms);
        m_ping_timer->start(ping_period_ms);
    }, Qt::QueuedConnection);

    DBGF(3, "ESZ7CLI::%s(): esz7 board client started\n\n", __func__);
    return OK;
}

void ESZ7CLI::slot_parse_response(void *obj, void *arg, int argsize)
{
    DBGF(3, "obj: 0x{:08x}, arg: 0x{:08x}, size: {}", (uintptr_t)obj, (uintptr_t)arg, argsize);
    const char *errormsg;
    if (!ESZ7PKT::esz7_pkt_check((char *)arg, argsize, &errormsg)) {
        DBGF(0, "esz7_pkt_check() error: {}", errormsg);
        return;
    }

    auto *esz7cli = (ESZ7CLI*)obj;
    const esz7_pkt_t *pkt = (esz7_pkt_t *)arg;
    ESZ7PKT::esz7_pkt_dbg_print(pkt);

    if (pkt->cmd == PKT_CMD_PONG) {
        DBG(3, "PONG");
        emit esz7cli->signal_start_ping_timer(ESZ7_LED_STATUS::ACTIVE);
        return;
    }

    int datalen;
    const char *data = ESZ7PKT::esz7_pkt_data(pkt, &datalen);

    if (pkt->cmd == PKT_CMD_MESG) {
        QString msg(QByteArray(data, datalen));
        if (msg.isEmpty()) {
            msg = tr("Some error");
        }
        emit esz7cli->signal_message(tr("Response message"), msg, QMessageBox::Warning);
        return;
    }

    if (pkt->cmd == PKT_CMD_READ_REG) {
        uint32_t value = *(uint32_t *)data;
        emit esz7cli->signal_refresh_register(pkt->regno, value);
        return;
    }

    if (pkt->cmd == PKT_CMD_WRITE_REG) {
        uint32_t value = 0;
        if (data) {
            value = *(uint32_t *)data;
        }
        emit esz7cli->signal_register_updated(pkt->regno, value);
        return;
    }

    if (datalen > 0)    {
        printf("data: '%.*s'\n", datalen, data);
    }
    QString msg(QByteArray(data, datalen));
    emit esz7cli->signal_message(tr("Some message"), msg, QMessageBox::Warning);
}

void ESZ7CLI::slot_show_errors(void *obj, void *arg, int argsize)
{
    DBGF(3, "obj: 0x{:08x}, arg: 0x{:08x}, size: {}", (uintptr_t)obj, (uintptr_t)arg, argsize);

    auto *esz7cli = (ESZ7CLI*)obj;

    const char *errormsg = nullptr;
    if (!ESZ7PKT::esz7_pkt_check((char *)arg, argsize, &errormsg)) {
        DBGF(0, "response error: {}", errormsg);
        QString msg(errormsg);
        emit esz7cli->signal_message(tr("Invalid response"), msg, QMessageBox::Warning);
        return;
    }

    const esz7_pkt_t *pkt = (esz7_pkt_t *)arg;
    ESZ7PKT::esz7_pkt_dbg_print(pkt);

    if (pkt->cmd == PKT_CMD_PING) {
        emit esz7cli->signal_start_ping_timer(ESZ7_LED_STATUS::INACTIVE);
        return;
    }

    int datalen;
    const char *data = ESZ7PKT::esz7_pkt_data(pkt, &datalen);

    if (datalen > 0) {
        printf("data: '%.*s'\n", datalen, data);
    }

    //// QString msg = QString::fromLatin1(strerror);
    QString msg(QByteArray(data, datalen));
    emit esz7cli->signal_message(tr("Response error"), msg, QMessageBox::Warning);
}

void ESZ7CLI::send_packet(esz7_pkt_t **packet)
{
    if (packet) {
        ESZ7CLIENT::esz7_client_send_packet(m_client, *packet);
        ESZ7PKT::esz7_pkt_free(packet);
    }
}

void ESZ7CLI::set_hostname(const QString &hostname)
{
    m_host = hostname;
}

void ESZ7CLI::set_port(int port)
{
    m_port = port;
}

void ESZ7CLI::set_led_icon_gray()
{
    MW::mw()->set_esz7_led_icon_gray();
}

void ESZ7CLI::set_led_icon_ping()
{
    MW::mw()->set_esz7_led_icon_ping();
}

void ESZ7CLI::set_led_icon_active()
{
    MW::mw()->set_esz7_led_icon_active();
}

void ESZ7CLI::set_led_icon_inactive()
{
    MW::mw()->set_esz7_led_icon_inactive();
}

void ESZ7CLI::read_from_adc_reg(int regnum)
{
    char buf[32] = {0};
    int len = snprintf(buf, sizeof(buf)-1, "read %d", regnum);
    esz7_pkt_t *packet = ESZ7PKT::esz7_pkt_make_read_cmd(PKT_CMD_READ_REG, regnum);
    ESZ7PKT::esz7_pkt_set_data(&packet, buf, len);
    send_packet(&packet);
}

void ESZ7CLI::write_to_adc_reg(int regnum, uint32_t value)
{
    char buf[32] = {0};
    int len = snprintf(buf, sizeof(buf)-1, "write %d 0x%08x", regnum, value);
    esz7_pkt_t *packet = ESZ7PKT::esz7_pkt_make_write_cmd(PKT_CMD_WRITE_REG, regnum, value);
    ESZ7PKT::esz7_pkt_set_data(&packet, buf, len);
    send_packet(&packet);
}
