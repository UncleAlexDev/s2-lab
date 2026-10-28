/**
  \file   dpt_dump.h
  \brief  To show Dual Pressure Dump file
  \author Aleksey Lekhtin
  \date   06.07.2026
*/

#ifndef DP_DUMP_H
#define DP_DUMP_H

#include "dp_dump_t.h"
#include "channels_points.h"

#include <fstream>

#include <QChartView>
#include <QDialogButtonBox>
#include <QSpinBox>

#ifndef QT6_ONLY
QT_CHARTS_USE_NAMESPACE
#endif

//---------------------------------------------------------
class DPDUMP : public dpdump_t
{
    //Q_OBJECT;

  public:
    enum class SUM_OR_SUB_MODE
    {
        NONE,
        SUM,
        SUB
    };

  public:
    explicit DPDUMP();
    virtual ~DPDUMP();

    bool create(const std::string &dirname);
    void close();
    void dump_data(const channels_points &chs);

    inline QString filename() {
        return QString::fromStdString(m_filename);
    }

    inline QString &strerror() {
        return m_strerror;
    }

  private:
    void read_status();
    void save_status() const;

    size_t write_data_block(const dump_t &dump);
    size_t write(const char *bytes, size_t nbytes);
    bool is_open();
    bool good();

  private:
    std::string m_filename{};
    std::ofstream m_file{};
    int m_err{};
    QString m_strerror{};
    static inline off_t m_off_prev{};
    dump_t m_dump{};
};

#endif // DP_DUMP_H
