#ifndef __COMMON_INCLUDED__
#define __COMMON_INCLUDED__

#include <iostream>

union MyFloat
{
	uint8_t data_u8[4];
};

union MyU16
{
	uint8_t data_u8[2];
};

union MyU32
{
	uint8_t data_u8[4];
};

union MyU64
{
	uint8_t data_u8[8];
};

#define u8float(data)   (*((float *)data))
#define u8double(data)   (*((double *)data))
#define u8short(data)   (*((short *)data))
#define u8long(data)   (*((long *)data))
#define u8u16(data)   (*((uint16_t *)data))
#define u8u32(data)   (*((uint32_t *)data))
#define u8u64(data)   (*((uint64_t *)data))

#endif