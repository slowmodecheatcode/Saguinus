#pragma once

#include <stdarg.h>

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

    if(num == 0){
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
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

static void s32ToCharacterArray(s32 num, s8* buffer){
    u32 ctr = 0;
    u32 ctr2 = 0;
    if(num < 0){
        buffer[ctr2++] = '-';
        num = -num;
    }else if(num == 0){
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
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
    s32 intValue = (s32)num;
    f32 decimalValue = num - intValue;
    s8 preDecimal[16];
    s8 postDecimal[16];

    if(decimalValue < 0){
        decimalValue = -decimalValue;
        intValue = -intValue;
        preDecimal[0] = '-';
        u32ToCharacterArray(intValue, preDecimal + 1);
    }else{
        u32ToCharacterArray(intValue, preDecimal);
    }

    u32 ctr = 0;
    for(u32 i = 0; i < precision; i++){
        if(decimalValue == 0){
            postDecimal[ctr++] = '0';
        }else{
            decimalValue *= 10;
            s32 v = (s32)decimalValue;
            postDecimal[ctr++] = v + '0';
            decimalValue -= v;
        }
    }
    postDecimal[ctr] = '\0';

    ctr = 0;
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

static void createDebugString(s8* buffer, const s8* txt, va_list argptr){
    u32 ctr = 0;
    const s8* c = txt;
    s8 tbuf[32];
    while(*c != '\0'){
        if(*c == '%'){
            c++;
            if(*c == 'i'){
                s32 v = va_arg(argptr, s32);
                s32ToCharacterArray(v, tbuf);
                s8* tc = tbuf;
                while(*tc != '\0'){
                    buffer[ctr++] = *tc;
                    tc++;
                }
            }else if(*c == 'u'){
                u32 v = va_arg(argptr, u32);
                u32ToCharacterArray(v, tbuf);
                s8* tc = tbuf;
                while(*tc != '\0'){
                    buffer[ctr++] = *tc;
                    tc++;
                }
            }else if(*c == 'f'){
                f64 v = va_arg(argptr, f64);
                f32ToCharacterArray(v, tbuf);
                s8* tc = tbuf;
                while(*tc != '\0'){
                    buffer[ctr++] = *tc;
                    tc++;
                }
            }
            else if(*c == 'b'){
                s32 v = va_arg(argptr, s32);
                if(v){
                    buffer[ctr++] = 't';
                    buffer[ctr++] = 'r';
                    buffer[ctr++] = 'u';
                    buffer[ctr++] = 'e';
                }else{
                    buffer[ctr++] = 'f';
                    buffer[ctr++] = 'a';
                    buffer[ctr++] = 'l';
                    buffer[ctr++] = 's';
                    buffer[ctr++] = 'e';
                }
            }

        }else{
            buffer[ctr++] = *c;
        }

        c++;
    }

    buffer[ctr] = '\0';
}