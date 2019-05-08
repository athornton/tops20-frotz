/*
 * missing.h
 *
 * Declarations and definitions for standard things that may be missing
 * on older systems.
 *
 */

#ifndef MISSING_H
#define MISSING_H

#ifdef NO_MEMMOVE
void *my_memmove(void *, const void *, size_t);
#define memmove my_memmove
#endif

#ifdef NO_STRDUP
char *my_strdup(const char *);
char *my_strndup(const char *, size_t);
#define strdup	my_strdup
#define strndup	my_strndup
#endif

#ifdef NO_LIBGEN_H
char *my_basename(const char *);
#define basename my_basename
#endif

#endif /* MISSING_H */
