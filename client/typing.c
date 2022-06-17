// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

#include<stdio.h>
#include"typing.h"
#include"rd.h"
void *type_race(void *arg)
{
    struct typebuf *tp = arg, tbuf = *tp;
    char *buf = tbuf.cbuf;
    size_t capa = tbuf.sz, ind = 0;
    for(int ch = rd(); ch != 021; ch = rd())
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
                    }
                    else
                        ring;
                }
                else
                    ring;
        }
    }
    return NULL;
}
