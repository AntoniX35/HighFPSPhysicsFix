// Read an INI file into easy-to-access fullname/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih
/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#include "STL.h"

#ifndef __INI_H__
#define __INI_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

    /* Typedef for prototype of handler function. */
    typedef int (*ini_handler)(void* user, const char* section,
        const char* name, const char* value);

    /* Typedef for prototype of fgets-style reader function. */
    typedef char* (*ini_reader)(char* str, int num, void* stream);

    /* Parse given INI-style file. May have [section]s, fullname=value pairs
       (whitespace stripped), and comments starting with ';' (semicolon). Section
       is "" if fullname=value pair parsed before any section heading. fullname:value
       pairs are also supported as a concession to Python's configparser.

       For each fullname=value pair parsed, call handler function with given user
       pointer as well as section, fullname, and value (data only valid for duration
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
       filename. Used for implementing custom or string-based I/O. */
    int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
        void* user);

    /* Nonzero to allow multi-line value parsing, in the style of Python's
       configparser. If allowed, ini_parse() will call the handler with the same
       fullname for each subsequent line parsed. */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

       /* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
          the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

          /* Nonzero to allow inline comments (with valid SKMP_FORCEINLINE comment characters
             specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
             Python 3.2+ configparser behaviour. */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";#"
#endif

             /* Nonzero to use stack, zero to use heap (malloc/free). */
#ifndef INI_USE_STACK
#define INI_USE_STACK 0
#endif

/* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Maximum line length for any line in INI file. */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 1024 * 32
#endif

#ifdef __cplusplus
}
#endif

/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 128
#define MAX_NAME 128


#endif /* __INI_H__ */


#ifndef __INIREADER_H__
#define __INIREADER_H__

#include <map>
#include <set>
#include <string>

// Read an INI file into easy-to-access fullname/value pairs. (Note that I've gone
// for simplicity here rather than speed, but it should be pretty decent.)
class INIReader
{
public:
    // Empty Constructor
    INIReader() : _init(false), _error(-1) { };

    // Construct INIReader and parse given filename. See ini.h for more info
    // about the parsing.
    INIReader(const std::string& filename);

    // Construct INIReader and parse given file. See ini.h for more info
    // about the parsing.
    INIReader(FILE* file);

    void Load(const std::string& filename);
    void Clear();

    // Return the result of ini_parse(), i.e., 0 on success, line number of
    // first error on parse error, or -1 on file open error.
    int ParseError() const;

    // Return the list of sections found in ini file
    //const stl::iunordered_set<std::string>& Sections() const;

    // Get a string value from INI file, returning default_value if not found.
    const char* Get(const std::string& section, const std::string& name,
        const char* default_value) const;

    const std::string* Get(const std::string& section, const std::string& name) const;

    // Get an integer (long) value from INI file, returning default_value if
    // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
    std::int64_t Get(const std::string& section, const std::string& name, std::int64_t default_value) const;

    template <typename T, typename = std::enable_if_t<!std::is_same_v<T, bool> && (std::is_integral_v<T> || std::is_enum_v<T>) && std::is_convertible_v<T, std::int64_t>>>
    std::int64_t Get(const std::string& section, const std::string& name, std::int64_t default_value) const
    {
        return static_cast<T>(Get(section, name, static_cast<std::int64_t>(default_value)));
    }

    // Get a real (floating point double) value from INI file, returning
    // default_value if not found or not a valid floating point value
    // according to strtod().
    double Get(const std::string& section, const std::string& name, double default_value) const;

    // Get a single precision floating point number value from INI file, returning
    // default_value if not found or not a valid floating point value
    // according to strtof().
    float Get(const std::string& section, const std::string& name, float default_value) const;

    // Get a boolean value from INI file, returning default_value if not found or if
    // not a valid true/false value. Valid true values are "true", "yes", "on", "1",
    // and valid false values are "false", "no", "off", "0" (not case sensitive).
    bool Get(const std::string& section, const std::string& name, bool default_value) const;



    const char* ParseValue(const std::string* a_in, const char* default_value) const;
    std::int64_t ParseValue(const std::string* a_in, std::int64_t default_value) const;
    double ParseValue(const std::string* a_in, double default_value) const;
    float ParseValue(const std::string* a_in, float default_value) const;
    bool ParseValue(const std::string* a_in, bool default_value) const;


    bool Exists(const std::string& section, const std::string& name) const;

    void RemoveSection(const std::string &section);

    inline auto& GetSections() const {
        return _values;
    }

protected:
    int _error;
    bool _init;

    using storage_type = stl::iunordered_map<std::string, std::string, std::allocator<std::pair<const std::string, std::string>>>;

    stl::iunordered_map<std::string, storage_type, std::allocator<std::pair<const std::string, storage_type>>> _values;
    //stl::iunordered_set<std::string> _sections;
    static int ValueHandler(void* user, const char* section, const char* name,
        const char* value);
};

#endif  // __INIREADER_H__
