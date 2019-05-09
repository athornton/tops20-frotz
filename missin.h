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
void *A00263(void *, const void *, size_t);
#define memmove A00263
#endif

#ifdef NO_STRDUP
char *A00264(const char *);
char *A00265(const char *, size_t);
#define strdup	A00264
#define strndup	A00265
#endif

#ifdef NO_LIBGEN_H
char *A00266(const char *);
#define basename A00266
#endif

#endif /* MISSING_H */
