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
#include <QBrush>
#include "memorymodel.h"

//MemoryTableModel
const QColor MemoryTableModel::pc = QColor::fromRgbF(0.75, 0.25, 0.25, 0.5); //red
const QColor MemoryTableModel::bc = QColor::fromRgbF(0.25, 0.75, 0.25, 0.5); //green
const QColor MemoryTableModel::de = QColor::fromRgbF(0.75, 0.75, 0.25, 0.5); //yellow
const QColor MemoryTableModel::hl = QColor::fromRgbF(0.50, 0.50, 0.50, 0.5); //grey-ish
const QColor MemoryTableModel::sp = QColor::fromRgbF(0.25, 0.25, 0.75, 0.5); //blue

MemoryTableModel::MemoryTableModel(Processor *proc, QObject *parent) : QAbstractTableModel(parent), processor(proc) {
    connect(processor, &Processor::memoryBlockUpdated, this, &MemoryTableModel::memoryBlockUpdated);
    connect(processor, &Processor::programCounterChanged, this, &MemoryTableModel::pcChanged);
    connect(processor, &Processor::stackPointerChanged, this, &MemoryTableModel::spChanged);
    connect(processor, &Processor::registerBChanged, this, &MemoryTableModel::bcChanged);
    connect(processor, &Processor::registerCChanged, this, &MemoryTableModel::bcChanged);
    connect(processor, &Processor::registerDChanged, this, &MemoryTableModel::deChanged);
    connect(processor, &Processor::registerEChanged, this, &MemoryTableModel::deChanged);
    connect(processor, &Processor::registerHChanged, this, &MemoryTableModel::hlChanged);
    connect(processor, &Processor::registerLChanged, this, &MemoryTableModel::hlChanged);

    oldPC = processor->getProgramCounter(); oldBC = processor->getBCRegisterPair(); oldDE = processor->getDERegisterPair();
    oldHL = processor->getHLRegisterPair(); oldSP = processor->getStackPointer();
}
Qt::ItemFlags MemoryTableModel::flags(const QModelIndex &index) const {//override
    Qt::ItemFlags superFlags = QAbstractTableModel::flags(index);
    return index.isValid() ? superFlags | Qt::ItemIsEditable : superFlags; //headers are not editable
}
QVariant MemoryTableModel::data(const QModelIndex &index, int role) const {//override
    QBrush brush; brush.setStyle(Qt::Dense3Pattern);
    if(!index.isValid()) return QVariant(); //invalid. We return headers in headerData().
    memaddr_t address = (memaddr_t)(((index.row() << 4) + index.column()) & 0xFFFF);
    switch(role) {
    case Qt::EditRole:
    case Qt::DisplayRole: return QVariant(getHex8(processor->getMemoryByte(address)));
    case Qt::WhatsThisRole:
    case Qt::AccessibleDescriptionRole:
    case Qt::ToolTipRole: return QVariant(tr("Memory at address ") + getHex16(address) + tr("H"));
    case Qt::FontRole: return QVariant(QFont("Monospace"));
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::BackgroundRole:
        //Take note of priority order (highest to lowest). Developer may reorder it if wanted
        if(address == processor->getProgramCounter()) brush.setColor(pc);
        else if(address == processor->getHLRegisterPair()) brush.setColor(hl);
        else if(address == processor->getStackPointer()) brush.setColor(sp);
        else if(address == processor->getBCRegisterPair()) brush.setColor(bc);
        else if(address == processor->getDERegisterPair()) brush.setColor(de);
        else return QVariant(); //default if none match
        return QVariant(brush);
    default: return QVariant(); //use default for rest
    }
}
void MemoryTableModel::pcChanged() {
    QVector<int> roles(1, Qt::BackgroundRole);
    if(oldPC != processor->getProgramCounter()) {
        emit dataChanged(createIndex((oldPC >> 4) & 0xFFFu, oldPC & 0xFu),
                         createIndex((oldPC >> 4) & 0xFFFu, oldPC & 0xFu), roles);
        oldPC = processor->getProgramCounter();
        emit dataChanged(createIndex((oldPC >> 4) & 0xFFFu, oldPC & 0xFu),
                         createIndex((oldPC >> 4) & 0xFFFu, oldPC & 0xFu), roles);
    }
}
void MemoryTableModel::bcChanged() {
    QVector<int> roles(1, Qt::BackgroundRole);
    if(oldBC != processor->getBCRegisterPair()) {
        emit dataChanged(createIndex((oldBC >> 4) & 0xFFFu, oldBC & 0xFu),
                         createIndex((oldBC >> 4) & 0xFFFu, oldBC & 0xFu), roles);
        oldBC = processor->getBCRegisterPair();
        emit dataChanged(createIndex((oldBC >> 4) & 0xFFFu, oldBC & 0xFu),
                         createIndex((oldBC >> 4) & 0xFFFu, oldBC & 0xFu), roles);
    }
}
void MemoryTableModel::deChanged() {
    QVector<int> roles(1, Qt::BackgroundRole);
    if(oldDE != processor->getDERegisterPair()) {
        emit dataChanged(createIndex((oldDE >> 4) & 0xFFFu, oldDE & 0xFu),
                         createIndex((oldDE >> 4) & 0xFFFu, oldDE & 0xFu), roles);
        oldDE = processor->getDERegisterPair();
        emit dataChanged(createIndex((oldDE >> 4) & 0xFFFu, oldDE & 0xFu),
                         createIndex((oldDE >> 4) & 0xFFFu, oldDE & 0xFu), roles);
    }
}
void MemoryTableModel::hlChanged() {
    QVector<int> roles(1, Qt::BackgroundRole);
    if(oldHL != processor->getHLRegisterPair()) {
        emit dataChanged(createIndex((oldHL >> 4) & 0xFFFu, oldHL & 0xFu),
                         createIndex((oldHL >> 4) & 0xFFFu, oldHL & 0xFu), roles);
        oldHL = processor->getHLRegisterPair();
        emit dataChanged(createIndex((oldHL >> 4) & 0xFFFu, oldHL & 0xFu),
                         createIndex((oldHL >> 4) & 0xFFFu, oldHL & 0xFu), roles);
    }
}
void MemoryTableModel::spChanged() {
    QVector<int> roles(1, Qt::BackgroundRole);
    if(oldSP != processor->getStackPointer()) {
        emit dataChanged(createIndex((oldSP >> 4) & 0xFFFu, oldSP & 0xFu),
                         createIndex((oldSP >> 4) & 0xFFFu, oldSP & 0xFu), roles);
        oldSP = processor->getStackPointer();
        emit dataChanged(createIndex((oldSP >> 4) & 0xFFFu, oldSP & 0xFu),
                         createIndex((oldSP >> 4) & 0xFFFu, oldSP & 0xFu), roles);
    }
}
int MemoryTableModel::rowCount(const QModelIndex &parent) const {return parent.isValid() ? 0 : 0xFFF+1;} //override
int MemoryTableModel::columnCount(const QModelIndex &parent) const {return parent.isValid() ? 0 : 16;} //override
QVariant MemoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const {//override
    switch(orientation) {
    case Qt::Horizontal: return columnHeader(section, role);
    case Qt::Vertical: return rowHeader(section, role);
    default: return QVariant();
    }
}
bool MemoryTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {//override
    if(!index.isValid()) return false;
    if(role != Qt::EditRole) return false;
    if(!value.canConvert<QString>()) return false;
    memaddr_t address = (memaddr_t)(((index.row() << 4) + index.column()) & 0xFFFF);
    bool success;
    long long newData = value.toString().toLongLong(&success, 16);
    if(!success) return false;
    if(newData < 0 || newData > 0xFF) return false;
    disconnect(processor, &Processor::memoryBlockUpdated, this, &MemoryTableModel::memoryBlockUpdated);
    processor->setMemoryByte(address, (data8_t)newData);
    emit dataChanged(index, index, QVector<int>(1, Qt::DisplayRole));
    connect(processor, &Processor::memoryBlockUpdated, this, &MemoryTableModel::memoryBlockUpdated);
    return true;
}
//private slots
void MemoryTableModel::memoryBlockUpdated(memaddr_t startLoc, memsize_t blockSize) {
    QVector<int> roles(blockSize, Qt::DisplayRole);
    switch(blockSize) {
    case 0u: break;
    case 0xFFFFu + 1u: emit dataChanged(createIndex(0, 0), createIndex(0xFFF, 15), roles); break;
    case 1u: emit dataChanged(createIndex((startLoc >> 4) & 0xFFFu, startLoc & 0xFu), createIndex((startLoc >> 4) & 0xFFFu, startLoc & 0xFu), roles); break;
    default: {
        memaddr_t min, max;
        for(memsize_t i = 0; i < blockSize; i++) {
            memaddr_t current = (memaddr_t)((startLoc + i) & 0xFFFFu);
            if(i == 0) min = max = current;
            else {
                if(min > current) min = current;
                if(max < current) max = current;
            }
        }
        min = (min >> 4) & 0xFFFu;
        max = (max >> 4) & 0xFFFu;
        roles = QVector<int>((max - min + 1) * 16, Qt::DisplayRole);
        emit dataChanged(createIndex(min, 0), createIndex(max, 15), roles);
        break;
    }
    }
}
