/**
  \file   network.h
  \author Aleksey Lekhtin
  \date   Dec 10, 2024
  \brief  Simple server DGRAM UDP
*/

#ifndef NETWORK_H_
#define NETWORK_H_

//---------------------------------------------------------
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#endif

#ifdef __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
#endif
//---------------------------------------------------------

#include "packet_aps_t.h"
#include "packet_atlas_t.h"
////#include "channels_points.h"

#include <mutex>
#include <string>

#include <QMessageBox>
#include <QObject>


#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif

#define PKT_DEF_MAX_SIZE (((500 * 4) * 4 + 48) + 100)

class CHARTS;
class BBIPoints;

//---------------------------------------------------------
/**
 * @brief The NETWORK class
 */
class NETWORK : public QObject
{
    Q_OBJECT;

    enum class RUN_STATUS
    {
        ERR = (int8_t)-1,
        DOWN = 0,
        RUN = 1,
        GOESDOWN = 2
    };

  public:
    explicit NETWORK();
    ~NETWORK() override;

    inline static NETWORK *srv{};

  public:
    bool run();
    int stop();
    static bool running();
    QString host() const;
    int port() const;
    //// void set_atlas_host(const QString &host);
    //// void set_atlas_port(int atlas_port);
    const char *strerror() const;
    static const char *sock_type(int type);
    static const char *sock_family(int family);
    static std::string status_string(NETWORK::RUN_STATUS status);
    static void dump(const std::vector<float> &points);

  private:
    bool open_socket_as_udp();
    int create_udp_socket(const std::string &host, int port);
    int setup_universal_udp1(const std::string &host, int port);
    int setup_universal_udp(const std::string &host, int port);
    bool ads_data_connect();
    void socket_close();
    void server_thread_func();
    bool get_bytes();
    void parse_atlas_packet();
    void parse_aps_packet();
    void free_packet_aps();
    //---------------------------------------------------------
    void parse_atlas_data_bytes();
    void dbg_print_atlas_packet();
    void free_packet_atlas();

  signals:
    void signal_points_updated();
    void signal_server_down(QString message);
    void signal_showstatus(QString message);
    void signal_message(QString title, QString msg, QMessageBox::Icon icon);

  private:
    typedef struct socket_t
    {
        std::string host;
        int port;
        int type;
        int family;
        SOCKET sockfd;
        //// SOCKADDR sockaddr;
        SOCKADDR_STORAGE sockaddr;
    } socket_t;

  private:
    std::string m_atlas_host{};
    int m_atlas_port{};
    std::string m_ads_host{};
    int m_ads_data_port{};
    std::string m_sureshot_host{};
    int m_sureshot_data_port{};
    inline static std::atomic<RUN_STATUS> m_run_status{RUN_STATUS::DOWN};
    std::string m_strerror;
    socket_t m_sock{};
    uint32_t m_got_packet_counter{};
    char m_got_bytes[PKT_DEF_MAX_SIZE]{};
    size_t m_got_bytes_size{0};

    uint64_t m_pckt_timestamp{0};       // Время начала приема пакетов
    packet_atlas_t m_pckt_atlas;
    packet_aps_t m_pckt_aps{};

    struct timespec m_timespec{};       // Время приема данных
    struct timespec m_timespec_prev{};  // Время предыдущего приема данных

    std::mutex _mutex;
};

#endif // NETWORK_H_
