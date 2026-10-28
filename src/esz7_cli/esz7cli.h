/**
  \file   esz7cli.h
  \author Aleksey Lekhtin
  \date   Feb 10, 2025
  \brief  Control client for esz7 board
*/

#ifndef ESZCLI_H
#define ESZCLI_H

#include <QObject>
#include <QMessageBox>
#include <QString>
#include <QTimer>

#include "esz7_client.h"

enum class ESZ7_LED_STATUS
{
    NONE = -1,
    PING = 0,
    ACTIVE,
    INACTIVE,
};

class ESZ7CLI : public QObject
{
    Q_OBJECT;

public:
    explicit ESZ7CLI();
    virtual ~ESZ7CLI();

public:
    bool    run();
    void    start_ping();
    void    stop_ping();
    void    set_hostname(const QString &hostname);
    void    set_port(int port);
    void    send_packet(esz7_pkt_t **packet);

    void    set_led_icon_gray();
    void    set_led_icon_ping();
    void    set_led_icon_active();
    void    set_led_icon_inactive();

    static  void slot_parse_response(void *obj, void *arg, int argsize);
    static  void slot_show_errors(void *obj, void *arg, int argsize);

public:
    void    read_from_adc_reg(int regnum);
    void    write_to_adc_reg(int regnum, uint32_t value);

signals:
    void    signal_start_ping_timer(ESZ7_LED_STATUS status);
    void    signal_message(QString title, QString msg, QMessageBox::Icon icon);
    void    signal_refresh_register(int regno, uint32_t value);
    void    signal_register_updated(int regno, uint32_t value);

private:
    esz7_client_t   *m_client{};
    QString         m_host{};
    int             m_port{};
    int             ping_period_ms{};
    QTimer          *m_ping_timer{};
};

#endif // ESZCLI_H
