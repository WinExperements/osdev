#ifndef MSTRING_H
#define MSTRING_H
#include <typedefs.h>
void memcpy(void *vd, const void *vs, unsigned length);
bool strcmp(char *s1,char *s2);
bool strcpy(char *to,char *from);
const char *strchr(const char *s, char ch);
char *strtok(char *s, const char *delim);
char *strdup(char *src);
#endif