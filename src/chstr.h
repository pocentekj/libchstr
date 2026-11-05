#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistr.h>
#include <unitypes.h>
#include <uniwidth.h>

#include <klib/kvec.h>

/**
 * Returns width as visible columns.
 */
static inline size_t ch_string_width(const char *s) {
    return u8_strwidth((const uint8_t *)s, "UTF-8");
}

static inline bool ch_string_utf8_valid(const char *s) {
    const uint8_t *txt = (const uint8_t *)s;
    return u8_check(txt, u8_strlen(txt)) == NULL;
}

/**
 * Convert string to lower/upper case using UTF-8 rather than bytes.
 * Note that this is meant to be used only in simple cases.
 * Returns NULL on failure, converted string on success. Returned string
 * must be released by the caller.
 */
char *ch_string_to_lowercase(const char *s);
char *ch_string_to_uppercase(const char *s);

/**
 * Center string `s` according to max width `w` and store in `out`.
 * `out` must be released by the caller after use.
 */
int ch_string_center(const char *s, size_t w, char **out);

/**
 * Truncate in place string `s` to visible columns `maxcols`.
 * Returns 'true' if string has been modified, `false` otherwise.
 */
bool ch_string_truncate(char *s, size_t maxcols);

/**
 * Check if `s` starts with `search`.
 */
bool ch_starts_with(const char *s, const char *search);

/**
 * Read entire file on `path` into string.
 *
 * If `max_size` is provided, it will be used as the upper size limits for files
 * that should be loaded. This function will reject bigger files and return NULL.
 * If `max_size` is NULL, the upper limit will be set to SIZE_MAX - 1.
 *
 * Returns pointer to string on success, NULL otherwise. The pointer returned
 * must be released by the caller.
 *
 * In case of failure, `errno` is set either to `EFBIG` when file size exceeds
 * limit, or it's set by standard library functions.
 */
char *ch_string_from_file(const char *path, size_t *max_size);

typedef struct {
    kvec_t(char *) data;
} string_array;

string_array *ch_string_array_new(void);
void ch_string_array_destroy(string_array *arr);

static inline size_t ch_string_array_len(const string_array *arr) {
    return arr ? kv_size(arr->data) : 0;
}

static inline char *ch_string_array_at(const string_array *arr, size_t i) {
    return (arr && i < kv_size(arr->data)) ? kv_A(arr->data, i) : NULL;
}

/**
 * Read entire file on `path` line by line, and store lines in `out` string_array.
 * Returns `true` on success, `false` on error. `errno` may be set by underlying
 * functions, such as `fopen`.
 */
bool ch_read_lines(const char *path, string_array *out);

/**
 * A set of functions to parse numbers from strings. All of them returns `true`
 * on successful conversion and `false` on error, and store output in `dest` of
 * appropriate type. `errno` may be set by underlying functions.
 */
bool ch_parse_double(const char *s, double *dest);
bool ch_parse_ll(const char *s, long long *dest);
bool ch_parse_int(const char *s, int *dest);
