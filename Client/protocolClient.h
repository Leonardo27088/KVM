#pragma once

#include <stdint.h>

#define PORT 27015
#define DEFAULT_BUFLEN 512

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    float normX;
    float normY;
    uint16_t code;
    int32_t value;
} MousePacket;
#pragma pack(pop)

MousePacket packet;

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    uint32_t code;
    int32_t value;
} KeyPacket;
#pragma pack(pop)