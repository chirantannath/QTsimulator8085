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
#include "processor.h"

//Processor

Processor::Processor(QObject *parent) //The content of this constructor is LARGE. Take your time to go through it.
    : QObject(parent),
      microprograms(new std::function<void()>[256]),
      memory(new data8_t[MEMORY_SIZE]),
      io(new data8_t[IO_PORT_SIZE]){
    const std::function<void()> UNUSED = [&](){
        unused = 1u; emit unusedInstruction(memory[pc]);
        //pc++; pc &= 0xFFFFu; emit programCounterChanged(); This is an error
    };
    std::memset((void *)memory, 0, sizeof(data8_t) * MEMORY_SIZE);
    std::memset((void *)io, 0, sizeof(data8_t) * IO_PORT_SIZE);
    intrVec = 0;
    pc = sp = 0u;
    a = b = c = d = e = h = l = 0u; f = 0u;
    ie = intr = inta = trap = rst7_5 = rst6_5 = rst5_5 = sod = sid = halt = unused = trap_lowToHigh = 0u;
    m5_5 = m6_5 = m7_5 = 1u; //Initial state is these external interrupts are masked.

    //Microprograms (or, what to do on each opcode) is coded here.

    //NOP (no operation); hex machine code 0x00.
    microprograms[NOP]      = [&](){pc++; pc &= 0xFFFFu; emit programCounterChanged();};
    //LXI B, word (load register pair immediate BC); hex machine code 0x01.
    microprograms[LXI_B]    = [&](){
        c = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerCChanged();
        b = memory[(pc + 2u) & 0xFFFFu] & 0xFFu; emit registerBChanged();
        pc += 3; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //STAX B (store accumulator indirect BC); hex machine code 0x02.
    microprograms[STAX_B]   = [&](){
        memory[PACK(b, c)] = a & 0xFFu; emit memoryBlockUpdated(PACK(b, c), 1u);
        if(PACK(b, c) == PACK(h, l)) emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INX B (increment register pair BC); hex machine code 0x03.
    microprograms[INX_B]    = [&](){
        data16_calc_t temp = PACK(b, c); temp++; temp &= 0xFFFFu;
        UNPACK(b, c, temp); emit registerBChanged(); emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INR B (increment register B); hex machine code 0x04.
    microprograms[INR_B]    = [&](){
        data8_calc_t temp = b+1u; temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((b & 0x0Fu) + 1u) > 0x0Fu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        b = temp & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCR B (decrement register B); hex machine code 0x05.
    microprograms[DCR_B]    = [&](){
        data8_calc_t temp = b+NEGATE8(1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (b & 0x0Fu) < 1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        b = temp & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MVI B, byte (move immediate to B); hex machine code 0x06.
    microprograms[MVI_B]    = [&](){
        b = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerBChanged();
        pc += 2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RLC (rotate left); hex machine code 0x07.
    microprograms[RLC]      = [&](){
        data8_t temp = a;
        a = (((temp & 0x80) >> 7) | (temp << 1)) & 0xFF; emit accumulatorChanged();
        SET_SPEC_FLAG(f, CARRY_FLAG, (temp & 0x80) == 0x80); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0x08u]    = UNUSED;
    //DAD B (double/direct add register pair BC to HL); hex machine code 0x09.
    microprograms[DAD_B]    = [&](){
        data16_calc_t temp = PACK(b, c) + PACK(h, l);
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFFFu); emit flagsChanged();
        UNPACK(h, l, temp & 0xFFFFu); emit registerHChanged(); emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //LDAX B (load accumulator indirect BC); hex machine code 0x0A.
    microprograms[LDAX_B]   = [&](){
        a = memory[PACK(b, c)] & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCX B (decrement register pair BC); hex machine code 0x0B.
    microprograms[DCX_B]    = [&](){
        data16_calc_t temp = PACK(b, c); DECREMENT16(temp); temp &= 0xFFFFu;
        UNPACK(b, c, temp); emit registerBChanged(); emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INR C (increment register C); hex machine code 0x0C.
    microprograms[INR_C]    = [&](){
        data8_calc_t temp = c+1u; temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((c & 0x0Fu) + 1u) > 0x0Fu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        c = temp & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCR C (decrement register C); hex machine code 0x0D.
    microprograms[DCR_C]    = [&](){
        data8_calc_t temp = c+NEGATE8(1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (c & 0x0Fu) < 1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        c = temp & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MVI C, byte (move immediate to C); hex machine code 0x0E.
    microprograms[MVI_C]    = [&](){
        c = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerCChanged();
        pc += 2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RRC (rotate right); hex machine code 0x0F.
    microprograms[RRC]      = [&](){
        data8_t temp = a;
        a = (((temp & 1) << 7) | (temp >> 1)) & 0xFF; emit accumulatorChanged();
        SET_SPEC_FLAG(f, CARRY_FLAG, (temp & 0x1) == 0x1); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0x10u]    = UNUSED;
    //LXI D, word (load register pair immediate DE); hex machine code 0x11.
    microprograms[LXI_D]    = [&](){
        e = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerEChanged();
        d = memory[(pc + 2u) & 0xFFFFu] & 0xFFu; emit registerDChanged();
        pc += 3; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //STAX D (store accumulator indirect DE); hex machine code 0x12.
    microprograms[STAX_D]   = [&](){
        memory[PACK(d, e)] = a & 0xFFu; emit memoryBlockUpdated(PACK(d, e), 1u);
        if(PACK(d, e) == PACK(h, l)) emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INX D (increment register pair DE); hex machine code 0x13.
    microprograms[INX_D]    = [&](){
        data16_calc_t temp = PACK(d, e); temp++; temp &= 0xFFFFu;
        UNPACK(d, e, temp); emit registerDChanged(); emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INR D (increment register D); hex machine code 0x14.
    microprograms[INR_D]    = [&](){
        data8_calc_t temp = d+1u; temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((d & 0x0Fu) + 1u) > 0x0Fu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        d = temp & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCR D (decrement register D); hex machine code 0x15.
    microprograms[DCR_D]    = [&](){
        data8_calc_t temp = d+NEGATE8(1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (d & 0x0Fu) < 1); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        d = temp & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MVI D, byte (move immediate to D); hex machine code 0x16.
    microprograms[MVI_D]    = [&](){
        d = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerDChanged();
        pc += 2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RAL (rotate left through carry); hex machine code 0x17.
    microprograms[RAL]      = [&](){
        data8_t temp = a;
        a = temp << 1; if(CHECK_FLAG(f, CARRY_FLAG)) a |= 1u;
        a &= 0xFF; emit accumulatorChanged();
        SET_SPEC_FLAG(f, CARRY_FLAG, (temp & 0x80) == 0x80); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0x18u]    = UNUSED;
    //DAD D (double/direct add register pair DE to HL); hex machine code 0x19.
    microprograms[DAD_D]    = [&](){
        data16_calc_t temp = PACK(d, e) + PACK(h, l);
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFFFu); emit flagsChanged();
        UNPACK(h, l, temp & 0xFFFFu); emit registerHChanged(); emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //LDAX D (load accumulator indirect DE); hex machine code 0x1A.
    microprograms[LDAX_D]   = [&](){
        a = memory[PACK(d, e)] & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCX D (decrement register pair DE); hex machine code 0x1B.
    microprograms[DCX_D]    = [&](){
        data16_calc_t temp = PACK(d, e); DECREMENT16(temp); temp &= 0xFFFFu;
        UNPACK(d, e, temp); emit registerDChanged(); emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INR E (increment register E); hex machine code 0x1C.
    microprograms[INR_E]    = [&](){
        data8_calc_t temp = e+1u; temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((e & 0x0Fu) + 1u) > 0x0Fu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        e = temp & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCR E (decrement register E); hex machine code 0x1D.
    microprograms[DCR_E]    = [&](){
        data8_calc_t temp = e+NEGATE8(1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (e & 0x0Fu) < 1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        e = temp & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MVI E, byte (move immediate to E); hex machine code 0x1E.
    microprograms[MVI_E]    = [&](){
        e = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerEChanged();
        pc += 2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RAR (rotate right through carry); hex machine code 0x1F.
    microprograms[RAR]      = [&](){
        data8_t temp = a;
        a = temp >> 1; if(CHECK_FLAG(f, CARRY_FLAG)) a |= 0x80;
        a &= 0xFF; emit accumulatorChanged();
        SET_SPEC_FLAG(f, CARRY_FLAG, (temp & 0x1) == 0x1); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RIM (read interrupt masks); hex machine code 0x20.
    microprograms[RIM]      = [&](){
        a = ((sid << 7) | (rst7_5 << 6) | (rst6_5 << 5) | (rst5_5 << 4) | (ie << 3) |
             (m7_5 << 2) | (m6_5 << 1) | m5_5) & 0xFFu;
        emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //LXI H, word (load register pair immediate HL); hex machine code 0x21.
    microprograms[LXI_H]    = [&](){
        l = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerLChanged();
        h = memory[(pc + 2u) & 0xFFFFu] & 0xFFu; emit registerHChanged();
        emit MChanged();
        pc += 3; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SHLD word (store HL direct); hex machine code 0x22.
    microprograms[SHLD]   = [&](){
        memaddr_t offset = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        memory[offset] = l & 0xFFu; memory[(offset + 1) & 0xFFFFu] = h & 0xFFu;
        emit memoryBlockUpdated(offset, 2u);
        if(offset == PACK(h, l)) emit MChanged();
        pc+=3; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INX H (increment register pair HL); hex machine code 0x23.
    microprograms[INX_H]    = [&](){
        data16_calc_t temp = PACK(h, l); temp++; temp &= 0xFFFFu;
        UNPACK(h, l, temp); emit registerHChanged(); emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INR H (increment register H); hex machine code 0x24.
    microprograms[INR_H]    = [&](){
        data8_calc_t temp = h+1u; temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((h & 0x0Fu) + 1u) > 0x0Fu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        h = temp & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCR H (decrement register H); hex machine code 0x25.
    microprograms[DCR_H]    = [&](){
        data8_calc_t temp = h+NEGATE8(1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (h & 0x0Fu) < 1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        h = temp & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MVI H, byte (move immediate to H); hex machine code 0x26.
    microprograms[MVI_H]    = [&](){
        h = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerHChanged(); emit MChanged();
        pc += 2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DAA (decimal adjust accumulator); hex machine code 0x27.
    microprograms[DAA]      = [&](){
        data8_calc_t temp = a & 0xFFu;
        if(CHECK_FLAG(f, AUXILIARY_CARRY_FLAG) || (temp & 0x0Fu) > 9u) temp += 6u;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0x0Fu) + 6u) > 0x0Fu);
        if(((temp >> 4) & 0x0F) > 9u || CHECK_FLAG(f, CARRY_FLAG)) temp += 0x60u;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0x28u]    = UNUSED;
    //DAD H (double/direct add register pair HL to HL); hex machine code 0x29.
    microprograms[DAD_H]    = [&](){
        data16_calc_t temp = (data16_calc_t)PACK(h, l) << 1;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFFFu); emit flagsChanged();
        UNPACK(h, l, temp & 0xFFFFu); emit registerHChanged(); emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //LHLD word (load HL direct); hex machine code 0x2A.
    microprograms[LHLD]   = [&](){
        memaddr_t offset = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        l = memory[offset] & 0xFFu; h = memory[(offset + 1) & 0xFFFFu] & 0xFFu;
        emit registerHChanged(); emit registerLChanged(); emit MChanged();
        pc+=3; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCX H (decrement register pair HL); hex machine code 0x2B.
    microprograms[DCX_H]    = [&](){
        data16_calc_t temp = PACK(h, l); DECREMENT16(temp); temp &= 0xFFFFu;
        UNPACK(h, l, temp); emit registerHChanged(); emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INR L (increment register L); hex machine code 0x2C.
    microprograms[INR_L]    = [&](){
        data8_calc_t temp = l+1u; temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((l & 0x0Fu) + 1u) > 0x0Fu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        l = temp & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCR L (decrement register L); hex machine code 0x2D.
    microprograms[DCR_L]    = [&](){
        data8_calc_t temp = l+NEGATE8(1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (l & 0x0Fu) < 1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        l = temp & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MVI L, byte (move immediate to L); hex machine code 0x2E.
    microprograms[MVI_L]    = [&](){
        l = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit registerLChanged(); emit MChanged();
        pc += 2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMA (complement accumulator); hex machine code 0x2F.
    microprograms[CMA]      = [&](){
        a = ~a & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //Set interrupt masks (SIM); hex machine code 0x30.
    microprograms[SIM]      = [&](){
        if (a & 0x08u) {
            m5_5 = a & 1u; emit maskRestart5_5Changed();
            m6_5 = (a >> 1) & 1u; emit maskRestart6_5Changed();
            m7_5 = (a >> 2) & 1u; emit maskRestart7_5Changed();
        }
        if(a & 0x10u) {rst7_5 = 0u; emit restart7_5RequestStatusChanged();}
        if(a & 0x40u) {sod = (a >> 7) & 1u; emit serialOutput();}
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //LXI SP, word (load register pair immediate SP); hex machine code 0x31.
    microprograms[LXI_SP]   = [&](){
        sp = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        emit stackPointerChanged();
        pc+=3; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //STA word (store accumulator direct); hex machine code 0x32.
    microprograms[STA]      = [&](){
        memaddr_t offset = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        memory[offset] = a & 0xFFu; emit memoryBlockUpdated(offset, 1u);
        if(offset == PACK(h, l)) emit MChanged();
        pc+=3; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INX SP (increment register pair SP); hex machine code 0x33.
    microprograms[INX_SP]   = [&](){
        sp++; sp &= 0xFFFFu; emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INR M (increment memory); hex machine code 0x34.
    microprograms[INR_M]    = [&](){
        data8_calc_t temp = memory[PACK(h, l)]+1u; temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((memory[PACK(h, l)] & 0x0Fu) + 1u) > 0x0Fu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        memory[PACK(h, l)] = temp & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCR M (decrement memory); hex machine code 0x35.
    microprograms[DCR_M]    = [&](){
        data8_calc_t temp = memory[PACK(h, l)]+NEGATE8(1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (memory[PACK(h, l)] & 0x0Fu) < 1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        memory[PACK(h, l)] = temp & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MVI M, byte (move immediate to memory); hex machine code 0x36.
    microprograms[MVI_M]    = [&](){
        memory[PACK(h, l)] = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc += 2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //STC (set carry); hex machine code 0x37.
    microprograms[STC]      = [&](){
        SET_FLAG(f, CARRY_FLAG); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0x38u]    = UNUSED;
    //DAD H (double/direct add register pair HL to HL); hex machine code 0x39.
    microprograms[DAD_SP]   = [&](){
        data16_calc_t temp = PACK(h, l) + sp;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFFFu); emit flagsChanged();
        UNPACK(h, l, temp & 0xFFFFu); emit registerHChanged(); emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //LDA word (load accumulator direct); hex machine code 0x3A.
    microprograms[LDA]      = [&](){
        memaddr_t offset = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        a = memory[offset] & 0xFFu; emit accumulatorChanged();
        pc+=3; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCX SP (decrement register pair SP); hex machine code 0x3B.
    microprograms[DCX_SP]   = [&](){
        DECREMENT16(sp); sp &= 0xFFFFu; emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //INR A (increment accumulator); hex machine code 0x3C.
    microprograms[INR_A]    = [&](){
        data8_calc_t temp = a+1u; temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0x0Fu) + 1u) > 0x0Fu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //DCR A (decrement accumulator); hex machine code 0x3D.
    microprograms[DCR_A]    = [&](){
        data8_calc_t temp = a+NEGATE8(1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (a & 0x0Fu) < 1u); temp &= 0xFFu;
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MVI A, byte (move immediate to accumulator); hex machine code 0x3E.
    microprograms[MVI_A]    = [&](){
        a = memory[(pc + 1u) & 0xFFFFu] & 0xFFu; emit accumulatorChanged();
        pc += 2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMC (complement carry); hex machine code 0x3F.
    microprograms[CMC]      = [&](){
        SET_SPEC_FLAG(f, CARRY_FLAG, !CHECK_FLAG(f, CARRY_FLAG)); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV B, B (move register B to B); hex machine code 0x40.
    microprograms[MOV_B_B]  = microprograms[NOP]; //Why does this instruction exist??
    //MOV B, C (move register C to B); hex machine code 0x41.
    microprograms[MOV_B_C]  = [&](){
        b = c & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV B, D (move register D to B); hex machine code 0x42.
    microprograms[MOV_B_D]  = [&](){
        b = d & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV B, E (move register E to B); hex machine code 0x43.
    microprograms[MOV_B_E]  = [&](){
        b = e & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV B, H (move register H to B); hex machine code 0x44.
    microprograms[MOV_B_H]  = [&](){
        b = h & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV B, L (move register L to B); hex machine code 0x45.
    microprograms[MOV_B_L]  = [&](){
        b = l & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV B, M (move memory to B); hex machine code 0x46.
    microprograms[MOV_B_M]  = [&](){
        b = memory[PACK(h, l)] & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV B, A (move accumulator to B); hex machine code 0x47.
    microprograms[MOV_B_A]  = [&](){
        b = a & 0xFFu; emit registerBChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV C, B (move register B to C); hex machine code 0x48.
    microprograms[MOV_C_B]  = [&](){
        c = b & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV C, C (move register C to C); hex machine code 0x49.
    microprograms[MOV_C_C]  = microprograms[NOP]; //Why does this instruction exist??
    //MOV C, D (move register D to C); hex machine code 0x4A.
    microprograms[MOV_C_D]  = [&](){
        c = d & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV C, E (move register E to C); hex machine code 0x4B.
    microprograms[MOV_C_E]  = [&](){
        c = e & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV C, H (move register H to C); hex machine code 0x4C.
    microprograms[MOV_C_H]  = [&](){
        c = h & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV C, L (move register L to C); hex machine code 0x4D.
    microprograms[MOV_C_L]  = [&](){
        c = l & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV C, M (move memory to C); hex machine code 0x4E.
    microprograms[MOV_C_M]  = [&](){
        c = memory[PACK(h, l)] & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV C, A (move accumulator to C); hex machine code 0x4F.
    microprograms[MOV_C_A]  = [&](){
        c = a & 0xFFu; emit registerCChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV D, B (move register B to D); hex machine code 0x50.
    microprograms[MOV_D_B]  = [&](){
        d = b & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV D, C (move register C to D); hex machine code 0x51.
    microprograms[MOV_D_C]  = [&](){
        d = c & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV D, D (move register D to D); hex machine code 0x52.
    microprograms[MOV_D_D]  = microprograms[NOP]; //Why does this instruction exist??
    //MOV D, E (move register C to D); hex machine code 0x53.
    microprograms[MOV_D_E]  = [&](){
        d = e & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV D, H (move register C to D); hex machine code 0x54.
    microprograms[MOV_D_H]  = [&](){
        d = h & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV D, L (move register C to D); hex machine code 0x55.
    microprograms[MOV_D_L]  = [&](){
        d = l & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV D, M (move memory to D); hex machine code 0x56.
    microprograms[MOV_D_M]  = [&](){
        d = memory[PACK(h, l)] & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV D, A (move accumulator to D); hex machine code 0x57.
    microprograms[MOV_D_A]  = [&](){
        d = a & 0xFFu; emit registerDChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV E, B (move register B to E); hex machine code 0x58.
    microprograms[MOV_E_B]  = [&](){
        e = b & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV E, C (move register C to E); hex machine code 0x59.
    microprograms[MOV_E_C]  = [&](){
        e = c & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV E, D (move register D to E); hex machine code 0x5A.
    microprograms[MOV_E_D]  = [&](){
        e = d & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV E, E (move register E to E); hex machine code 0x5B.
    microprograms[MOV_E_E]  = microprograms[NOP]; //Why does this instruction exist??
    //MOV E, H (move register H to E); hex machine code 0x5C.
    microprograms[MOV_E_H]  = [&](){
        e = h & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV E, L (move register L to E); hex machine code 0x5D.
    microprograms[MOV_E_L]  = [&](){
        e = l & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV E, M (move memory to E); hex machine code 0x5E.
    microprograms[MOV_E_M]  = [&](){
        e = memory[PACK(h, l)] & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV E, A (move accumulator to E); hex machine code 0x5F.
    microprograms[MOV_E_A]  = [&](){
        e = a & 0xFFu; emit registerEChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV H, B (move register B to H); hex machine code 0x60.
    microprograms[MOV_H_B]  = [&](){
        h = b & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV H, C (move register C to H); hex machine code 0x61.
    microprograms[MOV_H_C]  = [&](){
        h = c & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV H, D (move register D to H); hex machine code 0x62.
    microprograms[MOV_H_D]  = [&](){
        h = d & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV H, E (move register E to H); hex machine code 0x63.
    microprograms[MOV_H_E]  = [&](){
        h = e & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV H, H (move register H to H); hex machine code 0x64.
    microprograms[MOV_H_H]  = microprograms[NOP]; //Why does this instruction exist??
    //MOV H, L (move register L to H); hex machine code 0x65.
    microprograms[MOV_H_L]  = [&](){
        h = l & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV H, M (move memory to H); hex machine code 0x66.
    microprograms[MOV_H_M]  = [&](){
        h = memory[PACK(h, l)] & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV H, A (move accumulator to H); hex machine code 0x67.
    microprograms[MOV_H_A]  = [&](){
        h = a & 0xFFu; emit registerHChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV L, B (move register B to L); hex machine code 0x68.
    microprograms[MOV_L_B]  = [&](){
        l = b & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV L, C (move register C to L); hex machine code 0x69.
    microprograms[MOV_L_C]  = [&](){
        l = c & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV L, D (move register D to L); hex machine code 0x6A.
    microprograms[MOV_L_D]  = [&](){
        l = d & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV L, E (move register E to L); hex machine code 0x6B.
    microprograms[MOV_L_E]  = [&](){
        l = e & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV L, H (move register H to L); hex machine code 0x6C.
    microprograms[MOV_L_H]  = [&](){
        l = h & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV L, L (move register L to L); hex machine code 0x6D.
    microprograms[MOV_L_L]  = microprograms[NOP]; //Why does this instruction exist??
    //MOV L, M (move memory to L); hex machine code 0x6E.
    microprograms[MOV_L_M]  = [&](){
        l = memory[PACK(h, l)] & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV L, A (move accumulator to L); hex machine code 0x6F.
    microprograms[MOV_L_A]  = [&](){
        l = a & 0xFFu; emit registerLChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV M, B (move register B to memory); hex machine code 0x70.
    microprograms[MOV_M_B]  = [&](){
        memory[PACK(h, l)] = b & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV M, C (move register C to memory); hex machine code 0x71.
    microprograms[MOV_M_C]  = [&](){
        memory[PACK(h, l)] = c & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV M, D (move register D to memory); hex machine code 0x72.
    microprograms[MOV_M_D]  = [&](){
        memory[PACK(h, l)] = d & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV M, E (move register E to memory); hex machine code 0x73.
    microprograms[MOV_M_E]  = [&](){
        memory[PACK(h, l)] = e & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV M, H (move register H to memory); hex machine code 0x74.
    microprograms[MOV_M_H]  = [&](){
        memory[PACK(h, l)] = h & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV M, L (move register L to memory); hex machine code 0x75.
    microprograms[MOV_M_L]  = [&](){
        memory[PACK(h, l)] = l & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //HLT (halt); hex machine code 0x76.
    microprograms[HLT]      = [&](){
        halt = 1u; //Remember to call halted() outside of this function
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV M, A (move accumulator to memory); hex machine code 0x77.
    microprograms[MOV_M_A]  = [&](){
        memory[PACK(h, l)] = a & 0xFFu; emit MChanged(); emit memoryBlockUpdated(PACK(h, l), 1u);
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV A, B (move register B to accumulator); hex machine code 0x78.
    microprograms[MOV_A_B]  = [&](){
        a = b & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV A, C (move register B to accumulator); hex machine code 0x79.
    microprograms[MOV_A_C]  = [&](){
        a = c & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV A, D (move register B to accumulator); hex machine code 0x7A.
    microprograms[MOV_A_D]  = [&](){
        a = d & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV A, E (move register B to accumulator); hex machine code 0x7B.
    microprograms[MOV_A_E]  = [&](){
        a = e & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV A, H (move register B to accumulator); hex machine code 0x7C.
    microprograms[MOV_A_H]  = [&](){
        a = h & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV A, L (move register B to accumulator); hex machine code 0x7D.
    microprograms[MOV_A_L]  = [&](){
        a = l & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV A, M (move register B to accumulator); hex machine code 0x7E.
    microprograms[MOV_A_M]  = [&](){
        a = memory[PACK(h, l)] & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //MOV A, A (move accumulator to accumulator); hex machine code 0x7F.
    microprograms[MOV_A_A]  = microprograms[NOP]; //Why does this instruction exist??
    //ADD B (add register B to accumulator); hex machine code 0x80.
    microprograms[ADD_B]    = [&](){
        data8_calc_t temp = a + b;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (b & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADD C (add register C to accumulator); hex machine code 0x81.
    microprograms[ADD_C]    = [&](){
        data8_calc_t temp = a + c;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (c & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADD D (add register D to accumulator); hex machine code 0x82.
    microprograms[ADD_D]    = [&](){
        data8_calc_t temp = a + d;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (d & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADD E (add register E to accumulator); hex machine code 0x83.
    microprograms[ADD_E]    = [&](){
        data8_calc_t temp = a + e;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (e & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADD H (add register H to accumulator); hex machine code 0x84.
    microprograms[ADD_H]    = [&](){
        data8_calc_t temp = a + h;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (h & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADD L (add register L to accumulator); hex machine code 0x85.
    microprograms[ADD_L]    = [&](){
        data8_calc_t temp = a + l;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (l & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADD M (add memory to accumulator); hex machine code 0x86.
    microprograms[ADD_M]    = [&](){
        data8_calc_t temp = a + memory[PACK(h, l)];
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (memory[PACK(h, l)] & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADD A (add accuulator to accumulator); hex machine code 0x87.
    microprograms[ADD_A]    = [&](){
        data8_calc_t temp = (data8_calc_t)a << 1;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((data8_calc_t)(a & 0xFu) << 1) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADC B (add register B to accumulator with carry); hex machine code 0x88.
    microprograms[ADC_B]    = [&](){
        data8_t cy= CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u;
        data8_calc_t temp = a + b + cy;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (b & 0xFu) + cy) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADC C (add register C to accumulator with carry); hex machine code 0x89.
    microprograms[ADC_C]    = [&](){
        data8_t cy= CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u;
        data8_calc_t temp = a + c + cy;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (c & 0xFu) + cy) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADC D (add register D to accumulator with carry); hex machine code 0x8A.
    microprograms[ADC_D]    = [&](){
        data8_t cy= CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u;
        data8_calc_t temp = a + d + cy;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (d & 0xFu) + cy) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADC E (add register E to accumulator with carry); hex machine code 0x8B.
    microprograms[ADC_E]    = [&](){
        data8_t cy= CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u;
        data8_calc_t temp = a + e + cy;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (e & 0xFu) + cy) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADC H (add register H to accumulator with carry); hex machine code 0x8C.
    microprograms[ADC_H]    = [&](){
        data8_t cy= CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u;
        data8_calc_t temp = a + h + cy;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (h & 0xFu) + cy) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADC L (add register L to accumulator with carry); hex machine code 0x8D.
    microprograms[ADC_L]    = [&](){
        data8_t cy= CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u;
        data8_calc_t temp = a + l + cy;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (l & 0xFu) + cy) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADC M (add memory to accumulator with carry); hex machine code 0x8E.
    microprograms[ADC_M]    = [&](){
        data8_t cy= CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u;
        data8_calc_t temp = a + memory[PACK(h, l)] + cy;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (memory[PACK(h, l)] & 0xFu) + cy) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADC A (add accumulator to accumulator with carry); hex machine code 0x8F.
    microprograms[ADC_A]    = [&](){
        data8_t cy= CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u;
        data8_calc_t temp = ((data8_calc_t)a << 1) + cy;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, (((data8_calc_t)(a & 0xFu) << 1) + cy) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUB B (subtract register B from accumulator); hex machine code 0x90.
    microprograms[SUB_B]    = [&](){
        data8_calc_t temp = a + NEGATE8(b);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < b); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (b & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == b);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUB C (subtract register C from accumulator); hex machine code 0x91.
    microprograms[SUB_C]    = [&](){
        data8_calc_t temp = a + NEGATE8(c);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < c); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (c & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == c);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUB D (subtract register D from accumulator); hex machine code 0x92.
    microprograms[SUB_D]    = [&](){
        data8_calc_t temp = a + NEGATE8(d);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < d); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (d & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == d);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUB E (subtract register E from accumulator); hex machine code 0x93.
    microprograms[SUB_E]    = [&](){
        data8_calc_t temp = a + NEGATE8(e);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < e); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (e & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == e);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUB H (subtract register H from accumulator); hex machine code 0x94.
    microprograms[SUB_H]    = [&](){
        data8_calc_t temp = a + NEGATE8(h);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < h); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (h & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == h);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUB L (subtract register L from accumulator); hex machine code 0x95.
    microprograms[SUB_L]    = [&](){
        data8_calc_t temp = a + NEGATE8(l);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < l); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (l & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == l);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUB M (subtract memory from accumulator); hex machine code 0x96.
    microprograms[SUB_M]    = [&](){
        data8_calc_t temp = a + NEGATE8(memory[PACK(h, l)]);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < memory[PACK(h, l)]); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (memory[PACK(h, l)] & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == memory[PACK(h, l)]);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUB A (subtract accumulator from accumulator); hex machine code 0x97.
    microprograms[SUB_A]    = [&](){
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_FLAG(f, ZERO_FLAG); //always 1
        UNSET_FLAG(f, SIGN_FLAG); //always 0
        SET_FLAG(f, PARITY_FLAG); //always 1
        emit flagsChanged();
        a = 0u; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SBB B (subtract register B from accumulator with borrow); hex machine code 0x98.
    microprograms[SBB_B]    = [&](){
        data8_t rhs = (b + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u)) & 0xFF;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SBB C (subtract register C from accumulator with borrow); hex machine code 0x99.
    microprograms[SBB_C]    = [&](){
        data8_t rhs = (c + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u)) & 0xFF;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SBB D (subtract register D from accumulator with borrow); hex machine code 0x9A.
    microprograms[SBB_D]    = [&](){
        data8_t rhs = (d + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u)) & 0xFF;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SBB E (subtract register E from accumulator with borrow); hex machine code 0x9B.
    microprograms[SBB_E]    = [&](){
        data8_t rhs = (e + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u)) & 0xFF;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SBB H (subtract register H from accumulator with borrow); hex machine code 0x9C.
    microprograms[SBB_H]    = [&](){
        data8_t rhs = (h + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u)) & 0xFF;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SBB L (subtract register L from accumulator with borrow); hex machine code 0x9D.
    microprograms[SBB_L]    = [&](){
        data8_t rhs = (l + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u)) & 0xFF;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SBB M (subtract memory from accumulator with borrow); hex machine code 0x9E.
    microprograms[SBB_M]    = [&](){
        data8_t rhs = (memory[PACK(h, l)] + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u)) & 0xFF;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SBB A (subtract accumulator from accumulator with borrow); hex machine code 0x9F.
    microprograms[SBB_A]    = [&](){
        SET_FLAG(f, CARRY_FLAG); //always 1
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        UNSET_FLAG(f, ZERO_FLAG); //always 0
        SET_FLAG(f, SIGN_FLAG); //always 1
        SET_FLAG(f, PARITY_FLAG); //always 1
        emit flagsChanged();
        a = 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANA B (AND register B with accumulator); hex machine code 0xA0.
    microprograms[ANA_B]    = [&](){
        a = a & b & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANA C (AND register C with accumulator); hex machine code 0xA1.
    microprograms[ANA_C]    = [&](){
        a = a & c & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANA D (AND register D with accumulator); hex machine code 0xA2.
    microprograms[ANA_D]    = [&](){
        a = a & d & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANA E (AND register E with accumulator); hex machine code 0xA3.
    microprograms[ANA_E]    = [&](){
        a = a & e & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANA H (AND register H with accumulator); hex machine code 0xA4.
    microprograms[ANA_H]    = [&](){
        a = a & h & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANA L (AND register L with accumulator); hex machine code 0xA5.
    microprograms[ANA_L]    = [&](){
        a = a & l & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANA M (AND memory with accumulator); hex machine code 0xA6.
    microprograms[ANA_M]    = [&](){
        a = a & memory[PACK(h, l)] & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANA A (AND accumulator with accumulator); hex machine code 0xA7.
    microprograms[ANA_A]    = [&](){
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XRA B (XOR register B with accumulator); hex machine code 0xA8.
    microprograms[XRA_B]    = [&](){
        a = (a ^ b) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XRA C (XOR register C with accumulator); hex machine code 0xA9.
    microprograms[XRA_C]    = [&](){
        a = (a ^ c) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XRA D (XOR register D with accumulator); hex machine code 0xAA.
    microprograms[XRA_D]    = [&](){
        a = (a ^ d) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XRA E (XOR register E with accumulator); hex machine code 0xAB.
    microprograms[XRA_E]    = [&](){
        a = (a ^ e) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XRA H (XOR register H with accumulator); hex machine code 0xAC.
    microprograms[XRA_H]    = [&](){
        a = (a ^ h) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XRA L (XOR register L with accumulator); hex machine code 0xAD.
    microprograms[XRA_L]    = [&](){
        a = (a ^ l) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XRA M (XOR memory with accumulator); hex machine code 0xAE.
    microprograms[XRA_M]    = [&](){
        a = (a ^ memory[PACK(h, l)]) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XRA A (XOR accumulator with accumulator); hex machine code 0xAF.
    microprograms[XRA_A]    = [&](){
        a = 0u; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_FLAG(f, ZERO_FLAG); //always 1
        UNSET_FLAG(f, SIGN_FLAG); //always 0
        SET_FLAG(f, PARITY_FLAG); //always 1
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORA B (OR register B with accumulator); hex machine code 0xB0.
    microprograms[ORA_B]    = [&](){
        a = (a | b) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORA C (OR register C with accumulator); hex machine code 0xB1.
    microprograms[ORA_C]    = [&](){
        a = (a | c) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORA D (OR register D with accumulator); hex machine code 0xB2.
    microprograms[ORA_D]    = [&](){
        a = (a | d) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORA E (OR register E with accumulator); hex machine code 0xB3.
    microprograms[ORA_E]    = [&](){
        a = (a | e) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORA H (OR register H with accumulator); hex machine code 0xB4.
    microprograms[ORA_H]    = [&](){
        a = (a | h) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORA L (OR register L with accumulator); hex machine code 0xB5.
    microprograms[ORA_L]    = [&](){
        a = (a | l) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORA M (OR memory with accumulator); hex machine code 0xB6.
    microprograms[ORA_M]    = [&](){
        a = (a | memory[PACK(h, l)]) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORA A (OR accumulator with accumulator); hex machine code 0xB7.
    microprograms[ORA_A]    = [&](){
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMP B (compare register B against accumulator); hex machine code 0xB8.
    microprograms[CMP_B]    = [&](){
        data8_calc_t temp = a + NEGATE8(b);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < b); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (b & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == b);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMP C (compare register C against accumulator); hex machine code 0xB9.
    microprograms[CMP_C]    = [&](){
        data8_calc_t temp = a + NEGATE8(c);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < c); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (c & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == c);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMP D (compare register D against accumulator); hex machine code 0xBA.
    microprograms[CMP_D]    = [&](){
        data8_calc_t temp = a + NEGATE8(d);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < d); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (d & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == d);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMP E (compare register E against accumulator); hex machine code 0xBB.
    microprograms[CMP_E]    = [&](){
        data8_calc_t temp = a + NEGATE8(e);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < e); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (e & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == e);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMP H (compare register H against accumulator); hex machine code 0xBC.
    microprograms[CMP_H]    = [&](){
        data8_calc_t temp = a + NEGATE8(h);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < h); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (h & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == h);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMP L (compare register L against accumulator); hex machine code 0xBD.
    microprograms[CMP_L]    = [&](){
        data8_calc_t temp = a + NEGATE8(l);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < l); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (l & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == l);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMP M (compare memory against accumulator); hex machine code 0xBE.
    microprograms[CMP_M]    = [&](){
        data8_calc_t temp = a + NEGATE8(memory[PACK(h, l)]);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < memory[PACK(h, l)]); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (memory[PACK(h, l)] & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == memory[PACK(h, l)]);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CMP A (compare accumulator against accumulator); hex machine code 0xBF.
    microprograms[CMP_A]    = [&](){
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_FLAG(f, ZERO_FLAG); //always 1
        UNSET_FLAG(f, SIGN_FLAG); //always 0
        SET_FLAG(f, PARITY_FLAG); //always 1
        emit flagsChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RNZ (return on nonzero); hex machine code 0xC0.
    microprograms[RNZ]      = [&](){
        if(!CHECK_FLAG(f, ZERO_FLAG)) {
            pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
            sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        } else pc++;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //POP B (pop BC from stack); hex machine code 0xC1.
    microprograms[POP_B]    = [&](){
        c = memory[sp & 0xFFFFu]; sp++; emit registerCChanged();
        b = memory[sp & 0xFFFFu]; sp++; emit registerBChanged();
        sp &= 0xFFFFu; emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //JNZ address (jump on nonzero); hex machine code 0xC2.
    microprograms[JNZ]      = [&](){
        if(!CHECK_FLAG(f, ZERO_FLAG))
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //JMP address (jump); hex machine code 0xC3.
    microprograms[JMP]      = [&](){
        pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CNZ address (Call on nonzero); hex machine code 0xC4.
    microprograms[CNZ]      = [&](){
        if(!CHECK_FLAG(f, ZERO_FLAG)) {
            pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        } else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //PUSH B (push BC on stack); hex machine code 0xC5.
    microprograms[PUSH_B]   = [&](){
        sp--; sp &= 0xFFFFu; memory[sp] = b & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = c & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ADI byte (add immediate to accumulator); hex machine code 0xC6.
    microprograms[ADI]      = [&](){
        data8_t rhs = memory[(pc + 1) & 0xFFFFu] & 0xFFu;
        data8_calc_t temp = a + rhs;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (rhs & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RST 0 (restart 0); hex machine code 0xC7.
    microprograms[RST_0]    = [&](){
        pc++; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); //pc--; pc &= 0xFFFFu; //Go to previous correct location (not required)
        pc = 0u; emit programCounterChanged();
    };
    //RZ (return on zero); hex machine code 0xC8.
    microprograms[RZ]       = [&](){
        if(CHECK_FLAG(f, ZERO_FLAG)) {
            pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
            sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        } else pc++;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RET (return); hex machine code 0xC9.
    microprograms[RET]      = [&](){
        pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
        sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //JZ address (jump on zero); hex machine code 0xCA.
    microprograms[JZ]       = [&](){
        if(CHECK_FLAG(f, ZERO_FLAG))
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0xCBu]    = UNUSED;
    //CZ address (call on zero); hex machine code 0xCC.
    microprograms[CZ]      = [&](){
        if(CHECK_FLAG(f, ZERO_FLAG)) {
            pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        } else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CALL address (call); hex machine code 0xCD.
    microprograms[CALL]     = [&](){
        pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
        pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ACI byte (add immediate to accumulator); hex machine code 0xCE.
    microprograms[ACI]      = [&](){
        data8_t rhs = (memory[(pc + 1) & 0xFFFFu] & 0xFFu) + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u);
        data8_calc_t temp = a + rhs;
        SET_SPEC_FLAG(f, CARRY_FLAG, temp > 0xFFu); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) + (rhs & 0xFu)) > 0xFu);
        SET_SPEC_FLAG(f, ZERO_FLAG, temp == 0u);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RST 1 (restart 1); hex machine code 0xCF.
    microprograms[RST_1]    = [&](){
        pc++; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); //pc--; pc &= 0xFFFFu; //Go to previous correct location (not required)
        pc = 8u; emit programCounterChanged();
    };
    //RNC (return on NOT carry); hex machine code 0xD0.
    microprograms[RNC]       = [&](){
        if(!CHECK_FLAG(f, CARRY_FLAG)) {
            pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
            sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        } else pc++;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //POP D (pop DE from stack); hex machine code 0xD1.
    microprograms[POP_D]    = [&](){
        e = memory[sp & 0xFFFFu]; sp++; emit registerEChanged();
        d = memory[sp & 0xFFFFu]; sp++; emit registerDChanged();
        sp &= 0xFFFFu; emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //JNC address (jump on NOT carry); hex machine code 0xD2.
    microprograms[JNC]      = [&](){
        if(!CHECK_FLAG(f, CARRY_FLAG))
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //OUT port (output); hex machine code 0xD3.
    microprograms[OUT]      = [&](){
        ioaddr_t port = memory[(pc + 1) & 0xFFFFu] & 0xFFu;
        io[port] = a & 0xFFu; emit ioPortUpdated(port);
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CNC address (Call on NOT carry); hex machine code 0xD4.
    microprograms[CNC]      = [&](){
        if(!CHECK_FLAG(f, CARRY_FLAG)) {
            pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        } else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //PUSH D (push DE on stack); hex machine code 0xD5.
    microprograms[PUSH_D]   = [&](){
        sp--; sp &= 0xFFFFu; memory[sp] = d & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = e & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SUI byte (subtract immediate from accumulator); hex machine code 0xD6.
    microprograms[SUI]      = [&](){
        data8_t rhs = memory[(pc + 1) & 0xFFFFu] & 0xFFu;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RST 2 (restart 2); hex machine code 0xD7.
    microprograms[RST_2]    = [&](){
        pc++; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); //pc--; pc &= 0xFFFFu; //Go to previous correct location (not required)
        pc = 16u; emit programCounterChanged();
    };
    //RC (return on carry); hex machine code 0xD8.
    microprograms[RC]       = [&](){
        if(CHECK_FLAG(f, CARRY_FLAG)) {
            pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
            sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        } else pc++;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0xD9u]    = UNUSED;
    //JC address (jump on carry); hex machine code 0xDA.
    microprograms[JC]       = [&](){
        if(CHECK_FLAG(f, CARRY_FLAG))
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //Input (IN); hex machine code 0xDB.
    microprograms[IN]       = [&](){
        ioaddr_t port = memory[(pc + 1) & 0xFFFFu] & 0xFFu;
        a = io[port] & 0xFFu; emit accumulatorChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CC address (call on carry); hex machine code 0xDC.
    microprograms[CC]      = [&](){
        if(CHECK_FLAG(f, ZERO_FLAG)) {
            pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        } else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0xDDu]    = UNUSED;
    //SBI byte (subtract immediate from accumulator with borrow); hex machine code 0xDE.
    microprograms[SBI]      = [&](){
        data8_t rhs = (memory[(pc + 1) & 0xFFFFu] & 0xFFu) + (CHECK_FLAG(f, CARRY_FLAG) ? 1u : 0u);
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        a = temp & 0xFFu; emit accumulatorChanged();
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RST 3 (restart 3); hex machine code 0xDF.
    microprograms[RST_3]    = [&](){
        pc++; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); //pc--; pc &= 0xFFFFu; //Go to previous correct location (not required)
        pc = 24u; emit programCounterChanged();
    };
    //RPO (return on parity odd); hex machine code 0xE0.
    microprograms[RPO]       = [&](){
        if(!CHECK_FLAG(f, PARITY_FLAG)) {
            pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
            sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        } else pc++;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //POP H (pop HL from stack); hex machine code 0xE1.
    microprograms[POP_H]    = [&](){
        l = memory[sp & 0xFFFFu]; sp++; emit registerLChanged();
        h = memory[sp & 0xFFFFu]; sp++; emit registerHChanged(); emit MChanged();
        sp &= 0xFFFFu; emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //JPO address (jump on parity odd); hex machine code 0xE2.
    microprograms[JPO]      = [&](){
        if(!CHECK_FLAG(f, PARITY_FLAG))
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //Exchange stack top with HL (XTHL); hex machine code 0xE3.
    microprograms[XTHL]     = [&](){
        data8_t temp;
        SWAP(l, memory[sp & 0xFFFFu], temp); l &= 0xFFu; memory[sp & 0xFFFFu] &= 0xFFu;
        emit registerLChanged(); emit memoryBlockUpdated(sp & 0xFFFFu, 1u);
        sp++;
        SWAP(h, memory[sp & 0xFFFFu], temp); h &= 0xFFu; memory[sp & 0xFFFFu] &= 0xFFu;
        emit registerHChanged(); emit memoryBlockUpdated(sp & 0xFFFFu, 1u);
        sp--;
        emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CPO address (Call on parity odd); hex machine code 0xE4.
    microprograms[CPO]      = [&](){
        if(!CHECK_FLAG(f, PARITY_FLAG)) {
            pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        } else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //PUSH H (push HL on stack); hex machine code 0xE5.
    microprograms[PUSH_H]   = [&](){
        sp--; sp &= 0xFFFFu; memory[sp] = h & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = l & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ANI byte (AND immediate with accumulator); hex machine code 0xE6.
    microprograms[ANI]      = [&](){
        a = a & memory[(pc + 1) & 0xFFFFu] & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        SET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 1
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RST 4 (restart 4); hex machine code 0xE7.
    microprograms[RST_4]    = [&](){
        pc++; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); //pc--; pc &= 0xFFFFu; //Go to previous correct location (not required)
        pc = 32u; emit programCounterChanged();
    };
    //RPE (return on parity even); hex machine code 0xE8.
    microprograms[RPE]       = [&](){
        if(CHECK_FLAG(f, PARITY_FLAG)) {
            pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
            sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        } else pc++;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //PCHL (Jump HL indirect; or move HL to PC); hex machine code 0xE9.
    microprograms[PCHL]    = [&](){pc = PACK(h, l); pc &= 0xFFFFu; emit programCounterChanged();};
    //JPE address (jump on parity odd); hex machine code 0xEA.
    microprograms[JPE]      = [&](){
        if(CHECK_FLAG(f, PARITY_FLAG))
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //XCHG (exchange HL with DE); hex machine code 0xEB.
    microprograms[XCHG]     = [&](){
        data8_t temp;
        SWAP(l, e, temp); l &= 0xFFu; e &= 0xFFu;
        emit registerLChanged(); emit registerEChanged();
        SWAP(h, d, temp); h &= 0xFFu; d &= 0xFFu;
        emit registerHChanged(); emit registerDChanged(); emit MChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CPE address (Call on parity even); hex machine code 0xEC.
    microprograms[CPE]      = [&](){
        if(CHECK_FLAG(f, PARITY_FLAG)) {
            pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        } else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0xEDu]    = UNUSED;
    //XRI byte (AND immediate with accumulator); hex machine code 0xEE.
    microprograms[XRI]      = [&](){
        a = (a ^ memory[(pc + 1) & 0xFFFFu]) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RST 5 (restart 5); hex machine code 0xEF.
    microprograms[RST_5]    = [&](){
        pc++; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); //pc--; pc &= 0xFFFFu; //Go to previous correct location (not required)
        pc = 40u; emit programCounterChanged();
    };
    //RP (return on Plus); hex machine code 0xF0.
    microprograms[RP]        = [&](){
        if(!CHECK_FLAG(f, SIGN_FLAG)) {
            pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
            sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        } else pc++;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //POP PSW (pop processor status word from stack); hex machine code 0xF1.
    microprograms[POP_PSW]    = [&](){
        f = memory[sp & 0xFFFFu]; sp++; emit flagsChanged();
        a = memory[sp & 0xFFFFu]; sp++; emit accumulatorChanged();
        sp &= 0xFFFFu; emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //JP address (jump on Plus); hex machine code 0xF2.
    microprograms[JP]       = [&](){
        if(!CHECK_FLAG(f, SIGN_FLAG))
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //Disable interrupts (DI); hex machine code 0xF3.
    microprograms[DI]       = [&](){
        ie = 0u; emit interruptEnableStatusChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CP address (call on Plus); hex machine code 0xF4.
    microprograms[CP]       = [&](){
        if(!CHECK_FLAG(f, SIGN_FLAG)) {
            pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        } else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //PUSH PSW (push processor status word on stack); hex machine code 0xF5.
    microprograms[PUSH_PSW]   = [&](){
        sp--; sp &= 0xFFFFu; memory[sp] = a & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = f & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //ORI byte (OR immediate with accumulator); hex machine code 0xF6.
    microprograms[ORI]      = [&](){
        a = (a | memory[(pc + 1) & 0xFFFFu]) & 0xFFu; emit accumulatorChanged();
        UNSET_FLAG(f, CARRY_FLAG); //always 0
        UNSET_FLAG(f, AUXILIARY_CARRY_FLAG); //always 0
        SET_SPEC_FLAG(f, ZERO_FLAG, a == 0);
        SET_SPEC_FLAG(f, SIGN_FLAG, (a & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[a]); emit flagsChanged();
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RST 6 (restart 6); hex machine code 0xF7.
    microprograms[RST_6]    = [&](){
        pc++; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); //pc--; pc &= 0xFFFFu; //Go to previous correct location (not required)
        pc = 48u; emit programCounterChanged();
    };
    //RM (return on Minus); hex machine code 0xF8.
    microprograms[RM]        = [&](){
        if(CHECK_FLAG(f, SIGN_FLAG)) {
            pc = PACK(memory[(sp + 1u) & 0xFFFFu] & 0xFFu, memory[sp & 0xFFFFu] & 0xFFu);
            sp+=2; sp &= 0xFFFFu; emit stackPointerChanged();
        } else pc++;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //SPHL (move HL to SP); hex machine code 0xF9.
    microprograms[SPHL]    = [&](){
        sp = PACK(h, l); sp &= 0xFFFFu; emit stackPointerChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //JM address (jump on Minus); hex machine code 0xFA.
    microprograms[JM]      = [&](){
        if(CHECK_FLAG(f, SIGN_FLAG))
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    //EI (enable interrupts); hex machine code 0xFB.
    microprograms[EI]       = [&](){
        ie = 1u; emit interruptEnableStatusChanged();
        pc++; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //CM address (Call on Minus); hex machine code 0xFC.
    microprograms[CM]      = [&](){
        if(CHECK_FLAG(f, SIGN_FLAG)) {
            pc += 3; pc &= 0xFFFFu; //Go to immediate next instruction
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged(); pc -= 3; pc &= 0xFFFFu; //Go to previous correct location
            pc = PACK(memory[(pc + 2u) & 0xFFFFu] & 0xFFu, memory[(pc + 1u) & 0xFFFFu] & 0xFFu);
        } else pc += 3;
        pc &= 0xFFFFu; emit programCounterChanged();
    };
    microprograms[0xFDu]    = UNUSED;
    //CPI byte (compare immediate against accumulator); hex machine code 0xFE.
    microprograms[CPI]      = [&](){
        data8_t rhs = memory[(pc + 1) & 0xFFFFu] & 0xFFu;
        data8_calc_t temp = a + NEGATE8(rhs);
        SET_SPEC_FLAG(f, CARRY_FLAG, a < rhs); temp &= 0xFFu;
        SET_SPEC_FLAG(f, AUXILIARY_CARRY_FLAG, ((a & 0xFu) < (rhs & 0xFu)));
        SET_SPEC_FLAG(f, ZERO_FLAG, a == rhs);
        SET_SPEC_FLAG(f, SIGN_FLAG, (temp & 0x80u) == 0x80u);
        SET_SPEC_FLAG(f, PARITY_FLAG, PARITY_LOOKUP[temp]);
        emit flagsChanged();
        pc+=2; pc &= 0xFFFFu; emit programCounterChanged();
    };
    //RST 7 (restart 7); hex machine code 0xFF.
    microprograms[RST_7]    = [&](){
        pc++; pc &= 0xFFFFu; //Go to immediate next instruction
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged(); //pc--; pc &= 0xFFFFu; //Go to previous correct location (not required)
        pc = 56u; emit programCounterChanged();
    };
}
Processor::~Processor() {
    delete[] microprograms;
    delete[] memory;
    delete[] io;
}
void Processor::copyTo(data8_t *const dest, memaddr_t startLoc, memsize_t length) const {
    memaddr_t srcAddr; memsize_t destLoc;
    for(srcAddr = startLoc & 0xFFFFu, destLoc = 0; destLoc < length; srcAddr++, srcAddr &= 0xFFFFu, destLoc++)
        dest[destLoc] = memory[srcAddr];
}
//public slots
void Processor::overwrite(const data8_t *const src, memaddr_t startLoc, memsize_t length) {
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
void Processor::runFull() {
    halt = unused = 0u;
    while(!halt && !unused && stepNextInstruction());
    halt = unused = 0u;
}
#include <QCoreApplication>
bool Processor::stepNextInstruction() {
    microprograms[memory[pc & 0xFFFFu] & 0xFFu]();
    //Check HALT
    if(halt) {
        emit halted(); emit stepped(); return false;}
    //Check TRAP
    if(trap_lowToHigh) {
        trap_lowToHigh = 0; //Disable next TRAP. Now we need a function call to 0024H.
        sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
        emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
        emit stackPointerChanged();
        pc = 0x0024u; emit programCounterChanged();
    }
    else if(ie) {//Only do the next checks if interrupts are enabled
        ie = 0u; //interrupts are disabled on recognising one
        if(!m7_5 && rst7_5) {
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged();
            pc = 0x003Cu; emit programCounterChanged();
        }
        else if (!m6_5 && rst6_5) {
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged();
            pc = 0x0034u; emit programCounterChanged();
        }
        else if (!m5_5 && rst5_5) {
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged();
            pc = 0x002Cu; emit programCounterChanged();
        }
        else if (intr) {
            sp--; sp &= 0xFFFFu; memory[sp] = (pc >> 8) & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            sp--; sp &= 0xFFFFu; memory[sp] = pc & 0xFFu;
            emit memoryBlockUpdated(sp, 1u); if(sp == PACK(h, l)) emit MChanged();
            emit stackPointerChanged();
            pc = (intrVec << 3) & 0xFFFFu; emit programCounterChanged();
        }
    }
    emit stepped();
    QCoreApplication::processEvents(); //push GUI events
    return true;
}
void Processor::setMemoryByte(memaddr_t address, data8_t data) {
    address &= 0xFFFFu; data &= 0xFFu;
    memory[address] = data;
    emit memoryBlockUpdated(address, 1u);
    if(address == PACK(h, l)) emit MChanged();
}
void Processor::setInputByte(ioaddr_t address, data8_t data) {
    address &= 0xFFu; data &= 0xFFu;
    io[address] = data; emit ioPortUpdated(address);
}
void Processor::setSerialInputLatch(bool flag) {sid = flag ? 1u : 0u;}
void Processor::setRestart7_5Request(bool flag) {rst7_5 = flag ? 1u : 0u; emit restart7_5RequestStatusChanged();}
void Processor::setRestart6_5Request(bool flag) {rst6_5 = flag ? 1u : 0u;}
void Processor::setRestart5_5Request(bool flag) {rst5_5 = flag ? 1u : 0u;}
void Processor::setTRAPRequest(bool flag) {
    if(!trap && flag) {trap_lowToHigh = trap = 1u;}
    else if (!flag) trap = 0u;
}
void Processor::setInterruptRequest(bool flag) {intr = flag ? 1u : 0u;}
void Processor::setINTRVector(data8_t value) {intrVec = (value >> 3) & 7u;}
void Processor::setProgramCounter(memaddr_t value) {pc = value & 0xFFFF; emit programCounterChanged();}
void Processor::resetMemory() {
    std::memset((void *)memory, 0, sizeof(data8_t) * MEMORY_SIZE);
    emit memoryBlockUpdated(0u, MEMORY_SIZE);
    emit MChanged();
}
void Processor::resetIOPorts() {
    std::memset((void *)io, 0, sizeof(data8_t) * IO_PORT_SIZE);
    emit ioPortsReset();
}
void Processor::RESET_IN() {
    haltExecution(); QCoreApplication::processEvents();//halt if running
    pc = sp = 0u;
    a = b = c = d = e = h = l = 0u; f = 0u; ie = sod = inta = rst7_5 = halt = 0u;
    m5_5 = m6_5 = m7_5 = 1u; //Initial state is these external interrupts are masked.
    emit accumulatorChanged(); emit registerBChanged(); emit registerCChanged(); emit registerDChanged();
    emit registerEChanged(); emit registerHChanged(); emit registerLChanged(); emit flagsChanged();
    emit MChanged(); emit interruptEnableStatusChanged(); emit maskRestart5_5Changed();
    emit maskRestart6_5Changed(); emit maskRestart7_5Changed(); emit serialOutput();
    emit programCounterChanged(); emit stackPointerChanged(); emit interruptEnableStatusChanged();
    emit restart7_5RequestStatusChanged(); emit interruptAcknowledgeStatusChanged();
}
void Processor::resetAll() {
    RESET_IN(); resetMemory(); resetIOPorts();
}
void Processor::haltExecution() {if(!halt) {halt = 1u; emit halted();}}
