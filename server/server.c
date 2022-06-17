// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

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
#define SLOPE 25214903917
#ifndef LOGFILE
#define LOGFILE "logs.txt"
#endif
#define GETCHR(filedes, var)read(filedes, &var, sizeof var)
#define PUTCHR(filedes, var)write(filedes, &var, sizeof var)
uint64_t java_util_Random_seed;
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
uint32_t nextui(void);
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
    time_t currt;
    char msgt;
    int succ;
    uint16_t prog;
    fd_set fds, *fdsp = &fds;
    struct timeval tv;
    int ready;
    if(track == 0)
    {
        track = nextui();
        succ = racetrack_insert(track);
        if(succ == 0)
            goto joinrace;
        else
        {
            log_printf("Track 0x%x could not be created.\n", track);
            msgt = 31;
            PUTCHR(cfd, msgt);
        }
    }
    else
    {
        track = ntohl(track);
        joinrace:
        succ = racetrack_join(track, cfd, name);
        if(succ == 0)
        {
            struct racetrack *race = racetrack_get(track);
            struct racer *player = race->racers + race->cnt - 1;
            if(race->status == 3)
                msgt = 19;
            else
                msgt = 17;
            PUTCHR(cfd, msgt);
            if(msgt == 19)
            {
                for(size_t i = 0; i < race->cnt; ++i)
                {
                    msgt = 37;
                    PUTCHR(race->racers[i].cli, msgt);
                    if(i < race->cnt - 1)
                        PUTCHR(cfd, msgt);
                    msgt = strlen(player->name);
                    PUTCHR(race->racers[i].cli, msgt);
                    write(race->racers[i].cli, player->name, msgt);
                    msgt = strlen(race->racers[i].name);
                    if(i < race->cnt - 1)
                    {
                        PUTCHR(cfd, msgt);
                        write(cfd, race->racers[i].name, msgt);
                    }
                }
                while(race->status == 3)
                {
                    GETCHR(cfd, msgt);
                    if(player == race->racers)
                    {
                        if(msgt == 19)
                        {
                            currt = time(NULL);
                            race->end = currt + 63;
                            for(size_t i = 0; i < race->cnt; ++i)
                                PUTCHR(cfd, msgt);
                            usleep(3000000);
                            msgt = 19;
                            for(size_t i = 0; i < race->cnt; ++i)
                                PUTCHR(cfd, msgt);
                            for(size_t i = 0; i < race->cnt; ++i)
                            {
                                msgt = 41;
                                PUTCHR(cfd, msgt);
                                msgt = race->plen;
                                PUTCHR(cfd, msgt);
                                write(cfd, race->paragraph, race->plen);
                            }
                            log_printf("Race %u has started\n", race->num);
                            race->status = 4;
                            msgt = 19;
                        }
                        else
                        {
                            log_printf("Host %s decided to terminate race %u\n", player->name, race->num);
                            race->status = 5;
                            msgt = 31;
                        }
                    }
                }
                if(msgt == 19)
                {
                    currt = time(NULL);
                    while(currt < race->end)
                    {
                        tv.tv_sec = 1;
                        tv.tv_usec = 0;
                        FD_ZERO(fdsp);
                        FD_SET(cfd, fdsp);
                        ready = select(cfd + 1, fdsp, NULL, NULL, &tv);
                        if(ready)
                            GETCHR(cfd, msgt);
                        else
                            msgt = 97;
                        if(msgt == 31)
                            currt = race->end;
                        else
                        {
                            if(msgt == 23)
                            {
                                if(player->progress < race->goal)
                                {
                                    GETCHR(cfd, prog);
                                    prog = ntohs(prog);
                                    player->progress = prog;
                                }
                            }
                            msgt = 29;
                            PUTCHR(cfd, msgt);
                            for(size_t i = 0; i < race->cnt; ++i)
                            {
                                prog = race->racers[i].progress;
                                prog = htons(prog);
                                PUTCHR(cfd, prog);
                            }
                            time(&currt);
                        }
                    }
                }
                else
                {
                    msgt = 31;
                    PUTCHR(cfd, msgt);
                }
            }
        }
        else
        {
            msgt = 13;
            PUTCHR(cfd, msgt);
            log_printf("Player %s could not join room %u\n", name, track);
        }
    }
    close(cfd);
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
    java_util_Random_seed = time(NULL);
    int succ = init_logger(LOGFILE) + racetrack_init();
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
uint32_t nextui(void)
{
    java_util_Random_seed *= SLOPE;
    java_util_Random_seed += 11;
    java_util_Random_seed &= 07777777777777777ul;
    return java_util_Random_seed;
}
