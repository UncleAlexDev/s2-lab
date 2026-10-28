/**
  \file   m_cpp
  \author Aleksey Lekhtin
  \brief  Raw file structure
  \date   27.11.2024
*/

#include <fmt/format.h>
#include <iostream>
#include <sys/stat.h>

#ifdef __linux__
#include <unistd.h>
#endif

#include "dbg.h"
#include "rawfile.h"
#include "utils.h"

//---------------------------------------------------------
RAWFILE::~RAWFILE()
{
#ifdef D_B_G
    printf("%s ++\n", __func__);
#endif
    rawfile_close();
#ifdef D_B_G
    printf("%s\n", __func__);
#endif
}

//---------------------------------------------------------
uint32_t RAWFILE::total_blocks() const
{
    return m_total_blocks;
}

//---------------------------------------------------------
uint32_t RAWFILE::total_values() const
{
    return m_total_values;
}

//---------------------------------------------------------
int RAWFILE::error() const
{
    return m_err;
}

//---------------------------------------------------------
const char *RAWFILE::str_error() const
{
    return m_strerror;
}

//---------------------------------------------------------
const std::string &RAWFILE::filename() const
{
    return m_filename;
}

//---------------------------------------------------------
void RAWFILE::set_filename(const char *filename)
{
    if (!filename) {
        m_filename.clear();
        return;
    }
    m_filename = filename;
}

//---------------------------------------------------------
void RAWFILE::close()
{
    DBG(1);
    rawfile_close();
}

//---------------------------------------------------------
int RAWFILE::open(const char *filename)
{
    m_err = 0;
    m_strerror[0] = 0;
    set_filename(filename);
    if (rawfile_open(filename) == -1) {
        return -1;
    }

    // parse blocks
    m_total_blocks = 0;
    m_total_values = 0;
    m_value1_max   = 0;
    m_value1_min   = 0;
    m_value2_max   = 0;
    m_value2_min   = 0;

    if (jump_to_first_block() == -1) {
        rawfile_close();
        return -1;
    }

    if (read_data_block() == -1) {
        rawfile_close();
        return -1;
    }

    uint32_t value1 = m_data.pressure1.at(0);
    uint32_t value2 = m_data.hookload.at(0);
    m_value1_min = value1;
    m_value1_max = value1;
    m_value2_min = value2;
    m_value2_max = value2;

    UTILS::windows_Filetime_to_secs(m_data.header.dataDate, &m_data_date_secs, &m_data_date_nsecs);
    strncpy(m_datestr, UTILS::secs_to_date_str(m_data_date_secs), sizeof(m_datestr)-1);

    int ret = 0;
    do {
        if (m_data.pressure1.empty() || m_data.hookload.empty()) {
            std::cout << fmt::format("-- block: {}\n", m_total_blocks);
            std::cout << fmt::format("-- pressure count: {}, hookload: {}\n", m_data.pressure1.size(), m_data.hookload.size());
            m_err = EINVAL;
            auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "{}\n{} #{}\n\n{}",
                     UTILS::transl("Seems data is corrupted."),
                     UTILS::transl("Detected empty data block"),
                     m_total_blocks + 1,
                     UTILS::transl("Further processing is stopped."));
            *s.out = '\0';
            std::cout << fmt::format("-- {}\n\n", m_strerror);
            ret = -1;
            break;
        }
        if (m_data.pressure1.empty() != m_data.hookload.empty()) {
            std::cout << fmt::format("-- block: {}\n", m_total_blocks);
            std::cout << fmt::format("-- pressure count: {}, hookload: {}\n", m_data.pressure1.size(), m_data.hookload.size());
            m_err = EINVAL;
            auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "{} #{}\n{}\n{} <> {}\n\n{}",
                UTILS::transl("Incorrect data block"), m_total_blocks + 1,
                UTILS::transl("Mismatch in the number of values."),
                m_data.pressure1.size(), m_data.hookload.size(),
                UTILS::transl("Further processing is stopped."));
            *s.out = '\0';
            std::cout << fmt::format("-- {}\n\n", m_strerror);
            ret = -1;
            break;
        }

        uint16_t num = m_data.pressure1.size();
        for(uint16_t i = 0; i < num; i++) {
            value1 = m_data.pressure1.at(i);
            value2 = m_data.hookload.at(i);

            if (value1 < m_value1_min) {
                m_value1_min = value1;
            } else if (value1 > m_value1_max) {
                m_value1_max = value1;
            }

            if (value2 < m_value2_min) {
                m_value2_min = value2;
            } else if (value2 > m_value2_max) {
                m_value2_max = value2;
            }
        }

        m_total_values += num;
        ++m_total_blocks;
        //if (m_total_blocks >= 4754) break;

        if (m_position >= m_length) {
            break;
        }

        if (read_data_block() == -1) {
            break;
        }
    } while(true);

    if (ret == -1) {
        rawfile_close();
        return -1;
    }

    if (jump_to_first_block() == -1) {
        rawfile_close();
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------
int RAWFILE::jump_to_first_block()
{
    m_position = sizeof(m_file_hdr);
    m_position = rawfile_seek(m_position, SEEK_SET);
    if (m_position == -1) {
        return -1;
    }
    return 0;
}

int RAWFILE::rawfile_open(const char *filename)
{
    rawfile_close();
    m_err = 0;
    m_strerror[0] = 0;
    if (filename[0] == 0) {
        m_err = ENFILE;
        auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "{}",
            UTILS::transl("Filename not specified").c_str());
        *s.out = '\0';
        return -1;
    }


    // 1. open file
    int mode = O_RDONLY;
#ifdef WIN32
    mode |= O_BINARY;
#endif
    struct stat st{};
    m_fd = ::open(m_filename.c_str(), mode);
    if (m_fd == -1 || fstat(m_fd, &st) == -1) {
        m_err = errno;
        auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "{}", ::strerror(m_err));
        *s.out = '\0';
        return -1;
    }
    if (st.st_size == 0) {
        m_err = EMFILE;
        auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "{}",
            UTILS::transl("Empty file").c_str());
        *s.out = '\0';
        ::close(m_fd);
        m_fd = -1;
        return -1;
    }
    m_length = st.st_size;

    // 2. check signature
    bool sign_ok = false;
    read(m_fd, &m_file_hdr, sizeof(m_file_hdr));  // 3 + 4 x 3 = 11
    if (strncmp(m_file_hdr.sign, RAWFILE::RAW_SIGN, sizeof(RAWFILE::RAW_SIGN)) == 0) {
        if (m_file_hdr.version[0] == 0 && m_file_hdr.version[1] == 3) {
            sign_ok = true;
        }
    }
    if (!sign_ok) {
        m_err = EINVAL;
        auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "{}",
            UTILS::transl("Wrong signature").c_str());
        *s.out = '\0';
        ::close(m_fd);
        m_fd = -1;
        return -1;
    }
    return 0;
}

//---------------------------------------------------------
void RAWFILE::rawfile_close()
{
    free_data_block();
    if (m_fd > -1) {
        ::close(m_fd);
    }
    m_fd = -1;
}

//---------------------------------------------------------
/**
 * @brief read block data from file
 * @param data - buffer for read data
 * @param count - number of byte for reading
 * @return int
 * @return number of bytes read
 * @return 0 - end of file
 * @return -1 - error
 */
ssize_t RAWFILE::rawfile_read(void *data, size_t count)
{
    m_err = 0;
    m_strerror[0] = 0;
    ssize_t num = read(m_fd, data, (unsigned int)count);
    if (num == -1)
    {
        m_err = errno;
        auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "{}", ::strerror(m_err));
        *s.out = '\0';
        return -1;
    }
    if (num == 0) {
        // end of file
        return 0;
    }
    m_position += (off_t)num;
    if (num != (ssize_t)count)
    {
        m_err = ENOENT;
#if defined _WIN64 || defined __MINGW64__
        auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "read {} bytes but expected {}", num, count);
#else
        auto s = fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "read {} bytes but expected {}", num, count);
#endif
        *s.out = '\0';
        return -1;
    }
    return num;
}

//---------------------------------------------------------
/**
 * @brief change position in file
 * @param offset - required position
 * @param whence - position according to
 * @return off_t
 * @return new position
 * @return - -1 if error
 */
_off64_t RAWFILE::rawfile_seek(_off64_t offset, int whence)
{
    m_err = 0;
    m_strerror[0] = 0;
    _off64_t pos = lseek64(m_fd, offset, whence);
    if (pos == -1)
    {
        m_err = errno;
        auto s =
            fmt::format_to_n(m_strerror, sizeof(m_strerror) - 1, "{}", ::strerror(m_err));
        *s.out = '\0';
        return -1;
    }
    return pos;
}

//---------------------------------------------------------
void RAWFILE::free_data_block()
{
    m_data.pressure1.resize(0);
    m_data.pressure2.resize(0);
    m_data.hookload.resize(0);
    m_data = {};
}

//---------------------------------------------------------
/**
 * @brief read one m_data block from file
 * @return int
 * @return number of bytes read
 * @return 0 - end of file
 * @return -1 - error
 */
ssize_t RAWFILE::read_data_block()
{
    ssize_t totalread = 0;
    ssize_t num = rawfile_read(&m_data.header, sizeof(m_data.header));
    if (num > 0) {
        totalread += num;
        uint16_t nelems;
        num = rawfile_read((void*)&nelems, sizeof(nelems));
        if (num > 0) {
            totalread += num;
            m_data.pressure1.resize(nelems);
            size_t nbytes = nelems * sizeof(uint32_t);
            num = rawfile_read(static_cast<void*>(m_data.pressure1.data()), nbytes);
            if (num > 0) {
                totalread += num;
                num = rawfile_read((void*)&nelems, sizeof(nelems));
                if (num > 0) {
                    totalread += num;
                    m_data.hookload.resize(nelems);
                    size_t nbytes = nelems * sizeof(uint32_t);
                    num = rawfile_read(static_cast<void*>(m_data.hookload.data()), nbytes);
                    if (num > 0) {
                        totalread += num;
                    }
                }
            }
        }
    }

    if (num == -1) {
        free_data_block();
        return -1;
    }

    if (num == 0) {
        return 0;
    }

    return totalread ;
}
