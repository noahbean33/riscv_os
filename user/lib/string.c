#include "string.h"

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

size_t strnlen(const char *s, size_t maxlen) {
    size_t len = 0;
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    return len;
}

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dst;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    // Kopy a maximum of n characters from src to dest
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    // Fill the remainder of dest (up to n) with null bytes
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;  // empty needle → match at beginning

    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && *h == *n) {
            h++;
            n++;
        }

        if (!*n) return (char *)haystack;  // whole needle matched
    }

    return NULL;  // not found
}

char *strcat(char *dest, const char *src) {
    char *p = dest;
    while (*p) p++;           // Go to the end of this
    while (*src) *p++ = *src++; // Copy src to dest
    *p = '\0';                // Add trailing zero
    return dest;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}

char* strtok(char* str, const char* delim) {
    static char* saved;
    if (str) saved = str;
    if (!saved) return NULL;

    // Skip any leading delimiters
    while (*saved && strchr(delim, *saved)) saved++;
    if (*saved == '\0') return NULL;

    char* token = saved;

    // search end of token
    while (*saved && !strchr(delim, *saved)) saved++;

    if (*saved) {
        *saved = '\0';   // Terminating the token
        saved++;         // Continue here next time
    } else {
        saved = NULL;
    }

    return token;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void *memset(void *dest, int val, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    for (size_t i = 0; i < n; i++) {
        d[i] = (unsigned char)val;
    }
    return dest;
}