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

//For clues, the following code and editor.cpp was adapted from:
//https://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html
//The difference lies in the names of classes used, some extra features, and that we used QPlainTextEdit instead of QRichTextEdit.

///GUI component used to edit source code. Provides line highlighting and line numbering. For syntax highlighting look in
///syntaxhighlighter.h.
class Editor : public QPlainTextEdit {
    Q_OBJECT
public:
    ///Constructor
    Editor(QWidget *parent = nullptr);
    ///Handle paint request by system for the line number area.
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    ///Calculate width of line number area.
    int lineNumberAreaWidth();
protected:
    ///Handle the event when this component is resized.
    void resizeEvent(QResizeEvent *event); //override
private slots:
    ///Called when the width of the line number area is updated. The parameter is unused as of yet but is supposed to get the
    ///current number of blocks(lines).
    void updateLineNumberAreaWidth(int);
    ///Called when the current line needs to be highlighted.
    void highlightCurrentLine();
    ///Update line number area on edit.
    void updateLineNumberArea(const QRect &rect, int dy);
public slots:
    ///Called when the system detects an error in the given line number.
    void setErrorLine(int);
private:
    ///The sub-component used to display line numbers.
    QWidget *lineNumberArea;
};

///The component (sub-component of Editor) used to display line numbers.
class LineNumberArea : public QWidget {
    Q_OBJECT
public:
    ///Constructor
    LineNumberArea(Editor *editor) : QWidget(editor), editor(editor) {}
    ///Calculate (preferred) size for this component.
    QSize sizeHint() const {return QSize(editor->lineNumberAreaWidth(), 0);} //override
protected:
    ///Handle paint request by system for the line number area.
    void paintEvent(QPaintEvent *event) {editor->lineNumberAreaPaintEvent(event);} //override
private:
    ///The editor to which this line number area belongs to.
    Editor *editor;
};

#endif // EDITOR_H
