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

static void concatenateCharacterStrings(s8* s1, s8* s2, u32* ctr){
    s8* c = s2;
    u32 index = *ctr;
    while(*c != '\0'){
        s1[index++] = *c;
        c++;
    }

    s1[index] = '\0';
    *ctr = index;
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
                concatenateCharacterStrings(buffer, tbuf, &ctr);
            }else if(*c == 'u'){
                u32 v = va_arg(argptr, u32);
                u32ToCharacterArray(v, tbuf);
                concatenateCharacterStrings(buffer, tbuf, &ctr);
            }else if(*c == 'f'){
                s8 d = *(c + 1);
                if(d >= '0' && d <= '9'){
                    c++;
                    f64 v = va_arg(argptr, f64);
                    f32ToCharacterArray(v, tbuf, d - '0');
                    concatenateCharacterStrings(buffer, tbuf, &ctr);
                }else{
                    f64 v = va_arg(argptr, f64);
                    f32ToCharacterArray(v, tbuf);
                    concatenateCharacterStrings(buffer, tbuf, &ctr);
                }
            }
            else if(*c == 'b'){
                s32 v = va_arg(argptr, s32);
                if(v){
                    concatenateCharacterStrings(buffer, "true", &ctr);
                }else{
                    concatenateCharacterStrings(buffer, "false", &ctr);
                }
            }
            else if(*c == 'q'){
                c++;
                f32* vp = va_arg(argptr, f32*);
                f32ToCharacterArray(vp[0], tbuf);
                concatenateCharacterStrings(buffer, tbuf, &ctr);
                concatenateCharacterStrings(buffer, ", ", &ctr);
                f32ToCharacterArray(vp[1], tbuf);
                concatenateCharacterStrings(buffer, tbuf, &ctr);
                concatenateCharacterStrings(buffer, ", ", &ctr);
                f32ToCharacterArray(vp[2], tbuf);
                concatenateCharacterStrings(buffer, tbuf, &ctr);
                concatenateCharacterStrings(buffer, ", ", &ctr);
                f32ToCharacterArray(vp[3], tbuf);
                concatenateCharacterStrings(buffer, tbuf, &ctr);
                break;
            }
            else if(*c == 'v'){
                c++;
                switch(*c){
                    case '2':{
                        c++;
                        f32* vp = va_arg(argptr, f32*);
                        f32ToCharacterArray(vp[0], tbuf);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        concatenateCharacterStrings(buffer, ", ", &ctr);
                        f32ToCharacterArray(vp[1], tbuf);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        break;
                    }
                    case '3':{
                        c++;
                        f32* vp = va_arg(argptr, f32*);
                        f32ToCharacterArray(vp[0], tbuf);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        concatenateCharacterStrings(buffer, ", ", &ctr);
                        f32ToCharacterArray(vp[1], tbuf);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        concatenateCharacterStrings(buffer, ", ", &ctr);
                        f32ToCharacterArray(vp[2], tbuf);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        break;
                    }
                    case '4':{
                        c++;
                        f32* vp = va_arg(argptr, f32*);
                        f32ToCharacterArray(vp[0], tbuf);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        concatenateCharacterStrings(buffer, ", ", &ctr);
                        f32ToCharacterArray(vp[1], tbuf);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        concatenateCharacterStrings(buffer, ", ", &ctr);
                        f32ToCharacterArray(vp[2], tbuf);
                        concatenateCharacterStrings(buffer, ", ", &ctr);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        f32ToCharacterArray(vp[3], tbuf);
                        concatenateCharacterStrings(buffer, tbuf, &ctr);
                        break;
                    }
                }
            }
        }else{
            buffer[ctr++] = *c;
        }

        c++;
    }

    buffer[ctr] = '\0';
}