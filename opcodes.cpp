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

/*Opcode definitions need to be defined separately here to avoid redefinitions in object code.*/

#include "opcodes.h"

const opcode NOP        ("NOP",     "NOP",      0x00u,  1);
const opcode LXI_B      ("LXI",     "LXI B",    0x01u,  3);
const opcode STAX_B     ("STAX",    "STAX B",   0x02u,  1);
const opcode INX_B      ("INX",     "INX B",    0x03u,  1);
const opcode INR_B      ("INR",     "INR B",    0x04u,  1);
const opcode DCR_B      ("DCR",     "DCR B",    0x05u,  1);
const opcode MVI_B      ("MVI",     "MVI B",    0x06u,  2);
const opcode RLC        ("RLC",     "RLC",      0x07u,  1);
//Unused instruction                            0x08.
const opcode DAD_B      ("DAD",     "DAD B",    0x09u,  1);
const opcode LDAX_B     ("LDAX",    "LDAX B",   0x0Au,  1);
const opcode DCX_B      ("DCX",     "DCX B",    0x0Bu,  1);
const opcode INR_C      ("INR",     "INR C",    0x0Cu,  1);
const opcode DCR_C      ("DCR",     "DCR C",    0x0Du,  1);
const opcode MVI_C      ("MVI",     "MVI C",    0x0Eu,  2);
const opcode RRC        ("RRC",     "RRC",      0x0Fu,  1);
//Unused instruction                            0x10.
const opcode LXI_D      ("LXI",     "LXI D",    0x11u,  3);
const opcode STAX_D     ("STAX",    "STAX D",   0x12u,  1);
const opcode INX_D      ("INX",     "INX D",    0x13u,  1);
const opcode INR_D      ("INR",     "INR D",    0x14u,  1);
const opcode DCR_D      ("DCR",     "DCR D",    0x15u,  1);
const opcode MVI_D      ("MVI",     "MVI D",    0x16u,  2);
const opcode RAL        ("RAL",     "RAL",      0x17u,  1);
//Unused instruction                            0x18.
const opcode DAD_D      ("DAD",     "DAD D",    0x19u,  1);
const opcode LDAX_D     ("LDAX",    "LDAX D",   0x1Au,  1);
const opcode DCX_D      ("DCX",     "DCX D",    0x1Bu,  1);
const opcode INR_E      ("INR",     "INR E",    0x1Cu,  1);
const opcode DCR_E      ("DCR",     "DCR E",    0x1Du,  1);
const opcode MVI_E      ("MVI",     "MVI E",    0x1Eu,  2);
const opcode RAR        ("RAR",     "RAR",      0x1Fu,  1);
const opcode RIM        ("RIM",     "RIM",      0x20u,  1);
const opcode LXI_H      ("LXI",     "LXI H",    0x21u,  3);
const opcode SHLD       ("SHLD",    "SHLD",     0x22u,  3);
const opcode INX_H      ("INX",     "INX H",    0x23u,  1);
const opcode INR_H      ("INR",     "INR H",    0x24u,  1);
const opcode DCR_H      ("DCR",     "DCR H",    0x25u,  1);
const opcode MVI_H      ("MVI",     "MVI H",    0x26u,  2);
const opcode DAA        ("DAA",     "DAA",      0x27u,  1);
const opcode DAD_H      ("DAD",     "DAD H",    0x29u,  1);
const opcode LHLD       ("LHLD",    "LHLD",     0x2Au,  3);
const opcode DCX_H      ("DCX",     "DCX H",    0x2Bu,  1);
const opcode INR_L      ("INR",     "INR L",    0x2Cu,  1);
const opcode DCR_L      ("DCR",     "DCR L",    0x2Du,  1);
const opcode MVI_L      ("MVI",     "MVI L",    0x2Eu,  2);
const opcode CMA        ("CMA",     "CMA",      0x2Fu,  1);
const opcode SIM        ("SIM",     "SIM",      0x30u,  1);
const opcode LXI_SP     ("LXI",     "LXI SP",   0x31u,  3);
const opcode STA        ("STA",     "STA",      0x32u,  3);
const opcode INX_SP     ("INX",     "INX SP",   0x33u,  1);
const opcode INR_M      ("INR",     "INR <",    0x34u,  1);
const opcode DCR_M      ("DCR",     "DCR H",    0x35u,  1);
const opcode MVI_M      ("MVI",     "MVI H",    0x36u,  2);
const opcode STC        ("STC",     "STC",      0x37u,  1);
//Unused instruction                            0x38.
const opcode DAD_SP     ("DAD",     "DAD SP",   0x39u,  1);
const opcode LDA        ("LDA",     "LDA",      0x3Au,  3);
const opcode DCX_SP     ("DCX",     "DCX SP",   0x3Bu,  1);
const opcode INR_A      ("INR",     "INR A",    0x3Cu,  1);
const opcode DCR_A      ("DCR",     "DCR A",    0x3Du,  1);
const opcode MVI_A      ("MVI",     "MVI A",    0x3Eu,  2);
const opcode CMC        ("CMC",     "CMC",      0x3Fu,  1);
const opcode MOV_B_B    ("MOV",     "MOV B, B", 0x40u,  1);
const opcode MOV_B_C    ("MOV",     "MOV B, C", 0x41u,  1);
const opcode MOV_B_D    ("MOV",     "MOV B, D", 0x42u,  1);
const opcode MOV_B_E    ("MOV",     "MOV B, E", 0x43u,  1);
const opcode MOV_B_H    ("MOV",     "MOV B, H", 0x44u,  1);
const opcode MOV_B_L    ("MOV",     "MOV B, L", 0x45u,  1);
const opcode MOV_B_M    ("MOV",     "MOV B, M", 0x46u,  1);
const opcode MOV_B_A    ("MOV",     "MOV B, A", 0x47u,  1);
const opcode MOV_C_B    ("MOV",     "MOV C, B", 0x48u,  1);
const opcode MOV_C_C    ("MOV",     "MOV C, C", 0x49u,  1);
const opcode MOV_C_D    ("MOV",     "MOV C, D", 0x4Au,  1);
const opcode MOV_C_E    ("MOV",     "MOV C, E", 0x4Bu,  1);
const opcode MOV_C_H    ("MOV",     "MOV C, H", 0x4Cu,  1);
const opcode MOV_C_L    ("MOV",     "MOV C, L", 0x4Du,  1);
const opcode MOV_C_M    ("MOV",     "MOV C, M", 0x4Eu,  1);
const opcode MOV_C_A    ("MOV",     "MOV C, A", 0x4Fu,  1);
const opcode MOV_D_B    ("MOV",     "MOV B, B", 0x50u,  1);
const opcode MOV_D_C    ("MOV",     "MOV B, C", 0x51u,  1);
const opcode MOV_D_D    ("MOV",     "MOV B, D", 0x52u,  1);
const opcode MOV_D_E    ("MOV",     "MOV B, E", 0x53u,  1);
const opcode MOV_D_H    ("MOV",     "MOV B, H", 0x54u,  1);
const opcode MOV_D_L    ("MOV",     "MOV B, L", 0x55u,  1);
const opcode MOV_D_M    ("MOV",     "MOV B, M", 0x56u,  1);
const opcode MOV_D_A    ("MOV",     "MOV B, A", 0x57u,  1);
const opcode MOV_E_B    ("MOV",     "MOV C, B", 0x58u,  1);
const opcode MOV_E_C    ("MOV",     "MOV C, C", 0x59u,  1);
const opcode MOV_E_D    ("MOV",     "MOV C, D", 0x5Au,  1);
const opcode MOV_E_E    ("MOV",     "MOV C, E", 0x5Bu,  1);
const opcode MOV_E_H    ("MOV",     "MOV C, H", 0x5Cu,  1);
const opcode MOV_E_L    ("MOV",     "MOV C, L", 0x5Du,  1);
const opcode MOV_E_M    ("MOV",     "MOV C, M", 0x5Eu,  1);
const opcode MOV_E_A    ("MOV",     "MOV C, A", 0x5Fu,  1);
const opcode MOV_H_B    ("MOV",     "MOV B, B", 0x60u,  1);
const opcode MOV_H_C    ("MOV",     "MOV B, C", 0x61u,  1);
const opcode MOV_H_D    ("MOV",     "MOV B, D", 0x62u,  1);
const opcode MOV_H_E    ("MOV",     "MOV B, E", 0x63u,  1);
const opcode MOV_H_H    ("MOV",     "MOV B, H", 0x64u,  1);
const opcode MOV_H_L    ("MOV",     "MOV B, L", 0x65u,  1);
const opcode MOV_H_M    ("MOV",     "MOV B, M", 0x66u,  1);
const opcode MOV_H_A    ("MOV",     "MOV B, A", 0x67u,  1);
const opcode MOV_L_B    ("MOV",     "MOV C, B", 0x68u,  1);
const opcode MOV_L_C    ("MOV",     "MOV C, C", 0x69u,  1);
const opcode MOV_L_D    ("MOV",     "MOV C, D", 0x6Au,  1);
const opcode MOV_L_E    ("MOV",     "MOV C, E", 0x6Bu,  1);
const opcode MOV_L_H    ("MOV",     "MOV C, H", 0x6Cu,  1);
const opcode MOV_L_L    ("MOV",     "MOV C, L", 0x6Du,  1);
const opcode MOV_L_M    ("MOV",     "MOV C, M", 0x6Eu,  1);
const opcode MOV_L_A    ("MOV",     "MOV C, A", 0x6Fu,  1);
const opcode MOV_M_B    ("MOV",     "MOV M, B", 0x70u,  1);
const opcode MOV_M_C    ("MOV",     "MOV M, C", 0x71u,  1);
const opcode MOV_M_D    ("MOV",     "MOV M, D", 0x72u,  1);
const opcode MOV_M_E    ("MOV",     "MOV M, E", 0x73u,  1);
const opcode MOV_M_H    ("MOV",     "MOV M, H", 0x74u,  1);
const opcode MOV_M_L    ("MOV",     "MOV M, L", 0x75u,  1);
const opcode HLT        ("MOV",     "HLT",      0x76u,  1);
const opcode MOV_M_A    ("MOV",     "MOV M, A", 0x77u,  1);
const opcode MOV_A_B    ("MOV",     "MOV A, B", 0x78u,  1);
const opcode MOV_A_C    ("MOV",     "MOV A, C", 0x79u,  1);
const opcode MOV_A_D    ("MOV",     "MOV A, D", 0x7Au,  1);
const opcode MOV_A_E    ("MOV",     "MOV A, E", 0x7Bu,  1);
const opcode MOV_A_H    ("MOV",     "MOV A, H", 0x7Cu,  1);
const opcode MOV_A_L    ("MOV",     "MOV A, L", 0x7Du,  1);
const opcode MOV_A_M    ("MOV",     "MOV A, M", 0x7Eu,  1);
const opcode MOV_A_A    ("MOV",     "MOV A, A", 0x7Fu,  1);
const opcode ADD_B      ("ADD",     "ADD B",    0x80u,  1);
const opcode ADD_C      ("ADD",     "ADD C",    0x81u,  1);
const opcode ADD_D      ("ADD",     "ADD D",    0x82u,  1);
const opcode ADD_E      ("ADD",     "ADD E",    0x83u,  1);
const opcode ADD_H      ("ADD",     "ADD H",    0x84u,  1);
const opcode ADD_L      ("ADD",     "ADD L",    0x85u,  1);
const opcode ADD_M      ("ADD",     "ADD M",    0x86u,  1);
const opcode ADD_A      ("ADD",     "ADD A",    0x87u,  1);
const opcode ADC_B      ("ADC",     "ADC B",    0x88u,  1);
const opcode ADC_C      ("ADC",     "ADC C",    0x89u,  1);
const opcode ADC_D      ("ADC",     "ADC D",    0x8Au,  1);
const opcode ADC_E      ("ADC",     "ADC E",    0x8Bu,  1);
const opcode ADC_H      ("ADC",     "ADC H",    0x8Cu,  1);
const opcode ADC_L      ("ADC",     "ADC L",    0x8Du,  1);
const opcode ADC_M      ("ADC",     "ADC M",    0x8Eu,  1);
const opcode ADC_A      ("ADC",     "ADC A",    0x8Fu,  1);
const opcode SUB_B      ("SUB",     "SUB B",    0x90u,  1);
const opcode SUB_C      ("SUB",     "SUB C",    0x91u,  1);
const opcode SUB_D      ("SUB",     "SUB D",    0x92u,  1);
const opcode SUB_E      ("SUB",     "SUB E",    0x93u,  1);
const opcode SUB_H      ("SUB",     "SUB H",    0x94u,  1);
const opcode SUB_L      ("SUB",     "SUB L",    0x95u,  1);
const opcode SUB_M      ("SUB",     "SUB M",    0x96u,  1);
const opcode SUB_A      ("SUB",     "SUB A",    0x97u,  1);
const opcode SBB_B      ("SUB",     "SBB B",    0x98u,  1);
const opcode SBB_C      ("SUB",     "SBB C",    0x99u,  1);
const opcode SBB_D      ("SUB",     "SBB D",    0x9Au,  1);
const opcode SBB_E      ("SUB",     "SBB E",    0x9Bu,  1);
const opcode SBB_H      ("SUB",     "SBB H",    0x9Cu,  1);
const opcode SBB_L      ("SUB",     "SBB L",    0x9Du,  1);
const opcode SBB_M      ("SUB",     "SBB M",    0x9Eu,  1);
const opcode SBB_A      ("SUB",     "SBB A",    0x9Fu,  1);
const opcode ANA_B      ("ANA",     "ANA B",    0xA0u,  1);
const opcode ANA_C      ("ANA",     "ANA C",    0xA1u,  1);
const opcode ANA_D      ("ANA",     "ANA D",    0xA2u,  1);
const opcode ANA_E      ("ANA",     "ANA E",    0xA3u,  1);
const opcode ANA_H      ("ANA",     "ANA H",    0xA4u,  1);
const opcode ANA_L      ("ANA",     "ANA L",    0xA5u,  1);
const opcode ANA_M      ("ANA",     "ANA M",    0xA6u,  1);
const opcode ANA_A      ("ANA",     "ANA A",    0xA7u,  1);
const opcode XRA_B      ("XRA",     "XRA B",    0xA8u,  1);
const opcode XRA_C      ("XRA",     "XRA C",    0xA9u,  1);
const opcode XRA_D      ("XRA",     "XRA D",    0xAAu,  1);
const opcode XRA_E      ("XRA",     "XRA E",    0xABu,  1);
const opcode XRA_H      ("XRA",     "XRA H",    0xACu,  1);
const opcode XRA_L      ("XRA",     "XRA L",    0xADu,  1);
const opcode XRA_M      ("XRA",     "XRA M",    0xAEu,  1);
const opcode XRA_A      ("XRA",     "XRA A",    0xAFu,  1);
const opcode ORA_B      ("ORA",     "ORA B",    0xB0u,  1);
const opcode ORA_C      ("ORA",     "ORA C",    0xB1u,  1);
const opcode ORA_D      ("ORA",     "ORA D",    0xB2u,  1);
const opcode ORA_E      ("ORA",     "ORA E",    0xB3u,  1);
const opcode ORA_H      ("ORA",     "ORA H",    0xB4u,  1);
const opcode ORA_L      ("ORA",     "ORA L",    0xB5u,  1);
const opcode ORA_M      ("ORA",     "ORA M",    0xB6u,  1);
const opcode ORA_A      ("ORA",     "ORA A",    0xB7u,  1);
const opcode CMP_B      ("CMP",     "CMP B",    0xB8u,  1);
const opcode CMP_C      ("CMP",     "CMP C",    0xB9u,  1);
const opcode CMP_D      ("CMP",     "CMP D",    0xBAu,  1);
const opcode CMP_E      ("CMP",     "CMP E",    0xBBu,  1);
const opcode CMP_H      ("CMP",     "CMP H",    0xBCu,  1);
const opcode CMP_L      ("CMP",     "CMP L",    0xBDu,  1);
const opcode CMP_M      ("CMP",     "CMP M",    0xBEu,  1);
const opcode CMP_A      ("CMP",     "CMP A",    0xBFu,  1);
const opcode RNZ        ("RNZ",     "RNZ",      0xC0u,  1);
const opcode POP_B      ("POP",     "POP B",    0xC1u,  1);
const opcode JNZ        ("JNZ",     "JNZ",      0xC2u,  3);
const opcode JMP        ("JMP",     "JMP",      0xC3u,  3);
const opcode CNZ        ("CNZ",     "CNZ",      0xC4u,  3);
const opcode PUSH_B     ("PUSH",    "PUSH B",   0xC5u,  1);
const opcode ADI        ("ADI",     "ADI",      0xC6u,  2);
const opcode RST_0      ("RST",     "RST 0",    0xC7u,  1);
const opcode RZ         ("RZ",      "RZ",       0xC8u,  1);
const opcode RET        ("RET",     "RET",      0xC9u,  1);
const opcode JZ         ("JZ",      "JZ",       0xCAu,  3);
//Unused instruction                            0xCB.
const opcode CZ         ("CZ",      "CZ",       0xCCu,  3);
const opcode CALL       ("CALL",    "CALL",     0xCDu,  3);
const opcode ACI        ("ACI",     "ACI",      0xCEu,  2);
const opcode RST_1      ("RST",     "RST_1",    0xCFu,  1);
const opcode RNC        ("RNC",     "RNC",      0xD0u,  1);
const opcode POP_D      ("POP",     "POP D",    0xD1u,  1);
const opcode JNC        ("JNC",     "JNC",      0xD2u,  3);
const opcode OUT        ("OUT",     "OUT",      0xD3u,  2);
const opcode CNC        ("CNC",     "CNC",      0xD4u,  3);
const opcode PUSH_D     ("PUSH",    "PUSH D",   0xD5u,  1);
const opcode SUI        ("SUI",     "SUI",      0xD6u,  2);
const opcode RST_2      ("RST",     "RST 2",    0xD7u,  1);
const opcode RC         ("RC",      "RC",       0xD8u,  1);
//Unused instruction                            0xD9.
const opcode JC         ("JC",      "JC",       0xDAu,  3);
const opcode IN         ("IN",      "IN",       0xDBu,  2);
const opcode CC         ("CC",      "CC",       0xDCu,  3);
//Unused instruction                            0xDD.
const opcode SBI        ("SBI",     "SBI",      0xDEu,  2);
const opcode RST_3      ("RST",     "RST_3",    0xDFu,  1);
const opcode RPO        ("RPO",     "RPO",      0xE0u,  1);
const opcode POP_H      ("POP",     "POP H",    0xE1u,  1);
const opcode JPO        ("JPO",     "JPO",      0xE2u,  3);
const opcode XTHL       ("XTHL",    "XTHL",     0xE3u,  1);
const opcode CPO        ("CPO",     "CPO",      0xE4u,  3);
const opcode PUSH_H     ("PUSH",    "PUSH H",   0xE5u,  1);
const opcode ANI        ("ANI",     "ANI",      0xE6u,  2);
const opcode RST_4      ("RST",     "RST 4",    0xE7u,  1);
const opcode RPE        ("RPE",     "RPE",      0xE8u,  1);
const opcode PCHL       ("PCHL",    "PCHL",     0xE9u,  1);
const opcode JPE        ("JPE",     "JPE",      0xEAu,  3);
const opcode XCHG       ("XCHG",    "XCHG",     0xEBu,  1);
const opcode CPE        ("CPE",     "CPE",      0xECu,  3);
//Unused instruction                            0xED.
const opcode XRI        ("XRI",     "XRI",      0xEEu,  2);
const opcode RST_5      ("RST",     "RST 5",    0xEFu,  1);
const opcode RP         ("RP",      "RP",       0xF0u,  1);
const opcode POP_PSW    ("POP",     "POP PSW",  0xF1u,  1);
const opcode JP         ("JP",      "JP",       0xF2u,  3);
const opcode DI         ("DI",      "DI",       0xF3u,  1);
const opcode CP         ("CP",      "CP",       0xF4u,  3);
const opcode PUSH_PSW   ("PUSH",    "PUSH PSW", 0xF5u,  1);
const opcode ORI        ("ORI",     "ORI",      0xF6u,  2);
const opcode RST_6      ("RST",     "RST 6",    0xF7u,  1);
const opcode RM         ("RM",      "RM",       0xF8u,  1);
const opcode SPHL       ("SPHL",    "SPHL",     0xF9u,  1);
const opcode JM         ("JM",      "JM",       0xFAu,  3);
const opcode EI         ("EI",      "EI",       0xFBu,  1);
const opcode CM         ("CM",      "CM",       0xFCu,  3);
//Unused instruction                            0xFD.
const opcode CPI        ("CPI",     "CPI",      0xFEu,  2);
const opcode RST_7      ("RST",     "RST 7",    0xFFu,  1);

const opcode *opcodesByCode[256] = {
    &NOP     , &LXI_B   , &STAX_B  , &INX_B   , &INR_B   , &DCR_B   , &MVI_B   , &RLC,
    nullptr  , &DAD_B   , &LDAX_B  , &DCX_B   , &INR_C   , &DCR_C   , &MVI_C   , &RRC,
    nullptr  , &LXI_D   , &STAX_D  , &INX_D   , &INR_D   , &DCR_D   , &MVI_D   , &RAL,
    nullptr  , &DAD_D   , &LDAX_D  , &DCX_D   , &INR_E   , &DCR_E   , &MVI_E   , &RAR,
    //TODO: Fill all opcodes!
};

#include <set>
bool isOpcode(const char * const name) {
    static std::set<const char *, CaseInsensitive> nameSet;
    if(!nameSet.empty()) {
        for(data8_calc_t i = 0; i < 256; i++)
            if(opcodesByCode[i]->bytesRequired >= 1) nameSet.insert(opcodesByCode[i]->mnemonic);
    }
    return nameSet.find(name) != nameSet.end();
}
