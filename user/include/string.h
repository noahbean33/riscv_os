#pragma once

#include <stddef.h>

size_t strlen(const char* str);
size_t strnlen(const char *s, size_t maxlen);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, unsigned int n);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strstr(const char *haystack, const char *needle);
char *strcat(char *dest, const char *src);
char* strtok(char* str, const char* delim);
char* strchr(const char* s, int c);

void *memset(void *dest, int value, unsigned long size);
void *memcpy(void *dest, const void *src, unsigned long size);