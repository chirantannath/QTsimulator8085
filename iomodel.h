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
#ifndef IOMODEL_H
#define IOMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QModelIndex>
#include <Qt>
#include <QVariant>
#include "processor.h"

///Model for table of I/O ports to and from the 8085.
class IOTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    ///Constructor.
    explicit IOTableModel(Processor *proc, QObject *parent = nullptr);
    ///Flags for each cell or header.
    Qt::ItemFlags flags(const QModelIndex &index) const; //override
    ///Data for each cell.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; //override
    ///Number of rows = 16.
    int rowCount(const QModelIndex &parent = QModelIndex()) const; //override
    ///Number of columns = 16.
    int columnCount(const QModelIndex &parent = QModelIndex()) const; //override
    ///Header data. Columns are from 0 to 9, A to F; and rows are the higher-significant digits for port addresses
    ///(00, 10, 20, ... E0, F0)
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const; //override
    ///Set port data.
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole); //override
private slots:
    ///Called when I/O port data got updated outside of the table.
    void ioPortUpdated(ioaddr_t address);
    ///Called when I/O ports are reset outside of the table.
    void ioPortsReset();
private:
    ///Processor of which I/O ports are to be modelled as a table.
    Processor *processor;
};

#endif // IOMODEL_H
