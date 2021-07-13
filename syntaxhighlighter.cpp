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
#include "syntaxhighlighter.h"
#include <Qt>
#include <sstream>

inline QTextCharFormat charErrorFormat() {
    QTextCharFormat format;
    format.setBackground(Qt::red);
    format.setForeground(Qt::black);
    return format;
}
inline QTextCharFormat opcodeFormat() {
    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    format.setForeground(QColor::fromRgb(0x7F, 0x00, 0xFF)); //violet
    return format;
}
inline QTextCharFormat pseudocodeFormat() {
    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    format.setForeground(Qt::darkRed);
    return format;
}
inline QTextCharFormat identifierFormat() {
    QTextCharFormat format;
    format.setForeground(Qt::darkGreen);
    return format;
}
inline QTextCharFormat numberFormat() {
    QTextCharFormat format;
    format.setForeground(Qt::darkYellow);
    return format;
}
#define hexNumberFormat() numberFormat()
inline QTextCharFormat commentFormat() {
    QTextCharFormat format;
    format.setFontItalic(true);
    format.setForeground(Qt::gray);
    return format;
}

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    QTextCharFormat defaultFormat;
    highlightingRules[Tokenizer::CHAR_ERROR] = charErrorFormat();
    highlightingRules[Tokenizer::STREAM_ERROR] = defaultFormat; //should not happen
    highlightingRules[Tokenizer::END_OF_FILE] = defaultFormat; //should be caught
    highlightingRules[Tokenizer::NONE] = defaultFormat; //should not happen
    highlightingRules[Tokenizer::WHITESPACE] = defaultFormat;   //
    highlightingRules[Tokenizer::NEWLINE] = defaultFormat;      //Leave whitespaces, newlines and separators as-it-is
    highlightingRules[Tokenizer::SEPARATOR] = defaultFormat;    //
    highlightingRules[Tokenizer::OPCODE] = opcodeFormat();
    highlightingRules[Tokenizer::PSEUDOCODE] = pseudocodeFormat();
    highlightingRules[Tokenizer::IDENTIFIER] = identifierFormat();
    highlightingRules[Tokenizer::NUMBER] = numberFormat();
    highlightingRules[Tokenizer::HEXNUMBER] = hexNumberFormat();
    highlightingRules[Tokenizer::COMMENT] = commentFormat();
}
void SyntaxHighlighter::highlightBlock(const QString &text) {
    //we know text will be a single line.
    const std::string stdText = text.toStdString();
    std::stringstream stream(stdText); Tokenizer tok(stream);
    int oldIndex = 0;
    do {
        tok.getNextToken();
        if(tok.ttype == Tokenizer::CHAR_ERROR) {
            if(oldIndex < text.length())setFormat(oldIndex, 1, highlightingRules[Tokenizer::CHAR_ERROR]);
            oldIndex++; tok.getNextChar(); continue;
        }
        int newIndex = tok.charNumber(); if(newIndex == -1) newIndex = text.length();
        if(oldIndex < text.length() && newIndex <= text.length())
            setFormat(oldIndex, newIndex - oldIndex, highlightingRules[tok.ttype]);
        oldIndex = newIndex;
    } while(tok.ttype != Tokenizer::END_OF_FILE);
}
