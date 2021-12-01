//
// Created by cxworks on 2020/6/16.
//

//===-- sanitizer_common_interceptors_format.inc ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Scanf/printf implementation for use in *Sanitizer interceptors.
// Follows http://pubs.opengroup.org/onlinepubs/9699919799/functions/fscanf.html
// and http://pubs.opengroup.org/onlinepubs/9699919799/functions/fprintf.html
// with a few common GNU extensions.
//
//===----------------------------------------------------------------------===//

#include <stdarg.h>
#include <string.h>
#include <limits.h>


typedef unsigned int u32;
typedef unsigned long long u64;

static const char *parse_number(const char *p, int *out) {
    *out = atoll(p);
    while (*p >= '0' && *p <= '9')
        ++p;
    return p;
}

static const char *maybe_parse_param_index(const char *p, int *out) {
    // n$
    if (*p >= '0' && *p <= '9') {
        int number;
        const char *q = parse_number(p, &number);
        if (*q == '$') {
            *out = number;
            p = q + 1;
        }
    }

    // Otherwise, do not change p. This will be re-parsed later as the field
    // width.
    return p;
}

static bool char_is_one_of(char c, const char *s) {
    return !!strchr(s, c);
}

static const char *maybe_parse_length_modifier(const char *p, char ll[2]) {
    if (char_is_one_of(*p, "jztLq")) {
        ll[0] = *p;
        ++p;
    } else if (*p == 'h') {
        ll[0] = 'h';
        ++p;
        if (*p == 'h') {
            ll[1] = 'h';
            ++p;
        }
    } else if (*p == 'l') {
        ll[0] = 'l';
        ++p;
        if (*p == 'l') {
            ll[1] = 'l';
            ++p;
        }
    }
    return p;
}

// Returns true if the character is an integer conversion specifier.
static bool format_is_integer_conv(char c) {
    return char_is_one_of(c, "diouxXn");
}

// Returns true if the character is an floating point conversion specifier.
static bool format_is_float_conv(char c) {
    return char_is_one_of(c, "aAeEfFgG");
}

// Returns string output character size for string-like conversions,
// or 0 if the conversion is invalid.
static int format_get_char_size(char convSpecifier,
                                const char lengthModifier[2]) {
    if (char_is_one_of(convSpecifier, "CS")) {
        return sizeof(wchar_t);
    }

    if (char_is_one_of(convSpecifier, "cs[")) {
        if (lengthModifier[0] == 'l' && lengthModifier[1] == '\0')
            return sizeof(wchar_t);
        else if (lengthModifier[0] == '\0')
            return sizeof(char);
    }

    return 0;
}

enum FormatStoreSize {
    // Store size not known in advance; can be calculated as wcslen() of the
    // destination buffer.
    FSS_WCSLEN = -2,
    // Store size not known in advance; can be calculated as strlen() of the
    // destination buffer.
    FSS_STRLEN = -1,
    // Invalid conversion specifier.
    FSS_INVALID = 0
};

// Returns the memory size of a format directive (if >0), or a value of
// FormatStoreSize.
static int format_get_value_size(char convSpecifier,
                                 const char lengthModifier[2],
                                 bool promote_float) {
    if (format_is_integer_conv(convSpecifier)) {
        switch (lengthModifier[0]) {
            case 'h':
                return lengthModifier[1] == 'h' ? sizeof(char) : sizeof(short);
            case 'l':
                return lengthModifier[1] == 'l' ? sizeof(long long) : sizeof(long);
            case 'q':
                return sizeof(long long);
            case 'L':
                return sizeof(long long);
            case 'j':
                return sizeof(INT32_MAX);
            case 'z':
                return sizeof(SIZE_T);
            case 't':
                return sizeof(INT32_MAX);
            case 0:
                return sizeof(int);
            default:
                return FSS_INVALID;
        }
    }

    if (format_is_float_conv(convSpecifier)) {
        switch (lengthModifier[0]) {
            case 'L':
            case 'q':
                return sizeof(long double);
            case 'l':
                return lengthModifier[1] == 'l' ? sizeof(long double)
                                                : sizeof(double);
            case 0:
                // Printf promotes floats to doubles but scanf does not
                return promote_float ? sizeof(double) : sizeof(float);
            default:
                return FSS_INVALID;
        }
    }

    if (convSpecifier == 'p') {
        if (lengthModifier[0] != 0)
            return FSS_INVALID;
        return sizeof(void *);
    }

    return FSS_INVALID;
}

struct ScanfDirective {
    int argIdx; // argument index, or -1 if not specified ("%n$")
    int fieldWidth;
    const char *begin;
    const char *end;
    bool suppressed; // suppress assignment ("*")
    bool allocate;   // allocate space ("m")
    char lengthModifier[2];
    char convSpecifier;
    bool maybeGnuMalloc;
};

// Parse scanf format string. If a valid directive in encountered, it is
// returned in dir. This function returns the pointer to the first
// unprocessed character, or 0 in case of error.
// In case of the end-of-string, a pointer to the closing \0 is returned.
static const char *scanf_parse_next(const char *p, bool allowGnuMalloc,
                                    ScanfDirective *dir) {
    memset(dir, 0, sizeof(*dir));
    dir->argIdx = -1;

    while (*p) {
        if (*p != '%') {
            ++p;
            continue;
        }
        dir->begin = p;
        ++p;
        // %%
        if (*p == '%') {
            ++p;
            continue;
        }
        if (*p == '\0') {
            return nullptr;
        }
        // %n$
        p = maybe_parse_param_index(p, &dir->argIdx);
        // *
        if (*p == '*') {
            dir->suppressed = true;
            ++p;
        }
        // Field width
        if (*p >= '0' && *p <= '9') {
            p = parse_number(p, &dir->fieldWidth);
            if (dir->fieldWidth <= 0)  // Width if at all must be non-zero
                return nullptr;
        }
        // m
        if (*p == 'm') {
            dir->allocate = true;
            ++p;
        }
        // Length modifier.
        p = maybe_parse_length_modifier(p, dir->lengthModifier);
        // Conversion specifier.
        dir->convSpecifier = *p++;
        // Consume %[...] expression.
        if (dir->convSpecifier == '[') {
            if (*p == '^')
                ++p;
            if (*p == ']')
                ++p;
            while (*p && *p != ']')
                ++p;
            if (*p == 0)
                return nullptr; // unexpected end of string
            // Consume the closing ']'.
            ++p;
        }
        // This is unfortunately ambiguous between old GNU extension
        // of %as, %aS and %a[...] and newer POSIX %a followed by
        // letters s, S or [.
        if (allowGnuMalloc && dir->convSpecifier == 'a' &&
            !dir->lengthModifier[0]) {
            if (*p == 's' || *p == 'S') {
                dir->maybeGnuMalloc = true;
                ++p;
            } else if (*p == '[') {
                // Watch for %a[h-j%d], if % appears in the
                // [...] range, then we need to give up, we don't know
                // if scanf will parse it as POSIX %a [h-j %d ] or
                // GNU allocation of string with range dh-j plus %.
                const char *q = p + 1;
                if (*q == '^')
                    ++q;
                if (*q == ']')
                    ++q;
                while (*q && *q != ']' && *q != '%')
                    ++q;
                if (*q == 0 || *q == '%')
                    return nullptr;
                p = q + 1; // Consume the closing ']'.
                dir->maybeGnuMalloc = true;
            }
        }
        dir->end = p;
        break;
    }
    return p;
}

static int scanf_get_value_size(ScanfDirective *dir) {
    if (dir->allocate) {
        if (!char_is_one_of(dir->convSpecifier, "cCsS["))
            return FSS_INVALID;
        return sizeof(char *);
    }

    if (dir->maybeGnuMalloc) {
        if (dir->convSpecifier != 'a' || dir->lengthModifier[0])
            return FSS_INVALID;
        // This is ambiguous, so check the smaller size of char * (if it is
        // a GNU extension of %as, %aS or %a[...]) and float (if it is
        // POSIX %a followed by s, S or [ letters).
        return sizeof(char *) < sizeof(float) ? sizeof(char *) : sizeof(float);
    }

    if (char_is_one_of(dir->convSpecifier, "cCsS[")) {
        bool needsTerminator = char_is_one_of(dir->convSpecifier, "sS[");
        unsigned charSize =
                format_get_char_size(dir->convSpecifier, dir->lengthModifier);
        if (charSize == 0)
            return FSS_INVALID;
        if (dir->fieldWidth == 0) {
            if (!needsTerminator)
                return charSize;
            return (charSize == sizeof(char)) ? FSS_STRLEN : FSS_WCSLEN;
        }
        return (dir->fieldWidth + needsTerminator) * charSize;
    }

    return format_get_value_size(dir->convSpecifier, dir->lengthModifier, false);
}

// Common part of *scanf interceptors.
// Process format string and va_list, and report all store ranges.
// Stops when "consuming" n_inputs input items.
static void scanf_common(int n_inputs, bool allowGnuMalloc,
                         const char *format, va_list aq) {
    const char *p = format;

    COMMON_INTERCEPTOR_READ_RANGE((void *) format, strlen(format) + 1);

    while (*p) {
        ScanfDirective dir;
        p = scanf_parse_next(p, allowGnuMalloc, &dir);
        if (!p)
            break;
        if (dir.convSpecifier == 0) {
            // This can only happen at the end of the format string.
            break;
        }
        // Here the directive is valid. Do what it says.
        if (dir.argIdx != -1) {
            // Unsupported.
            break;
        }
        if (dir.suppressed)
            continue;
        int size = scanf_get_value_size(&dir);
        if (size == FSS_INVALID) {
            break;
        }
        void *argp = va_arg(aq, void *);
        if (dir.convSpecifier != 'n')
            --n_inputs;
        if (n_inputs < 0)
            break;
        if (size == FSS_STRLEN) {
            size = strlen((const char *) argp) + 1;
        } else if (size == FSS_WCSLEN) {
            // FIXME: actually use wcslen() to calculate it.
            size = 0;
        }
        COMMON_INTERCEPTOR_WRITE_RANGE(argp, size);
    }
}

static void scanf_wrapper(int n_inputs, int64 format, va_list ap) {
    scanf_common(n_inputs, true, (char *) format, ap);
}


struct PrintfDirective {
    int fieldWidth;
    int fieldPrecision;
    int argIdx; // width argument index, or -1 if not specified ("%*n$")
    int precisionIdx; // precision argument index, or -1 if not specified (".*n$")
    const char *begin;
    const char *end;
    bool starredWidth;
    bool starredPrecision;
    char lengthModifier[2];
    char convSpecifier;
};

static const char *maybe_parse_number(const char *p, int *out) {
    if (*p >= '0' && *p <= '9')
        p = parse_number(p, out);
    return p;
}

static const char *maybe_parse_number_or_star(const char *p, int *out,
                                              bool *star) {
    if (*p == '*') {
        *star = true;
        ++p;
    } else {
        *star = false;
        p = maybe_parse_number(p, out);
    }
    return p;
}

// Parse printf format string. Same as scanf_parse_next.
static const char *printf_parse_next(const char *p, PrintfDirective *dir) {
    memset(dir, 0, sizeof(*dir));
    dir->argIdx = -1;
    dir->precisionIdx = -1;

    while (*p) {
        if (*p != '%') {
            ++p;
            continue;
        }
        dir->begin = p;
        ++p;
        // %%
        if (*p == '%') {
            ++p;
            continue;
        }
        if (*p == '\0') {
            return nullptr;
        }
        // %n$
        p = maybe_parse_param_index(p, &dir->precisionIdx);
        // Flags
        while (char_is_one_of(*p, "'-+ #0")) {
            ++p;
        }
        // Field width
        p = maybe_parse_number_or_star(p, &dir->fieldWidth,
                                       &dir->starredWidth);
        if (!p)
            return nullptr;
        // Precision
        if (*p == '.') {
            ++p;
            // Actual precision is optional (surprise!)
            p = maybe_parse_number_or_star(p, &dir->fieldPrecision,
                                           &dir->starredPrecision);
            if (!p)
                return nullptr;
            // m$
            if (dir->starredPrecision) {
                p = maybe_parse_param_index(p, &dir->precisionIdx);
            }
        }
        // Length modifier.
        p = maybe_parse_length_modifier(p, dir->lengthModifier);
        // Conversion specifier.
        dir->convSpecifier = *p++;
        dir->end = p;
        break;
    }
    return p;
}

static int printf_get_value_size(PrintfDirective *dir) {
    if (char_is_one_of(dir->convSpecifier, "cCsS")) {
        unsigned charSize =
                format_get_char_size(dir->convSpecifier, dir->lengthModifier);
        if (charSize == 0)
            return FSS_INVALID;
        if (char_is_one_of(dir->convSpecifier, "sS")) {
            return (charSize == sizeof(char)) ? FSS_STRLEN : FSS_WCSLEN;
        }
        return charSize;
    }

    return format_get_value_size(dir->convSpecifier, dir->lengthModifier, true);
}

#define SKIP_SCALAR_ARG(aq, convSpecifier, size)                   \
  do {                                                             \
    if (format_is_float_conv(convSpecifier)) {                     \
      switch (size) {                                              \
      case 8:                                                      \
        va_arg(*aq, double);                                       \
        break;                                                     \
      case 12:                                                     \
        va_arg(*aq, long double);                                  \
        break;                                                     \
      case 16:                                                     \
        va_arg(*aq, long double);                                  \
        break;                                                     \
      default:                                                     \
        return;                                                    \
      }                                                            \
    } else {                                                       \
      switch (size) {                                              \
      case 1:                                                      \
      case 2:                                                      \
      case 4:                                                      \
        va_arg(*aq, u32);                                          \
        break;                                                     \
      case 8:                                                      \
        va_arg(*aq, u64);                                          \
        break;                                                     \
      default:                                                     \
        return;                                                    \
      }                                                            \
    }                                                              \
  } while (0)

// Common part of *printf interceptors.
// Process format string and va_list, and report all load ranges.
static void printf_common(const char *format, va_list aq) {
    COMMON_INTERCEPTOR_READ_RANGE((void *) format, strlen(format) + 1);

    const char *p = format;

    while (*p) {
        PrintfDirective dir;
        p = printf_parse_next(p, &dir);
        if (!p)
            break;
        if (dir.convSpecifier == 0) {
            // This can only happen at the end of the format string.
            break;
        }
        // Here the directive is valid. Do what it says.
        if (dir.argIdx != -1 || dir.precisionIdx != -1) {
            // Unsupported.
            break;
        }
        if (dir.starredWidth) {
            // Dynamic width
            SKIP_SCALAR_ARG(&aq, 'd', sizeof(int));
        }
        if (dir.starredPrecision) {
            // Dynamic precision
            SKIP_SCALAR_ARG(&aq, 'd', sizeof(int));
        }
        // %m does not require an argument: strlen(errno).
        if (dir.convSpecifier == 'm')
            continue;
        int size = printf_get_value_size(&dir);
        if (size == FSS_INVALID) {
            static int ReportedOnce;
            if (!ReportedOnce++)
                break;
        }
        if (dir.convSpecifier == 'n') {
            void *argp = va_arg(aq, void *);
            COMMON_INTERCEPTOR_WRITE_RANGE(argp, size);
            continue;
        } else if (size == FSS_STRLEN) {
            if (void *argp = va_arg(aq, void *)) {
                if (dir.starredPrecision) {
                    // FIXME: properly support starred precision for strings.
                    size = 0;
                } else if (dir.fieldPrecision > 0) {
                    // Won't read more than "precision" symbols.
                    size = strnlen((const char *) argp, dir.fieldPrecision);
                    if (size < dir.fieldPrecision) size++;
                } else {
                    // Whole string will be accessed.
                    size = strlen((const char *) argp) + 1;
                }
                COMMON_INTERCEPTOR_READ_RANGE(argp, size);
            }
        } else if (size == FSS_WCSLEN) {
            if (void *argp = va_arg(aq, void *)) {
                // FIXME: Properly support wide-character strings (via wcsrtombs).
                size = 0;
                COMMON_INTERCEPTOR_READ_RANGE(argp, size);
            }
        } else {
            // Skip non-pointer args
            SKIP_SCALAR_ARG(&aq, dir.convSpecifier, size);
        }
    }
}


static void printf_wrapper(void *format, ...) {
    va_list ap;
    va_start(ap, (char *) format);
    printf_common((char *) format, ap);
    va_end(ap);
}

static void printf_common(int64 format, int64 aq) {
    va_list org;
    va_copy(org, *(va_list*)aq);
    printf_common((char *) format, org);
}

static void printf_common(int64 format, va_list aq) {
    printf_common((char *) format, aq);
}


