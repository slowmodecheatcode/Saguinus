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

static s32 binarySearch(u16* list, u16 value, u32 start, u32 end, s32 notFoundReturnValue = -1){
    while(end >= start){
        u32 mid = start + ((end - start) / 2);
        if(list[mid] == value){
            return mid;
        }
        if(value < list[mid]){
            end = mid - 1;
        }else{
            start = mid + 1;
        }
    }
    return notFoundReturnValue;
}

static void u32ToCharacterArray(u32 num, s8* buffer){
    u32 ctr = 0;
    u32 ctr2 = 0;

    s8 tbuf[32];
    while(num > 0){
        u32 digit = num % 10;
        tbuf[ctr++] = digit + '0';
        num /= 10;
    }
    for(s32 i = ctr - 1; i >= 0; i--){
        buffer[ctr2++] = tbuf[i];
    }

    buffer[ctr2] = '\0';
}

static void s32ToCharacterArray(s32 num, s8* buffer){
    u32 ctr = 0;
    u32 ctr2 = 0;
    if(num < 0){
        buffer[ctr2++] = '-';
        num = -num;
    }

    s8 tbuf[32];
    while(num > 0){
        u32 digit = num % 10;
        tbuf[ctr++] = digit + '0';
        num /= 10;
    }
    for(s32 i = ctr - 1; i >= 0; i--){
        buffer[ctr2++] = tbuf[i];
    }

    buffer[ctr2] = '\0';
}

static void f32ToCharacterArray(f32 num, s8* buffer, u32 precision = 2){
    u32 exp = 1;
    for(u32 i= 0; i < precision; i++){
        exp *= 10;
    }

    s32 intValue = (s32)num;
    f32 decimalValue = num - intValue;
    s32 decimalAsInt = (u32)(decimalValue * exp);
    s8 preDecimal[16];
    s8 postDecimal[16];

    s32ToCharacterArray(intValue, preDecimal);
    u32ToCharacterArray(decimalAsInt, postDecimal);

    u32 ctr = 0;
    s8* c = preDecimal;
    while(*c != '\0'){
        buffer[ctr++] = *c;
        c++;
    }
    buffer[ctr++] = '.';
    c = postDecimal;
    while(*c != '\0'){
        buffer[ctr++] = *c;
        c++;
    }
    buffer[ctr] = '\0';
}