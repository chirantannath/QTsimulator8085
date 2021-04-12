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
#ifndef PROCESSOR_H
#define PROCESSOR_H
    
#include <QObject>
#include <cstdint>

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
#define MEMORY_SIZE     ((memsize_t)0x10000)

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

//QT_BEGIN_NAMESPACE
///Utility functions.
namespace {
#define PACK(higher, lower) (data16_t)((((higher) << 8) | (lower)) & 0xFFFFu)
#define UNPACK(higher, lower, src) {(higher) = ((src) >> 8) & 0xFFu; (lower) = (src) & 0xFFu;}
}
//QT_END_NAMESPACE

/// Models an 8085 processor.
class Processor : public QObject
{
    Q_OBJECT
    
    ///All memory of the 8085.
    data8_t memory[MEMORY_SIZE];
    ///Accumulator register
    data8_t a;
    
    data8_t b;
    
    data8_t c;
    
    data8_t d;
    
    data8_t e;
    
    data8_t f;
    
    data8_t h;
    
    data8_t l;
    
public:
    explicit Processor(QObject *parent = nullptr) : QObject(parent) {}
    
public slots:
    
    void runFull(); //currently this is a no-op
    
    void setARegister(data8_t);
    
    void setBRegister(data8_t);
    
    void setCRegister(data8_t);
    
    void setDRegister(data8_t);
    
    void setERegister(data8_t);
    
    void setFlags(data8_t);
    
    void setHRegister(data8_t);
    
    void setLRegister(data8_t);
signals:

};

#endif // PROCESSOR_H
