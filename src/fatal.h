/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef FATAL_H
#define FATAL_H

#ifdef __cplusplus
extern "C" {
#endif

void fatal_printf(const char *format, ...);
void fatal_perror(const char *s);

#ifdef __cplusplus
}
#endif

#endif
