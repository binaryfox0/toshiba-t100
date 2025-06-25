#ifndef INTERNAL_H
#define INTERNAL_H

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

typedef struct __Vec2
{
    int x, y;
} __Vec2;

#ifdef __cplusplus
#include <type_traits>
#include <sstream>
#include <iomanip>

template <typename T, typename = typename std::enable_if<std::is_unsigned<T>::value>::type>
INLINE std::string to_hex(T val, bool postfix = false) {
    std::stringstream ss;
    ss << (postfix ? "0x" : "")
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