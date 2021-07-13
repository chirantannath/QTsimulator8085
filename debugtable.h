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
#ifndef DEBUGTABLE_H
#define DEBUGTABLE_H

#include <QAbstractTableModel>
#include <QObject>
#include <QModelIndex>
#include <Qt>
#include <QVariant>
#include <QString>
#include <QBrush>
#include <QColor>
#include <vector>
#include "commdefs.h"
#include "assembler.h"

class DebugTableModel : public QAbstractTableModel {
    Q_OBJECT

    int highlightedIndex = -1;

    static QVariant columnHeader(int column, int role) {
        static const char *columns[] = {"Address", "Label", "Instruction", "Operand", "B1", "B2", "B3"};
        static const char *columnHelp[] = {"Address of instruction in memory", "Label for this instruction line", "Instruction",
                                          "Operand for this instruction", "Byte 1 for this instruction", "Byte 2 for this instruction, if any",
                                          "Byte 3 for this instruction, if any"};
        switch(role) {

        case Qt::DisplayRole: return QVariant(QString(columns[column]));

        case Qt::WhatsThisRole:
        case Qt::AccessibleDescriptionRole:
        case Qt::ToolTipRole: return QVariant(QString(columnHelp[column]));

        case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);

        default: return QVariant();
        }
    }

    QVariant rowHeader(int row, int role) const {
        switch(role) {

        case Qt::DisplayRole: return QVariant(list[row].lineNumber);

        case Qt::BackgroundRole: return (row == highlightedIndex) ? QVariant(constructHighlightBrush()) : QVariant();

        default: return QVariant();
        }
    }

    static QBrush constructHighlightBrush() {
        QBrush brush;
        brush.setStyle(Qt::Dense3Pattern);
        brush.setColor(QColor::fromRgbF(0.75, 0.25, 0.25, 0.5)); //adjust as needed
        return brush;
    }

public:

    explicit DebugTableModel(QObject *parent = nullptr, const std::vector<Instruction> &vec = std::vector<Instruction>());

    int rowCount(const QModelIndex &parent = QModelIndex()) const; //override

    int columnCount(const QModelIndex &parent = QModelIndex()) const; //override

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; //override

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const; //override

    int getHighlighedIndex() const {return highlightedIndex;}

    std::vector<Instruction> list; //Do not change. Construct a new object everytime this is changed.
public slots:

    void setHighlightedIndex(int index);
};

#endif // DEBUGTABLE_H

