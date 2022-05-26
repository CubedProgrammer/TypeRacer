#ifndef Included_logging_h
#define Included_logging_h
int init_logger(const char *fname);
void log_puts(const char *str);
void log_printf(const char *fmt, ...);
void end_logging(void);
#endif
