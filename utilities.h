#pragma once

#define KILOBYTE(x) (x * 1024)
#define MEGABYTE(x) (x * KILOBYTE(1024))
#define GIGABYTE(x) (x * MEGABYTE(1024))

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;
typedef float f32;
typedef double f64;