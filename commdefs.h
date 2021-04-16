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
    bool operator()(const char *s1, const char *s2) const {
        for(; *s1 != '\0' && *s2 != '\0'; s1++, s2++) {
            char c1 = std::toupper(*s1), c2 = std::toupper(*s2);
            if(c1 != c2) return c1 < c2;
        }
        return *s1 != '\0' && *s2 == '\0';
    }
};

#endif // COMMDEFS
    
