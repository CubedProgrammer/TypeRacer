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
#define gch getchar()
#endif
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
        fputs("Enter host name of server you wish to connect to: ", stdout);
        char hbuf[121];
        rdln(hbuf, sizeof hbuf);
        puts(hbuf);
        host = hbuf;
    }
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
    return 0;
}
int rd(void)
{
    int ch;
    char cbuf[4];
#ifdef _WIN32
#else
    int bc = read(STDIN_FILENO, cbuf, sizeof cbuf);
    if(cbuf[0] == 033)
    {
        ch = 0;
        for(int i = 1; i < bc; i++)
        {
            ch <<= 8;
            ch += cbuf[i];
        }
    }
    else
        ch = cbuf[0];
#endif
    return ch;
}
