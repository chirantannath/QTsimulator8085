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
#include <Qt>
#include <QVariant>
#include "processor.h"
#include "mainwindow.h"
///Rows are of the form xxx0 (where xxx is hexadecimal; for eg. 2220, 2310, etc...
///Columns are of the form 0 to 9, A to F.
class MemoryTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    
    explicit MemoryTableModel(Processor *parent = nullptr) : QAbstractTableModel(parent), processor(parent) {}
    
    Qt::ItemFlags flags(const QModelIndex &index) const {
        Qt::ItemFlags superFlags = QAbstractTableModel::flags(index);
        return index.isValid() ? superFlags | Qt::ItemIsEditable : superFlags;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
        if(!index.isValid()) return QVariant(); //invalid. We return headers in headerData().
        memaddr_t address = (memaddr_t)(((index.row() << 4) + index.column()) & 0xFFFF);
        switch(role) {
        case Qt::EditRole:
        case Qt::DisplayRole: return QVariant(getHex8(processor->getMemoryByte(address)));
        
        case Qt::WhatsThisRole:
        case Qt::AccessibleDescriptionRole:
        case Qt::ToolTipRole: return QVariant(QString("Memory at address ") + QString::number(address, 16).toUpper() + QString("H"));
            
        case Qt::FontRole: return QVariant(QFont("Monospace"));
            
        default: return QVariant();
        }
    }
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const; //override
    
    int columnCount(const QModelIndex &parent = QModelIndex()) const; //override
    
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const; //override
private:
    Processor *processor;
};

#endif // MEMORYTABLEMODEL_H
