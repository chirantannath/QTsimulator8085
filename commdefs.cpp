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
#include "commdefs.h"

//CaseInsensitive
CaseInsensitive CaseInsensitive::comparator;

bool CaseInsensitive::operator()(const char *s1, const char *s2) const {
    for(; *s1 != '\0' && *s2 != '\0'; s1++, s2++) {
        char c1 = std::toupper(*s1), c2 = std::toupper(*s2);
        if(c1 != c2) return c1 < c2;
    }
    return *s1 == '\0' && *s2 != '\0';
}

//StringInsensitive
StringInsensitive StringInsensitive::comparator;

bool StringInsensitive::operator()(const std::string &a, const std::string &b) const {
    for(std::string::const_iterator s1 = a.begin(), s2 = b.begin(); s1 != a.end() && s2 != b.end(); s1++, s2++) {
        char c1 = std::toupper(*s1), c2 = std::toupper(*s2);
        if(c1 != c2) return c1 < c2;
    }
    return a.length() < b.length();
}

//globals
const char HEX_DIGITS[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '\0'};
const bool PARITY_LOOKUP[256] = {
    //Implementation Note: the following list was generated by Python (3):
    //parity_flag = lambda x: (sum(1 for ch in bin(x) if ch == '1') % 2) == 0
    //for x in range(256):
    //  if parity_flag(x): print('true, ', end='')
    //  else: print('false, ', end='')
    true, false, false, true, false, true, true, false, false, true, true, false, true, false,
    false, true, false, true, true, false, true, false, false, true, true, false, false, true,
    false, true, true, false, false, true, true, false, true, false, false, true, true, false,
    false, true, false, true, true, false, true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true, false, true, true, false, true, false,
    false, true, true, false, false, true, false, true, true, false, true, false, false, true,
    false, true, true, false, false, true, true, false, true, false, false, true, true, false,
    false, true, false, true, true, false, false, true, true, false, true, false, false, true,
    false, true, true, false, true, false, false, true, true, false, false, true, false, true,
    true, false, false, true, true, false, true, false, false, true, true, false, false, true,
    false, true, true, false, true, false, false, true, false, true, true, false, false, true,
    true, false, true, false, false, true, true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true, false, true, true, false, true, false,
    false, true, true, false, false, true, false, true, true, false, true, false, false, true,
    false, true, true, false, false, true, true, false, true, false, false, true, false, true,
    true, false, true, false, false, true, true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true, true, false, false, true, false, true,
    true, false, true, false, false, true, false, true, true, false, false, true, true, false,
    true, false, false, true
};

#include <QChar>

QString getHex8(data8_t value) {
    QString text; text.resize(2);
    text[0] = QChar(HEX_DIGITS[(value >> 4) & 0xF]);
    text[1] = QChar(HEX_DIGITS[value & 0xF]);
    return text;
}
QString getHex16(data16_t value) {
    QString text; text.resize(4);
    text[0] = QChar(HEX_DIGITS[(value >> 12) & 0xF]);
    text[1] = QChar(HEX_DIGITS[(value >> 8) & 0xF]);
    text[2] = QChar(HEX_DIGITS[(value >> 4) & 0xF]);
    text[3] = QChar(HEX_DIGITS[value & 0xF]);
    return text;
}
QString getBinDigit(data8_t value, int position) {
    const QChar bit[1] = {(value & (1 << position)) ? QChar('1') : QChar('0')};
    return QString(bit, 1);
}