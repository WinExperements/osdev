#ifndef MESSAGE_H
#define MESSAGE_H
typedef struct {
    int type;
    int len;
    void *message;
} msg_t;
int msg_send(int pid,int type,void *message);
msg_t msg_receive(int pid);
#endif
