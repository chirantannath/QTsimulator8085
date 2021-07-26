#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QCloseEvent>
#include <QSettings>
#include <random>
#include "commdefs.h"
#include "opcodes.h"
#include "processor.h"
#include "assembler.h"
#include "memorymodel.h"
#include "iomodel.h"
#include "debugtable.h"
#include "syntaxhighlighter.h"
#include "finddialog.h"

//We will use Qt's file handling features because they correctly handle various file encodings (UTF-8 or ISOxxx, etc...)
#include <QFileInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } //Forward declaration for the UI management class (generated by Qt).
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    ///Create a human-readable representation of the given syntax error, to be displayed.
    QString constructAssemblyError(SyntaxError ex) {
        QString msg = QString::fromStdString(ex.what);
        if(ex.lineNumber > 0) msg += tr("; line ") + QString::number(ex.lineNumber);
        if(ex.columnNumber > 0) msg += tr("; column ") + QString::number(ex.columnNumber);
        return msg;
    }
public:
    ///Constructor
    MainWindow(QWidget *parent = nullptr);
    ///Destructor
    ~MainWindow();
protected:
    ///Handle closing event (when the user wants to close the application).
    void closeEvent(QCloseEvent *);
private slots:
    ///Start assembly
    void assemble();
    ///Assembly finished successfully
    void assemblyFinished();
    ///Assembly resulted in an error.
    void assemblyError(SyntaxError ex);
    ///User requested a line in source to go to
    void goToLine();
    ///ui->runTarget field has been updated
    void runTargetUpdated();
    ///Clear debug table.
    void clearDebugTable();
    ///User toggled SID (Serial Input Data) button
    void sidToggled(bool value);
    ///User toggled TRAP interrupt button
    void trapToggled(bool value);
    ///User toggled R7.5 interrupt button
    void r7_5Toggled(bool value);
    ///User toggled R6.5 interrupt button
    void r6_5Toggled(bool value);
    ///User toggled R5.5 interrupt button
    void r5_5Toggled(bool value);
    ///User toggled INTR interrupt button
    void intrToggled(bool value);
    ///User selected an instruction (RST) for INTR vectored interrupts.
    void intrVecSelected(int rstIndex);
    ///ui->decimal field edited.
    void decimalEdited(const QString& text);
    ///ui->hexadecimal field edited.
    void hexadecimalEdited(const QString& text);
    ///ui->binary field edited.
    void binaryEdited(const QString& text);
    ///User requested a random number (ui->genRandomButton)
    void genRandom();
    ///Current file being edited has been modified.
    void fileModified(bool modified);
    ///User requested new file
    void newFile();
    ///User requested to open a file.
    void openFile();
    ///User requested to save a file.
    void saveFile();
    ///User requested to save file As...
    void saveFileAs();
    ///User toggled full screen check box menu.
    void fullScreen(bool);

    ///User requested a font change.
    void font();

    ///Fired when the accumulator register is changed.
    void accumulatorChanged();
    ///Fired when register B is changed.
    void registerBChanged();
    ///Fired when register C is changed.
    void registerCChanged();
    ///Fired when register D is changed.
    void registerDChanged();
    ///Fired when register E is changed.
    void registerEChanged();
    ///Fired when flags are changed (when register F is changed).
    void flagsChanged();
    ///Fired when register H is changed. Almost always accompanied by MChanged() signal.
    void registerHChanged();
    ///Fired when register L is changed. Almost always accompanied by MChanged() signal.
    void registerLChanged();
    ///Fired when the program counter register is changed.
    void programCounterChanged();
    ///Fired when the stack pointer register is changed.
    void stackPointerChanged();
    ///Fired when the value of pseudo register M changes (either by a direct write, by modifying registers H or L, or
    ///by external memory writes).
    void MChanged();
    ///Fired when a portion of the memory to this processor changes. It is guaranteed that startLoc will be always
    ///within the range 0x0000 to 0xFFFF inclusive, and blockSize will be within the range 0x0000 and 0x10000
    ///inclusive.
    void memoryBlockUpdated(memaddr_t startLoc,memsize_t blockSize);
    ///Fired when the value stored in an I/O port latch (indicated by address) is updated.
    void ioPortUpdated(ioaddr_t address);
    ///Fired when this processor executes a SIM instruction and a serial output bit is sent on SOD latch.
    void serialOutput();
    ///Fired when the request status latch for RST 7.5 is changed.
    void restart7_5RequestStatusChanged();
    ///Fired when this processor changes the masking state for RST 7.5.
    void maskRestart7_5Changed();
    ///Fired when this processor changes the masking state for RST 6.5.
    void maskRestart6_5Changed();
    ///Fired when this processor changes the masking state for RST 5.5.
    void maskRestart5_5Changed();
    ///Fired when this processor changes the interrupt enable latch status as a result of executing either DI or EI
    ///instruction.
    void interruptEnableStatusChanged();
    ///Fired when the INTA interrupt acknowledge value changes (either acknowedged or reset).
    void interruptAcknowledgeStatusChanged();
    ///Fired when processor halts its instruction execution (either externally or due to HLT).
    void halted();
    ///Start processor to run program
    void runOneShot();

    ///Check if currentlyOpenedFile was modified and offer to save. Returns false if entire operation was cancelled.
    bool checkUnsaved();

signals:
    //WARNING: TREAT THE FOLLOWING AS PRIVATE API
    ///Fire event to signal assembler to begin assembling.
    void __fireAssemblerEvent();
    ///Fire event to signal processor to begin running.
    void __fireOneShot();
private:
    ///UI management object
    Ui::MainWindow * const ui;
    ///Processor (execution engine)
    Processor * const processor;
    ///Assembler engine
    Assembler * const assembler;
    ///Table model for displaying memory to user
    MemoryTableModel * const memTable;
    ///Table model for displaying I/O ports to user
    IOTableModel * const ioTable;
    ///A default model for the debugging information table (ui->debugTable) which is empty.
    DebugTableModel * const emptyDebugTableModel;
    ///Table model for displaying debugging information to user.
    DebugTableModel *currentDebugTableModel; //Not const because can change
    ///Syntax highlighter engine
    SyntaxHighlighter *highlighter; //Not const because it depends upon components initialized AFTER const initialization
    ///Currently opened file info
    QFileInfo currentlyOpenedFile;
    ///Has the file been modified in this application?
    unsigned isFileModified : 1;
    ///Did the last assembly result in an error?
    unsigned lastAssemblyErrored : 1;
    ///Random number generator
    std::default_random_engine rng; //Not const because internal states change.
    ///Application settings
    QSettings * const settings;
    ///Find dialog (NOT const because depends upon ui->source)
    FindDialog *findDialog;
};
#endif // MAINWINDOW_H
