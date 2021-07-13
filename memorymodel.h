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
#include "commdefs.h"
#include "processor.h"
///Rows are of the form xxx0 (where xxx is hexadecimal; for eg. 2220, 2310, etc...
///Columns are of the form 0 to 9, A to F.
class MemoryTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    explicit MemoryTableModel(Processor *proc = nullptr, QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const; //override

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; //override

    int rowCount(const QModelIndex &parent = QModelIndex()) const; //override

    int columnCount(const QModelIndex &parent = QModelIndex()) const; //override

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const; //override

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole); //override

private slots:

    void memoryBlockUpdated(memaddr_t startLoc, memsize_t blockSize);

private:

    Processor *processor;

    static QVariant columnHeader(int column, int role) {
        QChar character[] = {QChar(HEX_DIGITS[column])};
        switch(role) {

        case Qt::DisplayRole: return QVariant(QString(character, 1));

        default: return QVariant();
        }
    }

    static QVariant rowHeader(int row, int role) {
        switch(role) {

        case Qt::DisplayRole: return QVariant(getHex16(row << 4));

        default: return QVariant();
        }
    }
};

#endif // MEMORYTABLEMODEL_H
