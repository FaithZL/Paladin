//
//  stringprint.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/1.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef stringprint_hpp
#define stringprint_hpp

inline void stringPrintfRecursive(std::string *s, const char *fmt) {
    const char *c = fmt;
    // No args left; make sure there aren't any extra formatting
    // specifiers.
    while (*c) {
        if (*c == '%') {
//            CHECK_EQ(c[1], '%');
            ++c;
        }
        *s += *c++;
    }
}

// 1. Copy from fmt to *s, up to the next formatting directive.
// 2. Advance fmt past the next formatting directive and return the
//    formatting directive as a string.
inline std::string copyToFormatString(const char **fmt_ptr, std::string *s) {
    const char *&fmt = *fmt_ptr;
    while (*fmt) {
        if (*fmt != '%') {
            *s += *fmt;
            ++fmt;
        } else if (fmt[1] == '%') {
            // "%%"; let it pass through
            *s += '%';
            *s += '%';
            fmt += 2;
        } else
            // fmt is at the start of a formatting directive.
            break;
    }
    
    std::string nextFmt;
    if (*fmt) {
        do {
            nextFmt += *fmt;
            ++fmt;
            // Incomplete (but good enough?) test for the end of the
            // formatting directive: a new formatting directive starts, we
            // hit whitespace, or we hit a comma.
        } while (*fmt && *fmt != '%' && !isspace(*fmt) && *fmt != ',' &&
                 *fmt != '[' && *fmt != ']' && *fmt != '(' && *fmt != ')');
    }
    
    return nextFmt;
}

template <typename T>
inline std::string formatOne(const char *fmt, T v) {
    // Figure out how much space we need to allocate; add an extra
    // character for the '\0'.
    size_t size = snprintf(nullptr, 0, fmt, v) + 1;
    std::string str;
    str.resize(size);
    snprintf(&str[0], size, fmt, v);
    str.pop_back();  // remove trailing NUL
    return str;
}

// General-purpose version of stringPrintfRecursive; add the formatted
// output for a single StringPrintf() argument to the final result string
// in *s.
template <typename T, typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, T v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    *s += formatOne(nextFmt.c_str(), v);
    stringPrintfRecursive(s, fmt, args...);
}

// Special case of StringPrintRecursive for float-valued arguments.
template <typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, float v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    if (nextFmt == "%f")
        // Always use enough precision so that the printed value gives
        // the exact floating-point value if it's used to initialize a
        // float.
        // https://randomascii.wordpress.com/2012/03/08/float-precisionfrom-zero-to-100-digits-2/
        *s += formatOne("%.9g", v);
    else
        // If a specific formatting string other than "%f" was specified,
        // just use that.
        *s += formatOne(nextFmt.c_str(), v);
    
    // Go forth and print the next arg.
    stringPrintfRecursive(s, fmt, args...);
}

// Specialization for doubles that always uses enough precision.  (It seems
// that this is the version that is actually called for floats.  I thought
// that float->double promotion wasn't supposed to happen in this case?)
template <typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, double v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    if (nextFmt == "%f")
        *s += formatOne("%.17g", v);
    else
        *s += formatOne(nextFmt.c_str(), v);
    stringPrintfRecursive(s, fmt, args...);
}

// StringPrintf() is a replacement for sprintf() (and the like) that
// returns the result as a std::string. This gives convenience/control
// of printf-style formatting in a more C++-ish way.
//
// Floating-point values with the formatting string "%f" are handled
// specially so that enough digits are always printed so that the original
// float/double can be reconstituted exactly from the printed digits.
template <typename... Args>
inline std::string StringPrintf(const char *fmt, Args... args) {
    std::string ret;
    stringPrintfRecursive(&ret, fmt, args...);
    return ret;
}

#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif  // __GNUG__



#endif /* stringprint_h */
