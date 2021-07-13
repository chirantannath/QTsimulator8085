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
#include <cctype>
#include <cstdio>
#include <iostream>
#include <stack>
#include <map>
#include <vector>
#include <functional>
#include <QException>
#include "commdefs.h"
#include "opcodes.h"

///Represents a single instruction line. This object is used to hold transitional information and is NOT used to execute the program.
///However, this object will be used to retain debugging information.
struct Instruction {
    ///Line number for this instruction.
    unsigned lineNumber;
    ///This is the address in memory assigned to this instruction line. This can be relative or exact.
    memaddr_t address;
    ///The label for this particular line. Example: "LOOP: CALL SUB" would assign "LOOP" here.
    std::string label;
    ///The opcode for this instruction (example: CALL). Only one of the valid instances.
    const opcode *code;
    ///The label to which this instruction will point to (example: "SUB"). This is an empty string if it is not one of the jump
    ///or call instructions and can then be ignored.
    std::string toLabel;
    ///Exact value of the operand. This can be ignored if code->bytesRequired is 1; only the least-significant 8 bits are valid if
    ///code->bytesRequired is 2, and the full value is valid if code->bytesRequired is 3. If code->bytesRequired is 3 and the
    ///operand is an address; it can be relative or exact. Also note that in some cases (even though code->bytesRequired is 1) the
    ///operand value may store a valid code or codes parameters (Think MOV or ADD which take 1 byte but still require an operand).
    ///In any case this extra exceptional information will be resolved and removed from this field by the assembler when this
    ///object is processed.
    data16_t operand;
    ///Extra operands if required (support for pseudocode).
    std::vector<data8_t> extraOperands;

    ///Default constructor
    Instruction() : lineNumber(0u), address(0u), label(""), code(nullptr), toLabel(""), operand(0u), extraOperands((size_t)0u) {}
    ///Default copy constructor (required because we are using std::string)
    Instruction(const Instruction &o) : lineNumber(o.lineNumber), address(o.address), label(o.label), code(o.code), toLabel(o.toLabel), operand(o.operand), extraOperands(o.extraOperands)
    {extraOperands.shrink_to_fit();}
    ///Default virtual destructor (REQUIRED for interop with Qt).
    virtual ~Instruction() = default;

    ///Is instruction line empty? used for comments.
    bool isEmpty() const {return code == nullptr;}
};
Q_DECLARE_METATYPE(Instruction);

struct InstructionAddressComparator {bool operator()(const Instruction &a, const Instruction &b) {return a.address < b.address;}};

struct Tokenizer;
///Indicates a syntax error in the assembler source. We inherit QException because it allows interop with Qt; as well as permits
///exceptions to propagate accross threads.
struct SyntaxError : public QException {
    ///Line number at which the error occured.
    const unsigned lineNumber;
    ///Column number at which the error occured.
    const unsigned columnNumber;
    ///Character position at which the error occured.
    const int position;
    ///Error message (always an in-program hardcoded string)
    const char * const what;
    ///Default constructor
    SyntaxError() : lineNumber(0u), columnNumber(0u), position(0u), what("") {};
    ///Constructor with correct parameters
    SyntaxError(unsigned lineNumber, unsigned columnNumber, int position, const char *const what) :
        lineNumber(lineNumber), columnNumber(columnNumber), position(position), what(what) {}
    ///Convenience constructor to take current state from a tokenizer
    inline SyntaxError(const Tokenizer &tok, const char *const what);
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
    ///Last token read and parsed. Can contain spaces.
    std::string token;
    ///Our source to read characters from
    std::istream &in;
    ///Token types which can be parsed by this tokenizer
    enum TokenType : int {
        ///Unrecognized or unacceptable character
        CHAR_ERROR = -3,
        ///Stream error
        STREAM_ERROR = -2,
        ///End of file
        END_OF_FILE = EOF,
        ///Placeholder if nothing is valid
        NONE = 0,
        ///Non-newline whitespace (recognized separately)
        WHITESPACE,
        ///A single newline
        NEWLINE,
        ///A single separator character such as comma (,) and the colon (:)
        SEPARATOR,
        ///Opcode mnemonic (SUB, ADD etc)
        OPCODE,
        ///Pseudocode mnemonic (ORG, DATA etc)
        PSEUDOCODE,
        ///Identifier; might be a register specifier or a label
        IDENTIFIER,
        ///Decimal number (contains only digits 0-9.)
        NUMBER,
        ///Hexadecimal number (contains digits 0-9, a-f, A-F and ends in H or h.)
        HEXNUMBER,
        ///Comment (starts with ';' and should be followed by a NEWLINE token)
        COMMENT
    } ttype;
    ///Current line number
    unsigned lineNumber;
    ///Current column number
    unsigned columnNumber;
    ///Position in stream. Casts are necessary; return -1 if we have reached end.
    int charNumber() const {return in.tellg() > 0 ? (int)in.tellg() - (int)putbackBuffer.size() : -1;}

    ///Constructor
    explicit Tokenizer(std::istream &in) : token(""), in(in), ttype(NONE), lineNumber(1u), columnNumber(1u) {}

    ///Obtain next token
    TokenType getNextToken();

    ///local putback buffer.
    std::stack<char> putbackBuffer;
    ///Get next character from stream
    char getNextChar();
    ///Peek next character from stream
    char peekNextChar();
    ///Push next character (unget) to stream
    void putBackChar(char ch) {putbackBuffer.push(ch);}
    ///Is stream good?
    bool isGood() {return putbackBuffer.empty() ? in.good() : true;}
    ///We have reached end of file or not
    bool isEndOfFile() {return putbackBuffer.empty() ? in.eof() : false;}

    ///Separator characters
    static bool isSeparator(char ch) {
        switch(ch) {
        //As of now, only : , and # are recognized. # indicates a pseudocode is beginning.
        case ',': case ':': case '#': return true;
        default: return false;
        }
    }
    ///is decimal number?
    static bool isNumber(const char * const s) {
        for(const char *p = s; *p != '\0'; p++) if(!std::isdigit(*p)) return false;
        return true;
    }
    ///is hexadecimal number? ends with H.
    static bool isHexNumber(const char * const s) {
        size_t len = std::strlen(s);
        if(len <= 1) return false;
        for(size_t i = 0; i < (len-1u); i++) if(!(std::isdigit(s[i]) || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >= 'A' && s[i] <= 'F'))) return false;
        return s[len-1] == 'h' || s[len-1] == 'H';
    }
};
//Q_DECLARE_METATYPE(Tokenizer) //This is an internal "worker" class, hence not a metatype.

///Translates 8085 assembly source line-by-line (one instruction at a time). Any syntax errors MUST BE CAUGHT in this stage.
struct LineTranslator {
    ///Source
    Tokenizer &tok;
    LineTranslator(Tokenizer &);
    ///Translate the next incoming line. May throw SyntaxErrors, will return an empty Instruction object if the current line is a comment.
    Instruction translateOneLine();
private:
    ///Token processors respective for each mnemonic.
    std::map<const char *, std::function<void(Instruction &, const char *)>, CaseInsensitive> mnemonicProcessors;
    ///Token processors respective for each pseudocode.
    std::map<const char *, std::function<void(Instruction &, const char *)>, CaseInsensitive> pseudocodeProcessors;
    ///Skip over whitespaces.
    void ignoreWhitespaces() {do {tok.getNextToken();} while(tok.ttype == Tokenizer::WHITESPACE);}
    ///Call when the instruction takes a single register (B,C,D,E,H,L,M,A) as argument (and NOT register pairs)
    void singleRegisterProc(Instruction &i, const char *);
    ///Call for STAX and LDAX instructions which accept only BC and DE register pairs as arguments
    void STAX_LDAX_Proc(Instruction &i, const char *);
    ///Call for instructions that take a single register pair as argument (BC, DE, HL or SP).
    void registerPairProc(Instruction &i, const char *);
    ///Call for PUSH and POP that take a single register pair as argument (BC, DE, HL or PSW/AF).
    void stackProc(Instruction &i, const char *);
};

#include "processor.h"
#include <unordered_map>
#include <QObject>

class Assembler : public QObject
{
    Q_OBJECT
    ///Instruction processors respective for each pseudocode.
    std::unordered_map<const opcode *, std::function<void(Instruction &, memaddr_t &)>> pseudocodeProcessors;
    ///Processor for which to assemble.
    Processor *processor;
public:
    ///Input source for the assembler.
    std::istream *in;
    ///Debugging information generated by the last assembly done.
    std::vector<Instruction> instructions;

    Assembler(Processor *proc = nullptr);

    virtual ~Assembler() = default;

private:

    void doAssembly();

public slots:

    void assemble();

signals:

    void assemblyFinished();

    void assemblyError(SyntaxError ex);
};


#define ASSEMBLER_H_registerHeaderMetaTypes() {\
    qRegisterMetaType<Instruction>("Instruction");\
    qRegisterMetaType<SyntaxError>("SyntaxError");\
    /*qRegisterMetaType<Tokenizer>("Tokenizer");*/\
}

#endif // ASSEMBLER_H
