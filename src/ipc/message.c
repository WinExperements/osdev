/* Messages IPC */
#include<typedefs.h>
#include<terminal.h>
#include<mm/pmm.h>
#include <ipc/message.h>
#include <process.h>
#include <mstring.h>
int msg_send(int pid,int type,void *message) {
    struct process *p = process_getProcess(pid);
    if (!p) return 2;
    if (p->message_count > 10) {
        printf("Cannot send message: Limit\n");
        return 1;
    }
    msg_t msg;
    msg.type = type;
    msg.message = (void *)message;
    msg.len = strlen(message);
    p->messages[p->message_count++] = msg;
    //process_update(pid,p);
    return 0;
}
msg_t msg_receive(int pid) {
    struct process *p = process_getProcess(pid);
    if (!p) {
    	msg_t m;
	    return m;
    }
    msg_t msg = p->messages[p->message_count-1];
    p->message_count--;
    //process_update(pid,p);
    return msg;
}
