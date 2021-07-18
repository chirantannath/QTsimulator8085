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
#include <QColor>
#include <QList>
#include <QTextEdit>
#include <QTextFormat>
#include <QTextBlock>
#include <QPainter>
#include <QString>
#include "editor.h"

//For clues, the following code was adapted from:
//https://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html
//The difference lies in the names of classes used, some extra features, and that we used QPlainTextEdit instead of QRichTextEdit.

Editor::Editor(QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);
    connect(this, &Editor::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
    connect(this, &Editor::updateRequest, this, &Editor::updateLineNumberArea);
    connect(this, &Editor::cursorPositionChanged, this, &Editor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}
int Editor::lineNumberAreaWidth() {
    int digits = 1;
    int max = (blockCount() > 1) ? blockCount() : 1;
    while(max >= 10) {max /= 10; ++digits;}
    return 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}
void Editor::updateLineNumberAreaWidth(int /*newBlockCount*/) {setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);} //dummy parameters not needed
void Editor::updateLineNumberArea(const QRect &rect, int dy) {
    if(dy != 0) lineNumberArea->scroll(0, dy);
    else lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if(rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}
void Editor::resizeEvent(QResizeEvent *e) {//override
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}
void Editor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if(!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}
void Editor::setErrorLine(int value) {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if(value < 1) highlightCurrentLine(); //just call normal
    QTextEdit::ExtraSelection selection;
    QColor redColor = QColor(Qt::red).lighter();
    selection.format.setBackground(redColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.setPosition(document()->findBlockByLineNumber(value-1).position());
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}
void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
