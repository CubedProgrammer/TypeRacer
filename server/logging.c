// /CubedProgrammer/TypeRacer, a type racing game.
// Copyright (C) 2022, github.com/CubedProgrammer owner of said account.

// This file is part of /CubedProgrammer/TypeRacer.

// /CubedProgrammer/TypeRacer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// /CubedProgrammer/TypeRacer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with /CubedProgrammer/TypeRacer. If not, see <https://www.gnu.org/licenses/>.

#include<stdarg.h>
#include<stdio.h>
#include<time.h>
#include"logging.h"
FILE *tr_logfile_handle;
int init_logger(const char *fname)
{
    tr_logfile_handle = fopen(fname, "a");
    if(tr_logfile_handle == NULL)
        return-1;
    else
        return 0;
}
void log_puts(const char *str)
{
    log_printf("%s\n", str);
}
void log_printf(const char *fmt, ...)
{
    long curr = time(NULL);
    int days = curr / 86400;
    int hours = curr % 86400 / 3600, mins = curr / 60 % 60;
    fprintf(tr_logfile_handle, "%li Day %i [%02i:%02i:%02li] ", curr, days, hours, mins, curr % 60);
    va_list ls;
    va_start(ls, fmt);
    vfprintf(tr_logfile_handle, fmt, ls);
    va_end(ls);
    fflush(tr_logfile_handle);
}
void end_logging(void)
{
    fclose(tr_logfile_handle);
}
