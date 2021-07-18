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
#ifndef MEMORYTABLEMODEL_H
#define MEMORYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QModelIndex>
#include <Qt>
#include <QVariant>
#include <QColor>
#include "commdefs.h"
#include "processor.h"
///Table model for displaying processor memory to user.
///Rows are of the form xxx0 (where xxx is hexadecimal; for eg. 2220, 2310, etc...
///Columns are of the form 0 to 9, A to F.
class MemoryTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ///Constructor
    explicit MemoryTableModel(Processor *proc, QObject *parent = nullptr);
    ///Flags describing each cell in the table.
    Qt::ItemFlags flags(const QModelIndex &index) const; //override
    ///Data for each cell in table.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; //override
    ///Number of rows = 4096.
    int rowCount(const QModelIndex &parent = QModelIndex()) const; //override
    ///Number of columns = 16.
    int columnCount(const QModelIndex &parent = QModelIndex()) const; //override
    ///Data for row and column headers.
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const; //override
    ///Set a particular byte value (supplied by user) to a memory address.
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole); //override
private slots:
    ///Processor memory updated outside of this class.
    void memoryBlockUpdated(memaddr_t startLoc, memsize_t blockSize);
    ///Processor program counter (PC) updated outside of this class.
    void pcChanged();
    ///Processor BC register pair updated outside of this class.
    void bcChanged();
    ///Processor DE register pair updated outside of this class.
    void deChanged();
    ///Processor HL register pair updated outside of this class.
    void hlChanged();
    ///Processor stack pointer (SP) updated outside of this class.
    void spChanged();
private:
    ///Processor object of which memory is to be displayed to user.
    Processor *processor;
    ///Construct column header.
    static QVariant columnHeader(int column, int role) {
        QChar character[] = {QChar(DIGITS[column])};
        switch(role) {
        case Qt::DisplayRole: return QVariant(QString(character, 1));
        default: return QVariant();
        }
    }
    ///Construct row header.
    static QVariant rowHeader(int row, int role) {
        switch(role) {
        case Qt::DisplayRole: return QVariant(getHex16(row << 4));
        default: return QVariant();
        }
    }
    ///Highlighting background colour for the address (cell) pointed to by program counter (PC).
    static const QColor pc;
    ///Highlighting background colour for the address (cell) pointed to by BC register pair.
    static const QColor bc;
    ///Highlighting background colour for the address (cell) pointed to by DE register pair.
    static const QColor de;
    ///Highlighting background colour for the address (cell) pointed to by HL register pair.
    static const QColor hl;
    ///Highlighting background colour for the address (cell) pointed to by stack pointer (SP).
    static const QColor sp;

    ///Old value of program counter (PC) before a call to pcChanged().
    volatile memaddr_t oldPC;
    ///Old value of BC register pair before a call to bcChanged().
    volatile memaddr_t oldBC;
    ///Old value of DE register pair before a call to deChanged().
    volatile memaddr_t oldDE;
    ///Old value of HL register pair before a call to hlChanged().
    volatile memaddr_t oldHL;
    ///Old value of SP register pair before a call to spChanged().
    volatile memaddr_t oldSP;
};

#endif // MEMORYTABLEMODEL_H
