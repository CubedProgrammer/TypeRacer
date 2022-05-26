#include<arpa/inet.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#define PORT 6971
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
    union sockaddru sau;
    socklen_t slen = sizeof(struct sockaddr_in);
    struct accept_client_arg aca;
    aca.sau = sau;
    aca.slen = slen;
    pthread_t pth;
    pthread_create(&pth, NULL, accept_clients, &aca);
    getchar();
    return 0;
}
