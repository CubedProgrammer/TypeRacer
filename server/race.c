// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<time.h>
#include<unistd.h>
#include"race.h"
#define DEFAULT_CAP 10
struct racetrack_ht tr_racetrack_htable;
char *tr_paragraph_all;
const char *tr_paragraph_array[3720];
size_t tr_paragraph_len[3720];
size_t tr_paragraph_cnt;
size_t racetrack_hash(uint32_t num)
{
    return num % tr_racetrack_htable.bcnt;
}
int racetrack_init(void)
{
    int succ = 0;
    tr_racetrack_htable.cnt = 0;
    tr_racetrack_htable.bcnt = DEFAULT_CAP;
    tr_racetrack_htable.buckets = malloc(sizeof(*tr_racetrack_htable.buckets) * DEFAULT_CAP);
    if(tr_racetrack_htable.buckets == NULL)
        succ = -1;
    else
    {
        memset(tr_racetrack_htable.buckets, 0, sizeof(*tr_racetrack_htable.buckets) * tr_racetrack_htable.bcnt);
        struct stat fdat;
        succ = stat("paragraphs.txt", &fdat);
        if(succ == 0)
        {
            size_t fsz = fdat.st_size;
            tr_paragraph_all = malloc(fsz);
            if(tr_paragraph_all != NULL)
            {
                FILE *f = fopen("paragraphs.txt", "r");
                if(f != NULL)
                {
                    fread(tr_paragraph_all, 1, fsz, f);
                    fclose(f);
                    size_t ind = 0, last = 0;
                    tr_paragraph_array[ind] = tr_paragraph_all;
                    for(size_t i = 0; i < fsz; ++i)
                    {
                        if(tr_paragraph_all[i] == '\n')
                        {
                            tr_paragraph_len[ind] = i - last;
                            last = i + 1;
                            ++ind;
                            if(i < fsz - 1)
                                tr_paragraph_array[ind] = tr_paragraph_all + last;
                        }
                    }
                    tr_paragraph_cnt = ind;
                }
                else
                    succ = -1;
            }
            else
                succ = -1;
        }
    }
    return succ;
}
int racetrack_insert(uint32_t num)
{
    int succ = 0;
    struct racetrack *track = racetrack_get(num);
    if(track == NULL)
    {
        if(tr_racetrack_htable.cnt + 1 > tr_racetrack_htable.bcnt * 5 / 3)
        {
            size_t bcnt = tr_racetrack_htable.bcnt * 3 >> 1, nh;
            struct racetrack *buckets = malloc(sizeof(*buckets) * bcnt);
            if(buckets == NULL)
                succ = -1;
            else
            {
                memset(buckets, 0, sizeof(*buckets) * bcnt);
                for(track = tr_racetrack_htable.buckets; track != tr_racetrack_htable.buckets + tr_racetrack_htable.bcnt; ++track)
                {
                    if(track->cap != 0)
                    {
                        nh = racetrack_hash(track->num);
                        while(buckets[nh].cap != 0)
                            nh = racetrack_hash(nh + 1);
                        buckets[nh] = *track;
                    }
                }
                free(tr_racetrack_htable.buckets);
                tr_racetrack_htable.buckets = buckets;
                tr_racetrack_htable.bcnt = bcnt;
            }
        }
        if(succ == 0)
        {
            size_t h = racetrack_hash(num);
            for(; tr_racetrack_htable.buckets[h].cap != 0; h = racetrack_hash(h + 1));
            tr_racetrack_htable.buckets[h].num = num;
            tr_racetrack_htable.buckets[h].cnt = 0;
            tr_racetrack_htable.buckets[h].cap = DEFAULT_CAP;
            tr_racetrack_htable.buckets[h].racers = malloc(sizeof(*tr_racetrack_htable.buckets[h].racers) * DEFAULT_CAP);
            tr_racetrack_htable.buckets[h].status = 3;
            size_t pchoice = time(NULL) % tr_paragraph_cnt;
            tr_racetrack_htable.buckets[h].paragraph = tr_paragraph_array[pchoice];
            tr_racetrack_htable.buckets[h].plen = tr_paragraph_len[pchoice];
            if(tr_racetrack_htable.buckets[h].racers == NULL)
            {
                tr_racetrack_htable.buckets[h].cap = 0;
                succ = -1;
            }
            else
                ++tr_racetrack_htable.cnt;
        }
    }
    else
        succ = -1;
    return succ;
}
struct racetrack *racetrack_get(uint32_t num)
{
    size_t h = racetrack_hash(num);
    struct racetrack *track = NULL;
    if(tr_racetrack_htable.buckets[h].cap != 0)
    {
        if(tr_racetrack_htable.buckets[h].num == num)
            track = tr_racetrack_htable.buckets + h;
        else
        {
            size_t og = h++;
            for(; h != og; ++h, h = racetrack_hash(h))
            {
                if(tr_racetrack_htable.buckets[h].num == num)
                {
                    track = tr_racetrack_htable.buckets + h;
                    h = og - 1;
                }
                else if(tr_racetrack_htable.buckets[h].cap == 0)
                    h = og - 1;
            }
        }
    }
    return track;
}
int racetrack_join(uint32_t num, int cfd, const char *name)
{
    int succ = 0;
    struct racetrack *track = racetrack_get(num);
    if(track == NULL)
    {
        succ = -1;
        goto ret;
    }
    if(track->cnt == track->cap)
    {
        track->cap += track->cap >> 1;
        void *mem = realloc(track->racers, track->cap * sizeof(*track->racers));
        if(mem == NULL)
        {
            succ = -1;
            track->cap = track->cnt;
            goto ret;
        }
        else
            track->racers = mem;
    }
    size_t ind = track->cnt, nlen = strlen(name);
    if(nlen > sizeof(track->racers[ind].name) - 1)
    {
        succ = -1;
        goto ret;
    }
    strcpy(track->racers[ind].name, name);
    track->racers[ind].cli = cfd;
    ++track->cnt;
    ret:
    return succ;
}
void racetrack_remove(uint32_t num)
{
    size_t h = racetrack_hash(num);
    if(tr_racetrack_htable.buckets[h].cap != 0)
    {
        if(tr_racetrack_htable.buckets[h].num == num)
        {
            rm:
            for(const struct racer *rp = tr_racetrack_htable.buckets[h].racers; rp != tr_racetrack_htable.buckets[h].racers + tr_racetrack_htable.buckets[h].cnt; ++rp)
                close(rp->cli);
            free(tr_racetrack_htable.buckets[h].racers);
            tr_racetrack_htable.buckets[h].cap = 0;
        }
        else
        {
            size_t og = h++;
            for(; h != og; ++h, h = racetrack_hash(h))
            {
                if(tr_racetrack_htable.buckets[h].num == num)
                    goto rm;
                else if(tr_racetrack_htable.buckets[h].cap == 0)
                    h = og - 1;
            }
        }
    }
}
