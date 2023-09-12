#ifndef EUCLID__LOG_H
#define EUCLID__LOG_H 1

void log__set_log_file(char *file);

void log_print(const char *fmt, ...);

void log_(const char *fmt, ...);

#endif