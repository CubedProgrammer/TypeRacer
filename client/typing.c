// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

#include<stdio.h>
#include<string.h>
#include"typing.h"
#include"rd.h"
#ifdef _WIN32
void type_race(void *arg)
#else
void *type_race(void *arg)
#endif
{
    struct typebuf *tp = arg, tbuf = *tp;
    char *buf = tbuf.cbuf;
    size_t capa = tbuf.sz, ind = 0;
    char fini = 0;
    int ch;
    for(ch = rd(); !fini && ch != 030; ch = fini ? ch : rd())
    {
        switch(ch)
        {
            case 0177:
                if(ind > 0)
                {
                    --ind;
                    buf[ind] = '\0';
                }
                else
                    ring;
                break;
            default:
                if(ch >= ' ' && ch < 0177)
                {
                    if(ind < capa)
                    {
                        buf[ind] = ch;
                        ++ind;
                        if(ind == tbuf.plen)
                            fini = strcmp(tbuf.para, buf) == 0;
                    }
                    else
                        ring;
                }
                else
                    ring;
        }
    }
    if(ch == 030)
    {
        buf[ind] = 030;
        buf[ind + 1] = '\0';
    }
#ifndef _WIN32
    return NULL;
#endif
}
