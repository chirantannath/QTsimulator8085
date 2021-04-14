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
    Q_PROPERTY(data8_t accumulator MEMBER a READ getAccumulator NOTIFY accumulatorChanged)
    ///Register B
    Q_PROPERTY(data8_t bRegister MEMBER b READ getBRegister NOTIFY registerBChanged)
    ///Register C
    Q_PROPERTY(data8_t cRegister MEMBER c READ getCRegister NOTIFY registerCChanged)
    ///Register D
    Q_PROPERTY(data8_t dRegister MEMBER d READ getDRegister NOTIFY registerDChanged)
    ///Register E
    Q_PROPERTY(data8_t eRegister MEMBER e READ getERegister NOTIFY registerEChanged)
    ///Flags (register F)
    Q_PROPERTY(flags_t flags MEMBER f READ getFlags NOTIFY flagsChanged)
    ///Register H
    Q_PROPERTY(data8_t hRegister MEMBER h READ getHRegister NOTIFY registerHChanged)
    ///Register L
    Q_PROPERTY(data8_t lRegister MEMBER l READ getLRegister NOTIFY registerLChanged)
    ///Pseudo register M
    Q_PROPERTY(data8_t M READ getM NOTIFY MChanged)
    ///Program status word (register pair AF)
    Q_PROPERTY(data16_t programStatusWord READ getProgramStatusWord STORED false)
    ///Register pair BC
    Q_PROPERTY(data16_t bcRegisterPair READ getBCRegisterPair STORED false)
    ///Register pair DE
    Q_PROPERTY(data16_t deRegisterPair READ getDERegisterPair STORED false)
    ///Register pair HL
    Q_PROPERTY(data16_t hlRegisterPair READ getHLRegisterPair STORED false)
    ///Program counter register
    Q_PROPERTY(memaddr_t programCounter MEMBER pc READ getProgramCounter NOTIFY programCounterChanged)
    ///Stack pointer register
    Q_PROPERTY(memaddr_t stackPointer MEMBER sp READ getStackPointer NOTIFY stackPointerChanged)
    ///Interrupt enable latch
    Q_PROPERTY(bool IE READ isInterruptEnabled NOTIFY interruptEnableStatusChanged)
    ///Interrupt request flag/latch
    Q_PROPERTY(bool INTR READ isInterruptRequested WRITE setInterruptRequest)
    ///Interrupt acknowledge flag/latch
    Q_PROPERTY(bool INTA READ isInterruptAcknowledged NOTIFY interruptAcknowledgeStatusChanged)
    ///Interrupt Service Routine vector instruction to execute on INTR (is always one of the RST instructions)
    Q_PROPERTY(data8_t INTRVector READ getINTRVector WRITE setINTRVector)
    ///TRAP interrupt flag/latch
    Q_PROPERTY(bool TRAP READ isTRAPRequested WRITE setTRAPRequest)
    ///RST 7.5 interrupt flag/latch
    Q_PROPERTY(bool RST7_5 READ isRestart7_5Requested WRITE setRestart7_5Request NOTIFY restart7_5RequestStatusChanged)
    ///RST 6.5 interrupt flag/latch
    Q_PROPERTY(bool RST6_5 READ isRestart6_5Requested WRITE setRestart6_5Request)
    ///RST 5.5 interrupt flag/latch
    Q_PROPERTY(bool RST5_5 READ isRestart5_5Requested WRITE setRestart5_5Request)
    ///RST 7.5 Mask latch (1 if disabled)
    Q_PROPERTY(bool M7_5 READ maskRestart7_5 NOTIFY maskRestart7_5Changed)
    ///RST 6.5 Mask latch (1 if disabled)
    Q_PROPERTY(bool M6_5 READ maskRestart6_5 NOTIFY maskRestart6_5Changed)
    ///RST 5.5 Mask latch (1 if disabled)
    Q_PROPERTY(bool M5_5 READ maskRestart5_5 NOTIFY maskRestart5_5Changed)
    ///Serial output data latch
    Q_PROPERTY(bool SOD READ serialOutputDataLatch NOTIFY serialOutput)
    ///Serial input data latch
    Q_PROPERTY(bool SID READ serialInputDataLatch WRITE setSerialInputLatch)
    
    ///Code to be executed for each opcode (first byte; all 256 combinations). Consider this to be the micro-program
    ///memory for the 8085, if it was modelled in a microprogrammed approach.
    std::function<void()> microprograms[256];
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
    ///Interrupt enable flag.
    unsigned ie : 1;
    ///INTR signal flag.
    unsigned intr : 1;
    ///Interrupt Acknowledge latch.
    unsigned inta : 1;
    ///Reset location for INTR signal (0 to 7 for RST 0 to 7).
    unsigned intrVec : 3;
    ///TRAP interrupt flag.
    unsigned trap : 1;
    ///RST 7.5 interrupt latch; reset on RESET_IN or by bit D4 of accumulator when SIM is executed.
    unsigned rst7_5 : 1;
    ///RST 6.5 interrupt latch.
    unsigned rst6_5 : 1;
    ///RST 5.5 interrupt latch.
    unsigned rst5_5 : 1;
    ///RST 7_5 interrupt mask latch (0 if enabled).
    unsigned m7_5 : 1;
    ///RST 6_5 interrupt mask latch (0 if enabled).
    unsigned m6_5 : 1;
    ///RST 5_5 interrupt mask latch (0 if enabled).
    unsigned m5_5 : 1;
    ///Serial output data latch.
    unsigned sod : 1;
    ///Serial input data latch.
    unsigned sid : 1;
public:
    /// Initializes this processor. All data storage locations (memory and all registers) are set to 0.
    explicit Processor(QObject *parent = nullptr) : QObject(parent) {
        memset(memory, 0, sizeof(memory));
        pc = sp = 0u;
        a = b = c = d = e = h = l = 0u; f = 0u; ie = intr = inta = trap = rst7_5 = rst6_5 = rst5_5 = sod = sid = 0u;
        m5_5 = m6_5 = m7_5 = 1u; //Initial state is these external interrupts are masked.
        
        //Insert instruction codes here
        microprograms[0x00u] = [&](){pc++; pc &= 0xFFFFu; emit programCounterChanged();};
        
        microprograms[0x2Fu] = [&](){//CMA (complement accumulator)
            a = ~a & 0xFFu; emit accumulatorChanged();
            pc++; pc &= 0xFFFFu; emit programCounterChanged();
        }; //A sample.
    }
    ///Explicit destructor required for proper inheritance to QObject
    virtual ~Processor() {}
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
    ///Get the current value stored in serial input data latch. This is the serial data bit read by RIM instruction.
    bool serialInputDataLatch() const {return sid;}
    ///Get the value stored in serial output data latch/last value sent by SIM.
    bool serialOutputDataLatch() const {return sod;}
    ///Gets the RST 7.5 mask status (true if disabled).
    bool maskRestart7_5() const {return m7_5;}
    ///Gets the RST 6.5 mask status (true if disabled).
    bool maskRestart6_5() const {return m6_5;}
    ///Gets the RST 5.5 mask status (true if disabled).
    bool maskRestart5_5() const {return m5_5;}
    ///True if RST 7.5 has been requested.
    bool isRestart7_5Requested() const {return rst7_5;}
    ///True if RST 6.5 has been requested.
    bool isRestart6_5Requested() const {return rst6_5;}
    ///True if RST 5.5 has been requested.
    bool isRestart5_5Requested() const {return rst5_5;}
    ///True if TRAP has been requested.
    bool isTRAPRequested() const {return trap;}
    ///Returns the RST instruction executed when INTR interrupt is requested.
    data8_t getINTRVector() const {return (data8_t)((intrVec << 3) | 0xC7u);}
    ///True if INTR interrupt has been requested.
    bool isInterruptRequested() const {return intr;}
    ///True if interrupts are enabled by the processor.
    bool isInterruptEnabled() const {return ie;}
    ///True if the last interrupt signal has been acknowledged.
    bool isInterruptAcknowledged() const {return inta;}
    ///Gets the data byte stored at the address "index" in the 64K memory of this processor.
    data8_t getMemoryByte(memaddr_t index) const {return memory[index];}
    ///Copy the current memory contents into dest; starting from startLoc address in this processor and copying length
    ///bytes. Note that while copying, if because of length, the addresses being copied overshoot 0xFFFF, this function
    ///"wraps around" and continues copying from 0x0000. If the destination buffer is smaller than length bytes, the
    ///behaviour is undefined.
    void copyTo(data8_t *const dest, memaddr_t startLoc, memsize_t length) const {
        memaddr_t srcAddr; memsize_t destLoc;
        for(srcAddr = startLoc & 0xFFFFu, destLoc = 0; destLoc < length; srcAddr++, srcAddr &= 0xFFFFu, destLoc++)
            dest[destLoc] = memory[srcAddr];
    }
    ///Copy the contents of source buffer src into the memory of this processor, starting from startLoc address in this
    ///processor and overwriting length bytes. Note that while overwriting, if because of length, the addresses being
    ///overwritten overshoot 0xFFFF, this function "wraps around" and continues copying from 0x0000. If the source
    ///buffer is smaller than length bytes, the behaviour is undefined. This fires the memoryBlockUpdated() and
    ///MChanged() signals.
    void overwrite(const data8_t *const src, memaddr_t startLoc, memsize_t length) {
        startLoc &= 0xFFFFu;
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
    void stepNextInstruction() {
        microprograms[pc](); //This is a sample. Edit required (external interrupt check, etc.)
    }
    ///Stores data at the address in the 64K memory of this processor. data is ANDed with 0xFF and address ANDed with
    ///0xFFFF before any operation is performed. Fires memoryBlockUpdated() and MChanged() signals.
    void setMemoryByte(memaddr_t address, data8_t data) {
        address &= 0xFFFFu; data &= 0xFFu;
        memory[address] = data;
        emit memoryBlockUpdated(address, 1u);
        if(address == PACK(h, l)) emit MChanged();
    }
    ///Sets the value in serial input latch. This is the value read by a subsequent RIM instruction.
    void setSerialInputLatch(bool flag) {sid = flag ? 1u : 0u;}
    ///Sets the request status for RST 7.5 (true if interrupt requested). Fires restart7_5RequestStatusChanged() signal.
    void setRestart7_5Request(bool flag) {rst7_5 = flag ? 1u : 0u; emit restart7_5RequestStatusChanged();}
    ///Sets the request status for RST 6.5 (true if interrupt requested).
    void setRestart6_5Request(bool flag) {rst6_5 = flag ? 1u : 0u;}
    ///Sets the request status for RST 5.5 (true if interrupt requested).
    void setRestart5_5Request(bool flag) {rst5_5 = flag ? 1u : 0u;}
    ///Sets the request status for TRAP (true if interrupt requested).
    void setTRAPRequest(bool flag) {trap = flag ? 1u : 0u;}
    ///Sets the request status for INTR (true if interrupt requested).
    void setInterruptRequest(bool flag) {intr = flag ? 1u : 0u;}
    ///Sets the RST instruction which is executed on INTR interrupt request. Note that only bits D5, D4, D3 are used
    ///and rest are ignored (the result is the instruction set is always RST 0 through 7).
    void setINTRVector(data8_t value) {intrVec = (value >> 3) & 7u;}
    
    ///Resets entire memory to 0 (all 65,536 bytes, may take time). Fires memoryBlockUpdated() and MChanged() signals.
    void resetMemory() {
        memset(memory, 0, sizeof(memory)); 
        emit memoryBlockUpdated(0u, MEMORY_SIZE); 
        emit MChanged();
    }
    ///Resets the entire processor state EXCEPT MEMORY (all registers to 0). This is equivalent to the RESET_IN signal
    ///to the 8085. Fires ALL signals EXCEPT memoryBlockUpdated().
    void RESET_IN() {//TODO: Reset must also affect interrupt masks
        pc = sp = 0u;
        a = b = c = d = e = h = l = 0u; f = 0u; ie = sod = inta = rst7_5 = 0u;
        m5_5 = m6_5 = m7_5 = 1u; //Initial state is these external interrupts are masked.
        emit accumulatorChanged(); emit registerBChanged(); emit registerCChanged(); emit registerDChanged();
        emit registerEChanged(); emit registerHChanged(); emit registerLChanged(); emit flagsChanged();
        emit MChanged(); emit interruptEnableStatusChanged(); emit maskRestart5_5Changed();
        emit maskRestart6_5Changed(); emit maskRestart7_5Changed(); emit serialOutput();
        emit programCounterChanged(); emit stackPointerChanged(); emit interruptEnableStatusChanged();
        emit restart7_5RequestStatusChanged(); emit interruptAcknowledgeStatusChanged();
    }
    ///Resets the entire object (all data to default values EXCEPT those values which are externally controlled). Fires 
    ///ALL signals defined by the Processor class.
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
    ///Fired when the value of pseudo register M changes (either by a direct write, by modifying registers H or L, or 
    ///by external memory writes).    
    void MChanged();
    ///Fired when this processor executes a SIM instruction and a serial output bit is sent on SOD latch.
    void serialOutput();
    ///Fired when the request status latch for RST 7.5 is changed.
    void restart7_5RequestStatusChanged();
    ///Fired when this processor changes the masking state for RST 7.5.
    void maskRestart7_5Changed();
    ///Fired when this processor changes the masking state for RST 6.5.
    void maskRestart6_5Changed();
    ///Fired when this processor changes the masking state for RST 5.5.
    void maskRestart5_5Changed();
    ///Fired when this processor changes the interrupt enable latch status as a result of executing either DI or EI 
    ///instruction.
    void interruptEnableStatusChanged();
    ///Fired when the INTA interrupt acknowledge value changes (either acknowedged or reset).
    void interruptAcknowledgeStatusChanged();
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
