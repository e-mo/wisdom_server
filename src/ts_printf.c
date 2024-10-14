#include <stdio.h>
#include <stdarg.h>
#include <time.h>

const char *ts_format = "<%d-%02d-%02d %02d:%02d:%02d> ";

int ts_printf(const char *format, ...) {
	va_list args;
	va_start(args, format);

	// Get local time
	time_t t = time(NULL); 
	struct tm tm = *localtime(&t);
	printf(ts_format, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	vprintf(format, args);
	va_end(args);
}
