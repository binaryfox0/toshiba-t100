#ifndef INTERNAL_H
#define INTERNAL_H

#include <stdint.h>

#if defined(_MSC_VER)
#   define INLINE __forceinline
#else
#   define INLINE static inline __attribute((always_inline))
#endif

#ifdef __cplusplus
#   define EXTERN_C_BEGIN extern "C" {
#   define EXTERN_C_END   }
#else
#   define EXTERN_C_BEGIN
#   define EXTERN_C_END
#endif


// OS stuff
#ifdef __linux__
#   define PLATFORM_LINUX 
#elif defined(_WIN32)
#   define PLATFORM_WIN32
#   error You can comment this line to test on Windows. I don't have it anyway.
#elif defined(__APPLE__) || defined(__MACH__)
#   define PLATFORM_MACOS
#   error You can comment this line to test on MacOS. It's very expensive though.
#elif defined(__ANDROID__)
#   error You have nothing to do, do you?
#else
#   error Unsupported platform, you can make a pull request to fix this
#endif

#ifdef PLATFORM_WIN32
#   include <sdkddkver.h>
// Source:
// - https://learn.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers?redirectedfrom=MSDN#macros-for-conditional-declarations
// - https://en.wikipedia.org/wiki/ANSI_escape_code#DOS_and_Windows 
#   if defined(NTDDI_VERSION) && (NTDDI_VERSION >= NTDDI_WIN10_TH2)
#       define ANSIES(str) str
#   endif
#else
#   define ANSIES(str) str
#endif

#if defined(BUILD_DEB) || defined(BUILD_RELWITHDEBINFO)
#   define BUILD_DEBUG
#endif

#if defined(BUILD_DEB) || defined(BUILD_RELWITHDEBINFO)
#   define info(fmt, ...) printf(ANSIES("\x1b[1;34m") "info" ANSIES("\x1b[0m") ": " fmt "\n", ##__VA_ARGS__)
#else
#   define info(fmt, ...)
#endif

#define warn(fmt, ...) printf(ANSIES("\x1b[1;33m") "warn" ANSIES("\x1b[0m") ": " fmt "\n", ##__VA_ARGS__)
#define error(fmt, ...) fprintf(stderr, ANSIES("\x1b[1;31m") "error" ANSIES("\x1b[0m") ": " fmt "\n", ##__VA_ARGS__)

#define ARRSZ(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct __Vec2
{
    float x, y;
} __Vec2;

#ifdef __cplusplus
#include <type_traits>
#include <sstream>
#include <iomanip>

template <typename T, typename = typename std::enable_if<std::is_unsigned<T>::value>::type>
INLINE std::string to_hex(T val, bool prefix = true) {
    std::stringstream ss;
    ss << (prefix ? "0x" : "")
       << std::hex << std::setw(sizeof(T) * 2)
       << std::uppercase << std::setfill('0')
       << static_cast<unsigned long>(val);  // cast avoids char printing weirdness
    return ss.str();
}

template <typename T>
INLINE std::string to_bin(T val) {
    char* raw = reinterpret_cast<char*>(&val);
    std::stringstream ss;
    ss << "0b";
    bool found_one = false;
    for (size_t i = 0; i < sizeof(T) * 8; ++i) {
        size_t byte_index = i / 8;
        size_t bit_index = 7 - (i % 8); // MSB first

        bool bit = (raw[byte_index] >> bit_index) & 0x1;

        if (bit)
            found_one = true;

        if (found_one)
            ss << (bit ? '1' : '0');
    }
    return ss.str();
}

template<typename T>
INLINE bool in_range(T begin, T end, T value) {
    return value >= begin && value <= end;
}
#endif

#endif