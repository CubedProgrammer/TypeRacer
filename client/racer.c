// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

#include<arpa/inet.h>
#include<netdb.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/ioctl.h>
#include<sys/select.h>
#include<sys/socket.h>
#include<termios.h>
#include<time.h>
#include<unistd.h>
#include"rd.h"
#include"typing.h"
#define PORT 6971
#define BARLEN (cols - maxnamlen - 2)
#define MAXBARLEN 240
#ifdef _WIN32
#define gch getch()
#define mssleep(ms)Sleep(ms)
#else
#define GETCHR(filedes, var)read(filedes, &var, sizeof var)
#define PUTCHR(filedes, var)write(filedes, &var, sizeof var)
#define mssleep(ms)usleep((ms) * 1000)
#endif
int connect_client(const char *host);
void *await_begin(void *arg);
int term_width(void)
{
    struct winsize sz;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &sz);
    return sz.ws_col;
}
int main(int argl, char *argv[])
{
    puts("Welcome to type racing!");
    puts("In this game, you will race against others to see who can type a paragraph the fastest.");
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
    char paragraph[5041];
    char utbuf[5041];
    memset(utbuf, 0, sizeof utbuf);
    int sock = -1;
    char conmsg[] = "Enter host name of server you wish to connect to: ";
    if(host == NULL)
    {
        nohost:
        fputs(conmsg, stdout);
        char hbuf[121];
        while(sock == -1)
        {
            rdln(hbuf, sizeof hbuf);
            host = hbuf;
            sock = connect_client(host);
            if(sock == -1)
                printf("\033\133F\033\13331mCould not connect, put in a different host name, check spelling.\n\033\133%zuC\033\133m", sizeof(conmsg) - 1);
        }
    }
    else
    {
        sock = connect_client(host);
        if(sock == -1)
            goto nohost;
    }
    puts(host);
    char name[60], oname[60];
    fputs("Enter your name: ", stdout);
    rdln(name, sizeof name);
    printf("\nYour name is %s.\n", name);
    char unsigned msgt = strlen(name);
    PUTCHR(sock, msgt);
    write(sock, name, msgt);
    uint32_t trackn;
    char trackbuf[9];
    char progbar[MAXBARLEN + 1];
    char spacebars[MAXBARLEN];
    memset(spacebars, ' ', MAXBARLEN);
    struct timeval tv, *tvp = &tv;
    fd_set fds, *fdsp = &fds;
    play:
    puts("Enter the room number to join, or zero to create a room");
    rdln(trackbuf, sizeof trackbuf);
    trackn = strtoul(trackbuf, NULL, 16);
    trackn = htonl(trackn);
    PUTCHR(sock, trackn);
    if(trackn == 0)
    {
        GETCHR(sock, trackn);
        trackn = ntohl(trackn);
        printf("Room is %08x\n", trackn);
        trackn = 0;
    }
    GETCHR(sock, msgt);
    if(msgt == 19)
    {
        if(trackn == 0)
        {
            puts("Press any key to begin the game...");
            pthread_t beginth;
            pthread_create(&beginth, NULL, await_begin, &sock);
        }
        GETCHR(sock, msgt);
        int cols;
        uint16_t prog;
        size_t plcnt = 0, maxnamlen = 0, correct;
        while(msgt != 31 && msgt != 19)
        {
            if(msgt == 37)
            {
                GETCHR(sock, msgt);
                read(sock, oname, msgt);
                oname[msgt] = '\0';
                if(msgt > maxnamlen)
                    maxnamlen = msgt;
                ++plcnt;
                printf("%s has entered the race.\n", oname);
            }
            GETCHR(sock, msgt);
        }
        if(msgt == 31)
        {
            close(sock);
            puts("Server asked to disconnect");
        }
        else
        {
            printf("\033\133%zuF", plcnt + 4);
            fputs("Game is beginning in 3", stdout);
            mssleep(997);
            fputs("\b2", stdout);
            mssleep(997);
            fputs("\b1", stdout);
            GETCHR(sock, msgt);
            uint16_t plen;
            if(msgt == 19)
            {
                puts("\b0");
                if(trackn != 0)
                    PUTCHR(sock, msgt);
                GETCHR(sock, msgt);
                if(msgt == 41)
                {
                    GETCHR(sock, plen);
                    plen = ntohs(plen);
                    read(sock, paragraph, plen);
                    paragraph[plen] = '\0';
                    struct typebuf tbuf;
                    tbuf.cbuf = utbuf;
                    utbuf[0] = '\0';
                    tbuf.sz = sizeof utbuf;
                    tbuf.para = paragraph;
                    tbuf.plen = plen;
                    pthread_t pth;
                    pthread_create(&pth, NULL, type_race, &tbuf);
                    time_t curr = time(NULL), end = curr + 60;
                    int tdiff, ltdiff = 60;
                    const char *ita, *itb;
                    size_t paraoff, textlen, utlen;
                    int proglen;
                    char quit = 0;
                    char finished = 0;
                    for(; !quit && curr <= end; time(&curr))
                    {
                        tdiff = end - curr;
                        printf("%d:%02d\n", tdiff / 60, tdiff % 60);
                        if(!finished)
                        {
                            cols = term_width();
                            utlen = strlen(utbuf);
                            if(utbuf[utlen - 1] == 030)
                                quit = 1;
                            paraoff = utlen - utlen % cols;
                            if(paraoff && paraoff == utlen)
                                paraoff -= cols;
                            for(ita = paragraph, itb = utbuf; *ita != '\0' && *ita == *itb; ++ita, ++itb);
                            correct = ita - paragraph;
                            textlen = plen - paraoff;
                            if(textlen > cols)
                                textlen = cols;
                            fwrite(paragraph + paraoff, 1, textlen, stdout);
                            if(cols > textlen)
                                fwrite(spacebars, 1, cols - textlen, stdout);
                        }
                        putchar('\n');
                        if(!finished)
                        {
                            if(utbuf[0] == paragraph[0])
                                fputs("\033\13332m", stdout);
                            fwrite(spacebars, 1, cols, stdout);
                            putchar('\r');
                            if(correct > paraoff)
                            {
                                fwrite(paragraph + paraoff, 1, correct - paraoff, stdout);
                            }
                            if(tdiff <= ltdiff - 2)
                            {
                                ltdiff = tdiff;
                                prog = correct;
                                prog = htons(prog);
                                msgt = 23;
                                PUTCHR(sock, msgt);
                                PUTCHR(sock, prog);
                            }
                            if(*itb != '\0')
                            {
                                fputs("\033\13331m", stdout);
                                if(correct > paraoff)
                                    fwrite(itb, 1, utlen - correct, stdout);
                                else
                                    fwrite(utbuf + paraoff, 1, utlen - paraoff, stdout);
                            }
                            else if(*ita == '\0')
                            {
                                printf("\rCongradulations, you finished with %li seconds remaining.", end - curr);
                                prog = plen;
                                prog = htons(prog);
                                msgt = 23;
                                PUTCHR(sock, msgt);
                                PUTCHR(sock, prog);
                                finished = 1;
                            }
                            fputs("\033\133m", stdout);
                        }
                        mssleep(49);
                        tv.tv_sec = tv.tv_usec = 0;
                        FD_ZERO(fdsp);
                        FD_SET(sock, fdsp);
                        if(select(sock + 1, fdsp, NULL, NULL, tvp))
                        {
                            GETCHR(sock, msgt);
                            if(msgt == 29)
                            {
                                for(size_t i = 0; i < plcnt; ++i)
                                {
                                    GETCHR(sock, prog);
                                    prog = ntohs(prog);
                                    proglen = prog * BARLEN / plen;
                                    memset(progbar, '-', proglen);
                                    if(proglen < BARLEN)
                                    {
                                        memset(progbar + proglen, ' ', BARLEN - proglen);
                                        progbar[proglen] = '>';
                                    }
                                    progbar[BARLEN] = '\0';
                                    printf("\n\033\133%zuC%s|", maxnamlen + 1, progbar);
                                }
                                printf("\033\133%zuF", plcnt);
                            }
                        }
                        fputs("\033\1332F", stdout);
                    }
                    if(quit)
                    {
                        puts("You decided to forfeit the race");
                        msgt = 31;
                        PUTCHR(sock, msgt);
                    }
                    else if(!finished)
                        puts("Unfortunately, you ran out of time, keep practicing!");
                }
                else
                {
                    close(sock);
                    puts("Could not receive paragraph for typing.");
                }
            }
            else
            {
                close(sock);
                puts("Connection closed unexpectedtly.");
            }
        }
    }
    else
        puts("Failed to enter room");
    puts("Press q to quit, any other key to play again.");
    msgt = 31;
    if(rd() != 'q')
    {
        msgt = 47;
        PUTCHR(sock, msgt);
        goto play;
    }
    PUTCHR(sock, msgt);
    close(sock);
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
    return 0;
}
void *await_begin(void *arg)
{
    int *sockp = arg;
    int sock = *sockp;
    rd();
    char c = 19;
    PUTCHR(sock, c);
    return NULL;
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
