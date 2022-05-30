#include<arpa/inet.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/socket.h>
#include<unistd.h>
#include"logging.h"
#include"race.h"
#define PORT 6971
#ifndef LOGFILE
#define LOGFILE "logs.txt"
#endif
#define GETCHR(filedes, var)read(filedes, &var, sizeof var)
#define PUTCHR(filedes, var)write(filedes, &var, sizeof var)
union sockaddru
{
    struct sockaddr_in sai;
    struct sockaddr sa;
};
struct accept_client_arg
{
    union sockaddru sau;
    socklen_t slen;
};
void *handle_client(void *arg)
{
    int cfd = *(int*)arg;
    char name[60];
    char namlen;
    GETCHR(cfd, namlen);
    if(namlen > 59)
        namlen = 59;
    namlen = read(cfd, name, namlen);
    name[namlen] = '\0';
    log_printf("Player %s has connected\n", name);
    uint32_t track;
    GETCHR(cfd, track);
    int succ;
    if(track == 0)
    {
    }
    else
    {
        track = ntohl(track);
        succ = racetrack_join(track, cfd, name);
        if(succ == 0)
        {
        }
        else
            log_printf("Player %s could not join room %u\n", name, track);
    }
    return NULL;
}
void *accept_clients(void *arg)
{
    struct accept_client_arg *acap = arg, aca = *acap;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
        fputs("Socket creation failed\n", stderr);
    else
    {
        aca.sau.sai.sin_family = AF_INET;
        aca.sau.sai.sin_addr.s_addr = INADDR_ANY;
        aca.sau.sai.sin_port = htons(PORT);
        int succ = bind(s, &aca.sau.sa, aca.slen);
        if(succ == 0)
        {
            succ = listen(s, 3);
            if(succ == 0)
            {
                puts("Listening for connections now");
                pthread_t pth;
                int cfd;
                for(;;)
                {
                    cfd = accept(s, &aca.sau.sa, &aca.slen);
                    pthread_create(&pth, NULL, handle_client, &cfd);
                }
            }
            else
                fputs("Could not listen for connections\n", stderr);
        }
        else
            fputs("Binding socket failed\n", stderr);
    }
    return NULL;
}
int main(int argl, char *argv[])
{
    puts("TypeRacer");
    int succ = init_logger(LOGFILE);
    if(succ == 0)
    {
        union sockaddru sau;
        socklen_t slen = sizeof(struct sockaddr_in);
        struct accept_client_arg aca;
        aca.sau = sau;
        aca.slen = slen;
        pthread_t pth;
        pthread_create(&pth, NULL, accept_clients, &aca);
        getchar();
        end_logging();
    }
    return succ;
}
