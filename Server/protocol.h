#include <stdint.h>

#define PORT 27015

#define PKT_MOVE 1
#define PKT_BTN 2
#define PKT_WHEEL 3
#define PKT_SWIPE 4
#define PKT_KEY 5

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    float normX;
    float normY;
    uint16_t code;
    int32_t value;
} MousePacket;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    uint32_t code;
    int32_t value;
} KeyPacket;
#pragma pack(pop)