#ifndef PRINTK_H
#define PRINTK_H
#define BLACK         0
#define BLUE          1
#define GREEN         2
#define CYAN          3
#define RED           4
#define MAGENTA       5
#define BROWN         6
#define LIGHT_GREY    7
#define DARK_GREY     8
#define LIGHT_BLUE    9
#define LIGHT_GREEN   10
#define LIGHT_CYAN    11
#define LIGHT_RED     12
#define LIGHT_MAGNETA 13
#define LIGHT_BROWN   14
#define WHITE         15
#define PRINTK_INFO 1
#define PRINTK_ERR 2
void printk_init();
void printk_clear();
void printk(char *msg);
void printkDec(int num);
#endif