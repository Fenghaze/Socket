#ifndef PROTO_H__
#define PROTO_H__
#include<stdint.h>
#define SERVERPORT      4567

struct DataPackage
{
    uint8_t name[32];
    uint8_t age;   /* data */
}__attribute__((packed));



#endif