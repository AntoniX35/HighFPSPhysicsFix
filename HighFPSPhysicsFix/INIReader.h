// Read an INI file into easy-to-access fullname/value pairs.
// 
// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih
/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/
#define  __forceinline

#ifndef INI_H
#define INI_H

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

    /* Nonzero if ini_handler callback should accept lineno parameter. */
#ifndef INI_HANDLER_LINENO
#define INI_HANDLER_LINENO 0
#endif

/* Typedef for prototype of handler function. */
#if INI_HANDLER_LINENO
    typedef int (*ini_handler)(void* user, const char* section,
        const char* name, const char* value,
        int lineno);
#else
    typedef int (*ini_handler)(void* user, const char* section,
        const char* name, const char* value);
#endif

    /* Typedef for prototype of fgets-style reader function. */
    typedef char* (*ini_reader)(char* str, int num, void* stream);

    /* Parse given INI-style file. May have [section]s, name=value pairs
       (whitespace stripped), and comments starting with ';' (semicolon). Section
       is "" if name=value pair parsed before any section heading. name:value
       pairs are also supported as a concession to Python's configparser.
       For each name=value pair parsed, call handler function with given user
       pointer as well as section, name, and value (data only valid for duration
       of handler call). Handler should return nonzero on success, zero on error.
       Returns 0 on success, line number of first error on parse error (doesn't
       stop on first error), -1 on file open error, or -2 on memory allocation
       error (only when INI_USE_STACK is zero).
    */
    int ini_parse(const char* filename, ini_handler handler, void* user);

    /* Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
       close the file when it's finished -- the caller must do that. */
    int ini_parse_file(FILE* file, ini_handler handler, void* user);

    /* Same as ini_parse(), but takes an ini_reader function pointer instead of
       filename. Used for implementing custom or string-based I/O (see also
       ini_parse_string). */
    int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
        void* user);

    /* Same as ini_parse(), but takes a zero-terminated string with the INI data
    instead of a file. Useful for parsing INI data from a network socket or
    already in memory. */
    int ini_parse_string(const char* string, ini_handler handler, void* user);

    /* Nonzero to allow multi-line value parsing, in the style of Python's
       configparser. If allowed, ini_parse() will call the handler with the same
       name for each subsequent line parsed. */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

       /* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
          the file. See https://github.com/benhoyt/inih/issues/21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

          /* Chars that begin a start-of-line comment. Per Python configparser, allow
             both ; and # comments at the start of a line by default. */
#ifndef INI_START_COMMENT_PREFIXES
#define INI_START_COMMENT_PREFIXES ";#"
#endif

             /* Nonzero to allow inline comments (with valid inline comment characters
                specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
                Python 3.2+ configparser behaviour. */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif

                /* Nonzero to use stack for line buffer, zero to use heap (malloc/free). */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/* Maximum line length for any line in INI file (stack or heap). Note that
   this must be 3 more than the longest line (due to '\r', '\n', and '\0'). */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

   /* Nonzero to allow heap line buffer to grow via realloc(), zero for a
      fixed-size buffer of INI_MAX_LINE bytes. Only applies if INI_USE_STACK is
      zero. */
#ifndef INI_ALLOW_REALLOC
#define INI_ALLOW_REALLOC 0
#endif

      /* Initial size in bytes for heap line buffer. Only applies if INI_USE_STACK
         is zero. */
#ifndef INI_INITIAL_ALLOC
#define INI_INITIAL_ALLOC 200
#endif

         /* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Nonzero to call the handler at the start of each new section (with
   name and value NULL). Default is to only call the handler on
   each name=value pair. */
#ifndef INI_CALL_HANDLER_ON_NEW_SECTION
#define INI_CALL_HANDLER_ON_NEW_SECTION 0
#endif

   /* Nonzero to allow a name without a value (no '=' or ':' on the line) and
      call the handler with value NULL in this case. Default is to treat
      no-value lines as an error. */
#ifndef INI_ALLOW_NO_VALUE
#define INI_ALLOW_NO_VALUE 0
#endif

      /* Nonzero to use custom ini_malloc, ini_free, and ini_realloc memory
         allocation functions (INI_USE_STACK must also be 0). These functions must
         have the same signatures as malloc/free/realloc and behave in a similar
         way. ini_realloc is only needed if INI_ALLOW_REALLOC is set. */
#ifndef INI_CUSTOM_ALLOCATOR
#define INI_CUSTOM_ALLOCATOR 0
#endif


#ifdef __cplusplus
}
#endif

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if !INI_USE_STACK
#if INI_CUSTOM_ALLOCATOR
#include <stddef.h>
void* ini_malloc(size_t size);
void ini_free(void* ptr);
void* ini_realloc(void* ptr, size_t size);
#else
#include <stdlib.h>
#define ini_malloc malloc
#define ini_free free
#define ini_realloc realloc
#endif
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Used by ini_parse_string() to keep track of string parsing state. */
typedef struct {
    const char* ptr;
    size_t num_left;
} ini_parse_string_ctx;

/* Strip whitespace chars off end of given string, in place. Return s. */
 char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
 char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to NUL at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
 static char* find_chars_or_comment(const char* s, const char* chars)
{
#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) &&
        !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
        was_space = isspace((unsigned char)(*s));
        s++;
    }
#else
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
#endif
    return (char*)s;
}

/* Similar to strncpy, but ensures dest (size bytes) is
   NUL-terminated, and doesn't pad with NULs. */
 static char* strncpy0(char* dest, const char* src, size_t size)
{
    /* Could use strncpy internally, but it causes gcc warnings (see issue #91) */
    size_t i;
    for (i = 0; i < size - 1 && src[i]; i++)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

/* See documentation in header file. */
 int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
    void* user)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
    int max_line = INI_MAX_LINE;
#else
    char* line;
    size_t max_line = INI_INITIAL_ALLOC;
#endif
#if INI_ALLOW_REALLOC && !INI_USE_STACK
    char* new_line;
    size_t offset;
#endif
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

#if !INI_USE_STACK
    line = (char*)ini_malloc(INI_INITIAL_ALLOC);
    if (!line) {
        return -2;
    }
#endif

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif

    /* Scan through stream line by line */
    while (reader(line, (int)max_line, stream) != NULL) {
#if INI_ALLOW_REALLOC && !INI_USE_STACK
        offset = strlen(line);
        while (offset == max_line - 1 && line[offset - 1] != '\n') {
            max_line *= 2;
            if (max_line > INI_MAX_LINE)
                max_line = INI_MAX_LINE;
            new_line = ini_realloc(line, max_line);
            if (!new_line) {
                ini_free(line);
                return -2;
            }
            line = new_line;
            if (reader(line + offset, (int)(max_line - offset), stream) == NULL)
                break;
            if (max_line >= INI_MAX_LINE)
                break;
            offset += strlen(line + offset);
        }
#endif

        lineno++;

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
            (unsigned char)start[1] == 0xBB &&
            (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (strchr(INI_START_COMMENT_PREFIXES, *start)) {
            /* Start-of-line comment */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {
            /* Non-blank line with leading whitespace, treat as continuation
               of previous name's value (as per Python configparser). */
            if (!HANDLER(user, section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
#if INI_CALL_HANDLER_ON_NEW_SECTION
                if (!HANDLER(user, section, NULL, NULL) && !error)
                    error = lineno;
#endif
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start) {
            /* Not a comment, must be a name[=:]value pair */
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = end + 1;
#if INI_ALLOW_INLINE_COMMENTS
                end = find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
#endif
                value = lskip(value);
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!HANDLER(user, section, name, value) && !error)
                    error = lineno;
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
#if INI_ALLOW_NO_VALUE
                * end = '\0';
                name = rstrip(start);
                if (!HANDLER(user, section, name, NULL) && !error)
                    error = lineno;
#else
                error = lineno;
#endif
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
}

#if !INI_USE_STACK
    ini_free(line);
#endif

    return error;
}

/* See documentation in header file. */
 int ini_parse_file(FILE* file, ini_handler handler, void* user)
{
    return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

/* See documentation in header file. */
 int ini_parse(const char* filename, ini_handler handler, void* user)
{
    FILE* file;
    int error;

    if (fopen_s(&file, filename, "r"))
        return -1;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}

/* An ini_reader function to read the next line from a string buffer. This
   is the fgets() equivalent used by ini_parse_string(). */
 static char* ini_reader_string(char* str, int num, void* stream) {
    ini_parse_string_ctx* ctx = (ini_parse_string_ctx*)stream;
    const char* ctx_ptr = ctx->ptr;
    size_t ctx_num_left = ctx->num_left;
    char* strp = str;
    char c;

    if (ctx_num_left == 0 || num < 2)
        return NULL;

    while (num > 1 && ctx_num_left != 0) {
        c = *ctx_ptr++;
        ctx_num_left--;
        *strp++ = c;
        if (c == '\n')
            break;
        num--;
    }

    *strp = '\0';
    ctx->ptr = ctx_ptr;
    ctx->num_left = ctx_num_left;
    return str;
}

/* See documentation in header file. */
 int ini_parse_string(const char* string, ini_handler handler, void* user) {
    ini_parse_string_ctx ctx;

    ctx.ptr = string;
    ctx.num_left = strlen(string);
    return ini_parse_stream((ini_reader)ini_reader_string, &ctx, handler,
        user);
}
#endif /* __INI_H__ */


#ifndef INIREADER_H
#define INIREADER_H

#include <map>
#include <string>

 // Read an INI file into easy-to-access name/value pairs. (Note that I've gone
 // for simplicity here rather than speed, but it should be pretty decent.)
 class INIReader
 {
 public:
     INIReader() {};
     // Construct INIReader and parse given filename. See ini.h for more info
     // about the parsing.
     explicit INIReader(const std::string& filename);

     // Construct INIReader and parse given buffer. See ini.h for more info
     // about the parsing.
     explicit INIReader(const char* buffer, size_t buffer_size);

     // Return the result of ini_parse(), i.e., 0 on success, line number of
     // first error on parse error, or -1 on file open error.
     int ParseError() const;

     // Get a string value from INI file, returning default_value if not found.
     std::string Get(const std::string& section, const std::string& name,
         const std::string& default_value) const;

     // Get a string value from INI file, returning default_value if not found,
     // empty, or contains only whitespace.
     std::string GetString(const std::string& section, const std::string& name,
         const std::string& default_value) const;

     // Get an integer (long) value from INI file, returning default_value if
     // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
     long GetInteger(const std::string& section, const std::string& name, long default_value) const;

     // Get a real (floating point double) value from INI file, returning
     // default_value if not found or not a valid floating point value
     // according to strtod().
     double GetReal(const std::string& section, const std::string& name, double default_value) const;

     // Get a boolean value from INI file, returning default_value if not found or if
     // not a valid true/false value. Valid true values are "true", "yes", "on", "1",
     // and valid false values are "false", "no", "off", "0" (not case sensitive).
     bool GetBoolean(const std::string& section, const std::string& name, bool default_value) const;

     // Return true if the given section exists (section must contain at least
     // one name=value pair).
     bool HasSection(const std::string& section) const;

     // Return true if a value exists with the given section and field names.
     bool HasValue(const std::string& section, const std::string& name) const;

 private:
     int _error;
     std::map<std::string, std::string> _values;
     static std::string MakeKey(const std::string& section, const std::string& name);
     static int ValueHandler(void* user, const char* section, const char* name,
         const char* value);
 };

#endif  // __INIREADER_H__


#ifndef __INIREADER__
#define __INIREADER__

#include <algorithm>
#include <cctype>
#include <cstdlib>
 using std::string;

  INIReader::INIReader(const string& filename)
 {
     _error = ini_parse(filename.c_str(), ValueHandler, this);
 }

  INIReader::INIReader(const char* buffer, size_t buffer_size)
 {
     string content(buffer, buffer_size);
     _error = ini_parse_string(content.c_str(), ValueHandler, this);
 }

  int INIReader::ParseError() const
 {
     return _error;
 }

  string INIReader::Get(const string& section, const string& name, const string& default_value) const
 {
     string key = MakeKey(section, name);
     // Use _values.find() here instead of _values.at() to support pre C++11 compilers
     return _values.count(key) ? _values.find(key)->second : default_value;
 }

string INIReader::GetString(const string& section, const string& name, const string& default_value) const
 {
     const string str = Get(section, name, "");
     return str.empty() ? default_value : str;
 }

long INIReader::GetInteger(const string& section, const string& name, long default_value) const
 {
     string valstr = Get(section, name, "");
     const char* value = valstr.c_str();
     char* end;
     // This parses "1234" (decimal) and also "0x4D2" (hex)
     long n = strtol(value, &end, 0);
     return end > value ? n : default_value;
 }

  double INIReader::GetReal(const string& section, const string& name, double default_value) const
 {
     string valstr = Get(section, name, "");
     const char* value = valstr.c_str();
     char* end;
     double n = strtod(value, &end);
     return end > value ? n : default_value;
 }

  bool INIReader::GetBoolean(const string& section, const string& name, bool default_value) const
 {
     string valstr = Get(section, name, "");
     // Convert to lower case to make string comparisons case-insensitive
     std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
     if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
         return true;
     else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
         return false;
     else
         return default_value;
 }

  bool INIReader::HasSection(const string& section) const
 {
     const string key = MakeKey(section, "");
     std::map<string, string>::const_iterator pos = _values.lower_bound(key);
     if (pos == _values.end())
         return false;
     // Does the key at the lower_bound pos start with "section"?
     return pos->first.compare(0, key.length(), key) == 0;
 }

  bool INIReader::HasValue(const string& section, const string& name) const
 {
     string key = MakeKey(section, name);
     return _values.count(key);
 }

  string INIReader::MakeKey(const string& section, const string& name)
 {
     string key = section + "=" + name;
     // Convert to lower case to make section/name lookups case-insensitive
     std::transform(key.begin(), key.end(), key.begin(), ::tolower);
     return key;
 }

  int INIReader::ValueHandler(void* user, const char* section, const char* name,
     const char* value)
 {
     if (!name)  // Happens when INI_CALL_HANDLER_ON_NEW_SECTION enabled
         return 1;
     INIReader* reader = static_cast<INIReader*>(user);
     string key = MakeKey(section, name);
     if (reader->_values[key].size() > 0)
         reader->_values[key] += "\n";
     reader->_values[key] += value ? value : "";
     return 1;
 }
#endif  // __INIREADER__
