#pragma once

#pragma region [INCLUDES]

//#include <Box2D/Box2D.h>

#include <cstdlib>     // General purpose utilities
//#include <cstddef>   // typedefs for types such as size_t, NULL and others
//#include <cstdarg>   // Handling of variable length argument lists

//#include <iosfwd>      // forward declarations of all classes in the input / output library
#include <sstream>     // std::basic_stringstream, std::basic_istringstream, std::basic_ostringstream class templates and several typedefs 
//#include <iomanip>     // Helper functions to control the format or input and output

//#include <vector>    // std::vector container
//#include <list>      // std::list container 
//#include <bitset>
#include <string>      // std::basic_string class template

//#include <cmath>       // Common mathematics functions
//#include <algorithm>   // Algorithms that operate on containers
////#include <climits>    // standardized way to query properties of fundamental types
////#include <iterator>  // Container iterators

#include "glm/vec2.hpp"
#include <random>

#pragma endregion [INCLUDES]

///////////////////////////////////////////////////////////////////

typedef unsigned int uint;
typedef unsigned long ulong;

// --------------------------------

// Smallest possible single value greater than zero.
#define EPSILON 1.401298E-45f

#define RATIO 0.5f
#define UNRATIO (1.0F/RATIO)

///////////////////////////////////////////////////////////////////

#pragma region [NUMERICS]

template <typename T> inline T sign(T val) { return val < 0 ? T(-1) : T(1); } // val < 0 ? -1 : 1;
template <typename T> inline T sign0(T val) { return val < -EPSILON ? -1 : val > EPSILON ? 1 : 0; }

#define MAX(a, b) (b > a ? b : a)
template<typename T> inline T max(T a, T b) { return (b > a ? b : a); }
#define MIN(a, b) (b < a ? b : a)
template<typename T> inline T min(T a, T b) { return (b < a ? b : a); }

// Clamp value between minimum and maximum.
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
template<typename T> inline T clamp(T val, T min, T max) { return val > max ? max : val < min ? min : val; }
template<typename T> inline T clamp01(T val) { return val > 1 ? 1 : val < 0 ? 0 : val; }

// val * val
template <typename T> inline T pow2(T val) { return val * val; }
// val * val * val
template <typename T> inline T pow3(T val) { return val * val * val; }
// val * val * val * val
template <typename T> inline T pow4(T val) { return val * val * val * val; }

// Performs division only when the denominator is greater than 0. (returns zero if not)
template<typename T, typename U> inline void safeDivide(T& num, U absDenom) { if (absDenom > EPSILON) num /= absDenom; else num *= U(0); }

// Performs division only when the denominator is greater than 0. (returns zero if not)
template<typename T> inline T safeDivided(T num, T absDenom) { return absDenom > EPSILON ? (num / absDenom) : 0; }
template<typename T, typename U> inline T safeDivided(const T& num, U absDenom) { return absDenom > EPSILON ? (num / absDenom) : (num * 0); }

// --------------------------------

// PI constant.
#define PI 3.14159265f

// degrees of 1 radian
#define RAD2DEG ( 180.0f / PI )
// radians of 1 degree
#define DEG2RAD ( PI / 180.0f )

// Convert degrees to radians  (radians = degrees * (pi/180))
template<typename T> inline T toRadians(T degrees) { return (degrees * DEG2RAD); }
// Convert radians to degrees  (degrees = radians * (180/pi))
template<typename T> inline T toDegrees(T radians) { return (radians * RAD2DEG); }

#define SQRTPI 1.7724538509055160f   // sqrt(pi)
#define SQRTPI_F2 1.1283791670955126f   // 2/sqrt(pi)
#define SQRT2 1.4142135623730950f   // sqrt(2)
#define SQRT3 1.7320508075688773f   // sqrt(3)
#define SQRTH 0.70710678118654752f  // sqrt(0.5) or 1/sqrt(2) or sqrt(2)/2

// --------------------------------

// pseudo-random number [0, 1]
#define RANDOM (((float)std::rand()) / RAND_MAX)
// pseudo-random number [-1, 1]
#define CRANDOM (2.0f * (RANDOM - 0.5f))
// pseudo-random number [0, <1]
#define IRANDOM (((float)std::rand()) / (RAND_MAX+1))

// pseudo-random number [0, a]
inline float Random(float _Max) { return RANDOM * _Max; }
// pseudo-random number [a, b]
inline float Random(float _Min, float _Max) { return RANDOM * (_Max - _Min) + _Min; }

// pseudo-random number [0, <a]
inline int RandomInt(int _Max) { return (int)(IRANDOM * _Max); }
// pseudo-random number [a, <b]
inline int RandomInt(int _Min, int _Max) { return (int)(IRANDOM * (_Max - _Min) + _Min); }

class Random
{
public:
    static void Init()
    {
        s_RandomEngine.seed(std::random_device()());
    }

    static float Float()
    {
        return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
    }

private:
    static std::mt19937 s_RandomEngine;
    static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};

// --------------------------------

// Returns bit number b[0 to 31] from int a.
inline bool Bit(int a, int b) { return (a >> b) & 1; }

// --------------------------------

namespace Interpolate
{

    // Interpolates between from and to by value (clamped between 0 and 1).
    inline float Linear(const float from, float to, float value)
    {
        if (value < 0.0f) return from;
        else if (value > 1.0f) return to;
        return (to - from) * value + from;

    }
    // Interpolates between from and to by value (unclamped).
    inline float Linearf(float from, float to, float value)
    {
        return (1.0f - value) * from + value * to;
    }

    // Calculate value between from and to in the range 0(=from) and 1(=to).
    inline float InverseLinearf(float from, float to, float value)
    {
        return (1.0f - (value - to) / (from - to));
    }

    // Interpolates between from and to by value in the range from2 and to2 (clamped between 0 and 1).
    inline float Linear(float from, float to, float from2, float to2, float value)
    {
        if (from2 < to2)
        {
            if (value < from2) value = from2;
            else if (value > to2) value = to2;
        }
        else
        {
            if (value < to2) value = to2;
            else if (value > from2) value = from2;
        }
        return (to - from) * ((value - from2) / (to2 - from2)) + from;
    }

    // Interpolates between from and to by value in the range from2 and to2 (unclamped).
    inline float Linearf(float from, float to, float from2, float to2, float value)
    {
        return (to - from) * ((value - from2) / (to2 - from2)) + from;
    }

    // Interpolate while easing in and out at the limits.
    inline float Hermite(float start, float end, float value)
    {
        return Linear(start, end, value * value * (3.0f - 2.0f * value));
    }

    // Interpolate while easing out around the end, when value is near one.
    inline float Sinerp(float start, float end, float value)
    {
        return Linear(start, end, sin(value * PI * 0.5f));
    }
    // Interpolate while easing in around the start, when value is near zero.
    inline float Coserp(float start, float end, float value)
    {
        return Linear(start, end, 1.0f - cos(value * PI * 0.5f));
    }

    // Interpolate correctly when the values wrap around 360 degrees.
    inline float Angle(float start, float end, float value)
    {
        float num = (end - start) - floor((end - start) / 360.f) * 360.f;
        if (num > 180.f) { num -= 360.f; }
        return start + num * value;
    }

}


#pragma endregion [NUMERICS]


// --------------------------------


#pragma region [VECTORS]


inline float dotProduct(const glm::vec2& lhs, const glm::vec2& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }
inline float crossProduct(const glm::vec2& lhs, const glm::vec2& rhs) { return lhs.x * rhs.y - lhs.y * rhs.x; }

inline glm::vec2 translatedVector(const glm::vec2& vector, const glm::vec2& dir, float len)
{
    return glm::vec2(vector.x + dir.x * len, vector.y + dir.y * len);
}
inline glm::vec2 flattenVector(const glm::vec2& vector, const glm::vec2& axis, float damp = 1)
{
    return translatedVector(vector, axis, safeDivided(dotProduct(vector, axis), dotProduct(axis, axis)) * damp);
}
inline glm::vec2 clipVector(const glm::vec2& vector, const glm::vec2& normal, float overbounce = 1)
{
    auto dot = dotProduct(vector, normal);
    return dot < 0 ? translatedVector(vector, normal, safeDivided(dot, dotProduct(normal, normal)) * overbounce) : vector;
}

#pragma endregion [VECTORS]


// --------------------------------


#pragma region [MISC]


template<typename T>
inline T NullCoalesce(T a, T b) { return a != NULL ? a : b; }
#define IFNULL( a, b ) ((a) != NULL ? (a) : (b))

//// Delete function object type. (std::for_each)
//struct Delete {
//    int cnt = 0;
//    template <class T>
//    void operator ()(T*& p) { if (p) ++cnt; delete p; p = NULL; }
//};

//// print<int> P = for_each(intArr, intArr + N, print<int>(cout));
//template<class T> struct print : public std::unary_function < T, void >
//{
//    print(std::function<void(T, int)> function) : fn(function), count(0) {}
//    void operator() (T x) { fn(x, count++); }
//    std::function<void(T, int)> fn;
//    int count;
//};

template <typename T>
struct instance_holder
{
    static T instance;
};

template <typename T>
T instance_holder<T>::instance;


#pragma endregion [MISC]


// --------------------------------


#pragma region [STRINGS]


#define STR(a) (std::string(a))
#define CSTR(a) ((std::string(a)).c_str())

#define STRCAT(a, b) (std::string(a) + b)
#define CSTRCAT(a, b) ((std::string(a) + b).c_str())

#define STRCAT3(a, b, c) ((std::string(a) + b) + c)
#define CSTRCAT3(a, b, c) (((std::string(a) + b) + c).c_str())

#define STRINGIFY(s) STRY(s)
#define STRY(s) #s

#define PARENTHESE( s ) ((s) != NULL ? (CSTRCAT3("(",CSTR(s),")")) : (""))

// ----------------

// format variable arguments
const char* va(const char* format, ...);

// format variable arguments
std::string format(const char *fmt, ...);

// ----------------


#pragma endregion [STRINGS]


///////////////////////////////////////////////////////////////////


#pragma region [DEBUG]


// output string
void DbgOut(const char *str);

// Debug output: file, line nr, and formatted va_list. (eg.: File.cs(85): format...)
void DbgOutput(const char *file, const int line, const char *pFormat, ...);

// Debug output format, prefixed by file and line nr. (eg.: File.cs(85): format...)
#define DBG_OUTPUT(...) DbgOutput(__FILE__, __LINE__, __VA_ARGS__)

// Debug output format.
#define DBG_WRITE(...) DbgOutput(NULL, 0, __VA_ARGS__)

// Debug output string.
#define DBG_OUTSTR(s) DbgOutput(NULL, 0, s)

// Debug output string. (without newline!)
#define DBG_STR(s) DbgOut(s)

// Debug output newline.
#define DBG_NEWLINE DbgOut("\n")

// Debug output line (80-2 hyphens)
#define DBG_LINE DbgOut(" ------------------------------------------------------------------------------\n")


// --------------------------------


// Debug output error.
#define ERR_OUTPUT(...) DbgOutput(__FILE__, __LINE__, "!ERR! " __VA_ARGS__)

// Debug output error.
#define ERR_OUTSTR(s) DbgOutput(__FILE__, __LINE__, ("!ERR! " ## s))


#pragma endregion [DEBUG]



///////////////////////////////////////////////////////////////////

#undef DECL_ENUM_ELEMENT
#undef BEGIN_ENUM
#undef END_ENUM

#ifndef GENERATE_ENUM_STRINGS
#define DECL_ENUM_ELEMENT( element ) element
#define BEGIN_ENUM( ENUM_NAME ) typedef enum tag##ENUM_NAME
#define END_ENUM( ENUM_NAME ) ENUM_NAME; \
            char* GetString##ENUM_NAME(enum tag##ENUM_NAME index);
#else
#define DECL_ENUM_ELEMENT( element ) #element
#define BEGIN_ENUM( ENUM_NAME ) char* gs_##ENUM_NAME [] =
#define END_ENUM( ENUM_NAME ) ; char* GetString##ENUM_NAME(enum \
            tag##ENUM_NAME index){ return gs_##ENUM_NAME [index]; }
#endif
