#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "commdefs.h"
#include "opcodes.h"
#include "processor.h"
#include "assembler.h"

extern const char HEX_DIGITS[17]; //1 extra for null

QString getHex8(data8_t value);

QString getHex16(data16_t value);

QString getBinDigit(data8_t value, int position);

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    
    void sidToggled(bool value);
    
    void trapToggled(bool value);
    
    void r7_5Toggled(bool value);
    
    void r6_5Toggled(bool value);
    
    void r5_5Toggled(bool value);
    
    void intrToggled(bool value);
    
    void intrVecSelected(int rstIndex);
    
    void decimalEdited(const QString& text);
    
    void hexadecimalEdited(const QString& text);
    
    void binaryEdited(const QString& text);
    
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
    
private:
    Ui::MainWindow *ui;
    Processor *processor;
};
#endif // MAINWINDOW_H
