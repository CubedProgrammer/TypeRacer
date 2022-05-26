#include<arpa/inet.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#define PORT 6971
int main(int argl, char *argv[])
{
    puts("Welcome to type racing!");
    const char *host = argv[1];
    if(host == NULL)
    {
        fputs("Enter host name of server you wish to connect to: ", stdout);
        char hbuf[121];
    }
    return 0;
}
