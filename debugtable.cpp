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
#include "debugtable.h"
#include <QVector>
#include <QFont>
#include <algorithm>

//DebugTable

DebugTableModel::DebugTableModel(QObject *parent, const std::vector<Instruction> &vec)
    : QAbstractTableModel(parent), list(vec) {
    std::stable_sort(list.begin(), list.end(), InstructionAddressComparator()); //We show rows sorted on instruction address.
}
int DebugTableModel::rowCount(const QModelIndex &parent) const {return parent.isValid() ? 0 : list.size();} //override
int DebugTableModel::columnCount(const QModelIndex &parent) const {return parent.isValid() ? 0 : 7;} //override
QVariant DebugTableModel::data(const QModelIndex &index, int role) const {//override
    if(!index.isValid()) return QVariant(); //Header data is in headerData()
    switch(role) {
    case Qt::DisplayRole:
        switch(index.column()) {
        case 0: return QVariant(getHex16(list[index.row()].address));
        case 1: return QVariant(QString::fromStdString(list[index.row()].label));
        case 2: return QVariant(QString(list[index.row()].code->name));
        case 3:
            if(!list[index.row()].toLabel.empty()) return QVariant(QString::fromStdString(list[index.row()].toLabel));
            switch(list[index.row()].code->bytesRequired) {
            case 1: return QVariant(QString(""));
            case 2: return QVariant(getHex8(list[index.row()].operand & 0xFFu));
            case 3: return QVariant(getHex16(list[index.row()].operand));
            default: return QVariant();
            }
        case 4: return QVariant(getHex8(list[index.row()].code->code));
        case 5: return QVariant(list[index.row()].code->bytesRequired >= 2 ? getHex8(list[index.row()].operand & 0xFFu) : QString(""));
        case 6: return QVariant(list[index.row()].code->bytesRequired == 3 ? getHex8((list[index.row()].operand >> 8) & 0xFFu) : QString(""));
        default: return QVariant();
        }
    case Qt::FontRole: return QVariant(QFont("Monospace"));
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::BackgroundRole: return index.row() == highlightedIndex ? QVariant(constructHighlightBrush()) : QVariant();
    default: return QVariant();
    }
}
QVariant DebugTableModel::headerData(int section, Qt::Orientation orientation, int role) const {//override
    switch(orientation) {
    case Qt::Horizontal: return columnHeader(section, role);
    case Qt::Vertical: return rowHeader(section, role);
    }
    return QVariant();
}
//public slots
void DebugTableModel::setHighlightedIndex(int index) {
    if(highlightedIndex != index) {
        int old = highlightedIndex;
        QVector<int> roles(columnCount(), Qt::BackgroundRole);
        highlightedIndex = index;
        if(old != -1) {//background change
            emit dataChanged(createIndex(old, 0), createIndex(old, roles.length()-1), roles);
            emit headerDataChanged(Qt::Vertical, old, old);
        }
        if(index != -1) {//background change
            emit dataChanged(createIndex(index, 0), createIndex(index, roles.length()-1), roles);
            emit headerDataChanged(Qt::Vertical, index, index);
        }
    }
}
