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
#include <QPushButton>
#include <QTextDocument>
#include <QRegularExpression>
#include <QString>
#include <QMessageBox>
#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent, Editor *editor) : QDialog(parent), editor(editor), lastFound() {
    setupUi(this);
    connect(this->findBtn, &QPushButton::clicked, this, &FindDialog::doFind);
    connect(this->replaceBtn, &QPushButton::clicked, this, &FindDialog::doReplace);
    connect(this->replaceAllBtn, &QPushButton::clicked, this, &FindDialog::doReplaceAll);
    connect(this->exitBtn, &QPushButton::clicked, this, &FindDialog::hide);
}
//protected
void FindDialog::closeEvent(QCloseEvent *evt) {evt->ignore(); hide();} //hide instead
//private slots
void FindDialog::doFind() {
    QString findWhat = findTarget->text();
    QTextDocument::FindFlags fflags;
    fflags.setFlag(QTextDocument::FindBackward, backward->isChecked());
    fflags.setFlag(QTextDocument::FindCaseSensitively, matchCase->isChecked());
    fflags.setFlag(QTextDocument::FindWholeWords, wholeWords->isChecked());
    lastFound = regexp->isChecked() ? editor->document()->find(QRegularExpression(findWhat), editor->textCursor(), fflags) :
                                               editor->document()->find(findWhat, editor->textCursor(), fflags);
    if(lastFound.isNull()) QMessageBox::information(this, windowTitle(), "Could not find in file.");
    else editor->setTextCursor(lastFound);
}
void FindDialog::doReplace() {
    QTextCursor current = editor->textCursor();
    if(current.hasSelection()) {current.insertText(replaceTarget->text()); editor->setTextCursor(current);}
}
void FindDialog::doReplaceAll() {do {doReplace(); doFind();} while (!lastFound.isNull());}
