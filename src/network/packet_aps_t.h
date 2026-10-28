/**
  \file   packet_aps_t.h
  \author Aleksey Lekhtin
  \date   Dec 10, 2024
  \brief  The SIU packet structure
*/

#ifndef PACKET_STRUCT_SIU_H_
#define PACKET_STRUCT_SIU_H_

#include <cstdint>
#include <sys/types.h>

struct packet_aps_t
{
    uint8_t sign[3];
    uint8_t msg60;
    uint8_t sensor;
    uint32_t timestamp_sec; // абсолютное время
    uint32_t timestamp_us;  // микросекунды абсолютного времени
    uint32_t filenum;       // номер архивного файла
    uint32_t sample_num;    // номер первого сэмпла (step:100)
    int32_t pnts[100];      // точки измерений
    static constexpr size_t npnts = sizeof(pnts) / sizeof(pnts[0]);
};

#endif // PACKET_STRUCT_SIU_H_
