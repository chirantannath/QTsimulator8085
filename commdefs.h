/*MIT License

Copyright (c) 2021 Chirantan Nath

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#ifndef COMMDEFS
#define COMMDEFS

#include <cstdint>
#include <cctype>
#include <cstring>

/// Data type 8-bit for 8085.
typedef uint_least8_t   data8_t; //"least" to optimise memory consumption (depends on target architecture)
/// 8-bit data calculation type; this is the type on which 8-bit calculations are to be done. This will be larger than
/// data8_t to permit checking of overflow/underflow.
typedef uint_fast16_t   data8_calc_t; //"fast" to optimise computational speed (depends on target architecture)
/// Data type 16-bit for the 8085.
typedef uint_least16_t  data16_t;
/// 16-bit data calculation type; this is the type on which 16-bit calculations are to be done. This will be larger than
/// data16_t to permit checking of overflow/underflow.
typedef uint_fast32_t   data16_calc_t;

/// Address type for the 8085; exactly the same as data16_t. Think of this as the pointer type for the 8085.
typedef data16_t        memaddr_t;
/// Memory size type for the 8085. This will be larger than memaddr_t because values of this type may be equal to (or
/// greater than) 0xFFFF(65535).
typedef uint_fast32_t   memsize_t;
/// Memory size (in units of data8_t blocks) accessible to this processor. The current implementation uses all 65536
/// bytes of the memory address space.
#define MEMORY_SIZE     ((memsize_t)0x10000u)

/// I/O port address type for the 8085; exactly the same as data8_t.
typedef data8_t         ioaddr_t;
/// Number of I/O ports supported by this implementation (all 256).
#define IO_PORT_SIZE    256

/// Type for the flag register, exactly the same as data8_t.
typedef data8_t         flags_t;
/// Sign flag (S) at D7 of flag register.
#define SIGN_FLAG               ((flags_t)0x80u)
/// Zero flag (Z) at D6 of flag register.
#define ZERO_FLAG               ((flags_t)0x40u)
/// Auxiliary carry flag (AC) at D4 of flag register.
#define AUXILIARY_CARRY_FLAG    ((flags_t)0x10u)
/// Parity flag (P) at D2 of flag register
#define PARITY_FLAG             ((flags_t)0x04u)
/// Carry flag (CY) at D0 of flag register
#define CARRY_FLAG              ((flags_t)0x01u)
/// Allowed bits in the flag register/flags_t type
#define ALLOWED_FLAGS           ((flags_t)0xD5u)

///A case-insensitive comparator function object for const char*.
struct CaseInsensitive {
    bool operator()(const char *, const char *) const;
    static CaseInsensitive comparator;
};

#include <string>
///A case-insensitive comparator function object for std::string.
struct StringInsensitive {
    bool operator()(const std::string&, const std::string&) const;
    static StringInsensitive comparator;
};

#include <QString>
///Store digits from 0 to 9, A to Z. One extra for NULL.
extern const char DIGITS[37]; //1 extra for null
///Get a 0-padded 2-digit hexadecimal representation for value.
QString getHex8(data8_t value);
///Get a 0-padded 4-digit hexadecimal representation for value.
QString getHex16(data16_t value);
///Get "0" or "1" according to whether bit at position <position> in value is 0 or 1 respectively. Positions are counted from 0
///from least-significant bit position.
QString getBinDigit(data8_t value, int position);
///Convert value into a string representation with given base {min 2, max 36, digits are from 0 to 9, A to Z.}
std::string unsignedNumber(unsigned long long value, unsigned short base=10);

///Pack two 8-bit values into a 16-bit value (most significant byte first)
#define PACK(higher, lower) ((((higher) << 8) | (lower)) & 0xFFFFu)
///Unpack(extract) two 8-bot values from a 16-bit value (most significant byte first)
#define UNPACK(higher, lower, src) {(higher) = ((src) >> 8) & 0xFFu; (lower) = (src) & 0xFFu;}
///Set a flag into a flags_t data byte
#define SET_FLAG(data, flag) ((data) = ((data) | (flag)) & ALLOWED_FLAGS)
///Check a flag from a flags_t data byte. The following should be nonzero if flag is present
///in data.
#define CHECK_FLAG(data, flag) ((data) & (flag))
///Unset a flag into a flags_t data byte
#define UNSET_FLAG(data, flag) ((data) = (data) & ~(flag) & ALLOWED_FLAGS)
///Set a flag in a flags_t data byte to the specific value.
#define SET_SPEC_FLAG(data, flag, value) {\
if (value) {SET_FLAG(data, flag);}\
else {UNSET_FLAG(data, flag);}}

//The following are required because not all architectures use 2's complement.
///Negate a 4-bit signed integer in 2's complement form.
#define NEGATE4(x) ((~(x) + 1u) & 0xFu)
///Negate an 8-bit signed integer in 2's complement form.
#define NEGATE8(x) ((~(x) + 1u) & 0xFFu)
///Negate a 16-bit signed integer in 2's complement form.
#define NEGATE16(x) ((~(x) + 1u) & 0xFFFFu)
///Decrement a 4-bit lvalue in 2's complemennt form.
#define DECREMENT4(x) ((x) = (((x) + 0xFu) & 0xFu))
///Decrement an 8-bit lvalue in 2's complement form.
#define DECREMENT8(x) ((x) = (((x) + 0xFFu) & 0xFFu))
///Decrement a 16-bit lvalue in 2's complement form.
#define DECREMENT16(x) ((x) = (((x) + 0xFFFFu) & 0xFFFFu))
///Swap two values with given temporary location.
#define SWAP(a, b, t) {(t) = (a); (a) = (b); (b) = (t);}

///Lookup table for parity flag values. We use lookup tables for faster resolving.
extern const bool PARITY_LOOKUP[256];

#include <QMetaType>

namespace commdefs_h {
///Required for interoperability with the Qt multithreading system. To pass signals between multiple threads, the Qt
///system needs to know what type names are we using to transfer data, including typedefs. MUST be among the first
///functions called in main().
inline void registerHeaderMetaTypes() {
    qRegisterMetaType<data8_t>("data8_t");
    qRegisterMetaType<data8_calc_t>("data8_calc_t");
    qRegisterMetaType<data16_t>("data16_t");
    qRegisterMetaType<data16_calc_t>("data16_calc_t");
    qRegisterMetaType<memaddr_t>("memaddr_t");
    qRegisterMetaType<memsize_t>("memsize_t");
    qRegisterMetaType<flags_t>("flags_t");
    qRegisterMetaType<ioaddr_t>("ioaddr_t");
}
}


#endif //COMMDEFS
