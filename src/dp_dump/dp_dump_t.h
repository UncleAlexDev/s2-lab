/**
  \file   dpt_dump_t.h
  \brief  Structure of Dual Pressure Dump file
  \author Aleksey Lekhtin
  \date   06.07.2026
*/

#ifndef DP_DUMP_T_H
#define DP_DUMP_T_H

#include <array>
#include <cstdint>
#include <vector>

struct dpdump_t
{
public:
    static constexpr uint8_t DP_DUMP_CHECK_BYTE = 0x7A;
    static inline std::array<char, 5> DP_SIGN = {'r', 'a', 'w', 'd', 'p'};
    static inline uint32_t DP_VERSION1 = 0x0001;

#pragma pack(push, 1)

    typedef struct file_hdr_t
    {
        std::array<char, 5> sign{}; // Сигнатура
        uint32_t version{};         // Версия дампа
        uint64_t date;              // Время создания файла, ms (from 1970-01-01)
    } file_hdr_t;

    typedef union ctrl_word_t
    {
        struct
        {
            uint32_t check_byte: 8; // DP_DUMP_CHECK_BYTE - 7A
            uint32_t unused : 24;  // unused now
        };
        uint32_t dword;
    } ctrl_word_t;

    typedef struct dump_data_block_header_t
    {
        ctrl_word_t ctrlw{{DP_DUMP_CHECK_BYTE, {}}};
        uint16_t block_number{};
        uint64_t timestamp{}; // ms
        int32_t encoder1{};
        int32_t encoder2{};
        uint16_t npnts{};
        uint64_t reserv{};
    } dump_data_block_header_t;

    typedef struct dump_data_block_t
    {
        dump_data_block_header_t header{};
        std::vector<float> pressure1{};
        std::vector<float> pressure2{};
        std::vector<float> hookload{};
    } dump_data_block_t;

    typedef struct dump_t
    {
        file_hdr_t file_header{};
        dump_data_block_t data{};
        // ... data1 ...
        //     .....
        // ... dataN ...

        inline void clear()
        {
            data.header = {};
            data.pressure1.resize(0);
            data.pressure2.resize(0);
            data.hookload.resize(0);
        }
    } dump_t;
#pragma pack(pop)
};

#endif // DP_DUMP_T_H
