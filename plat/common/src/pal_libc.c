/*
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"

void *memcpy(void *dst, const void *src, size_t len);
void *memset(void *dst, int val, size_t count);
int memcmp(void *s1, void *s2, size_t len);
void *memmove(void *dst, const void *src, size_t len);
size_t strlen(char *start);

int pal_memcmp(void *src, void *dest, size_t len)
{
  return memcmp(src, dest, len);
}

void *pal_memcpy(void *dst, const void *src, size_t len)
{
  return memcpy(dst, src, len);
}

void *pal_memset(void *dst, int val, size_t count)
{
  memset(dst, val, count);
  return dst;
}

size_t pal_strlen(char *str)
{
  return strlen(str);
}

/* Libc functions definition */

void *memcpy(void *dst, const void *src, size_t len)
{
    const char *s = src;
    char *d = dst;

    while (len--)
    {
        *d++ = *s++;
    }

    return dst;
}

void *memset(void *dst, int val, size_t count)
{
    unsigned char *ptr = dst;

    while (count--)
    {
        *ptr++ = (unsigned char)val;
    }

    return dst;
}

int memcmp(void *s1, void *s2, size_t len)
{
    unsigned char *s = s1;
    unsigned char *d = s2;
    unsigned char sc;
    unsigned char dc;

    while (len--)
    {
        sc = *s++;
        dc = *d++;
        if (sc - dc)
            return (sc - dc);
    }

    return 0;
}

void *memmove(void *dst, const void *src, size_t len)
{
        if ((size_t)dst - (size_t)src >= len) {
                /* destination not in source data, so can safely use memcpy */
                return memcpy(dst, src, len);
        } else {
                /* copy backwards... */
                const char *end = dst;
                const char *s = (const char *)src + len;
                char *d = (char *)dst + len;
                while (d != end)
                        *--d = *--s;
        }
        return dst;
}

size_t strlen(char *str)
{
   size_t length = 0;

  while (str[length] != '\0')
  {
    ++length;
  }
  return length;
}

// Reference: https://velog.io/@bakukun/%EA%B8%B0%EB%B3%B8-%EB%9D%BC%EC%9D%B4%EB%B8%8C%EB%9F%AC%EB%A6%AC%EB%A1%9C-C%EC%96%B8%EC%96%B4-strtok%ED%95%A8%EC%88%98-%EA%B5%AC%ED%98%84%ED%95%98%EA%B8%B0
char *pal_strtok(char *str,char *deli){	
	size_t i;
	size_t deli_len = pal_strlen(deli);
	static char* tmp = NULL;
	
	if(str != NULL && !tmp){
		tmp = str;
	}
	
	if(str == NULL && !tmp){
		return NULL;
	}
	
	char* idx = tmp;
	while(true){
		for(i = 0; i < deli_len; i++) { 
            if(*idx == deli[i]) { 
                idx++; 
                break;
            }
		}
		
		if(i == deli_len) {
            tmp = idx;
            break;
        }
    }
    				
    if(*tmp == '\0') {
        tmp = NULL;
        return tmp;
    }
 
    while(*tmp != '\0') {
        for(i = 0; i < deli_len; i++) {
            if(*tmp == deli[i]) {
                *tmp = '\0';
                break;
            }
        }
 
        tmp++;
        if (i < deli_len){
            break;
		}
    }
    return idx;
}

//Reference: https://www.techiedelight.com/ko/implement-strcpy-function-c/
char* pal_strcpy(char* destination, const char* source)
{
    if (destination == NULL) {
        return NULL;
    }
 
    char *ptr = destination;
 
    while (*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }
 
    *destination = '\0';
 
    return ptr;
}
