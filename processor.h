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
#include <cstring>
#include <iostream>
#include "commdefs.h"
#include "opcodes.h" //Include here. This header requires typedefs defined above.

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
    std::function<void()> * const microprograms;
    ///All memory of the 8085.
    volatile data8_t * const memory;
    ///All I/O port latches for the 8085.
    volatile data8_t * const io;

    //The above 3 are kept track of separately to prevent the size of the Processor object from getting overtly large.

    ///Accumulator register
    volatile data8_t a;
    ///Register B
    volatile data8_t b;
    ///Register C
    volatile data8_t c;
    ///Register D
    volatile data8_t d;
    ///Register E
    volatile data8_t e;
    ///Register F = Flags
    volatile flags_t f;
    ///Register H
    volatile data8_t h;
    ///Register L
    volatile data8_t l;
    ///Program counter register
    volatile memaddr_t pc;
    ///Stack pointer register
    volatile memaddr_t sp;
    ///Interrupt enable flag.
    volatile unsigned ie : 1;
    ///INTR signal flag.
    volatile unsigned intr : 1;
    ///Interrupt Acknowledge latch.
    volatile unsigned inta : 1;
    ///Reset location for INTR signal (0 to 7 for RST 0 to 7).
    volatile unsigned intrVec : 3;
    ///TRAP interrupt flag.
    volatile unsigned trap : 1;
    ///Checks if TRAP last made a low-to-high transition. This is important because actual 8085 hardware only recognises TRAP
    ///when it makes that very low-to-high transition.
    volatile unsigned trap_lowToHigh : 1;
    ///RST 7.5 interrupt latch; reset on RESET_IN or by bit D4 of accumulator when SIM is executed.
    volatile unsigned rst7_5 : 1;
    ///RST 6.5 interrupt latch.
    volatile unsigned rst6_5 : 1;
    ///RST 5.5 interrupt latch.
    volatile unsigned rst5_5 : 1;
    ///RST 7_5 interrupt mask latch (0 if enabled).
    volatile unsigned m7_5 : 1;
    ///RST 6_5 interrupt mask latch (0 if enabled).
    volatile unsigned m6_5 : 1;
    ///RST 5_5 interrupt mask latch (0 if enabled).
    volatile unsigned m5_5 : 1;
    ///Serial output data latch.
    volatile unsigned sod : 1;
    ///Serial input data latch.
    volatile unsigned sid : 1;
    ///Flag which is 0 (false) when this processor is in running state. Used to implement HLT
    ///instruction.
    volatile unsigned halt : 1;
    ///Flag which gets set on unused/invalid instruction use.
    volatile unsigned unused : 1;
public:
    /// Initializes this processor. All data storage locations (memory and all registers) are set to 0.
    explicit Processor(QObject *parent = nullptr);
    ///Explicit destructor required for proper inheritance to QObject
    virtual ~Processor();
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
    data8_t getMemoryByte(memaddr_t index) const {return memory[index & 0xFFFFu];}
    ///Gets the byte stored at the I/O port latch referred to by index.
    data8_t getOutputByte(ioaddr_t index) const {return io[index & 0xFFu];}
    ///Copy the current memory contents into dest; starting from startLoc address in this processor and copying length
    ///bytes. Note that while copying, if because of length, the addresses being copied overshoot 0xFFFF, this function
    ///"wraps around" and continues copying from 0x0000. If the destination buffer is smaller than length bytes, the
    ///behaviour is undefined.
    void copyTo(data8_t *const dest, memaddr_t startLoc, memsize_t length) const;

public slots:
    ///Copy the contents of source buffer src into the memory of this processor, starting from startLoc address in this
    ///processor and overwriting length bytes. Note that while overwriting, if because of length, the addresses being
    ///overwritten overshoot 0xFFFF, this function "wraps around" and continues copying from 0x0000. If the source
    ///buffer is smaller than length bytes, the behaviour is undefined. This fires the memoryBlockUpdated() and
    ///MChanged() signals.
    void overwrite(const data8_t *const src, memaddr_t startLoc, memsize_t length);
    ///Runs the entire program stored in the memory of this processor system, starting from the instruction stored at
    ///the location pointed to by the program counter register. Many signals may be fired repeatedly (as per
    ///instructions executed). In any case, programCounterChanged() is fired at least once. Fires halted() upon successful
    ///completed execution.
    void runFull();
    ///Execute exactly 1 instruction pointed to by the current address stored in the program counter register. Multiple
    ///signals may be fired as per the instruction executed. In any case, programCounterChanged() is always fired.
    ///This function returns true if the processor is ready to execute the next instruction on its program counter. Returns
    ///false if the last instruction executed was an HLT instruction. Fires stepped() just before exit.
    bool stepNextInstruction();
    ///Stores data at the address in the 64K memory of this processor. data is ANDed with 0xFF and address ANDed with
    ///0xFFFF before any operation is performed. Fires memoryBlockUpdated() and MChanged() signals.
    void setMemoryByte(memaddr_t address, data8_t data);
    ///Sets the data of the I/O port latch at address. This is the value read by a subsequent IN <address>.
    void setInputByte(ioaddr_t address, data8_t data);
    ///Sets the value in serial input latch. This is the value read by a subsequent RIM instruction.
    void setSerialInputLatch(bool flag);
    ///Sets the request status for RST 7.5 (true if interrupt requested). Fires restart7_5RequestStatusChanged() signal.
    void setRestart7_5Request(bool flag);
    ///Sets the request status for RST 6.5 (true if interrupt requested).
    void setRestart6_5Request(bool flag);
    ///Sets the request status for RST 5.5 (true if interrupt requested).
    void setRestart5_5Request(bool flag);
    ///Sets the request status for TRAP (true if interrupt requested).
    void setTRAPRequest(bool flag);
    ///Sets the request status for INTR (true if interrupt requested).
    void setInterruptRequest(bool flag);
    ///Sets the RST instruction which is executed on INTR interrupt request. Note that only bits D5, D4, D3 are used
    ///and rest are ignored (the result is the instruction set is always RST 0 through 7).
    void setINTRVector(data8_t value);

    ///Resets entire memory to 0 (all 65,536 bytes, may take time). Fires memoryBlockUpdated() and MChanged() signals.
    void resetMemory();
    ///Resets all I/O port latches to 0. Fires ioPortsReset() signal.
    void resetIOPorts();
    ///Resets the entire processor state EXCEPT MEMORY (all registers to 0). This is equivalent to the RESET_IN signal
    ///to the 8085. Fires ALL signals EXCEPT memoryBlockUpdated(), ioPortUpdated() and ioPortsReset() signals. Note that this
    ///does NOT stop the processor if it is running (simulating instruction execution in its memory) on another thread. In that
    ///case, haltExecution() must be called and wait for the other thread to finish.
    void RESET_IN();
    ///Resets the entire object (all data to default values EXCEPT those values which are only externally controlled).
    ///Fires ALL signals defined by the Processor class.
    void resetAll();
    ///Halts execution of this processor.
    void haltExecution();
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
    void memoryBlockUpdated(memaddr_t startLoc,memsize_t blockSize);
    ///Fired when the value stored in an I/O port latch (indicated by address) is updated.
    void ioPortUpdated(ioaddr_t address);
    ///Fired when ALL I/O port latches are reset to 0.
    void ioPortsReset();
    ///Fired when processor halts its instruction execution (either externally or due to HLT).
    void halted();
    ///Fired when processor encounters an unused instruction value.
    void unusedInstruction(data8_t);
    ///Fired from stepNextInstruction() just before it returns.
    void stepped();
};

#endif // PROCESSOR_H
