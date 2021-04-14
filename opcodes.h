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
#ifndef OPCODES_H
#define OPCODES_H

#include <QMetaType>

#ifndef PROCESSOR_H
#include "processor.h"
#endif

///Description for instructions.
struct __opcode {
    ///Mnemonic (e.g "LXI")
    const char * const mnemonic;
    ///Full assembly code to specify opcode (e.g "LXI B")
    const char * const name;
    ///Opcode for this instruction
    const data8_t opcode;
    ///Bytes required for this instruction; 1,2, or 3 for valid instructions and 0 for unused
    ///ones.
    const unsigned bytesRequired : 2;
   
    ///Default constructor (DO NOT USE! required for interoperability with Qt.)
    __opcode() : mnemonic(nullptr), name(nullptr), opcode(0u), bytesRequired(0u) {}
    ///Constructor meant to be used
    __opcode(const char * const m, const char * const n, const data8_t op, const unsigned br) 
        : mnemonic(m), name(n), opcode(op & 0xFFu), bytesRequired(br & 3u) {}
    __opcode(const __opcode &op) = default;
    virtual ~__opcode() = default;
    operator data8_t() const {return opcode & 0xFFu;}
    //operator size_t() const {return opcode & 0xFFu;}
};
typedef const __opcode opcode;
Q_DECLARE_METATYPE(opcode)

///No operation (NOP); hex machine code 0x00.
opcode NOP      ("NOP",     "NOP",      0x00u,  1);
///Load register pair immediate BC (LXI B); hex machine code 0x01.
opcode LXI_B    ("LXI",     "LXI B",    0x01u,  3);
///Store accumulator indirect BC (STAX B); hex machine code 0x02.
opcode STAX_B   ("STAX",    "STAX B",   0x02u,  1);
///Increment register pair BC (INX B); hex machine code 0x03.
opcode INX_B    ("INX",     "INX B",    0x03u,  1);
///Increment register B (INR B); hex machine code 0x04.
opcode INR_B    ("INR",     "INR B",    0x04u,  1);
///Decrement register B (DCR B); hex machine code 0x05.
opcode DCR_B    ("DCR",     "DCR B",    0x05u,  1);
///Move immediate to B (MVI B); hex machine code 0x06.
opcode MVI_B    ("MVI",     "MVI B",    0x06u,  2);
///Rotate left (RLC); hex machine code 0x07.
opcode RLC      ("RLC",     "RLC",      0x07u,  1);
//Unused instruction 0x08.
///Double/direct add register pair BC to HL (DAD B); hex machine code 0x09.
opcode DAD_B    ("DAD",     "DAD B",    0x09u,  1);
///Load accumulator indirect BC (LDAX B); hex machine code 0x0A.
opcode LDAX_B   ("LDAX",    "LDAX B",   0x0Au,  1);
///Decrement register pair BC (DCX B); hex machine code 0x0B.
opcode DCX_B    ("DCX",     "DCX B",    0x0Bu,  1);
///Increment register C (INR C); hex machine code 0x0C.
opcode INR_C    ("INR",     "INR C",    0x0Cu,  1);
///Decrement register C (DCR C); hex machine code 0x0D.
opcode DCR_C    ("DCR",     "DCR C",    0x0Du,  1);
///Move immediate to C (MVI C); hex machine code 0x0E.
opcode MVI_C    ("MVI",     "MVI C",    0x0Eu,  2);
///Rotate right (RRC); hex machine code 0x0F.
opcode RRC      ("RRC",     "RRC",      0x0Fu,  1);
//Unused instruction 0x10.
///Load register pair immediate DE (LXI D); hex machine code 0x11.
opcode LXI_D    ("LXI",     "LXI D",    0x11u,  3);
///Store accumulator indirect DE (STAX D); hex machine code 0x12.
opcode STAX_D   ("STAX",    "STAX D",   0x12u,  1);
///Increment register pair DE (INX D); hex machine code 0x13.
opcode INX_D    ("INX",     "INX D",    0x13u,  1);
///Increment register D (INR D); hex machine code 0x14.
opcode INR_D    ("INR",     "INR D",    0x14u,  1);
///Decrement register D (DCR D); hex machine code 0x15.
opcode DCR_D    ("DCR",     "DCR D",    0x15u,  1);
///Move immediate to D (MVI D); hex machine code 0x16.
opcode MVI_D    ("MVI",     "MVI D",    0x16u,  2);
///Rotate left through carry (RAL); hex machine code 0x17.
opcode RAL      ("RAL",     "RAL",      0x17u,  1);
//Unused instruction 0x18.
///Double/direct add register pair DE to HL (DAD D); hex machine code 0x19.
opcode DAD_D    ("DAD",     "DAD D",    0x19u,  1);
///Load accumulator indirect DE (LDAX D); hex machine code 0x1A.
opcode LDAX_D   ("LDAX",    "LDAX D",   0x1Au,  1);
///Decrement register pair DE (DCX D); hex machine code 0x1B.
opcode DCX_D    ("DCX",     "DCX D",    0x1Bu,  1);
///Increment register E (INR E); hex machine code 0x1C.
opcode INR_E    ("INR",     "INR E",    0x1Cu,  1);
///Decrement register E (DCR E); hex machine code 0x1D.
opcode DCR_E    ("DCR",     "DCR E",    0x1Du,  1);
///Move immediate to E (MVI E); hex machine code 0x1E.
opcode MVI_E    ("MVI",     "MVI E",    0x1Eu,  2);
///Rotate right through carry (RAR); hex machine code 0x1F.
opcode RAR      ("RAR",     "RAR",      0x1Fu,  1);
///Read interrupt masks (RIM); hex machine code 0x20.
opcode RIM      ("RIM",     "RIM",      0x20u,  1);
///Load register pair immediate HL (LXI H); hex machine code 0x21.
opcode LXI_H    ("LXI",     "LXI H",    0x21u,  3);
///Store HL direct (SHLD), hex machine code 0x22.
opcode SHLD     ("SHLD",    "SHLD",     0x22u,  3);
///Increment register pair HL (INX H); hex machine code 0x23.
opcode INX_H    ("INX",     "INX H",    0x23u,  1);
///Increment register H (INR H); hex machine code 0x24.
opcode INR_H    ("INR",     "INR H",    0x24u,  1);
///Decrement register H (DCR H); hex machine code 0x25.
opcode DCR_H    ("DCR",     "DCR H",    0x25u,  1);
///Move immediate to H (MVI H); hex machine code 0x26.
opcode MVI_H    ("MVI",     "MVI H",    0x26u,  2);
///Decimal adjust accumulator (DAA); hex machine code 0x27.
opcode DAA      ("DAA",     "DAA",      0x27u,  1);
//Unised instruction 0x28.
///Double/direct add register pair DE to HL (DAD D); hex machine code 0x29.
opcode DAD_H    ("DAD",     "DAD H",    0x29u,  1);
///Load HL direct (LHLD), hex machine code 0x2A.
opcode LHLD     ("LHLD",    "LHLD",     0x2Au,  3);
///Decrement register pair HL (DCX H); hex machine code 0x2B.
opcode DCX_H    ("DCX",     "DCX H",    0x2Bu,  1);
///Increment register L (INR L); hex machine code 0x2C.
opcode INR_L    ("INR",     "INR L",    0x2Cu,  1);
///Decrement register L (DCR L); hex machine code 0x2D.
opcode DCR_L    ("DCR",     "DCR L",    0x2Du,  1);
///Move immediate to L (MVI L); hex machine code 0x2E.
opcode MVI_L    ("MVI",     "MVI L",    0x2Eu,  2);
///Complement accumulator (CMA); hex machine code 0x2F.
opcode CMA      ("CMA",     "CMA",      0x2Fu,  1);
///Set interrupt masks (SIM); hex machine code 0x30.
opcode SIM      ("SIM",     "SIM",      0x30u,  1);


namespace opcodes_h {
inline void registerHeaderMetaTypes() {
    qRegisterMetaType<opcode>("opcode");
}
}

#endif // OPCODES_H
    
