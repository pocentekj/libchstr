#include "chstr.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unicase.h>
#include <unistr.h>
#include <unitypes.h>
#include <uniwidth.h>

#include <klib/kvec.h>

bool ch_starts_with(const char *s, const char *search) {
    size_t min_len = strlen(search);
    if (strlen(s) < min_len) return false;
    return strncmp(s, search, min_len) == 0;
}

typedef uint8_t *(*case_fn)(const uint8_t *, size_t, const char *, uninorm_t, uint8_t *, size_t *);

static char *ch_string_case(const char *s, case_fn convert) {
    const uint8_t *u8 = (const uint8_t *)s;
    size_t out_len = 0;

    uint8_t *buf = convert(u8, u8_strlen(u8), NULL, NULL, NULL, &out_len);
    if (buf == NULL) return NULL;

    uint8_t *terminated = realloc(buf, out_len + 1);
    if (terminated == NULL) {
        free(buf);
        return NULL;
    }

    terminated[out_len] = '\0';
    return (char *)terminated;
}

char *ch_string_to_lowercase(const char *s) { return ch_string_case(s, u8_tolower); }

char *ch_string_to_uppercase(const char *s) { return ch_string_case(s, u8_toupper); }

int ch_string_center(const char *s, size_t w, char **out) {
    if (!s || !out || w > INT_MAX) return -1;
    *out = NULL;

    size_t sw = ch_string_width(s);
    if (sw > w) return -1;
    int l = (w - sw) / 2;
    int r = w - l - sw;

    return asprintf(out, "%*s%s%*s", l, "", s, r, "");
}

bool ch_string_truncate(char *s, size_t maxcols) {
    const uint8_t *txt = (const uint8_t *)s;
    size_t remaining = u8_strlen(txt);
    size_t offset = 0, columns = 0;

    while (offset < remaining) {
        ucs4_t ch;
        int n = u8_mbtouc(&ch, txt + offset, remaining - offset);

        // invalid UTF-8
        if (n < 0) return false;

        int w = u8_width(txt + offset, (size_t)n, "UTF-8");

        if (w < 0 || (size_t)w > maxcols - columns) break;

        columns += (size_t)w;
        offset += (size_t)n;
    }

    if (offset == remaining) return false;

    s[offset] = '\0';

    return true;
}

char *ch_string_from_file(const char *path, size_t *max_size) {
    if (!path) return NULL;

    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        return NULL;
    }

    char *contents = NULL;

    size_t cap = (max_size && *max_size < SIZE_MAX - 1) ? *max_size : SIZE_MAX - 1;
    if (fseek(fp, 0L, SEEK_END) != 0) {
        perror("fseek");
        goto error;
    }

    long file_size = ftell(fp);
    if (file_size == -1) {
        perror("ftell");
        goto error;
    }

    if ((uintmax_t)file_size >= (uintmax_t)cap) {
        errno = EFBIG;
        goto error;
    }

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        perror("fseek");
        goto error;
    }

    contents = malloc((size_t)file_size + 1);
    if (!contents) goto error;

    size_t nbytes = fread(contents, 1, (size_t)file_size, fp);
    if (nbytes != (size_t)file_size) goto error;

    fclose(fp);
    contents[file_size] = '\0';

    return contents;

error:
    if (fp) fclose(fp);
    if (contents) free(contents);

    return NULL;
}

string_array *ch_string_array_new(void) {
    string_array *arr = calloc(1, sizeof *arr);
    if (!arr) return NULL;
    kv_init(arr->data);
    return arr;
}

void ch_string_array_destroy(string_array *arr) {
    if (!arr) return;

    for (size_t i = 0; i < kv_size(arr->data); i++) {
        free(kv_A(arr->data, i));
    }

    kv_destroy(arr->data);
    free(arr);
    arr = NULL;
}

bool ch_read_lines(const char *path, string_array *out) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        return false;
    }

    char *line = NULL;
    size_t max_size = 0;
    ssize_t nread = 0;

    while ((nread = getline(&line, &max_size, fp)) != -1) {
        if (nread > 0 && line[nread - 1] == '\n') line[--nread] = '\0';
        kv_push(char *, out->data, strdup(line));
    }

    free(line);
    fclose(fp);

    return true;
}

bool ch_parse_double(const char *s, double *dest) {
    if (!s || !dest) return false;

    errno = 0;

    char *endptr = NULL;
    double val = strtod(s, &endptr);

    if (endptr == s || *endptr != '\0' || errno == ERANGE) {
        return false;
    }

    *dest = val;

    return true;
}

bool ch_parse_ll(const char *s, long long *dest) {
    if (!s || !dest) return false;

    errno = 0;

    char *endptr = NULL;
    long long val = strtoll(s, &endptr, 10);

    if (errno == ERANGE) return false;

    if (endptr == s || *endptr != '\0') return false;

    *dest = val;

    return true;
}

bool ch_parse_int(const char *s, int *dest) {
    if (!s || !dest) return false;

    long long val = 0;
    if (!ch_parse_ll(s, &val)) return false;

    if (val < INT_MIN || val > INT_MAX) {
        errno = ERANGE;
        return false;
    }

    *dest = (int)val;

    return true;
}
