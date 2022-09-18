#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>

#include "utils.h"

static char *def_log_file = NULL;

static void __log_prt__(const char *fmt, va_list ap);

void log_print(const char *fmt, ...) {

 	if (access("log", F_OK) != 0) {
		// create the log folder
		mkdir("log", S_IRWXU);
	}

	va_list ap;
	va_start(ap, fmt);
	__log_prt__(fmt, ap);
	va_end(ap);
}

void log__set_log_file(char *file) {
	if (def_log_file) {
		log_print("[ error ] exit. log__set_log_file().\n");
		exit(EXIT_FAILURE);
	}

	def_log_file = obj_alloc(strlen(file) + 1, OBJ_TYPE__RAW_BYTES);
	strcpy(def_log_file, file);
}

static void __log_prt__(const char *fmt, va_list ap) {

	if (def_log_file == NULL) {
		vfprintf(stdout, fmt, ap);
		fflush(stdout);
		return;
	}

	FILE *log_fd = fopen(def_log_file, "a");

	vfprintf(log_fd, fmt, ap);
	fflush(log_fd);

	fclose(log_fd);
}