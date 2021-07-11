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
#include <QString>
#include <QVector>
#include <QFont>
#include "commdefs.h"
#include "iomodel.h"

//IOTableModel

IOTableModel::IOTableModel(Processor *proc, QObject *parent) : QAbstractTableModel(parent), processor(proc) {
    connect(processor, &Processor::ioPortUpdated, this, &IOTableModel::ioPortUpdated);
    connect(processor, &Processor::ioPortsReset, this, &IOTableModel::ioPortsReset);
}
Qt::ItemFlags IOTableModel::flags(const QModelIndex &index) const {//override
    Qt::ItemFlags superFlags = QAbstractTableModel::flags(index);
    return index.isValid() ? superFlags | Qt::ItemIsEditable : superFlags; //headers are not editable
}
QVariant IOTableModel::data(const QModelIndex &index, int role) const {//override
    if(!index.isValid()) return QVariant(); //invalid. We return headers in headerData().
    ioaddr_t address = (ioaddr_t)(((index.row() << 4) + index.column()) & 0xFF);
    switch(role) {
    case Qt::EditRole:
    case Qt::DisplayRole: return QVariant(getHex8(processor->getOutputByte(address)));

    case Qt::WhatsThisRole:
    case Qt::AccessibleDescriptionRole:
    case Qt::ToolTipRole: return QVariant(tr("Value at I/O port address ") + getHex8(address) + tr("H"));

    case Qt::FontRole: return QVariant(QFont("Monospace"));

    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);

    default: return QVariant(); //use default for rest
    }
}
int IOTableModel::rowCount(const QModelIndex &parent) const {return parent.isValid() ? 0 : 16;}//override
int IOTableModel::columnCount(const QModelIndex &parent) const {return parent.isValid() ? 0 : 16;}//override
QVariant IOTableModel::headerData(int section, Qt::Orientation orientation, int role) const {//override
    QChar character[] = {QChar(HEX_DIGITS[section])};
    QString str(character, 1);
    switch(role) {
    case Qt::DisplayRole:
        switch(orientation) {
        case Qt::Horizontal: return QVariant(str);
        case Qt::Vertical: return QVariant(str + tr("0"));
        default: return QVariant();
        }
    default: return QVariant();
    }
}
bool IOTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {//override
    if(!index.isValid()) return false;
    if(role != Qt::EditRole) return false;
    if(!value.canConvert<QString>()) return false;
    ioaddr_t address = (ioaddr_t)(((index.row() << 4) + index.column()) & 0xFF);
    bool success;
    long long newData = value.toString().toLongLong(&success, 16);
    if(!success) return false;
    if(newData < 0 || newData > 0xFF) return false;
    disconnect(processor, &Processor::ioPortUpdated, this, &IOTableModel::ioPortUpdated);
    processor->setInputByte(address, (data8_t)newData);
    emit dataChanged(index, index, QVector<int>(1, Qt::DisplayRole));
    connect(processor, &Processor::ioPortUpdated, this, &IOTableModel::ioPortUpdated);
    return true;
}
//private slots
void IOTableModel::ioPortUpdated(ioaddr_t address) {
    emit dataChanged(createIndex((address >> 4) & 0xF, address & 0xF), createIndex((address >> 4) & 0xF, address & 0xF),
                     QVector<int>(1, Qt::DisplayRole));
}
void IOTableModel::ioPortsReset() {emit dataChanged(createIndex(0, 0), createIndex(0xF, 0xF), QVector<int>(256, Qt::DisplayRole));}
