#include "Globals.h"

#include <Windows.h>   // OutputDebugString, FormatMessage, ModuleFilePath...
#include <vector>    // std::vector container
#include <sstream>     // std::basic_stringstream, std::basic_istringstream, std::basic_ostringstream class templates and several typedefs 


// --------------------------------


#pragma region [STRINGS]


const char* va(const char* pFormat, ...)
{
    //assert(pFormat);
    if (pFormat == nullptr)
        return "INVALID";
#define	VA_MAX_STRING 4096
    static char buffer[VA_MAX_STRING];

    va_list argList;
    va_start(argList, pFormat);
    int count = vsnprintf_s(buffer, VA_MAX_STRING, _TRUNCATE, pFormat, argList);
    va_end(argList);

    if (count < 0)
        throw std::system_error(GetLastError(), std::system_category());

    char *buf = &buffer[0];
    memcpy(buf, buffer, count + 1);
    return buf;
}

std::string format(const char *pFormat, ...)
{
    std::string retStr("");

    va_list argList;
    va_start(argList, pFormat);

    // Get formatted string length, adding one for NULL
    std::size_t len = _vscprintf(pFormat, argList) + 1;

    // Create a char vector to hold the formatted string.
    std::vector<char> buffer(len, '\0');

    if (vsnprintf_s(&buffer[0], buffer.size(), len, pFormat, argList) > 0)
        retStr = &buffer[0];

    va_end(argList);

    return retStr;
}

#pragma endregion [STRINGS]


///////////////////////////////////////////////////////////////////


#pragma region [DEBUG]


// output string
void DbgOut(const char *str)
{
    OutputDebugStringA(str); // Visual Studio
    std::printf(str); // default output stream
}

// source: http://windowscecleaner.blogspot.nl/2013/04/debug-output-tricks-for-visual-studio.html
// Debug output: file, line nr, and formatted va_list. (eg.: File.cs(85): format...)
void DbgOutput(const char *file, const int line, const char *pFormat, ...)
{
    // std::this_thread::get_id()
#define DBG_MAX_STRING 1024
    char buffer[DBG_MAX_STRING] = { 0 };
    int preStrLen = 0, count = 0;

    const char* linestr = ((line) != NULL ? ((std::ostringstream() << ("(") << (line) << (")")).str().c_str()) : (""));

    //int preStrLen = sprintf_s(buffer, DEBUG_MAX_STRING, "%s(%d): ", file, line);
    if (file != NULL)
        preStrLen = sprintf_s(buffer, DBG_MAX_STRING - 1, "%s%s: ", file, linestr);

    va_list argList;
    va_start(argList, pFormat);
    count = vsnprintf_s(buffer + preStrLen, DBG_MAX_STRING - 1 - preStrLen, _TRUNCATE, pFormat, argList);
    va_end(argList);

    if (count < 0)
        throw std::system_error(GetLastError(), std::system_category());

    buffer[preStrLen + count] = '\n';
    buffer[preStrLen + count + 1] = '\0';
    DbgOut(buffer);
}

// --------------------------------

// Text describing error code.
std::string GetErrorText(DWORD dwErrorCode)
{
    LPVOID lpMsgBuf;
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD bufLen = FormatMessage(flags, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

    if (!bufLen)
        return std::string();

    LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
    std::string result(lpMsgStr, lpMsgStr + bufLen - 2); // without newline

    LocalFree(lpMsgBuf);

    return result;
}
//static string GetLastErrorText() { return GetErrorText(GetLastError()); }

/*
// Debug output error code description, prefixed by a string(id).
#define ERR_CODE(s, e) ERR_OUTPUT("%s - %s (0x%x)", s, GetErrorText(e).c_str(), e)

std::string GetErrorText(DWORD dwErrorCode);

class win32_error : public std::exception
{
public:
    typedef std::exception _Mybase;
    explicit win32_error(const std::string& msg) : _Mybase(msg.c_str()), errcode(GetLastError()) {}
    explicit win32_error(const char *msg) : _Mybase(msg), errcode(GetLastError()) {}
    explicit win32_error(int err, const std::string& msg) : _Mybase(msg.c_str()), errcode(err) {}
    explicit win32_error(int err, const char *msg) : _Mybase(msg), errcode(err) {}
    const int& code() const throw() { return (errcode); }
    const std::string desc() const throw() { return GetErrorText(errcode); }
protected:
    int errcode;	// the stored error code
};
*/

#pragma endregion [DEBUG]


///////////////////////////////////////////////////////////////////
