#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
// #include <errno.h>

#include "env.h"
#include "utils.h"

static char *def_log_file = NULL;

static void __log_prt__(const char *fmt, va_list ap);

static void __log_prt_raw__(const char *fmt, va_list ap);

extern OLAPEnv olap_env;

void log_print(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	__log_prt__(fmt, ap);
	va_end(ap);
}

void log_(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	__log_prt_raw__(fmt, ap);
	va_end(ap);
}

void log__set_log_file(char *file)
{
	if (def_log_file)
	{
		log_print("[ error ] exit. log__set_log_file().\n");
		exit(EXIT_FAILURE);
	}

	def_log_file = obj_alloc(strlen(olap_env.OLAP_HOME) + strlen(file) + 1, OBJ_TYPE__RAW_BYTES);
	sprintf(def_log_file, "%s%s", olap_env.OLAP_HOME, file);
	// strcpy(def_log_file, file);
}

static void __log_prt__(const char *fmt, va_list ap)
{

	if (def_log_file == NULL)
	{
		// vfprintf(stdout, fmt, ap);
		// fflush(stdout);
		return;
	}

	time_t time_p;
	time(&time_p);
	struct tm *tmins = gmtime(&time_p);

	FILE *log_fd = fopen(def_log_file, "a");
	for (int i = 0; i < 5 && log_fd == NULL; i++)
	{
		usleep(10000);
		log_fd = fopen(def_log_file, "a");
	}

	if (log_fd == NULL)
	{
		fprintf(stderr, "Can't open the log file<%s>.\n", def_log_file);
		return;
	}

	fprintf(log_fd, "%02d-%02d-%d %02d:%02d:%02d ", tmins->tm_mon + 1, tmins->tm_mday, tmins->tm_year + 1900, tmins->tm_hour, tmins->tm_min, tmins->tm_sec);
	vfprintf(log_fd, fmt, ap);

	fflush(log_fd);
	fclose(log_fd);
}

static void __log_prt_raw__(const char *fmt, va_list ap) {

	if (def_log_file == NULL)
		return;

	FILE *log_fd = fopen(def_log_file, "a");
	for (int i = 0; i < 5 && log_fd == NULL; i++)
	{
		usleep(10000);
		log_fd = fopen(def_log_file, "a");
	}

	if (log_fd == NULL)
	{
		fprintf(stderr, "Can't open the log file<%s>.\n", def_log_file);
		return;
	}

	vfprintf(log_fd, fmt, ap);

	fflush(log_fd);
	fclose(log_fd);
}