// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

#include<stdio.h>
#include<string.h>
#ifdef _WIN32
#include<conio.h>
#else
#include<unistd.h>
#endif
#include"rd.h"
char rdcbuf[4];
int rdcbufcnt;
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
    for(size_t i = 0; i < sz; i++)
        putchar(' ');
    printf("\033\133%zuD", sz);
    buf[sz] = '\0';
}
int rd(void)
{
    int ch;
    char cbuf[4];
#ifdef _WIN32
    char tmp = getch();
    switch(tmp)
    {
        case'\r':
            ch = '\n';
            break;
        case-64:
            tmp = getch();
            switch(tmp)
            {
                case 0117:
                    ch = 0x5b46;
                    break;
                case 0107:
                    ch = 0x5b48;
                    break;
                case 0123:
                    ch = 0x5b337e;
                    break;
                case 0115:
                    ch = 0x5b43;
                    break;
                case 0113:
                    ch = 0x5b44;
                    break;
                default:
                    ring;
            }
            break;
        case'\b':
            ch = 0177;
            break;
        default:
            ch = tmp;
    }
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
