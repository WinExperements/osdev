#ifndef STDIO_H
#define STDIO_H
#define NULL 0
typedef enum {false,true} bool;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef void FILE;
typedef unsigned int size_t;
void printf(char *message,...);
FILE *fopen(char *file,char *mode);
int fread(void *buff,int size,int count,FILE *file);
int fclose(FILE *file);
int fwrite(char *,int size,int how,FILE *f);
int fread(void *buff,int size,int how,FILE *f);
int fclose(FILE *);
#endif
