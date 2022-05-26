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
