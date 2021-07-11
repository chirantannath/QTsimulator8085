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
#include "commdefs.h"

///Description for instructions.
struct __opcode {
    ///Mnemonic (e.g "LXI")
    const char * const mnemonic;
    ///Full assembly code to specify opcode (e.g "LXI B")
    const char * const name;
    ///Opcode for this instruction
    const data8_t code;
    ///Bytes required for this instruction; 1,2, or 3 for valid instructions and 0 for unused
    ///ones.
    const unsigned bytesRequired : 2;
    ///True if this opcode slot describes a pseudocode.
    const unsigned isPseudocode : 1;

    ///Default constructor (DO NOT USE! required for interoperability with Qt.)
    __opcode() : mnemonic(nullptr), name(nullptr), code(0u), bytesRequired(0u), isPseudocode(0) {};
    ///Constructor meant to be used
    __opcode(const char * const mnemonic, const char * const fullname,
             const data8_t code, const unsigned bytesRequired) : mnemonic(mnemonic), name(fullname),
        code(code), bytesRequired(bytesRequired), isPseudocode(0) {}
    ///Constructor for pseudocode
    __opcode(const char * const name) : mnemonic(name), name(name), code(0u), bytesRequired(0u), isPseudocode(1){}
    ///Copy constructor
    __opcode(const __opcode &op) = default;
    ///Destructor (virtual REQUIRED for interop with Qt)
    virtual ~__opcode() = default;
    ///Automatic conversion to data8_t type
    operator data8_t() const {return code & 0xFFu;}
};
Q_DECLARE_METATYPE(__opcode);
typedef __opcode opcode;

///No operation (NOP); hex machine code 0x00.
extern const opcode NOP;
///Load register pair immediate BC (LXI B); hex machine code 0x01.
extern const opcode LXI_B;
///Store accumulator indirect BC (STAX B); hex machine code 0x02.
extern const opcode STAX_B;
///Increment register pair BC (INX B); hex machine code 0x03.
extern const opcode INX_B;
///Increment register B (INR B); hex machine code 0x04.
extern const opcode INR_B;
///Decrement register B (DCR B); hex machine code 0x05.
extern const opcode DCR_B;
///Move immediate to B (MVI B); hex machine code 0x06.
extern const opcode MVI_B;
///Rotate left (RLC); hex machine code 0x07.
extern const opcode RLC;
//Unused instruction 0x08.
///Double/direct add register pair BC to HL (DAD B); hex machine code 0x09.
extern const opcode DAD_B;
///Load accumulator indirect BC (LDAX B); hex machine code 0x0A.
extern const opcode LDAX_B;
///Decrement register pair BC (DCX B); hex machine code 0x0B.
extern const opcode DCX_B;
///Increment register C (INR C); hex machine code 0x0C.
extern const opcode INR_C;
///Decrement register C (DCR C); hex machine code 0x0D.
extern const opcode DCR_C;
///Move immediate to C (MVI C); hex machine code 0x0E.
extern const opcode MVI_C;
///Rotate right (RRC); hex machine code 0x0F.
extern const opcode RRC;
//Unused instruction 0x10.
///Load register pair immediate DE (LXI D); hex machine code 0x11.
extern const opcode LXI_D;
///Store accumulator indirect DE (STAX D); hex machine code 0x12.
extern const opcode STAX_D;
///Increment register pair DE (INX D); hex machine code 0x13.
extern const opcode INX_D;
///Increment register D (INR D); hex machine code 0x14.
extern const opcode INR_D;
///Decrement register D (DCR D); hex machine code 0x15.
extern const opcode DCR_D;
///Move immediate to D (MVI D); hex machine code 0x16.
extern const opcode MVI_D;
///Rotate left through carry (RAL); hex machine code 0x17.
extern const opcode RAL;
//Unused instruction 0x18.
///Double/direct add register pair DE to HL (DAD D); hex machine code 0x19.
extern const opcode DAD_D;
///Load accumulator indirect DE (LDAX D); hex machine code 0x1A.
extern const opcode LDAX_D;
///Decrement register pair DE (DCX D); hex machine code 0x1B.
extern const opcode DCX_D;
///Increment register E (INR E); hex machine code 0x1C.
extern const opcode INR_E;
///Decrement register E (DCR E); hex machine code 0x1D.
extern const opcode DCR_E;
///Move immediate to E (MVI E); hex machine code 0x1E.
extern const opcode MVI_E;
///Rotate right through carry (RAR); hex machine code 0x1F.
extern const opcode RAR;
///Read interrupt masks (RIM); hex machine code 0x20.
extern const opcode RIM;
///Load register pair immediate HL (LXI H); hex machine code 0x21.
extern const opcode LXI_H;
///Store HL direct (SHLD), hex machine code 0x22.
extern const opcode SHLD;
///Increment register pair HL (INX H); hex machine code 0x23.
extern const opcode INX_H;
///Increment register H (INR H); hex machine code 0x24.
extern const opcode INR_H;
///Decrement register H (DCR H); hex machine code 0x25.
extern const opcode DCR_H;
///Move immediate to H (MVI H); hex machine code 0x26.
extern const opcode MVI_H;
///Decimal adjust accumulator (DAA); hex machine code 0x27.
extern const opcode DAA;
//Unised instruction 0x28.
///Double/direct add register pair DE to HL (DAD D); hex machine code 0x29.
extern const opcode DAD_H;
///Load HL direct (LHLD), hex machine code 0x2A.
extern const opcode LHLD;
///Decrement register pair HL (DCX H); hex machine code 0x2B.
extern const opcode DCX_H;
///Increment register L (INR L); hex machine code 0x2C.
extern const opcode INR_L;
///Decrement register L (DCR L); hex machine code 0x2D.
extern const opcode DCR_L;
///Move immediate to L (MVI L); hex machine code 0x2E.
extern const opcode MVI_L;
///Complement accumulator (CMA); hex machine code 0x2F.
extern const opcode CMA;
///Set interrupt masks (SIM); hex machine code 0x30.
extern const opcode SIM;
///Load register pair immediate SP (LXI SP); hex machine code 0x31.
extern const opcode LXI_SP;
///Store accumulator direct (STA); hex machine code 0x32.
extern const opcode STA;
///Increment register pair SP (INX SP); hex machine code 0x33.
extern const opcode INX_SP;
///Increment memory (INR M); hex machine code 0x34.
extern const opcode INR_M;
///Decrement memory (DCR M); hex machine code 0x35.
extern const opcode DCR_M;
///Move immediate to memory (MVI M); hex machine code 0x36.
extern const opcode MVI_M;
///Set carry (STC); hex machine code 0x37.
extern const opcode STC;
//Unused instruction 0x38.
///Double/direct add register pair SP to HL (DAD SP); hex machine code 0x39.
extern const opcode DAD_SP;
///Load accumulator direct (LDA); hex machine code 0x3A.
extern const opcode LDA;
///Decrement register pair SP (DCX SP); hex machine code 0x3B.
extern const opcode DCX_SP;
///Increment accumulator (INR A); hex machine code 0x3C.
extern const opcode INR_A;
///Decrement accumulator (DCR A); hex machine code 0x3D.
extern const opcode DCR_A;
///Move immediate to accumulator (MVI A); hex machine code 0x3E.
extern const opcode MVI_A;
///Complement carry (CMC); hex machine code 0x3F.
extern const opcode CMC;
///Move register B to B (MOV B, B); hex machine code 0x40.
extern const opcode MOV_B_B;
///Move register C to B (MOV B, C); hex machine code 0x41.
extern const opcode MOV_B_C;
///Move register D to B (MOV B, D); hex machine code 0x42.
extern const opcode MOV_B_D;
///Move register E to B (MOV B, E); hex machine code 0x43.
extern const opcode MOV_B_E;
///Move register H to B (MOV B, H); hex machine code 0x44.
extern const opcode MOV_B_H;
///Move register L to B (MOV B, L); hex machine code 0x45.
extern const opcode MOV_B_L;
///Move memory to B (MOV B, M); hex machine code 0x46.
extern const opcode MOV_B_M;
///Move accumulator to B (MOV B, A); hex machine code 0x47.
extern const opcode MOV_B_A;
///Move register B to C (MOV C, B); hex machine code 0x48.
extern const opcode MOV_C_B;
///Move register C to C (MOV C, C); hex machine code 0x49.
extern const opcode MOV_C_C;
///Move register D to C (MOV C, D); hex machine code 0x4A.
extern const opcode MOV_C_D;
///Move register E to C (MOV C, E); hex machine code 0x4B.
extern const opcode MOV_C_E;
///Move register H to C (MOV C, H); hex machine code 0x4C.
extern const opcode MOV_C_H;
///Move register L to C (MOV C, L); hex machine code 0x4D.
extern const opcode MOV_C_L;
///Move memory to C (MOV C, M); hex machine code 0x4E.
extern const opcode MOV_C_M;
///Move accumulator to C (MOV C, A); hex machine code 0x4F.
extern const opcode MOV_C_A;
///Move register B to D (MOV D, B); hex machine code 0x50.
extern const opcode MOV_D_B;
///Move register C to D (MOV D, C); hex machine code 0x51.
extern const opcode MOV_D_C;
///Move register D to D (MOV D, D); hex machine code 0x52.
extern const opcode MOV_D_D;
///Move register E to D (MOV D, E); hex machine code 0x53.
extern const opcode MOV_D_E;
///Move register H to D (MOV D, H); hex machine code 0x54.
extern const opcode MOV_D_H;
///Move register L to D (MOV D, L); hex machine code 0x55.
extern const opcode MOV_D_L;
///Move memory to D (MOV D, M); hex machine code 0x56.
extern const opcode MOV_D_M;
///Move accumulator to D (MOV D, A); hex machine code 0x57.
extern const opcode MOV_D_A;
///Move register B to E (MOV E, B); hex machine code 0x58.
extern const opcode MOV_E_B;
///Move register C to E (MOV E, C); hex machine code 0x59.
extern const opcode MOV_E_C;
///Move register D to E (MOV E, D); hex machine code 0x5A.
extern const opcode MOV_E_D;
///Move register E to E (MOV E, E); hex machine code 0x5B.
extern const opcode MOV_E_E;
///Move register H to E (MOV E, H); hex machine code 0x5C.
extern const opcode MOV_E_H;
///Move register L to E (MOV E, L); hex machine code 0x5D.
extern const opcode MOV_E_L;
///Move memory to E (MOV E, M); hex machine code 0x5E.
extern const opcode MOV_E_M;
///Move accumulator to E (MOV E, A); hex machine code 0x5F.
extern const opcode MOV_E_A;
///Move register B to H (MOV H, B); hex machine code 0x60.
extern const opcode MOV_H_B;
///Move register C to H (MOV H, C); hex machine code 0x61.
extern const opcode MOV_H_C;
///Move register D to H (MOV H, D); hex machine code 0x62.
extern const opcode MOV_H_D;
///Move register E to H (MOV H, E); hex machine code 0x63.
extern const opcode MOV_H_E;
///Move register H to H (MOV H, H); hex machine code 0x64.
extern const opcode MOV_H_H;
///Move register L to H (MOV H, L); hex machine code 0x65.
extern const opcode MOV_H_L;
///Move memory to H (MOV H, M); hex machine code 0x66.
extern const opcode MOV_H_M;
///Move accumulator to H (MOV H, A); hex machine code 0x67.
extern const opcode MOV_H_A;
///Move register B to L (MOV L, B); hex machine code 0x68.
extern const opcode MOV_L_B;
///Move register C to L (MOV L, C); hex machine code 0x69.
extern const opcode MOV_L_C;
///Move register D to L (MOV L, D); hex machine code 0x6A.
extern const opcode MOV_L_D;
///Move register E to L (MOV L, E); hex machine code 0x6B.
extern const opcode MOV_L_E;
///Move register H to L (MOV L, H); hex machine code 0x6C.
extern const opcode MOV_L_H;
///Move register L to L (MOV L, L); hex machine code 0x6D.
extern const opcode MOV_L_L;
///Move memory to L (MOV L, M); hex machine code 0x6E.
extern const opcode MOV_L_M;
///Move accumulator to L (MOV L, A); hex machine code 0x6F.
extern const opcode MOV_L_A;
///Move register B to memory (MOV M, B); hex machine code 0x70.
extern const opcode MOV_M_B;
///Move register C to memory (MOV M, B); hex machine code 0x71.
extern const opcode MOV_M_C;
///Move register D to memory (MOV M, B); hex machine code 0x72.
extern const opcode MOV_M_D;
///Move register E to memory (MOV M, B); hex machine code 0x73.
extern const opcode MOV_M_E;
///Move register H to memory (MOV M, B); hex machine code 0x74.
extern const opcode MOV_M_H;
///Move register L to memory (MOV M, B); hex machine code 0x75.
extern const opcode MOV_M_L;
///Halt (HLT); hex machine code 0x76.
extern const opcode HLT;
///Move accumulator to memory (MOV M, A); hex machine code 0x77.
extern const opcode MOV_M_A;
///Move register B to accumulator (MOV A, B); hex machine code 0x78.
extern const opcode MOV_A_B;
///Move register C to accumulator (MOV A, C); hex machine code 0x79.
extern const opcode MOV_A_C;
///Move register D to accumulator (MOV A, D); hex machine code 0x7A.
extern const opcode MOV_A_D;
///Move register E to accumulator (MOV A, E); hex machine code 0x7B.
extern const opcode MOV_A_E;
///Move register H to accumulator (MOV A, H); hex machine code 0x7C.
extern const opcode MOV_A_H;
///Move register L to accumulator (MOV A, L); hex machine code 0x7D.
extern const opcode MOV_A_L;
///Move memory to accumulator (MOV A, M); hex machine code 0x7E.
extern const opcode MOV_A_M;
///Move accumulator to accumulator (MOV A, A); hex machine code 0x7F.
extern const opcode MOV_A_A;
///Add register B to accumulator (ADD B); hex machine code 0x80.
extern const opcode ADD_B;
///Add register C to accumulator (ADD C); hex machine code 0x81.
extern const opcode ADD_C;
///Add register D to accumulator (ADD D); hex machine code 0x82.
extern const opcode ADD_D;
///Add register E to accumulator (ADD E); hex machine code 0x83.
extern const opcode ADD_E;
///Add register H to accumulator (ADD H); hex machine code 0x84.
extern const opcode ADD_H;
///Add register L to accumulator (ADD L); hex machine code 0x85.
extern const opcode ADD_L;
///Add memory to accumulator (ADD M); hex machine code 0x86.
extern const opcode ADD_M;
///Add accumulator to accumulator (ADD A); hex machine code 0x87.
extern const opcode ADD_A;
///Add register B to accumulator with carry (ADC B); hex machine code 0x88.
extern const opcode ADC_B;
///Add register C to accumulator with carry (ADC C); hex machine code 0x89.
extern const opcode ADC_C;
///Add register D to accumulator with carry (ADC D); hex machine code 0x8A.
extern const opcode ADC_D;
///Add register E to accumulator with carry (ADC E); hex machine code 0x8B.
extern const opcode ADC_E;
///Add register H to accumulator with carry (ADC H); hex machine code 0x8C.
extern const opcode ADC_H;
///Add register L to accumulator with carry (ADC L); hex machine code 0x8D.
extern const opcode ADC_L;
///Add memory to accumulator with carry (ADC M); hex machine code 0x8E.
extern const opcode ADC_M;
///Add accumulator to accumulator with carry (ADC A); hex machine code 0x8F.
extern const opcode ADC_A;
///Subtract register B from accumulator (SUB B); hex machine code 0x90.
extern const opcode SUB_B;
///Subtract register C from accumulator (SUB C); hex machine code 0x91.
extern const opcode SUB_C;
///Subtract register D from accumulator (SUB D); hex machine code 0x92.
extern const opcode SUB_D;
///Subtract register E from accumulator (SUB E); hex machine code 0x93.
extern const opcode SUB_E;
///Subtract register H from accumulator (SUB H); hex machine code 0x94.
extern const opcode SUB_H;
///Subtract register L from accumulator (SUB L); hex machine code 0x95.
extern const opcode SUB_L;
///Subtract memory from accumulator (SUB M); hex machine code 0x96.
extern const opcode SUB_M;
///Subtract accumulator from accumulator (SUB A); hex machine code 0x97.
extern const opcode SUB_A;
///Subtract register B from accumulator with borrow (SBB B); hex machine code 0x98.
extern const opcode SBB_B;
///Subtract register C from accumulator with borrow (SBB C); hex machine code 0x99.
extern const opcode SBB_C;
///Subtract register D from accumulator with borrow (SBB D); hex machine code 0x9A.
extern const opcode SBB_D;
///Subtract register E from accumulator with borrow (SBB E); hex machine code 0x9B.
extern const opcode SBB_E;
///Subtract register H from accumulator with borrow (SBB H); hex machine code 0x9C.
extern const opcode SBB_H;
///Subtract register L from accumulator with borrow (SBB L); hex machine code 0x9D.
extern const opcode SBB_L;
///Subtract memory from accumulator with borrow (SBB M); hex machine code 0x9E.
extern const opcode SBB_M;
///Subtract accumulator from accumulator with borrow (SBB A); hex machine code 0x9F.
extern const opcode SBB_A;
///AND register B with accumulator (ANA B); hex machine code 0xA0.
extern const opcode ANA_B;
///AND register C with accumulator (ANA C); hex machine code 0xA1.
extern const opcode ANA_C;
///AND register D with accumulator (ANA D); hex machine code 0xA2.
extern const opcode ANA_D;
///AND register E with accumulator (ANA E); hex machine code 0xA3.
extern const opcode ANA_E;
///AND register H with accumulator (ANA H); hex machine code 0xA4.
extern const opcode ANA_H;
///AND register L with accumulator (ANA L); hex machine code 0xA5.
extern const opcode ANA_L;
///AND memory with accumulator (ANA M); hex machine code 0xA6.
extern const opcode ANA_M;
///AND accumulator with accumulator (ANA A); hex machine code 0xA7.
extern const opcode ANA_A;
///XOR register B with accumulator(XRA B); hex machine code 0xA8.
extern const opcode XRA_B;
///XOR register C with accumulator(XRA C); hex machine code 0xA9.
extern const opcode XRA_C;
///XOR register D with accumulator(XRA D); hex machine code 0xAA.
extern const opcode XRA_D;
///XOR register E with accumulator(XRA E); hex machine code 0xAB.
extern const opcode XRA_E;
///XOR register H with accumulator(XRA H); hex machine code 0xAC.
extern const opcode XRA_H;
///XOR register L with accumulator(XRA L); hex machine code 0xAD.
extern const opcode XRA_L;
///XOR memory with accumulator(XRA M); hex machine code 0xAE.
extern const opcode XRA_M;
///XOR accumulator with accumulator(XRA A); hex machine code 0xAF.
extern const opcode XRA_A;
///OR register B with accumulator(ORA B); hex machine code 0xB0.
extern const opcode ORA_B;
///OR register C with accumulator(ORA C); hex machine code 0xB1.
extern const opcode ORA_C;
///OR register D with accumulator(ORA D); hex machine code 0xB2.
extern const opcode ORA_D;
///OR register E with accumulator(ORA E); hex machine code 0xB3.
extern const opcode ORA_E;
///OR register H with accumulator(ORA H); hex machine code 0xB4.
extern const opcode ORA_H;
///OR register L with accumulator(ORA L); hex machine code 0xB5.
extern const opcode ORA_L;
///OR memory with accumulator(ORA M); hex machine code 0xB6.
extern const opcode ORA_M;
///OR accumulator with accumulator(ORA A); hex machine code 0xB7.
extern const opcode ORA_A;
///Compare register B against accumulator (CMP B); hex machine code 0xB8.
extern const opcode CMP_B;
///Compare register C against accumulator (CMP C); hex machine code 0xB9.
extern const opcode CMP_C;
///Compare register D against accumulator (CMP D); hex machine code 0xBA.
extern const opcode CMP_D;
///Compare register E against accumulator (CMP E); hex machine code 0xBB.
extern const opcode CMP_E;
///Compare register H against accumulator (CMP H); hex machine code 0xBC.
extern const opcode CMP_H;
///Compare register L against accumulator (CMP L); hex machine code 0xBD.
extern const opcode CMP_L;
///Compare memory against accumulator (CMP M); hex machine code 0xBE.
extern const opcode CMP_M;
///Compare accumulator against accumulator (CMP A); hex machine code 0xBF.
extern const opcode CMP_A;
///Return on nonzero (RNZ); hex machine code 0xC0.
extern const opcode RNZ;
///Pop BC from stack (POP B); hex machine code 0xC1.
extern const opcode POP_B;
///Jump on nonzero (JNZ); hex machine code 0xC2.
extern const opcode JNZ;
///Jump (JMP); hex machine code 0xC3.
extern const opcode JMP;
///Call on nonzero (CNZ); hex machine code 0xC4.
extern const opcode CNZ;
///Push BC on stack (PUSH B); hex machine code 0xC5.
extern const opcode PUSH_B;
///Add immediate to accumulator (ADI); hex machine code 0xC6.
extern const opcode ADI;
///Restart 0 (RST 0); hex machine code 0xC7.
extern const opcode RST_0;
///Return on zero (RZ); hex machine code 0xC8.
extern const opcode RZ;
///Return (RET); hex machine code 0xC9.
extern const opcode RET;
///Jump on zero (JZ); hex machine code 0xCA.
extern const opcode JZ;
//Unused instruction 0xCB.
///Call on zero (CZ); hex machine code 0xCC.
extern const opcode CZ;
///Call (CALL); hex machine code 0xCD.
extern const opcode CALL;
///Add immediate to accumulator with carry (ACI); hex machine code 0xCE.
extern const opcode ACI;
///Restart 1 (RST 1); hex machine code 0xCF.
extern const opcode RST_1;
///Return on NOT carry (RNC); hex machine code 0xD0.
extern const opcode RNC;
///Pop DE from stack (POP D); hex machine code 0xD1.
extern const opcode POP_D;
///Jump on NOT carry (JNC); hex machine code 0xD2.
extern const opcode JNC;
///Output (OUT); hex machine code 0xD3.
extern const opcode OUT;
///Call on NOT carry (CNC); hex machine code 0xD4.
extern const opcode CNC;
///Push DE on stack (PUSH D); hex machine code 0xD5.
extern const opcode PUSH_D;
///Subtract immediate from accumulator (SUI); hex machine code 0xD6.
extern const opcode SUI;
///Restart 2 (RST 2); hex machine code 0xD7.
extern const opcode RST_2;
///Return on carry (RC); hex machine code 0xD8.
extern const opcode RC;
//Unused instruction 0xD9.
///Jump on carry (JC); hex machine code 0xDA.
extern const opcode JC;
///Input (IN); hex machine code 0xDB.
extern const opcode IN;
///Call on carry (CC); hex machine code 0xDC.
extern const opcode CC;
//Unused instruction 0xDD.
///Subtract immediate from accumulator with borrow (SBI); hex machine code 0xDE.
extern const opcode SBI;
///Restart 3 (RST 3); hex machine code 0xDF.
extern const opcode RST_3;
///Return on parity odd (RPO); hex machine code 0xE0.
extern const opcode RPO;
///Pop HL from stack (POP H); hex machine code 0xE1.
extern const opcode POP_H;
///Jump on parity odd (JPO); hex machine code 0xE2.
extern const opcode JPO;
///Exchange stack top with HL (XTHL); hex machine code 0xE3.
extern const opcode XTHL;
///Call on parity odd (CPO); hex machine code 0xE4.
extern const opcode CPO;
///Push HL on stack (PUSH H); hex machine code 0xE5.
extern const opcode PUSH_H;
///AND immediate with accumulator (ANI); hex machine code 0xE6.
extern const opcode ANI;
///Restart 4 (RST 4); hex machine code 0xE7.
extern const opcode RST_4;
///Return on parity even (RPE); hex machine code 0xE8.
extern const opcode RPE;
///Jump HL indirect; or move HL to PC (PCHL); hex machine code 0xE9.
extern const opcode PCHL;
///Jump on parity even (JPE); hex machine code 0xEA.
extern const opcode JPE;
///Exchange HL with DE (XCHG); hex machine code 0xEB.
extern const opcode XCHG;
///Call on parity even (CPE); hex machine code 0xEC.
extern const opcode CPE;
//Unused instruction 0xED.
///XOR immediate with accumulator (XRI); hex machine code 0xEE.
extern const opcode XRI;
///Restart 5 (RST 5); hex machine code 0xEF.
extern const opcode RST_5;
///Return on Plus (RP); hex machine code 0xF0;
extern const opcode RP;
///Pop processor status word (POP PSW); hex machine code 0xF1.
extern const opcode POP_PSW;
///Jump on Plus (JP); hex machine code 0xF2.
extern const opcode JP;
///Disable interrupts (DI); hex machine code 0xF3.
extern const opcode DI;
///Call on Plus (CP); hex machine code 0xF4.
extern const opcode CP;
///Push processor status word (PUSH PSW); hex machine code 0xF5.
extern const opcode PUSH_PSW;
///OR immediate with accumulator (ORI); hex machine code 0xF6.
extern const opcode ORI;
///Restart 6 (RST 6); hex machine code 0xF7.
extern const opcode RST_6;
///Return on Minus (RM); hex machine code 0xF8.
extern const opcode RM;
///Move HL to SP (SPHL); hex machine code 0xF9.
extern const opcode SPHL;
///Jump on Minus (JM); hex machine code 0xFA.
extern const opcode JM;
///Enable interrupts (EI); hex machine code 0xFB.
extern const opcode EI;
///Call on Minus (CM); hex machine code 0xFC.
extern const opcode CM;
//Unused instruction 0xFD.
///Compare immediate against accumulator (CPI); hex machine code 0xFE.
extern const opcode CPI;
///Restart 7 (RST 7); hex machine code 0xFF.
extern const opcode RST_7;

//As of right now; two opcodes are supported.
///Indicates the address offset from which to put instructions
extern const opcode ORG;
///Lists data bytes to be put without translation into the processor memory.
extern const opcode DATA;

///Checks if the string refers to a valid pseudocode name.
extern bool isPseudocode(const char * const name);
///Gets the corresponding opcode identifier object corresponding to the pseudocode name, or nullptr if not found.
extern const opcode *getPseudocode(const char * const name);

///Checks if the string refers to a valid opcode (example "LXI" or "MOV" or such). Here name is the mnemonic required.
extern bool isOpcode(const char * const name);
///Get an opcode corresponding to it's name if possible. Returns nullptr if such is not possible.
extern const opcode *getOpcode(const char * const name);
///Map opcodes by bytecode.
extern const opcode *opcodesByCode[256];

#define OPCODES_H_registerHeaderMetaTypes() {\
    qRegisterMetaType<__opcode>("opcode"); \
    qRegisterMetaType<opcode>("opcode"); \
}

#endif // OPCODES_H

