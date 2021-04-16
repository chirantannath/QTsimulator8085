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
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QMetaType>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <cctype>
#include <iostream>
#include <sstream>
#include <QException>
#include "commdefs.h"
#include "opcodes.h"

///Represents a single instruction line.
struct Instruction {
    ///This is the address in memory assigned to this instruction line. This can be relative or exact.
    memaddr_t address;
    ///The label for this particular line. Example: "LOOP: CALL SUB" would assign "LOOP" here.
    std::string label;
    ///The opcode for this instruction (example: CALL). Only one of the valid instances.
    opcode * const code;
    ///The label to which this instruction will point to (example: "SUB"). This is an empty string if it is not one of the jump
    ///or call instructions and can then be ignored. 
    std::string toLabel;
    ///Exact value of the operand. This can be ignored if code->bytesRequired is 1; only the least-significant 8 bits are valid if
    ///code->bytesRequired is 2, and the full value is valid if code->bytesRequired is 3.
    data16_t operand;
    
    ///Default constructor
    Instruction() : address(0u), label(""), code(nullptr), toLabel(""), operand(0u) {}
    ///Default copy constructor (required because we are using std::string)
    Instruction(const Instruction &o) : address(o.address), label(o.label), code(o.code), toLabel(o.toLabel), operand(o.operand)
    {}
    ///Default virtual destructor (REQUIRED for interop with Qt).
    virtual ~Instruction() = default;
};
Q_DECLARE_METATYPE(Instruction);

///Indicates a syntax error in the assembler source. We inherit QException because it allows interop with Qt; as well as permits
///exceptions to propagate accross threads.
struct SyntaxError : public QException {
    ///Line number at which the error occured.
    const unsigned lineNumber;
    ///Column number at which the error occured.
    const unsigned columnNumber;
    ///Error message (always an in-program hardcoded string)
    const char * const what;
    ///Default constructor
    SyntaxError() : lineNumber(), columnNumber(), what("") {};
    ///Constructor with correct parameters
    SyntaxError(unsigned lineNumber, unsigned columnNumber, const char *const what) : 
        lineNumber(lineNumber), columnNumber(columnNumber), what(what) {}
    ///Default copy constructor
    SyntaxError(const SyntaxError &o) = default;
    ///Virtual destructor required for interop with Qt.
    virtual ~SyntaxError() = default;
    
    /*The following functions are overriden from QException. To understand what they do (or rather, why they are required),
    consider the documentation for QException in The Qt Documentation.*/
    void raise() const { throw *this; }
    SyntaxError *clone() const {return new SyntaxError(*this);}
};
Q_DECLARE_METATYPE(SyntaxError)

///Splits an assembler source stream into individual tokens.
struct Tokenizer {
    ///Last token read and parsed
    std::string token;
    ///Our source to read characters from
    std::istream &in;
    ///Token types which can be parsed by this tokenizer
    enum TokenType {
        
    } ttype;
    
    ///Constructor
    Tokenizer(std::istream &in) : in(in) {}
};
//Q_DECLARE_METATYPE(Tokenizer) //This is an internal "worker" class, hence not a metatype.

#define ASSEMBLER_H_registerHeaderMetaTypes() {\
    qRegisterMetaType<Instruction>("Instruction");\
    qRegisterMetaType<SyntaxError>("SyntaxError");\
    /*qRegisterMetaType<Tokenizer>("Tokenizer");*/\
}

#endif // ASSEMBLER_H
    
