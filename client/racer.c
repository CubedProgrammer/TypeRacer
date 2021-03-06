// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

#ifndef _WIN32
#include<arpa/inet.h>
#include<netdb.h>
#include<pthread.h>
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#ifndef _WIN32
#include<sys/ioctl.h>
#include<sys/select.h>
#include<sys/socket.h>
#include<termios.h>
#endif

#include<time.h>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#include<winsock2.h>
#include<windows.h>
#else
#include<unistd.h>
#endif

#include"rd.h"
#include"typing.h"

#define PORT 6971
#define BARLEN (cols - maxnamlen - 2)
#define MAXBARLEN 240
#define MIN_COLS 70

#ifdef _WIN32
#define gch getch()
#define mssleep(ms)Sleep(ms)
#define GETCHR(filedes, var)recv(filedes, &var, sizeof(var), 0);
#define PUTCHR(filedes, var)send(filedes, &var, sizeof(var), 0);
#else
#define GETCHR(filedes, var)read(filedes, &var, sizeof var)
#define PUTCHR(filedes, var)write(filedes, &var, sizeof var)
#define mssleep(ms)usleep((ms) * 1000)
#endif

int connect_client(const char *host);
void *await_begin(void *arg);
int term_width(void)
{
#ifdef _WIN32
    HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO conbuf;
    GetConsoleScreenBufferInfo(hand, &conbuf);
    return conbuf.srWindow.X;
#else
    struct winsize sz;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &sz);
    return sz.ws_col;
#endif
}
int main(int argl, char *argv[])
{
    puts("Welcome to type racing!");
    puts("In this game, you will race against others to see who can type a paragraph the fastest.");
    setvbuf(stdout, NULL, _IONBF, 0);
#ifdef _WIN32
    // Microsoft socket data
    WSADATA mssd;
    if(WSAStartup(MAKEWORD(2, 2), &mssd))
    {
        puts("WSAStartup failed miserably.");
        return-1;
    }
    HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD cm;
    GetConsoleMode(hand, &cm);
    cm |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hand, cm);
#else
    struct termios old, curr;
    tcgetattr(STDIN_FILENO, &old);
    memcpy(&curr, &old, sizeof(struct termios));
    curr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &curr);
#endif
    int succ = 0;
    if(term_width() < MIN_COLS)
    {
        succ = 1;
        fprintf(stderr, "\033\13331mTerminal width must be greater than %i characters.\033\133m\n", MIN_COLS - 1);
        goto end;
    }
    const char *host = argv[1];
    char paragraph[5041];
    char utbuf[5041];
#ifdef _WIN32
    SOCKET sock = INVALID_SOCKET;
#else
    int sock = -1;
#endif
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
#ifdef _WIN32
        if(sock == INVALID_SOCKET)
#else
        if(sock == -1)
#endif
            goto nohost;
    }
    puts(host);
    char name[60];
    char oname[3600];
    fputs("Enter your name: ", stdout);
    rdln(name, sizeof name);
    printf("\nYour name is %s.\n", name);
    char unsigned msgt = strlen(name);
    PUTCHR(sock, msgt);
    write(sock, name, msgt);
    size_t wc;
    uint32_t trackn;
    char trackbuf[9];
    char progbar[MAXBARLEN + 1];
    char spacebars[MAXBARLEN];
    memset(spacebars, ' ', MAXBARLEN);
    struct timeval tv, *tvp = &tv;
    fd_set fds, *fdsp = &fds;
    int ready;
    play:
    memset(utbuf, 0, sizeof utbuf);
    puts("Enter the room number to join, or zero to create a room");
    rdln(trackbuf, sizeof trackbuf);
    trackn = strtoul(trackbuf, NULL, 16);
    trackn = htonl(trackn);
    PUTCHR(sock, trackn);
    if(trackn == 0)
    {
        msgt = 31;
        while(msgt != 53)
        {
            FD_ZERO(fdsp);
            FD_SET(sock, fdsp);
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            ready = select(sock + 1, fdsp, NULL, NULL, tvp);
            if(ready)
                GETCHR(sock, msgt);
            else
                puts("Server took too long to respond.");
        }
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
#ifdef _WIN32
#else
            pthread_t beginth;
            pthread_create(&beginth, NULL, await_begin, &sock);
#endif
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
                read(sock, oname + 60 * plcnt, msgt);
                oname[60 * plcnt + msgt] = '\0';
                if(msgt > maxnamlen)
                    maxnamlen = msgt;
                printf("\033\1331;33m%s\033\133m has entered the race.\n", oname + 60 * plcnt);
                ++plcnt;
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
            fputs("Game is beginning in \033\13331m3\033\133m                    \033\13320D", stdout);
            mssleep(997);
            fputs("\033\13331m\b2\033\133m", stdout);
            mssleep(997);
            fputs("\033\13333m\b1\033\133m", stdout);
            GETCHR(sock, msgt);
            uint16_t plen;
            if(msgt == 19)
            {
                puts("\033\13332m\b0\033\133m");
                if(trackn != 0)
                    PUTCHR(sock, msgt);
                GETCHR(sock, msgt);
                if(msgt == 41)
                {
                    GETCHR(sock, plen);
                    plen = ntohs(plen);
                    read(sock, paragraph, plen);
                    paragraph[plen] = '\0';
                    wc = 1;
                    for(const char *it = paragraph; *it != '\0'; ++it)
                        wc += *it == ' ';
                    struct typebuf tbuf;
                    tbuf.cbuf = utbuf;
                    utbuf[0] = '\0';
                    tbuf.sz = sizeof utbuf;
                    tbuf.para = paragraph;
                    tbuf.plen = plen;
                    tbuf.ccnt = 0;
                    fputs("\033\1333E", stdout);
                    for(size_t i = 0; i < plcnt; ++i)
                    {
                        fwrite(spacebars, 1, maxnamlen + 1, stdout);
                        putchar('\r');
                        fputs(oname + i * 60, stdout);
                        putchar('\n');
                    }
                    printf("\033\133%zuF", plcnt + 3);
#ifdef _WIN32
#else
                    pthread_t pth;
                    pthread_create(&pth, NULL, type_race, &tbuf);
#endif
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
                        printf("\033\1331m%d:%02d\033\133m            \033\13312D\n", tdiff / 60, tdiff % 60);
                        if(!finished)
                        {
                            cols = term_width();
                            utlen = strlen(utbuf);
                            if(utbuf[utlen - 1] == 030)
                                quit = 1;
                            paraoff = utlen - utlen % cols;
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
                                fwrite(paragraph + paraoff, 1, correct - paraoff, stdout);
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
                                printf("\r\033\1331mYou finished with %lis. Accuracy: %.1f%%. Speed: %.1f words/min.", end - curr, plen * 100.0 / tbuf.ccnt, wc * 60.0 / (curr + 60 - end));
                                fwrite(spacebars, 1, cols - 70, stdout);
                                putchar('\r');
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
                        ready = select(sock + 1, fdsp, NULL, NULL, tvp);
                        if(ready)
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
                        if(finished)
                        {
                            tv.tv_sec = 0;
                            tv.tv_usec = 800;
                            FD_ZERO(fdsp);
                            FD_SET(STDIN_FILENO, fdsp);
                            ready = select(STDIN_FILENO + 1, fdsp, NULL, NULL, &tv);
                            if(ready && rd() == 030)
                                quit = 1;
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
    puts("\033\1331mPress q to quit, any other key to play again.\033\133m        ");
    msgt = 31;
    if(rd() != 'q')
    {
        msgt = 47;
        PUTCHR(sock, msgt);
        goto play;
    }
    PUTCHR(sock, msgt);
    close(sock);
    end:
#ifdef _WIN32
    WSACleanup();
#else
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
    puts("Thank you for playing.");
    return succ;
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
#ifdef _WIN32
SOCKET
#else
int
#endif
connect_client(const char *host)
{
#ifdef _WIN32
    SOCKET sock
#else
    int sock
#endif
    = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if(sock != INVALID_SOCKET)
#else
    if(sock > 0)
#endif
    {
        struct sockaddr_in sai, *saip = &sai;
        sai.sin_family = AF_INET;
        sai.sin_port = htons(PORT);
        inet_aton(host, &sai.sin_addr);
        int succ = connect(sock, (struct sockaddr *)saip, sizeof sai);
#ifdef _WIN32
        if(succ == SOCKET_ERROR)
            sock = INVALID_SOCKET;
#else
        if(succ != 0)
            sock = -1;
#endif
        else
        {
        }
    }
    return sock;
}
