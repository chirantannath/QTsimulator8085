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
#include <QMetaType>
#include <functional>
#include <utility>
#include <cstdint>
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
#define PACK(higher, lower) ((((higher) << 8) | (lower)) & 0xFFFFu)
#define UNPACK(higher, lower, src) {(higher) = ((src) >> 8) & 0xFFu; (lower) = (src) & 0xFFu;}
}
//QT_END_NAMESPACE

/// Models an 8085 processor.
class Processor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(data8_t accumulator MEMBER a READ getAccumulator WRITE setAccumulator 
               RESET resetAccumulator NOTIFY accumulatorChanged)
    Q_PROPERTY(data8_t bRegister MEMBER b READ getBRegister WRITE setBRegister
               RESET resetBRegister NOTIFY registerBChanged)
    Q_PROPERTY(data8_t cRegister MEMBER c READ getCRegister WRITE setCRegister
               RESET resetCRegister NOTIFY registerCChanged)
    Q_PROPERTY(data8_t dRegister MEMBER d READ getDRegister WRITE setDRegister
               RESET resetDRegister NOTIFY registerDChanged)
    Q_PROPERTY(data8_t eRegister MEMBER e READ getERegister WRITE setERegister
               RESET resetERegister NOTIFY registerEChanged)
    Q_PROPERTY(flags_t flags MEMBER f READ getFlags WRITE setFlags
               RESET resetFlags NOTIFY flagsChanged)
    Q_PROPERTY(data8_t hRegister MEMBER h READ getHRegister WRITE setHRegister
               RESET resetHRegister NOTIFY registerHChanged)
    Q_PROPERTY(data8_t lRegister MEMBER l READ getLRegister WRITE setLRegister
               RESET resetLRegister NOTIFY registerLChanged)
    Q_PROPERTY(data8_t M READ getM WRITE setM NOTIFY MChanged STORED false)
    Q_PROPERTY(data16_t bcRegisterPair READ getBCRegisterPair WRITE setBCRegisterPair
               RESET resetBCRegisterPair STORED false)
    Q_PROPERTY(data16_t deRegisterPair READ getDERegisterPair WRITE setDERegisterPair
               RESET resetDERegisterPair STORED false)
    Q_PROPERTY(data16_t hlRegisterPair READ getHLRegisterPair WRITE setHLRegisterPair
               RESET resetHLRegisterPair STORED false)
    Q_PROPERTY(memaddr_t programCounter MEMBER pc READ getProgramCounter WRITE setProgramCounter
               RESET resetProgramCounter NOTIFY programCounterChanged)
    Q_PROPERTY(memaddr_t stackPointer MEMBER sp READ getStackPointer WRITE setStackPointer
               RESET resetStackPointer NOTIFY stackPointerChanged)
        
    ///All memory of the 8085.
    data8_t memory[MEMORY_SIZE];
    ///Accumulator register
    data8_t a;
    
    data8_t b;
    
    data8_t c;
    
    data8_t d;
    
    data8_t e;
    
    flags_t f;
    
    data8_t h;
    
    data8_t l;
    
    memaddr_t pc;
    
    memaddr_t sp;
    
    std::function<void()> microprograms[256];
public:
    
    explicit Processor(QObject *parent = nullptr) : QObject(parent) {
        memset(memory, 0, sizeof(memory));
        a = b = c = d = e = h = l = 0; f = 0;
        
        microprograms[0x2Fu] = [&] () {
            a = ~a & 0xFFu; emit accumulatorChanged();
            pc++; pc &= 0xFFu; emit programCounterChanged();
        }; //A sample.
    }
    
    data8_t getAccumulator() const {return a;}
    
    data8_t getBRegister() const {return b;}
    
    data8_t getCRegister() const {return c;}
    
    data8_t getDRegister() const {return d;}
    
    data8_t getERegister() const {return e;}
    
    flags_t getFlags() const {return f;}
    
    data8_t getHRegister() const {return h;}
    
    data8_t getLRegister() const {return l;}
    
    data8_t getM() const {return memory[PACK(h, l)];}
    
    data16_t getBCRegisterPair() const {return PACK(b, c);}
    
    data16_t getDERegisterPair() const {return PACK(d, e);}
    
    data16_t getHLRegisterPair() const {return PACK(h, l);}
    
    memaddr_t getProgramCounter() const {return pc;}
    
    memaddr_t getStackPointer() const {return sp;}
    
    void copyTo(data8_t *const dest, memaddr_t startLoc, memsize_t length) const {
        memaddr_t srcAddr; memsize_t destLoc;
        for(srcAddr = startLoc, destLoc = 0; destLoc < length; srcAddr++, srcAddr &= 0xFFFFu, destLoc++)
            dest[destLoc] = memory[srcAddr];
    }
    
    void overwrite(const data8_t *const src, memaddr_t startLoc, memsize_t length) {
        memaddr_t destAddr; memsize_t srcLoc;
        memsize_t minAddr = startLoc, maxAddr = startLoc;
        for(destAddr = startLoc, srcLoc = 0; srcLoc < length; destAddr++, destAddr &= 0xFFFFu, srcLoc++) {
            memory[destAddr] = src[srcLoc] & 0xFFu;
            if(minAddr > destAddr) minAddr = destAddr;
            if(maxAddr < destAddr) maxAddr = destAddr;
        }
        emit memoryBlockUpdated(minAddr, maxAddr - minAddr + 1u);
    }
public slots:
    
    void runFull() {} //currently this is a no-op
    
    void setAccumulator(data8_t value) {a = value & 0xFFu; emit accumulatorChanged();}
    
    void setBRegister(data8_t value) {b = value & 0xFFu; emit registerBChanged();}
    
    void setCRegister(data8_t value) {c = value & 0xFFu; emit registerCChanged();}
    
    void setDRegister(data8_t value) {d = value & 0xFFu; emit registerDChanged();}
    
    void setERegister(data8_t value) {e = value & 0xFFu; emit registerEChanged();}
    
    void setFlags(flags_t flags) {
        f = flags & (ZERO_FLAG | SIGN_FLAG | PARITY_FLAG | CARRY_FLAG | AUXILIARY_CARRY_FLAG) & 0xFFu; 
        emit flagsChanged();
    }
    
    void setHRegister(data8_t value) {h = value & 0xFFu; emit registerHChanged(); emit MChanged();}
    
    void setLRegister(data8_t value) {l = value & 0xFFu; emit registerLChanged(); emit MChanged();}
    
    void setM(data8_t value) {memory[PACK(h, l)] = value & 0xFFu; emit MChanged();}
    
    void setBCRegisterPair(data16_t value) {
        UNPACK(b, c, value & 0xFFFFu); 
        emit registerBChanged(); 
        emit registerCChanged();
    }
    
    void setDERegisterPair(data16_t value) {
        UNPACK(d, e, value & 0xFFFFu);
        emit registerDChanged();
        emit registerEChanged();
    }
    
    void setHLRegisterPair(data16_t value) {
        UNPACK(h, l, value & 0xFFFFu);
        emit registerHChanged();
        emit registerLChanged();
        emit MChanged();
    }
    
    void setProgramCounter(memaddr_t value) {pc = value & 0xFFFFu; emit programCounterChanged();}
    
    void setStackPointer(memaddr_t value) {sp = value & 0xFFFFu; emit stackPointerChanged();}
    
    void resetAccumulator() {setAccumulator(0u);}
    
    void resetBRegister() {setBRegister(0u);}
    
    void resetCRegister() {setCRegister(0u);}
    
    void resetDRegister() {setDRegister(0u);}
    
    void resetERegister() {setERegister(0u);}
    
    void resetFlags() {setFlags(0u);}
    
    void resetHRegister() {setHRegister(0u);}
    
    void resetLRegister() {setLRegister(0u);}
    
    void resetBCRegisterPair() {setBRegister(0u); setCRegister(0u);}
    
    void resetDERegisterPair() {setDRegister(0u); setERegister(0u);}
    
    void resetHLRegisterPair() {setHRegister(0u); setLRegister(0u);}
    
    void resetProgramCounter() {setProgramCounter(0u);}
    
    void resetStackPointer() {setStackPointer(0u);}
    
    void resetMemory() {
        memset(memory, 0, sizeof(memory)); 
        emit memoryBlockUpdated(0u, MEMORY_SIZE); 
        emit MChanged();
    }
signals:
    
    void accumulatorChanged();
    
    void registerBChanged();
    
    void registerCChanged();
    
    void registerDChanged();
    
    void registerEChanged();
    
    void flagsChanged();
    
    void registerHChanged();
    
    void registerLChanged();
    
    void programCounterChanged();
    
    void stackPointerChanged();
    
    void MChanged();
    
    void memoryBlockUpdated(memaddr_t startLoc, memsize_t blockSize);
};

namespace processor_h {
inline void registerHeaderMetaTypes() {
    qRegisterMetaType<data8_t>("data8_t");
    qRegisterMetaType<data8_calc_t>("data8_calc_t");
    qRegisterMetaType<data16_t>("data16_t");
    qRegisterMetaType<data16_calc_t>("data16_calc_t");
    qRegisterMetaType<memaddr_t>("memaddr_t");
    qRegisterMetaType<memsize_t>("memsize_t");
    qRegisterMetaType<flags_t>("flags_t");
}
}

#endif // PROCESSOR_H
