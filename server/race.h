// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

#ifndef Included_race_h
#define Included_race_h
#include<stddef.h>
#include<stdint.h>
struct racer
{
    char name[60];
    int cli;
    size_t progress;
};

struct racetrack
{
    const char *paragraph;
    size_t plen;
    struct racer *volatile racers;
    volatile size_t cnt, cap;
    volatile int status;
    volatile time_t end;
    uint32_t num;
};

struct racetrack_ht
{
    struct racetrack *buckets;
    size_t cnt, bcnt;
};

int racetrack_init(void);
int racetrack_insert(uint32_t num);
struct racetrack *racetrack_get(uint32_t num);
int racetrack_join(uint32_t num, int cfd, const char *name);
void racetrack_remove(uint32_t num);
#endif
