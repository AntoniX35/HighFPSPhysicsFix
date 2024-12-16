#include "ext/INIReader.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

/* Strip whitespace chars off end of given string, in place. Return s. */
SKMP_FORCEINLINE static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
SKMP_FORCEINLINE static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. SKMP_FORCEINLINE comment must
   be prefixed by a whitespace character to register as a comment. */
SKMP_FORCEINLINE static char* find_chars_or_comment(const char* s, const char* chars)
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

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
SKMP_FORCEINLINE static char* strncpy0(char* dest, const char* src, std::size_t size)
{
    strncpy_s(dest, size, src, size);
    dest[size - 1] = '\0';
    return dest;
}

/* See documentation in header file. */
static int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
    void* user)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
#else
    char* line;
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
    line = (char*)_mm_malloc(INI_MAX_LINE, 32);
    if (!line) {
        return -2;
    }
#endif

    /* Scan through stream line by line */
    while (reader(line, INI_MAX_LINE, stream) != nullptr) {
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

        if (*start == ';' || *start == '#') {
            /* Per Python configparser, allow both ; and # comments at the
               start of a line */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {

#if INI_ALLOW_INLINE_COMMENTS
            end = find_chars_or_comment(start, nullptr);
            if (*end)
                *end = '\0';
            rstrip(start);
#endif

            /* Non-blank line with leading whitespace, treat as continuation
               of previous fullname's value (as per Python configparser). */
            if (!handler(user, section, prev_name, start) && !error)
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
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start) {
            /* Not a comment, must be a fullname[=:]value pair */
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
#if INI_ALLOW_INLINE_COMMENTS
                end = find_chars_or_comment(value, nullptr);
                if (*end)
                    *end = '\0';
#endif
                rstrip(value);

                /* Valid fullname[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler(user, section, name, value) && !error)
                    error = lineno;
            }
            else if (!error) {
                /* No '=' or ':' found on fullname[=:]value line */
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    _mm_free(line);
#endif

    return error;
}

/* See documentation in header file. */
static int ini_parse_file(FILE* file, ini_handler handler, void* user)
{
    return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

/* See documentation in header file. */
static int ini_parse(const char* filename, ini_handler handler, void* user)
{
    FILE* file;
    int error;

    if (fopen_s(&file, filename, "r"))
        return -1;

    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}

INIReader::INIReader(const std::string& filename)
{
    _error = ini_parse(filename.c_str(), ValueHandler, this);
    _init = true;
}

void INIReader::Load(const std::string& filename)
{
    if (!_init) {
        _error = ini_parse(filename.c_str(), ValueHandler, this);
        _init = true;
    }
}

void INIReader::Clear()
{
    _values.clear();
}

INIReader::INIReader(FILE* file)
{
    _error = ini_parse_file(file, ValueHandler, this);
    _init = true;
}

int INIReader::ParseError() const
{
    return _error;
}

/*SKMP_FORCEINLINE const stl::iunordered_set<std::string>& INIReader::Sections() const
{
    return _values;
}*/

const std::string* INIReader::Get(const std::string& section, const std::string& name) const
{
    auto it1 = _values.find(section);
    if (it1 == _values.end())
        return nullptr;

    auto it2 = it1->second.find(name);
    if (it2 == it1->second.end())
        return nullptr;

    return std::addressof(it2->second);
}

const char* INIReader::Get(const std::string& section, const std::string& name, const char* default_value) const
{
    return ParseValue(Get(section, name), default_value);
}

std::int64_t INIReader::Get(const std::string& section, const std::string& name, std::int64_t default_value) const
{
    return ParseValue(Get(section, name), default_value);
}

double INIReader::Get(const std::string& section, const std::string& name, double default_value) const
{
    return ParseValue(Get(section, name), default_value);
}

float INIReader::Get(const std::string& section, const std::string& name, float default_value) const
{
    return ParseValue(Get(section, name), default_value);
}

bool INIReader::Get(const std::string& section, const std::string& name, bool default_value) const
{
    return ParseValue(Get(section, name), default_value);
}

const char* INIReader::ParseValue(const std::string* a_in, const char* default_value) const
{
    if (!a_in)
        return default_value;

    return a_in->c_str();
}

std::int64_t INIReader::ParseValue(const std::string* a_in, std::int64_t default_value) const
{
    if (!a_in)
        return default_value;

    const char* value = a_in->c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtoll(value, &end, 0);
    return end > value ? n : default_value;
}

double INIReader::ParseValue(const std::string* a_in, double default_value) const
{
    if (!a_in)
        return default_value;

    const char* value = a_in->c_str();
    char* end;
    double n = strtod(value, &end);
    return end > value ? n : default_value;
}

float INIReader::ParseValue(const std::string* a_in, float default_value) const
{
    if (!a_in)
        return default_value;

    const char* value = a_in->c_str();
    char* end;
    float n = strtof(value, &end);
    return end > value ? n : default_value;
}

bool INIReader::ParseValue(const std::string* a_in, bool default_value) const
{
    if (!a_in)
        return default_value;

    const char* value = a_in->c_str();

    if (_stricmp(value, "true") == 0 || _stricmp(value, "yes") == 0 || _stricmp(value, "on") == 0 || _stricmp(value, "1") == 0)
        return true;
    else if (_stricmp(value, "false") == 0 || _stricmp(value, "no") == 0 || _stricmp(value, "off") == 0 || _stricmp(value, "0") == 0)
        return false;
    else
        return default_value;
}

bool INIReader::Exists(const std::string& section, const std::string& name) const
{
    auto it1 = _values.find(section);
    if (it1 == _values.end())
        return false;

    auto it2 = it1->second.find(name);
    if (it2 == it1->second.end())
        return false;

    return true;
}

void INIReader::RemoveSection(const std::string& section)
{
    _values.erase(section);
}

int INIReader::ValueHandler(void* user, const char* section, const char* name,
    const char* value)
{
    INIReader* reader = (INIReader*)user;

    auto r = reader->_values.try_emplace(section);
    auto o = r.first->second.try_emplace(name, value);

    if (!o.second) {
        o.first->second += "\n";
        o.first->second += value;
    }

    //_DMESSAGE("add: %s, %s : %s", section, name, value);

    return 1;
}
