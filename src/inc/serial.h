#ifndef SERIAL_H
#define SERIAL_H
int init_serial();
int serial_received();
char read_serial();
void write_serial(char a);
void write_serialString(const char *c);
void write_serialHex(int u);
void write_serialInt(int u);
#endif
