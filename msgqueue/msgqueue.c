#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    int msgid;
    int status;
    char str1[ ]={"test message:hello!"};
    char str2[ ]={"test message:godbye!"};
    struct msgbuf
    {
        long msgtype;
        char msgtext[1024];
    }sndmsg, rcvmsg;

    if((msgid=msgget(IPC_PRIVATE,0666))==-1)
    {
        printf("msgget error!\n");
        exit(254);
    }
    sndmsg.msgtype = 111;
    sprintf(sndmsg.msgtext,"%s", str1);
    if(msgsnd(msgid,(struct msgbuf *)&sndmsg,sizeof(str1)+1,0)==-1)
    {
        printf("msgsnd error!\n");
        exit(254);
    }
    sndmsg.msgtype = 222;
    sprintf(sndmsg.msgtext, "%s", str2);
    if(msgsnd(msgid,(struct msgbuf *)&sndmsg,sizeof(str2)+1,0)==-1)
    {
        printf("msgsnd error!\n");
        exit(254);
    }
    if((status=msgrcv(msgid,(struct msgbuf *)&rcvmsg,80,222,IPC_NOWAIT))==-1)
    {
        printf("msg rcv error!\n");
        exit(254);
    }

    printf("The received message: %s.\n", rcvmsg.msgtext);
    //msgctl(msgid, IPC_RMID,0);
    exit(0);
}