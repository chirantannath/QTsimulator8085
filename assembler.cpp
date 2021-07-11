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
#include "assembler.h"

//SyntaxError

inline SyntaxError::SyntaxError(const Tokenizer &tok, const char *const what)
    : lineNumber(tok.lineNumber), columnNumber(tok.columnNumber), position(tok.charNumber), what(what) {}

//Tokenizer

Tokenizer::TokenType Tokenizer::getNextToken() {
    char ch;
    token.clear();
    ttype = NONE;
    //check for EOF
    if(isEndOfFile()) return ttype = END_OF_FILE;
    //check for errors
    if(!isGood()) return ttype = STREAM_ERROR;
    //check for non-newline whitespace
    if(std::isspace(ch = peekNextChar()) && ch != '\n') {
        while(isGood() && std::isspace(ch = getNextChar()) && ch != '\n') token.append(1, ch);
        if(isGood()) putBackChar(ch); //put back the character which did not follow rule
        columnNumber += token.length();
        return ttype = WHITESPACE;
    }
    //check for newline
    if(peekNextChar() == '\n') {
        token.append(1, '\n');
        getNextChar(); //advance
        lineNumber++; columnNumber=1;
        return ttype = NEWLINE;
    }
    //check for separator
    if(isSeparator(ch = peekNextChar())) {
        token.append(1, ch);
        getNextChar(); //advance
        columnNumber++;
        return ttype = SEPARATOR;
    }
    //alphanumeric characters checked here
    if(std::isalnum(ch = peekNextChar())) {
        while(isGood() && std::isalnum(ch = getNextChar())) token.append(1, ch);
        if(isGood()) putBackChar(ch); //put back the character which did not follow rule
        columnNumber+=token.length();
        //now to see whether OPCODE, IDENTIFIER, NUMBER, or HEXNUMBER
        if(isOpcode(token.c_str())) return ttype = OPCODE;
        else if(isPseudocode(token.c_str())) return ttype = PSEUDOCODE;
        else if(isNumber(token.c_str())) return ttype = NUMBER;
        else if(isHexNumber(token.c_str())) return ttype = HEXNUMBER;
        else return ttype = IDENTIFIER; //unrecognized alphanumeric sequences may be identifiers
    }
    //Comments
    if((ch = peekNextChar()) == ';') {
        while(isGood() && (ch = getNextChar()) != '\n') token.append(1, ch);
        if(isGood()) putBackChar(ch); //put back the character which did not follow rule
        columnNumber+=token.length();
        return ttype = COMMENT;
    }
    //if we reach here, the underlying stream might have had an error irrespective of putback buffer
    if(in.eof()) return ttype = END_OF_FILE;
    if(!in.good()) return ttype = STREAM_ERROR;
    return ttype = CHAR_ERROR; //unrecognized character
}
char Tokenizer::getNextChar() {
    charNumber++;
    if(putbackBuffer.empty()) return in.get();
    else {
        char temp = putbackBuffer.top();
        putbackBuffer.pop();
        return temp;
    }
}
char Tokenizer::peekNextChar() {
    if(putbackBuffer.empty()) {
        char temp = in.get();
        putbackBuffer.push(temp);
        return temp;
    }
    else return putbackBuffer.top();
}

//LineTranslator

LineTranslator::LineTranslator(Tokenizer &tok)  : tok(tok) {
    /*Mnemonic processors should catch most syntax and semantic errors.
    Mnemonic processors accept an Instruction reference to suitably modify; and the mnemonic for which they were called.
    In some cases; they will modify the i.code field to some valid object (not nullptr) if the processor has enough
    information to resolve the instruction. In any case, instruction bytecode resolution is finished in this stage itself.*/
    const std::function<void(Instruction &, const char *)> noOperandProc = [&](Instruction &i, const char *name){i.code = getOpcode(name);}; //Call when the instruction doesn't require any operands
    mnemonicProcessors.insert({"NOP", noOperandProc});
    mnemonicProcessors.insert({"HLT", noOperandProc});
    mnemonicProcessors.insert({"RLC", noOperandProc});
    mnemonicProcessors.insert({"RRC", noOperandProc});
    mnemonicProcessors.insert({"RAL", noOperandProc});
    mnemonicProcessors.insert({"RAR", noOperandProc});
    mnemonicProcessors.insert({"RIM", noOperandProc});
    mnemonicProcessors.insert({"SIM", noOperandProc});
    mnemonicProcessors.insert({"DAA", noOperandProc});
    mnemonicProcessors.insert({"CMA", noOperandProc});
    mnemonicProcessors.insert({"STC", noOperandProc});
    mnemonicProcessors.insert({"CMC", noOperandProc});
    mnemonicProcessors.insert({"RET", noOperandProc});
    mnemonicProcessors.insert({"RNZ", noOperandProc});
    mnemonicProcessors.insert({"RZ", noOperandProc});
    mnemonicProcessors.insert({"RNC", noOperandProc});
    mnemonicProcessors.insert({"RC", noOperandProc});
    mnemonicProcessors.insert({"RPO", noOperandProc});
    mnemonicProcessors.insert({"RPE", noOperandProc});
    mnemonicProcessors.insert({"RP", noOperandProc});
    mnemonicProcessors.insert({"RM", noOperandProc});
    mnemonicProcessors.insert({"PCHL", noOperandProc});
    mnemonicProcessors.insert({"SPHL", noOperandProc});
    mnemonicProcessors.insert({"XTHL", noOperandProc});
    mnemonicProcessors.insert({"XCHG", noOperandProc});
    mnemonicProcessors.insert({"DI", noOperandProc});
    mnemonicProcessors.insert({"EI", noOperandProc});

    const std::function<void(Instruction &, const char *)> byteProc = [&](Instruction &i, const char *name) { //Call when the instruction requires a byte number as the operand (say 243 or 23H, etc.)
        i.code = getOpcode(name); //Wherever this occurs this has to be the first call. The name pointer points to a resource that may change unintentionally. (Rust vibes anyone?)
        ignoreWhitespaces();
        if(tok.ttype == Tokenizer::NUMBER) {
            unsigned x; std::sscanf(tok.token.c_str(), "%u", &x);
            if(x > 255u) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to 255 or FFH");
            i.operand = x & 0xFFu;
        }
        else if(tok.ttype == Tokenizer::HEXNUMBER) {
            unsigned x; std::sscanf(tok.token.c_str(), "%x%*[hH]", &x);
            if(x > 0xFFu) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFH or 255");
            i.operand = x & 0xFFu;
        }
        else throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFH or 255");
    };
    mnemonicProcessors.insert({"ADI", byteProc});
    mnemonicProcessors.insert({"ACI", byteProc});
    mnemonicProcessors.insert({"SUI", byteProc});
    mnemonicProcessors.insert({"SBI", byteProc});
    mnemonicProcessors.insert({"ANI", byteProc});
    mnemonicProcessors.insert({"XRI", byteProc});
    mnemonicProcessors.insert({"ORI", byteProc});
    mnemonicProcessors.insert({"CPI", byteProc});
    mnemonicProcessors.insert({"OUT", byteProc});
    mnemonicProcessors.insert({"IN", byteProc});

    const std::function<void(Instruction &, const char *)> wordProc = [&](Instruction &i, const char *name) { //Call when the instruction requires a 2-byte number as the operand (say 16384 or 2312H, etc.)
        i.code = getOpcode(name);
        ignoreWhitespaces();
        if(tok.ttype == Tokenizer::NUMBER) {
            unsigned long long x; std::sscanf(tok.token.c_str(), "%llu", &x);
            if(x > 65535u) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to 65535 or FFFFH");
            i.operand = (data16_t)(x & 0xFFFFu);
        }
        else if(tok.ttype == Tokenizer::HEXNUMBER) {
            unsigned long long x; std::sscanf(tok.token.c_str(), "%llx%*[hH]", &x);
            if(x > 0xFFFFu) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFFFH or 65535");
            i.operand = (data16_t)(x & 0xFFFFu);
        }
        else throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFFFH or 65535");
    };
    mnemonicProcessors.insert({"SHLD", wordProc});
    mnemonicProcessors.insert({"LHLD", wordProc});
    mnemonicProcessors.insert({"STA", wordProc});
    mnemonicProcessors.insert({"LDA", wordProc});

    const std::function<void(Instruction &, const char *)> labelProc = [&](Instruction &i, const char *name) { //Call when the instruction takes a label as argument (jumps and calls)
        i.code = getOpcode(name);
        ignoreWhitespaces();
        if(tok.ttype != Tokenizer::IDENTIFIER)
            throw SyntaxError(tok, "Expected an identifier(target label)");
        i.toLabel = tok.token;
    };
    mnemonicProcessors.insert({"JMP", labelProc});
    mnemonicProcessors.insert({"JNZ", labelProc});
    mnemonicProcessors.insert({"JZ", labelProc});
    mnemonicProcessors.insert({"JNC", labelProc});
    mnemonicProcessors.insert({"JC", labelProc});
    mnemonicProcessors.insert({"JPO", labelProc});
    mnemonicProcessors.insert({"JPE", labelProc});
    mnemonicProcessors.insert({"JP", labelProc});
    mnemonicProcessors.insert({"JM", labelProc});
    mnemonicProcessors.insert({"CALL", labelProc});
    mnemonicProcessors.insert({"CNZ", labelProc});
    mnemonicProcessors.insert({"CZ", labelProc});
    mnemonicProcessors.insert({"CNC", labelProc});
    mnemonicProcessors.insert({"CC", labelProc});
    mnemonicProcessors.insert({"CPO", labelProc});
    mnemonicProcessors.insert({"CPE", labelProc});
    mnemonicProcessors.insert({"CP", labelProc});
    mnemonicProcessors.insert({"CM", labelProc});

    mnemonicProcessors.insert({"ADD", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0x80u | i.operand]; i.operand = 0;}});
    mnemonicProcessors.insert({"ADC", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0x88u | i.operand]; i.operand = 0;}});
    mnemonicProcessors.insert({"SUB", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0x90u | i.operand]; i.operand = 0;}});
    mnemonicProcessors.insert({"SBB", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0x98u | i.operand]; i.operand = 0;}});
    mnemonicProcessors.insert({"ANA", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0xA0u | i.operand]; i.operand = 0;}});
    mnemonicProcessors.insert({"XRA", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0xA8u | i.operand]; i.operand = 0;}});
    mnemonicProcessors.insert({"ORA", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0xB0u | i.operand]; i.operand = 0;}});
    mnemonicProcessors.insert({"CMP", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0xB8u | i.operand]; i.operand = 0;}});
    mnemonicProcessors.insert({"INR", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0x04u | (i.operand << 3)]; i.operand = 0;}});
    mnemonicProcessors.insert({"DCR", [&](Instruction &i, const char *name) {singleRegisterProc(i, name); i.code = opcodesByCode[0x05u | (i.operand << 3)]; i.operand = 0;}});

    mnemonicProcessors.insert({"MOV", [&](Instruction &i, const char *) { //Call this for the MOV instructions which are the only ones which take two 1-byte register names as an argument.
        ignoreWhitespaces(); data8_t code = 0x40u;
        if(!(tok.ttype == Tokenizer::IDENTIFIER && tok.token.length() == 1))
            throw SyntaxError(tok, "Expected a byte register name (B,C,D,E,H,L,M,A)");
        switch(tok.token[0]) {
        case 'b': case 'B': code |= (0 << 3); break; //temporary value
        case 'c': case 'C': code |= (1 << 3); break; //temporary value
        case 'd': case 'D': code |= (2 << 3); break; //temporary value
        case 'e': case 'E': code |= (3 << 3); break; //temporary value
        case 'h': case 'H': code |= (4 << 3); break; //temporary value
        case 'l': case 'L': code |= (5 << 3); break; //temporary value
        case 'm': case 'M': code |= (6 << 3); break; //temporary value
        case 'a': case 'A': code |= (7 << 3); break; //temporary value
        default: throw SyntaxError(tok, "Expected a byte register name (B,C,D,E,H,L,M,A)");
        }
        ignoreWhitespaces();
        if(tok.ttype == Tokenizer::SEPARATOR && tok.token[0] == ',') ignoreWhitespaces(); //ignore separator if present
        if(!(tok.ttype == Tokenizer::IDENTIFIER && tok.token.length() == 1))
            throw SyntaxError(tok, "Expected a byte register name (B,C,D,E,H,L,M,A)");
        switch(tok.token[0]) {
        case 'b': case 'B': code |= 0; break; //temporary value
        case 'c': case 'C': code |= 1; break; //temporary value
        case 'd': case 'D': code |= 2; break; //temporary value
        case 'e': case 'E': code |= 3; break; //temporary value
        case 'h': case 'H': code |= 4; break; //temporary value
        case 'l': case 'L': code |= 5; break; //temporary value
        case 'm': case 'M': code |= 6; break; //temporary value
        case 'a': case 'A': code |= 7; break; //temporary value
        default: throw SyntaxError(tok, "Expected a byte register name (B,C,D,E,H,L,M,A)");
        }
        if(code == HLT) throw SyntaxError(tok, "Moving from M to M is not supported");
        i.code = opcodesByCode[code];
    }});

    mnemonicProcessors.insert({"LDAX", [&](Instruction &i, const char *name) {STAX_LDAX_Proc(i, name); i.code = opcodesByCode[0x0Au | (i.operand << 4)]; i.operand = 0;}});
    mnemonicProcessors.insert({"STAX", [&](Instruction &i, const char *name) {STAX_LDAX_Proc(i, name); i.code = opcodesByCode[0x02u | (i.operand << 4)]; i.operand = 0;}});

    mnemonicProcessors.insert({"INX", [&](Instruction &i, const char *name) {registerPairProc(i, name); i.code = opcodesByCode[0x03u | (i.operand << 4)]; i.operand = 0;}});
    mnemonicProcessors.insert({"DCX", [&](Instruction &i, const char *name) {registerPairProc(i, name); i.code = opcodesByCode[0x0Bu | (i.operand << 4)]; i.operand = 0;}});
    mnemonicProcessors.insert({"DAD", [&](Instruction &i, const char *name) {
                                   //std::printf("Here in DAD\n"); std::fflush(stdout);
                                   registerPairProc(i, name);
                                   i.code = opcodesByCode[0x09u | (i.operand << 4)];
                                   i.operand = 0;}});

    mnemonicProcessors.insert({"LXI", [&](Instruction &i, const char *) { //Call for LXI that takes a single register pair (BC, DE, HL or SP) and a word integer as arguments.
        static CaseInsensitive comparator;
        ignoreWhitespaces(); data8_t code = 0x01u;
        if(tok.ttype != Tokenizer::IDENTIFIER) throw SyntaxError(tok, "Expected either B, D, H or SP");
        if(!comparator(tok.token.c_str(), "SP") && !comparator("SP", tok.token.c_str())) code |= (3 << 4);
        else if(tok.token.length() != 1) throw SyntaxError(tok, "Expected either B, D, H or SP");
        else switch(tok.token[0]) {
        case 'b': case 'B': code |= (0 << 4); break;
        case 'd': case 'D': code |= (1 << 4); break;
        case 'h': case 'H': code |= (2 << 4); break;
        }
        i.code = opcodesByCode[code];
        ignoreWhitespaces();
        if(tok.ttype == Tokenizer::SEPARATOR && tok.token[0] == ',') ignoreWhitespaces(); //ignore separator if present
        if(tok.ttype == Tokenizer::NUMBER) {
            unsigned long long x; std::sscanf(tok.token.c_str(), "%llu", &x);
            if(x > 65535u) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to 65535 or FFFFH");
            i.operand = (data16_t)(x & 0xFFFFu);
        }
        else if(tok.ttype == Tokenizer::HEXNUMBER) {
            unsigned long long x; std::sscanf(tok.token.c_str(), "%llx%*[hH]", &x);
            if(x > 0xFFFFu) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFFFH or 65535");
            i.operand = (data16_t)(x & 0xFFFFu);
        }
        else throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFFFH or 65535");
    }});

    mnemonicProcessors.insert({"MVI", [&](Instruction &i, const char *) { //Call this for the MVI instructions which takes a 1-byte register name and a 1-byte integer (eg. 134 or 22H, etc.) as arguments.
        ignoreWhitespaces(); data8_t code = 0x06u;
        if(!(tok.ttype == Tokenizer::IDENTIFIER && tok.token.length() == 1))
            throw SyntaxError(tok, "Expected a byte register name (B,C,D,E,H,L,M,A)");
        switch(tok.token[0]) {
        case 'b': case 'B': code |= (0 << 3); break; //temporary value
        case 'c': case 'C': code |= (1 << 3); break; //temporary value
        case 'd': case 'D': code |= (2 << 3); break; //temporary value
        case 'e': case 'E': code |= (3 << 3); break; //temporary value
        case 'h': case 'H': code |= (4 << 3); break; //temporary value
        case 'l': case 'L': code |= (5 << 3); break; //temporary value
        case 'm': case 'M': code |= (6 << 3); break; //temporary value
        case 'a': case 'A': code |= (7 << 3); break; //temporary value
        default: throw SyntaxError(tok, "Expected a byte register name (B,C,D,E,H,L,M,A)");
        }
        ignoreWhitespaces();
        if(tok.ttype == Tokenizer::SEPARATOR && tok.token[0] == ',') ignoreWhitespaces(); //ignore separator if present
        i.code = opcodesByCode[code];
        if(tok.ttype == Tokenizer::NUMBER) {
            unsigned x; std::sscanf(tok.token.c_str(), "%u", &x);
            if(x > 255u) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to 255 or FFH");
            i.operand = x & 0xFFu;
        }
        else if(tok.ttype == Tokenizer::HEXNUMBER) {
            unsigned x; std::sscanf(tok.token.c_str(), "%x%*[hH]", &x);
            if(x > 0xFFu) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFH or 255");
            i.operand = x & 0xFFu;
        }
        else throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFH or 255");
    }});

    mnemonicProcessors.insert({"PUSH", [&](Instruction &i, const char *name) {stackProc(i, name); i.code = opcodesByCode[0xC5u | (i.operand << 4)]; i.operand = 0;}});
    mnemonicProcessors.insert({"POP", [&](Instruction &i, const char *name) {stackProc(i, name); i.code = opcodesByCode[0xC1u | (i.operand << 4)]; i.operand = 0;}});

    mnemonicProcessors.insert({"RST", [&](Instruction &i, const char *) { //Call for RST instructions which take an integer from 0 to 7 inclusive.
        unsigned n;
        ignoreWhitespaces();
        if(tok.ttype != Tokenizer::NUMBER) throw SyntaxError(tok, "Expected an integer between 0 to 7 inclusive");
        std::sscanf(tok.token.c_str(), "%u", &n);
        if(n > 7) throw SyntaxError(tok, "Expected an integer between 0 and 7 inclusive");
        i.code = opcodesByCode[0xC7u | (n << 3)];
    }});

    pseudocodeProcessors.insert({"ORG", [&](Instruction &i, const char *) {//For #ORG pseudocode.
        i.code = &ORG;
        ignoreWhitespaces();
        if(tok.ttype == Tokenizer::NUMBER) {
            unsigned long long x; std::sscanf(tok.token.c_str(), "%llu", &x);
            if(x > 65535u) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to 65535 or FFFFH");
            i.operand = (data16_t)(x & 0xFFFFu);
        }
        else if(tok.ttype == Tokenizer::HEXNUMBER) {
            unsigned long long x; std::sscanf(tok.token.c_str(), "%llx%*[hH]", &x);
            if(x > 0xFFFFu) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFFFH or 65535");
            i.operand = (data16_t)(x & 0xFFFFu);
        }
        else throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFFFH or 65535");
        ignoreWhitespaces();
    }});

    pseudocodeProcessors.insert({"DATA", [&](Instruction &i, const char *) {
        i.code = &DATA;
        ignoreWhitespaces();
        //expect a 16-bit number to indicate target storing address
        if(tok.ttype == Tokenizer::NUMBER) {
            unsigned long long x; std::sscanf(tok.token.c_str(), "%llu", &x);
            if(x > 65535u) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to 65535 or FFFFH");
            i.operand = (data16_t)(x & 0xFFFFu);
        }
        else if(tok.ttype == Tokenizer::HEXNUMBER) {
            unsigned long long x; std::sscanf(tok.token.c_str(), "%llx%*[hH]", &x);
            if(x > 0xFFFFu) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFFFH or 65535");
            i.operand = (data16_t)(x & 0xFFFFu);
        }
        else throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFFFH or 65535");
        //Now we will parse a list of numbers
        while(true) {
            unsigned x;
            ignoreWhitespaces(); //advance
            switch(tok.ttype) {
            case Tokenizer::NUMBER:
                std::sscanf(tok.token.c_str(), "%u", &x);
                if(x > 255u) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to 255 or FFH");
                i.extraOperands.push_back(x & 0xFFu); break;
            case Tokenizer::HEXNUMBER:
                std::sscanf(tok.token.c_str(), "%x%*[hH]", &x);
                if(x > 0xFFu) throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFH or 255");
                i.extraOperands.push_back(x & 0xFFu); break;
            case Tokenizer::END_OF_FILE: case Tokenizer::NEWLINE: case Tokenizer::COMMENT: goto outOfLoop;
            case Tokenizer::SEPARATOR:
                if(tok.token[0] != ',') throw SyntaxError(tok, "Expected \',\'");
                else break;
            default: throw SyntaxError(tok, "Expected a nonnegative number less than or equal to FFH or 255");
            }
        }
    outOfLoop:
        i.extraOperands.shrink_to_fit();
    }});
}
Instruction LineTranslator::translateOneLine() {
    Instruction i; i.lineNumber = tok.lineNumber;
    std::function<void(Instruction &, const char *)> caller;
    std::string name; const char *cptr;
    ignoreWhitespaces();
    if(tok.ttype == Tokenizer::COMMENT || tok.ttype == Tokenizer::NEWLINE || tok.ttype == Tokenizer::END_OF_FILE)
        //Return empty expression identifier if this is a comment or an empty line. We jump to the end for this.
        goto translationEnd;
    if(tok.ttype == Tokenizer::SEPARATOR && tok.token[0] == '#') {
        //Pseudocode
        ignoreWhitespaces();
        //There should be pseudocode here
        if(tok.ttype != Tokenizer::PSEUDOCODE) throw SyntaxError(tok, "Expected pseudocode");
        name = std::string(tok.token); cptr = name.c_str();
        caller = pseudocodeProcessors.find(cptr)->second;
        caller(i, cptr);
        goto translationEnd;
    }
    if(tok.ttype == Tokenizer::IDENTIFIER) {
        //Identifiers at the start to indicate labels. Make sure it's a label
        i.label = std::string(tok.token); //force copy
        ignoreWhitespaces();
        //Now we should have : separator.
        if(!(tok.ttype == Tokenizer::SEPARATOR && tok.token[0] == ':'))
            throw SyntaxError(tok, "Expected \':\'");
        ignoreWhitespaces(); //go to next token, hopefully an opcode mnemonic.
    }
    //There should be an opcode mnemonic here
    if(tok.ttype == Tokenizer::OPCODE) {
        //We would need to translate the rest of the line according to the mnemonic given.
        name = std::string(tok.token); cptr = name.c_str();
        caller = mnemonicProcessors.find(cptr)->second; //This initializes i.code and i.operand.
        caller(i, cptr);
        ignoreWhitespaces(); //advance to end of line
    }
    else throw SyntaxError(tok, "Expected opcode mnemonic");
translationEnd:
    if(tok.ttype == Tokenizer::COMMENT || tok.ttype == Tokenizer::WHITESPACE) ignoreWhitespaces();
    if(tok.ttype != Tokenizer::END_OF_FILE && tok.ttype != Tokenizer::NEWLINE)
        throw SyntaxError(tok, "Expected end of line or end of file");
    return i;
}
void LineTranslator::singleRegisterProc(Instruction &i, const char *) {
    ignoreWhitespaces();
    if(!(tok.ttype == Tokenizer::IDENTIFIER && tok.token.length() == 1))
        throw SyntaxError(tok, "Expected a byte register name (B,C,D,E,H,L,M,A)");
    switch(tok.token[0]) {
    case 'b': case 'B': i.operand = 0; break; //temporary value
    case 'c': case 'C': i.operand = 1; break; //temporary value
    case 'd': case 'D': i.operand = 2; break; //temporary value
    case 'e': case 'E': i.operand = 3; break; //temporary value
    case 'h': case 'H': i.operand = 4; break; //temporary value
    case 'l': case 'L': i.operand = 5; break; //temporary value
    case 'm': case 'M': i.operand = 6; break; //temporary value
    case 'a': case 'A': i.operand = 7; break; //temporary value
    default: throw SyntaxError(tok, "Expected a byte register name (B,C,D,E,H,L,M,A)");
    }
}
void LineTranslator::STAX_LDAX_Proc(Instruction &i, const char *) {
    ignoreWhitespaces();
    if(!(tok.ttype == Tokenizer::IDENTIFIER && tok.token.length() == 1))
        throw SyntaxError(tok, "Expected either B or D");
    switch(tok.token[0]) {
    case 'b': case 'B': i.operand = 0; break; //temporary value
    case 'd': case 'D': i.operand = 1; break; //temporary value
    default: throw SyntaxError(tok, "Expected either B or D");
    }
}
void LineTranslator::registerPairProc(Instruction &i, const char *) {
    ignoreWhitespaces();
    if(tok.ttype != Tokenizer::IDENTIFIER) throw SyntaxError(tok, "Expected either B, D, H or SP");
    if(!CaseInsensitive::comparator(tok.token.c_str(), "SP") && !CaseInsensitive::comparator("SP", tok.token.c_str())) i.operand = 3;
    else if(tok.token.length() != 1) throw SyntaxError(tok, "Expected either B, D, H or SP");
    else switch(tok.token[0]) {
    case 'b': case 'B': i.operand = 0; break;
    case 'd': case 'D': i.operand = 1; break;
    case 'h': case 'H': i.operand = 2; break;
    }
}
void LineTranslator::stackProc(Instruction &i, const char *) {
    ignoreWhitespaces();
    if(tok.ttype != Tokenizer::IDENTIFIER) throw SyntaxError(tok, "Expected either B, D, H or PSW");
    if(!CaseInsensitive::comparator(tok.token.c_str(), "PSW") && !CaseInsensitive::comparator("PSW", tok.token.c_str())) i.operand = 3;
    else if(tok.token.length() != 1) throw SyntaxError(tok, "Expected either B, D, H or PSW");
    else switch(tok.token[0]) {
    case 'b': case 'B': i.operand = 0; break;
    case 'd': case 'D': i.operand = 1; break;
    case 'h': case 'H': i.operand = 2; break;
    }
}

//Assembler

Assembler::Assembler(Processor *proc) : QObject(proc), processor(proc), in(nullptr) {
    pseudocodeProcessors.insert({&ORG, [&](Instruction &i, memaddr_t &targetOffset) {targetOffset = i.operand & 0xFFFFu;}});
    pseudocodeProcessors.insert({&DATA, [&](Instruction &i, memaddr_t &) {processor->overwrite(i.extraOperands.data(), i.operand & 0xFFFFu, i.extraOperands.size());}});
}
#include <algorithm>
void Assembler::doAssembly() {
    Tokenizer tok(*in); LineTranslator translator(tok); memaddr_t targetOffset = 0;
    std::map<std::string, size_t, StringInsensitive> labelTable; //table for labels to instructions (index references stored)
    instructions.clear();
    //Put addresses to instructions
    do {
        Instruction i = translator.translateOneLine();
        if(i.code == nullptr) continue;
        else if(i.code->isPseudocode) {
            pseudocodeProcessors.find(i.code)->second(i, targetOffset); continue;}
        i.address = targetOffset;
        instructions.push_back(i);
        targetOffset = (targetOffset + i.code->bytesRequired) & 0xFFFFu;
        if(!i.label.empty()) labelTable.insert({i.label, instructions.size()-1u});
    } while (tok.ttype != Tokenizer::END_OF_FILE);
    //Put target operand addresses for JMPs and CALLs.
    for(size_t i = 0; i < instructions.size(); i++)
        if(!instructions[i].toLabel.empty()) {
            std::map<std::string, size_t, StringInsensitive>::iterator node = labelTable.find(instructions[i].toLabel);
            if(node == labelTable.end()) throw SyntaxError(instructions[i].lineNumber, 0, 0, "Target label not found");
            instructions[i].operand = (data16_t) (instructions[node->second].address & 0xFFFFu);
        }

    std::stable_sort(instructions.begin(), instructions.end(), InstructionAddressComparator()); //sort according to instruction addresses.

    //Put instructions into processor memory.
    for(size_t i = 0; i < instructions.size(); i++) {
        data8_t buf[3]; buf[0] = instructions[i].code->code;
        if(instructions[i].code->bytesRequired >= 2) buf[1] = (data8_t)(instructions[i].operand & 0xFFu);
        if(instructions[i].code->bytesRequired == 3) buf[2] = (data8_t)((instructions[i].operand >> 8) & 0xFFu);
        processor->overwrite(buf, instructions[i].address, instructions[i].code->bytesRequired);
    }
}
//public slots
void Assembler::assemble() {
    try {doAssembly();} catch (SyntaxError const &ex) {
        emit assemblyError(ex); return;
    }
    emit assemblyFinished();
}
