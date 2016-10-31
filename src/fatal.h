/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef FATAL_H
#define FATAL_H

#ifdef __cplusplus
extern "C" {
#endif

void fatal_printf(const char *format, ...);
void fatal_perror(const char *s);
void fatal(const char *file, int line, const char *format, ...);

#define FATAL(...) {fatal(__FILE__,__LINE__,__VA_ARGS__);}


#ifdef __cplusplus
}
#endif

#endif
