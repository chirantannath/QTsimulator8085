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
///Pack two 8-bit values into a 16-bit value (most significant byte first)
#define PACK(higher, lower) ((((higher) << 8) | (lower)) & 0xFFFFu)
///Unpack(extract) two 8-bot values from a 16-bit value (most significant byte first)
#define UNPACK(higher, lower, src) {(higher) = ((src) >> 8) & 0xFFu; (lower) = (src) & 0xFFu;}
}
//QT_END_NAMESPACE

/// Models an 8085 processor.
class Processor : public QObject
{
    Q_OBJECT
    /*These are property definitions; required for interoperability with Qt. Consider "The Property System" in Qt 
      Documentation.*/    
    ///Accumulator register
    Q_PROPERTY(data8_t accumulator MEMBER a READ getAccumulator WRITE setAccumulator 
               RESET resetAccumulator NOTIFY accumulatorChanged)
    ///Register B
    Q_PROPERTY(data8_t bRegister MEMBER b READ getBRegister WRITE setBRegister
               RESET resetBRegister NOTIFY registerBChanged)
    ///Register C
    Q_PROPERTY(data8_t cRegister MEMBER c READ getCRegister WRITE setCRegister
               RESET resetCRegister NOTIFY registerCChanged)
    ///Register D
    Q_PROPERTY(data8_t dRegister MEMBER d READ getDRegister WRITE setDRegister
               RESET resetDRegister NOTIFY registerDChanged)
    ///Register E
    Q_PROPERTY(data8_t eRegister MEMBER e READ getERegister WRITE setERegister
               RESET resetERegister NOTIFY registerEChanged)
    ///Flags (register F)
    Q_PROPERTY(flags_t flags MEMBER f READ getFlags WRITE setFlags
               RESET resetFlags NOTIFY flagsChanged)
    ///Register H
    Q_PROPERTY(data8_t hRegister MEMBER h READ getHRegister WRITE setHRegister
               RESET resetHRegister NOTIFY registerHChanged)
    ///Register L
    Q_PROPERTY(data8_t lRegister MEMBER l READ getLRegister WRITE setLRegister
               RESET resetLRegister NOTIFY registerLChanged)
    ///Pseudo register M
    Q_PROPERTY(data8_t M READ getM WRITE setM NOTIFY MChanged STORED false)
    ///Program status word (register pair AF)
    Q_PROPERTY(data16_t programStatusWord READ getProgramStatusWord WRITE setProgramStatusWord
               RESET resetProgramStatusWord STORED false)
    ///Register pair BC
    Q_PROPERTY(data16_t bcRegisterPair READ getBCRegisterPair WRITE setBCRegisterPair
               RESET resetBCRegisterPair STORED false)
    ///Register pair DE
    Q_PROPERTY(data16_t deRegisterPair READ getDERegisterPair WRITE setDERegisterPair
               RESET resetDERegisterPair STORED false)
    ///Register pair HL
    Q_PROPERTY(data16_t hlRegisterPair READ getHLRegisterPair WRITE setHLRegisterPair
               RESET resetHLRegisterPair STORED false)
    ///Program counter register
    Q_PROPERTY(memaddr_t programCounter MEMBER pc READ getProgramCounter WRITE setProgramCounter
               RESET resetProgramCounter NOTIFY programCounterChanged)
    ///Stack pointer register
    Q_PROPERTY(memaddr_t stackPointer MEMBER sp READ getStackPointer WRITE setStackPointer
               RESET resetStackPointer NOTIFY stackPointerChanged)
        
    ///All memory of the 8085.
    data8_t memory[MEMORY_SIZE];
    ///Accumulator register
    data8_t a;
    ///Register B
    data8_t b;
    ///Register C
    data8_t c;
    ///Register D
    data8_t d;
    ///Register E
    data8_t e;
    ///Register F = Flags
    flags_t f;
    ///Register H
    data8_t h;
    ///Register L
    data8_t l;
    ///Program counter register
    memaddr_t pc;
    ///Stack pointer register
    memaddr_t sp;
    ///Code to be executed for each opcode (first byte; all 256 combinations). Consider this to be the micro-program
    ///memory for the 8085, if it was modelled in a microprogrammed approach.
    std::function<void()> microprograms[256];
public:
    /// Initializes this processor. All data storage locations (memory and all registers) are set to 0.
    explicit Processor(QObject *parent = nullptr) : QObject(parent) {
        memset(memory, 0, sizeof(memory));
        a = b = c = d = e = h = l = 0; f = 0;
        
        microprograms[0x2Fu] = [&] () {//0x2Fu == CMA (complement accumulator)
            a = ~a & 0xFFu; emit accumulatorChanged();
            pc++; pc &= 0xFFu; emit programCounterChanged();
        }; //A sample.
    }
    ///Get the current value of accumulator register
    data8_t getAccumulator() const {return a;}
    ///Get the current value of register B
    data8_t getBRegister() const {return b;}
    ///Get the current value of register C
    data8_t getCRegister() const {return c;}
    ///Get the current value of register D
    data8_t getDRegister() const {return d;}
    ///Get the current value of register E
    data8_t getERegister() const {return e;}
    ///Get the current flags (value of register F)
    flags_t getFlags() const {return f;}
    ///Get the current value of register H
    data8_t getHRegister() const {return h;}
    ///Get the current value of register L
    data8_t getLRegister() const {return l;}
    ///Get the current value of pseudo register M
    data8_t getM() const {return memory[PACK(h, l)];}
    ///Get the current program status word (register pair AF)
    data16_t getProgramStatusWord() const {return PACK(a, f);}
    ///Get the current value of register pair bc
    data16_t getBCRegisterPair() const {return PACK(b, c);}
    ///Get the current value of register pair de
    data16_t getDERegisterPair() const {return PACK(d, e);}
    ///Get the current value of register pair hl
    data16_t getHLRegisterPair() const {return PACK(h, l);}
    ///Get the current value of program counter register
    memaddr_t getProgramCounter() const {return pc;}
    ///Get the current value of stack pointer register
    memaddr_t getStackPointer() const {return sp;}
    ///Copy the current memory contents into dest; starting from startLoc address in this processor and copying length
    ///bytes. Note that while copying, if because of length, the addresses being copied overshoot 0xFFFF, this function
    ///"wraps around" and continues copying from 0x0000. If the destination buffer is smaller than length bytes, the
    ///behaviour is undefined.
    void copyTo(data8_t *const dest, memaddr_t startLoc, memsize_t length) const {
        memaddr_t srcAddr; memsize_t destLoc;
        for(srcAddr = startLoc, destLoc = 0; destLoc < length; srcAddr++, srcAddr &= 0xFFFFu, destLoc++)
            dest[destLoc] = memory[srcAddr];
    }
    ///Copy the contents of source buffer src into the memory of this processor, starting from startLoc address in this
    ///processor and overwriting length bytes. Note that while overwriting, if because of length, the addresses being
    ///overwritten overshoot 0xFFFF, this function "wraps around" and continues copying from 0x0000. If the source
    ///buffer is smaller than length bytes, the behaviour is undefined. This fires the memoryBlockUpdated() and
    ///MChanged() signals.
    void overwrite(const data8_t *const src, memaddr_t startLoc, memsize_t length) {
        memaddr_t destAddr; memsize_t srcLoc;
        memsize_t minAddr = startLoc, maxAddr = startLoc;
        for(destAddr = startLoc, srcLoc = 0; srcLoc < length; destAddr++, destAddr &= 0xFFFFu, srcLoc++) {
            memory[destAddr] = src[srcLoc] & 0xFFu;
            if(minAddr > destAddr) minAddr = destAddr;
            if(maxAddr < destAddr) maxAddr = destAddr;
        }
        emit memoryBlockUpdated(minAddr, (memsize_t)maxAddr - (memsize_t)minAddr + 1u);
        emit MChanged();
    }
public slots:
    ///Runs the entire program stored in the memory of this processor system, starting from the instruction stored at
    ///the location pointed to by the program counter register. Many signals may be fired repeatedly (as per 
    ///instructions executed). In any case, programCounterChanged() is fired at least once.
    void runFull() {} //currently this is a no-op (Implementation required)
    ///Execute exactly 1 instruction pointed to by the current address stored in the program counter register. Multiple
    ///signals may be fired as per the instruction executed. In any case, programCounterChanged() is always fired.
    void stepNextInstruction() {microprograms[pc]();} //This is a sample. Edit required (external interrupt check, etc.)
    ///Set the new value of accumulator register. Fires accumulatorChanged() signal.
    void setAccumulator(data8_t value) {a = value & 0xFFu; emit accumulatorChanged();}
    ///Set the new value of register B. Fires registerBChanged() signal.
    void setBRegister(data8_t value) {b = value & 0xFFu; emit registerBChanged();}
    ///Set the new value of register C. Fires registerCChanged() signal.
    void setCRegister(data8_t value) {c = value & 0xFFu; emit registerCChanged();}
    ///Set the new value of register D. Fires registerDChanged() signal.
    void setDRegister(data8_t value) {d = value & 0xFFu; emit registerDChanged();}
    ///Set the new value of register E. Fires registerEChanged() signal.
    void setERegister(data8_t value) {e = value & 0xFFu; emit registerEChanged();}
    ///Set the flags (new value for register F). Fires flagsChanged() signal.
    void setFlags(flags_t flags) {
        f = flags & (ZERO_FLAG | SIGN_FLAG | PARITY_FLAG | CARRY_FLAG | AUXILIARY_CARRY_FLAG) & 0xFFu; 
        emit flagsChanged();
    }
    ///Set the new value of register H. Fires registerHChanged() and MChanged() signals.
    void setHRegister(data8_t value) {h = value & 0xFFu; emit registerHChanged(); emit MChanged();}
    ///Set the new value of register L. Fires registerLChanged() and MChanged() signals.
    void setLRegister(data8_t value) {l = value & 0xFFu; emit registerLChanged(); emit MChanged();}
    ///Set the new value of pseudo register M. Fires MChanged() signal.
    void setM(data8_t value) {memory[PACK(h, l)] = value & 0xFFu; emit MChanged();}
    ///Set the new value of program status word (register pair AF). Fires accumulatorChanged() and flagsChanged()
    ///signals.
    void setProgramStatusWord(data16_t value) {
        UNPACK(a, f, value & 0xFFFFu);
        emit accumulatorChanged();
        emit flagsChanged();
    }
    ///Set the new value of register pair BC. Fires registerBChanged() and registerCChanged() signals.
    void setBCRegisterPair(data16_t value) {
        UNPACK(b, c, value & 0xFFFFu); 
        emit registerBChanged(); 
        emit registerCChanged();
    }
    ///Set the new value of register pair DE. Fires registerDChanged() and registerEChanged() signals.
    void setDERegisterPair(data16_t value) {
        UNPACK(d, e, value & 0xFFFFu);
        emit registerDChanged();
        emit registerEChanged();
    }
    ///Set the new value of register pair HL. Fires registerHChanged(), registerLChanged() and MChanged() signals.
    void setHLRegisterPair(data16_t value) {
        UNPACK(h, l, value & 0xFFFFu);
        emit registerHChanged();
        emit registerLChanged();
        emit MChanged();
    }
    ///Set the new value of program counter register. Fires programCounterChanged() signal.
    void setProgramCounter(memaddr_t value) {pc = value & 0xFFFFu; emit programCounterChanged();}
    ///Set the new value of stack pointer register. Fires stackPointerChanged() signal.
    void setStackPointer(memaddr_t value) {sp = value & 0xFFFFu; emit stackPointerChanged();}
    ///Resets the accumulator register to 0. Fires accumulatorChanged() signal.
    void resetAccumulator() {setAccumulator(0u);}
    ///Resets register B to 0. Fires registerBChanged() signal.
    void resetBRegister() {setBRegister(0u);}
    ///Resets register C to 0. Fires registerCChanged() signal.
    void resetCRegister() {setCRegister(0u);}
    ///Resets register D to 0. Fires registerDChanged() signal.
    void resetDRegister() {setDRegister(0u);}
    ///Resets register E to 0. Fires registerEChanged() signal.
    void resetERegister() {setERegister(0u);}
    ///Resets all flags (register F) to 0. Fires flagsChanged() signal.
    void resetFlags() {setFlags(0u);}
    ///Resets register H to 0. Fires registerHChanged() and MChanged() signals.
    void resetHRegister() {setHRegister(0u);}
    ///Resets register L to 0. Fires registerLChanged() and MChanged() signals.
    void resetLRegister() {setLRegister(0u);}
    ///Resets program status word (register pair AF) to 0. Fires accumulatorChanged() and flagsChanged() signals.
    void resetProgramStatusWord() {setAccumulator(0); setFlags(0);}
    ///Resets BC register pair to 0. Fires registerBChanged() and registerCChanged() signal.
    void resetBCRegisterPair() {setBRegister(0u); setCRegister(0u);}
    ///Resets DE register pair to 0. Fires registerDChanged() and registerEChanged() signal.
    void resetDERegisterPair() {setDRegister(0u); setERegister(0u);}
    ///Resets HL register pair to 0. Fires registerHChanged(), registerLChanged() and MChanged() signals.
    void resetHLRegisterPair() {h = l = 0u; emit registerHChanged(); emit registerLChanged(); emit MChanged();}
    ///Resets program counter register. Fires programCounterChanged() signal.
    void resetProgramCounter() {setProgramCounter(0u);}
    ///Resets stack pointer register. Fires stackPointerChanged() signal.
    void resetStackPointer() {setStackPointer(0u);}
    ///Resets entire memory to 0 (all 65,536 bytes, may take time). Fires memoryBlockUpdated() and MChanged() signals.
    void resetMemory() {
        memset(memory, 0, sizeof(memory)); 
        emit memoryBlockUpdated(0u, MEMORY_SIZE); 
        emit MChanged();
    }
    ///Resets the entire processor state EXCEPT MEMORY (all registers to 0). This is equivalent to the RESET_IN signal
    ///to the 8085. Fires accumulatorChanged(), flagsChanged(), registerBChanged(), registerCChanged(), 
    ///registerDChanged(), registerEChanged(), registerHChanged(), registerLChanged(), programCounterChanged()
    ///and stackPointerChanged() signals.
    void RESET_IN() {//TODO: Reset must also affect interrupt masks
        resetProgramStatusWord(); resetBCRegisterPair(); resetDERegisterPair(); resetHLRegisterPair();
        resetProgramCounter(); resetStackPointer();
    }
    ///Resets the entire object (all data to default values). Fires ALL signals defined by the Processor class.
    void resetAll() {
        RESET_IN(); resetMemory();
    }
signals:
    ///Fired when the accumulator register is changed.
    void accumulatorChanged();
    ///Fired when register B is changed.
    void registerBChanged();
    ///Fired when register C is changed.
    void registerCChanged();
    ///Fired when register D is changed.
    void registerDChanged();
    ///Fired when register E is changed.
    void registerEChanged();
    ///Fired when flags are changed (when register F is changed).
    void flagsChanged();
    ///Fired when register H is changed. Almost always accompanied by MChanged() signal.
    void registerHChanged();
    ///Fired when register L is changed. Almost always accompanied by MChanged() signal.
    void registerLChanged();
    ///Fired when the program counter register is changed.
    void programCounterChanged();
    ///Fired when the stack pointer register is changed.
    void stackPointerChanged();
    ///Fired when the value of pseudo register M changes (either by a direct write or by modifying registers H or L).    
    void MChanged();
    ///Fired when a portion of the memory to this processor changes. It is guaranteed that startLoc will be always 
    ///within the range 0x0000 to 0xFFFF inclusive, and blockSize will be within the range 0x0000 and 0x10000
    ///inclusive.
    void memoryBlockUpdated(memaddr_t startLoc, memsize_t blockSize);
};

namespace processor_h {
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
}
}

#endif // PROCESSOR_H
