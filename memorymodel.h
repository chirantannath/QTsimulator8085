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
#include <QChar>
#include <QString>
#include <QVector>
#include <QFont>
#include "commdefs.h"
#include "processor.h"
///Rows are of the form xxx0 (where xxx is hexadecimal; for eg. 2220, 2310, etc...
///Columns are of the form 0 to 9, A to F.
class MemoryTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    
    explicit MemoryTableModel(Processor *parent = nullptr) : QAbstractTableModel(parent), processor(parent) {
        connect(processor, &Processor::memoryBlockUpdated, this, &MemoryTableModel::memoryBlockUpdated);
    }
    
    Qt::ItemFlags flags(const QModelIndex &index) const {//override
        Qt::ItemFlags superFlags = QAbstractTableModel::flags(index);
        return index.isValid() ? superFlags | Qt::ItemIsEditable : superFlags; //headers are not editable
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {//override
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
            
        default: return QVariant(); //use default for rest
        }
    }
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const {return parent.isValid() ? 0 : 0xFFF+1;}
    
    int columnCount(const QModelIndex &parent = QModelIndex()) const {return parent.isValid() ? 0 : 16;}
    
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {//override
        switch(orientation) {
        case Qt::Horizontal: return columnHeader(section, role);
        case Qt::Vertical: return rowHeader(section, role);
        default: return QVariant();
        }
    }
    
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) {//override
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
private slots:
    
    void memoryBlockUpdated(memaddr_t startLoc, memsize_t blockSize) {
        QVector<int> roles(blockSize, Qt::DisplayRole);
        switch(blockSize) {
        case 0u: break;
        
        case 0xFFFFu + 1u: emit dataChanged(createIndex(0, 0), createIndex(0xFFF, 15), roles); break;
        
        case 1u: emit dataChanged(createIndex((startLoc >> 4) & 0xFFFu, startLoc & 0xFu), 
                                  createIndex((startLoc >> 4) & 0xFFFu, startLoc & 0xFu), roles); break;
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
        }
        }
    }

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
