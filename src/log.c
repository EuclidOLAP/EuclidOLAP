#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>

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

static void __log_prt__(const char *fmt, va_list ap) {

	FILE *log_fd = fopen("log/euclid.log", "a");

	vfprintf(log_fd, fmt, ap);
	// fprintf(log_fd, "\n");
	fflush(log_fd);

	fclose(log_fd);
}