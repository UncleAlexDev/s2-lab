/**
  \file   rawfile.h
  \author Aleksey Lekhtin
  \brief  Raw file structure
  \date   03.12.2024
*/

#ifndef RAW_FILE_H_
#define RAW_FILE_H_

#include <fcntl.h>
#include <sys/types.h>

#include <climits>
#include <cstdint>
#include <string>

#include <QObject>
#include <QString>

class RAWFILE : public QObject
{
    Q_OBJECT;

  public:

    static constexpr uint8_t RAW_CHECK_BYTE = 0;
    static inline char RAW_SIGN[3] = {'r', 'a', 'w'};

#pragma pack(push, 1)

    typedef struct file_hdr_t
    {
        char sign[3]{};
        uint32_t version[2]{};
    } file_hdr_t;

    typedef union ctrl_word_t
    {
        struct
        {
            uint8_t check_byte; // 7A
            uint8_t unused[7];
        };
        uint32_t reserved2;
    } ctrl_word_t;

    typedef struct data_block_hdr_t
    {
        uint16_t block_number{};
        uint64_t dataDate{};
        uint64_t dataRecv{};
        double encoder1{};
        double encoder2{};
        ctrl_word_t ctrlw{RAWFILE::RAW_CHECK_BYTE, {}};
        uint32_t rezerved3{};
    } data_block_hdr_t;

    typedef struct data_block_t
    {
        data_block_hdr_t header{};
        std::vector<uint32_t> pressure1{};
        std::vector<uint32_t> pressure2{};
        std::vector<uint32_t> hookload{};
    } data_block_t;

#pragma pack(pop)

  public:
    explicit RAWFILE() = default;
    ~RAWFILE();

    const std::string &filename() const;
    void set_filename(const char *filename);
    int open(const char *filename);
    void close();
    int jump_to_first_block();

    bool create(const std::string &dirname);
    size_t write(const char *bytes, size_t nbytes);
    size_t write_data_block();
    bool is_open();
    bool good();

    static QWidget *view_dump(const QString &dirname);

  public:
    uint32_t total_blocks() const;
    uint32_t total_values() const;
    int error() const;
    const char *str_error() const;

    uint32_t value1_max() const
    {
        return m_value1_max;
    }
    uint32_t value1_min() const
    {
        return m_value1_min;
    }
    uint32_t value2_max() const
    {
        return m_value2_max;
    }
    uint32_t value2_min() const
    {
        return m_value2_min;
    }
    const char *data_date() const
    {
        return m_datestr;
    }

    ssize_t read_data_block();
    inline data_block_t *data_block()
    {
        return &m_data;
    }

  private:
    int rawfile_open(const char *filename);
    void rawfile_close();
    _off64_t rawfile_seek(_off64_t offset, int whence);
    ssize_t rawfile_read(void *data, size_t count);
    void free_data_block();

  public:
    ssize_t m_length{};
    _off64_t m_position{};

    data_block_t m_data{};
    uint32_t m_total_blocks{};
    uint32_t m_total_values{};

    uint32_t m_value1_max{};
    uint32_t m_value1_min{};
    uint32_t m_value2_max{};
    uint32_t m_value2_min{};

    uint32_t m_data_date_secs{};
    uint32_t m_data_date_nsecs{};
    uint64_t m_date_ns{};
    char m_datestr[32]{};

  private:
    file_hdr_t m_file_hdr{};
    int m_fd{-1};
    int m_err{};
    char m_strerror[4 * 1024]{};
    std::string m_filename{};
};

#endif // RAW_FILE_H_
