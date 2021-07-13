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
#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QRect>
#include <QSize>

///GUI component used to edit source code. Provides syntax and line higlighting, and line numbering.
class Editor : public QPlainTextEdit {
    Q_OBJECT
public:

    Editor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);

    int lineNumberAreaWidth();

protected:

    void resizeEvent(QResizeEvent *event); //override

private slots:

    void updateLineNumberAreaWidth(int);

    void highlightCurrentLine();

    void updateLineNumberArea(const QRect &rect, int dy);

public slots:

    void setErrorLine(int);

private:

    QWidget *lineNumberArea;
    //int errorLine;
};

class LineNumberArea : public QWidget {
    Q_OBJECT
public:

    LineNumberArea(Editor *editor) : QWidget(editor), editor(editor) {}

    QSize sizeHint() const {return QSize(editor->lineNumberAreaWidth(), 0);} //override

protected:

    void paintEvent(QPaintEvent *event) {editor->lineNumberAreaPaintEvent(event);} //override

private:

    Editor *editor;
};

#endif // EDITOR_H
