#include<arpa/inet.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<termios.h>
#include<unistd.h>
#define ring putchar('\a')
#define PORT 6971
#ifdef _WIN32
#define gch getch()
#else
#define GETCHR(filedes, var)read(filedes, &var, sizeof var)
#define PUTCHR(filedes, var)write(filedes, &var, sizeof var)
#endif
char rdcbuf[4];
int rdcbufcnt;
int connect_client(const char *host);
int rd(void);
void rdln(char *buf, size_t bufsz)
{
    --bufsz;
    int ch = rd();
    size_t ind = 0, sz = 0;
    while(ch != '\n')
    {
        switch(ch)
        {
            case 0177:
                if(ind > 0)
                {
                    memmove(buf + ind - 1, buf + ind, sz - ind);
                    --ind;
                    --sz;
                    putchar('\b');
                }
                else
                    ring;
                break;
            case 0x5b44:
                if(ind > 0)
                {
                    putchar('\b');
                    --ind;
                }
                else
                    ring;
                break;
            case 0x5b43:
                if(ind < sz)
                {
                    fputs("\033\133C", stdout);
                    ++ind;
                }
                else
                    ring;
                break;
            case 0x5b46:
                if(ind < sz)
                    printf("\033\133%zuC", sz - ind);
                ind = sz;
                break;
            case 0x5b48:
                if(ind > 0)
                    printf("\033\133%zuD", ind);
                ind = 0;
                break;
            case 0x5b337e:
                if(ind < sz)
                {
                    memmove(buf + ind, buf + ind + 1, sz - ind - 1);
                    --sz;
                }
                else
                    ring;
                break;
            default:
                if(ch > 126 || ch < 32)
                    ring;
                else
                {
                    if(sz < bufsz)
                    {
                        memmove(buf + ind + 1, buf + ind, sz - ind);
                        buf[ind] = ch;
                        ++ind;
                        ++sz;
                        fputs("\033\133C", stdout);
                    }
                    else
                        ring;
                }
        }
        for(size_t i = 0; i < ind; i++)
            putchar('\b');
        for(size_t i = 0; i <= sz; i++)
            putchar(' ');
        for(size_t i = 0; i <= sz; i++)
            putchar('\b');
        fwrite(buf, 1, sz, stdout);
        for(size_t i = ind; i < sz; i++)
            putchar('\b');
        ch = rd();
    }
    printf("\033\133%zuD", ind);
    buf[sz] = '\0';
}
int main(int argl, char *argv[])
{
    puts("Welcome to type racing!");
    setvbuf(stdout, NULL, _IONBF, 0);
#ifdef _WIN32
#else
    struct termios old, curr;
    tcgetattr(STDIN_FILENO, &old);
    memcpy(&curr, &old, sizeof(struct termios));
    curr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &curr);
#endif
    const char *host = argv[1];
    if(host == NULL)
    {
        char conmsg[] = "Enter host name of server you wish to connect to: ";
        fputs(conmsg, stdout);
        char hbuf[121];
        int sock = -1;
        while(sock == -1)
        {
            rdln(hbuf, sizeof hbuf);
            host = hbuf;
            sock = connect_client(host);
            if(sock == -1)
                printf("\n\033\13331mCould not connect, put in a different host name, check spelling.\033\133F\033\133%zuC\033\133m", sizeof(conmsg) - 1);
        }
        puts(host);
        char name[60], oname[60];
        fputs("Enter your name: ", stdout);
        rdln(name, sizeof name);
        char msgt = strlen(name);
        PUTCHR(sock, msgt);
        write(sock, name, msgt);
        uint32_t trackn = 0;
        PUTCHR(sock, trackn);
        GETCHR(sock, msgt);
        if(msgt == 19)
        {
            GETCHR(sock, msgt);
            while(msgt == 31 && msgt != 19)
            {
                if(msgt == 37)
                {
                    GETCHR(sock, msgt);
                    read(sock, oname, msgt);
                    oname[msgt] = '\0';
                    printf("%s has entered the race.\n", oname);
                }
                GETCHR(sock, msgt);
            }
            if(msgt == 31)
                close(sock);
            else
            {
                fputs("Game is beginning in 3", stdout);
            }
        }
        else
            puts("Failed to enter room");
    }
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
    return 0;
}
int connect_client(const char *host)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock > 0)
    {
        struct sockaddr_in sai, *saip = &sai;
        sai.sin_family = AF_INET;
        sai.sin_port = htons(PORT);
        inet_aton(host, &sai.sin_addr);
        int succ = connect(sock, (struct sockaddr *)saip, sizeof sai);
        if(succ != 0)
            sock = -1;
        else
        {
        }
    }
    return sock;
}
int rd(void)
{
    int ch;
    char cbuf[4];
#ifdef _WIN32
#else
    if(rdcbufcnt != 0)
    {
        ch = rdcbuf[0];
        rdcbuf[0] = rdcbuf[1];
        rdcbuf[1] = rdcbuf[2];
        rdcbuf[2] = rdcbuf[3];
        --rdcbufcnt;
    }
    else
    {
        int bc = read(STDIN_FILENO, cbuf, sizeof cbuf);
        if(bc == 1)
            ch = cbuf[0];
        else if(cbuf[0] == 033)
        {
            ch = 0;
            for(int i = 1; i < bc; i++)
            {
                ch <<= 8;
                ch += cbuf[i];
            }
        }
        else
        {
            ch = cbuf[0];
            memcpy(rdcbuf, cbuf + 1, bc - 1);
            rdcbufcnt = bc - 1;
        }
    }
#endif
    return ch;
}
