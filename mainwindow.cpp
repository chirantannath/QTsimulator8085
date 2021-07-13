#include <QtGlobal>
#include <QChar>
#include <QPushButton>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QHeaderView>
#include <QGuiApplication>
#include <QList>
#include <QScreen>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QIODevice>
#include <QTextStream>
#include <QModelIndex>
#include <vector>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), emptyDebugTableModel(new DebugTableModel(this)), isFileModified(false)
{
    ui->setupUi(this);
    const int largeLimit = QGuiApplication::primaryScreen()->size().width();
    ui->splitter->setSizes(QList<int>({largeLimit, largeLimit})); //setSizes is relative
    /*NOTE:
    Processor was initially meant to live on a separate thread. We don't do that anymore because Processor sends events faster
    than the main event loop can process them and we run into a memory bottleneck. However, while writing code; still assume
    Processor and Assembler live in separate threads.

    We use event progress polling (QCoreApplication::processEvents()) instead to keep the application responsive.
    */
    processor = new Processor(this);
    assembler = new Assembler(processor);
    highlighter = new SyntaxHighlighter(ui->source->document());
    connect(this, &MainWindow::__fireAssemblerEvent, assembler, &Assembler::assemble);
    connect(assembler, &Assembler::assemblyFinished, this, &MainWindow::assemblyFinished);
    connect(assembler, &Assembler::assemblyError, this, &MainWindow::assemblyError);

    //Memory and I/O tables
    memTable = new MemoryTableModel(processor, processor);
    ui->memTableView->setModel(memTable);
    ui->memTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ioTable = new IOTableModel(processor, processor);
    ui->ioTableView->setModel(ioTable);
    ui->ioTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //Current empty debug table
    currentDebugTableModel = emptyDebugTableModel;
    ui->debugTableView->setModel(currentDebugTableModel);
    ui->debugTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //validators for Number Conversion Tools
    ui->decimal->setValidator(new QRegExpValidator(QRegExp("^\\s*[0123456789]*\\s*$"), ui->decimal));
    ui->hexadecimal->setValidator(new QRegExpValidator(QRegExp("^\\s*[0123456789abcdefABCDEF]*\\s*$"), ui->hexadecimal));
    ui->binary->setValidator(new QRegExpValidator(QRegExp("^\\s*[01]*\\s*$"), ui->binary));
    ui->runTarget->setValidator(new QRegExpValidator(QRegExp("^\\s*[0123456789abcdefABCDEF]{0,4}\\s*$"), ui->runTarget));
    ui->findTarget->setValidator(new QRegExpValidator(QRegExp("^\\s*[0123456789abcdefABCDEF]{0,4}\\s*$"), ui->findTarget));

    //connect events
    connect(ui->assembleButton, &QPushButton::clicked, this, &MainWindow::assemble);
    connect(ui->sid, &QPushButton::toggled, this, &MainWindow::sidToggled);
    connect(ui->trap, &QPushButton::toggled, this, &MainWindow::trapToggled);
    connect(ui->r7_5, &QPushButton::toggled, this, &MainWindow::r7_5Toggled);
    connect(ui->r6_5, &QPushButton::toggled, this, &MainWindow::r6_5Toggled);
    connect(ui->r5_5, &QPushButton::toggled, this, &MainWindow::r5_5Toggled);
    connect(ui->intr, &QPushButton::toggled, this, &MainWindow::intrToggled);
    connect(ui->intrVec, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::intrVecSelected);
    connect(ui->decimal, &QLineEdit::textEdited, this, &MainWindow::decimalEdited);
    connect(ui->hexadecimal, &QLineEdit::textEdited, this, &MainWindow::hexadecimalEdited);
    connect(ui->binary, &QLineEdit::textEdited, this, &MainWindow::binaryEdited);
    connect(ui->RESET_IN, &QPushButton::clicked, processor, &Processor::RESET_IN);
    connect(ui->memResetButton, &QPushButton::clicked, processor, &Processor::resetMemory);
    connect(ui->ioResetButton, &QPushButton::clicked, processor, &Processor::resetIOPorts);
    connect(ui->source, &Editor::modificationChanged, this, &MainWindow::fileModified);

    connect(ui->actionNew_Source_File, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen_Source_File, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave_Source_File, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSave_Source_File_As, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionFull_Screen, &QAction::toggled, this, [&](bool flag)
    {
        setWindowFlag(Qt::FramelessWindowHint, flag);
        if(flag) {showFullScreen();} else {showNormal();}
    });
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    connect(ui->actionAssemble, &QAction::triggered, ui->assembleButton, &QPushButton::clicked);

    connect(ui->actionAbout_Qt, &QAction::triggered, &QApplication::aboutQt);

    connect(ui->stepButton, &QPushButton::clicked, processor, &Processor::stepNextInstruction);
    connect(ui->oneShot, &QPushButton::clicked, processor, &Processor::runFull);

    connect(ui->findTarget, &QLineEdit::editingFinished, this, [&](){
        unsigned target = ui->findTarget->text().toUInt(nullptr, 16);
        ui->memTableView->setCurrentIndex(memTable->index((target >> 4) & 0xFFF, target & 0xF));
    });
    connect(ui->runTarget, &QLineEdit::editingFinished, this, &MainWindow::runTargetUpdated);

    connect(processor, &Processor::accumulatorChanged, this, &MainWindow::accumulatorChanged);
    connect(processor, &Processor::registerBChanged, this, &MainWindow::registerBChanged);
    connect(processor, &Processor::registerCChanged, this, &MainWindow::registerCChanged);
    connect(processor, &Processor::registerDChanged, this, &MainWindow::registerDChanged);
    connect(processor, &Processor::registerEChanged, this, &MainWindow::registerEChanged);
    connect(processor, &Processor::flagsChanged, this, &MainWindow::flagsChanged);
    connect(processor, &Processor::registerHChanged, this, &MainWindow::registerHChanged);
    connect(processor, &Processor::registerLChanged, this, &MainWindow::registerLChanged);
    connect(processor, &Processor::programCounterChanged, this, &MainWindow::programCounterChanged);
    connect(processor, &Processor::stackPointerChanged, this, &MainWindow::stackPointerChanged);
    connect(processor, &Processor::MChanged, this, &MainWindow::MChanged);
    connect(processor, &Processor::serialOutput, this, &MainWindow::serialOutput);
    connect(processor, &Processor::restart7_5RequestStatusChanged, this, &MainWindow::restart7_5RequestStatusChanged);
    connect(processor, &Processor::maskRestart7_5Changed, this, &MainWindow::maskRestart7_5Changed);
    connect(processor, &Processor::maskRestart6_5Changed, this, &MainWindow::maskRestart6_5Changed);
    connect(processor, &Processor::maskRestart5_5Changed, this, &MainWindow::maskRestart5_5Changed);
    connect(processor, &Processor::interruptEnableStatusChanged, this, &MainWindow::interruptEnableStatusChanged);
    connect(processor, &Processor::interruptAcknowledgeStatusChanged, this, &MainWindow::interruptAcknowledgeStatusChanged);

    //fire events to initialize
    ui->sid->setChecked(processor->serialInputDataLatch());
    ui->trap->setChecked(processor->isTRAPRequested());
    ui->r7_5->setChecked(processor->isRestart7_5Requested());
    ui->r6_5->setChecked(processor->isRestart6_5Requested());
    ui->r5_5->setChecked(processor->isRestart5_5Requested());
    ui->intr->setChecked(processor->isInterruptRequested());
    ui->intrVec->setCurrentIndex((processor->getINTRVector() >> 3) & 7);

    accumulatorChanged();
    registerBChanged();
    registerCChanged();
    registerDChanged();
    registerEChanged();
    flagsChanged();
    registerHChanged();
    registerLChanged();
    programCounterChanged();
    stackPointerChanged();
    MChanged();
    serialOutput();
    maskRestart7_5Changed();
    maskRestart6_5Changed();
    maskRestart5_5Changed();
    interruptEnableStatusChanged();
    interruptAcknowledgeStatusChanged();

    newFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//protected
void MainWindow::closeEvent(QCloseEvent *evt) {
    //do actions here
    if(!checkUnsaved()) {evt->ignore(); return;}
    processor->haltExecution();
    QMainWindow::closeEvent(evt);
}

//private slots

#include <vector>
#include <sstream>
void MainWindow::assemble() {
    ui->statusbar->showMessage(tr("Assembling..."));
    processor->resetAll();
    assembler->in = new std::stringstream(ui->source->toPlainText().toStdString());
    ui->sourceTab->setDisabled(true); //source disabled while assembling
    ui->debugTab->setDisabled(true);
    ui->memoryTab->setDisabled(true); //memory disabled while assembling
    emit __fireAssemblerEvent();
}
void MainWindow::assemblyFinished() {
    delete assembler->in;
    ui->sourceTab->setDisabled(false);
    ui->debugTab->setDisabled(false);
    ui->memoryTab->setDisabled(false);
    if(currentDebugTableModel != emptyDebugTableModel) currentDebugTableModel->deleteLater();
    currentDebugTableModel = new DebugTableModel(this, assembler->instructions);
    if(assembler->instructions.size() > 0) ui->runTarget->setText(QString::number(assembler->instructions[0].address, 16));
    else ui->runTarget->setText("");
    runTargetUpdated();
    ui->debugTableView->setModel(currentDebugTableModel);
    ui->leftWidget->setCurrentWidget(ui->debugTab); //go to debug page if possible
    ui->rightWidget->setCurrentWidget(ui->memoryTab);
    ui->statusbar->showMessage(tr("OK."));
}
void MainWindow::runTargetUpdated() {
    unsigned target = ui->runTarget->text().toUInt(nullptr, 16);
    processor->setProgramCounter(target & 0xFFFF);
}
#include <QTextCursor>
#include <QTextBlock>
void MainWindow::assemblyError(SyntaxError ex) {
    delete assembler->in;
    ui->sourceTab->setDisabled(false);
    ui->debugTab->setDisabled(false);
    ui->memoryTab->setDisabled(false);
    /*QTextCursor cursor = ui->source->textCursor();
    if(ex.lineNumber > 0) cursor.setPosition(ui->source->document()->findBlockByLineNumber(ex.lineNumber-1).position());
    if(ex.columnNumber > 0) cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, ex.columnNumber-1);
    ui->source->setTextCursor(cursor);*/
    ui->source->setErrorLine(ex.lineNumber);
    ui->leftWidget->setCurrentWidget(ui->sourceTab);
    ui->statusbar->showMessage(constructAssemblyError(ex));
}
void MainWindow::sidToggled(bool value) {
    processor->setSerialInputLatch(value);
    ui->sid->setText(QString(value ? "1" : "0"));
    ui->sidLabel->setText(QString(value ? "1" : "0"));
}
void MainWindow::trapToggled(bool value) {processor->setTRAPRequest(value); ui->trap->setText(QString(value ? "1" : "0"));}
void MainWindow::r7_5Toggled(bool value) {
    disconnect(processor, &Processor::restart7_5RequestStatusChanged, this, &MainWindow::restart7_5RequestStatusChanged);
    processor->setRestart7_5Request(value);
    ui->r7_5->setText(QString(value ? "1" : "0"));
    ui->i7_5->setText(QString(value ? "1" : "0"));
    connect(processor, &Processor::restart7_5RequestStatusChanged, this, &MainWindow::restart7_5RequestStatusChanged);
}
void MainWindow::r6_5Toggled(bool value) {
    processor->setRestart6_5Request(value);
    ui->r6_5->setText(QString(value ? "1" : "0"));
    ui->i6_5->setText(QString(value ? "1" : "0"));
}
void MainWindow::r5_5Toggled(bool value) {
    processor->setRestart5_5Request(value);
    ui->r5_5->setText(QString(value ? "1" : "0"));
    ui->i5_5->setText(QString(value ? "1" : "0"));
}
void MainWindow::intrToggled(bool value) {processor->setInterruptRequest(value); ui->intr->setText(QString(value ? "1" : "0"));}
void MainWindow::intrVecSelected(int rstIndex) {processor->setINTRVector((rstIndex << 3) | 0xC7);}
void MainWindow::decimalEdited(const QString& text) {
    unsigned long long value = text.toULongLong(nullptr, 10);
    ui->hexadecimal->setText(QString::number(value, 16).toUpper());
    ui->binary->setText(QString::number(value, 2));
}
void MainWindow::hexadecimalEdited(const QString &text) {
    unsigned long long value = text.toULongLong(nullptr, 16);
    ui->decimal->setText(QString::number(value, 10));
    ui->binary->setText(QString::number(value, 2));
}
void MainWindow::binaryEdited(const QString &text) {
    unsigned long long value = text.toULongLong(nullptr, 2);
    ui->hexadecimal->setText(QString::number(value, 16).toUpper());
    ui->decimal->setText(QString::number(value, 10));
}
void MainWindow::fileModified(bool modified) {
    isFileModified = modified;
    if(modified && !windowTitle().startsWith(QString("*"))) {setWindowTitle(QString("*") + windowTitle());}
    else if(!modified && windowTitle().startsWith(QString("*"))) {setWindowTitle(windowTitle().right(windowTitle().length() - 1));}
}
#include <QMessageBox>
bool MainWindow::checkUnsaved() {
    if(!isFileModified) return true;
    switch(QMessageBox::question(this, tr("Save Your Changes?")
                                 , tr("Save your changes to ")
                                 + ((currentlyOpenedFile.isFile()) ? currentlyOpenedFile.absoluteFilePath() : tr("Untitled"))
                                 + tr("?")
                                 , QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes)) {
    case QMessageBox::Yes: saveFile(); return true;
    case QMessageBox::No: return true;
    case QMessageBox::Cancel:
    default: return false;
    }
}
void MainWindow::newFile() {
    if(!checkUnsaved()) return;
    currentlyOpenedFile = QFileInfo();
    ui->source->setPlainText(QString("")); ui->source->document()->setModified(false); fileModified(false);
    setWindowTitle(tr("Untitled: QTSimulator8085"));
}
void MainWindow::openFile() {
    if(!checkUnsaved()) return;
    QString name = QFileDialog::getOpenFileName(this, tr("Open Source File"), QString(),
                                                tr("8085 Assembly Source Files (*.asm);;All Files (*.*)"));
    if(name.isNull()) return; //operation canncelled.
    currentlyOpenedFile = QFileInfo(name);
    QFile file(currentlyOpenedFile.absoluteFilePath());
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    ui->source->setPlainText(QTextStream(&file).readAll()); ui->source->document()->setModified(false); fileModified(false);
    file.close();
    setWindowTitle(currentlyOpenedFile.fileName() + tr(": QTSimulator8085"));
}
void MainWindow::saveFile() {
    if(!currentlyOpenedFile.isFile()) {saveFileAs(); return;}
    QFile file(currentlyOpenedFile.absoluteFilePath());
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream(&file)<<ui->source->toPlainText();
    file.close();
    setWindowTitle(currentlyOpenedFile.fileName() + tr(": QTSimulator8085"));
    ui->source->document()->setModified(false); fileModified(false);
}
void MainWindow::saveFileAs() {
    if(currentlyOpenedFile.isFile() && !checkUnsaved()) return; //cancelled
    QString name = QFileDialog::getSaveFileName(this, tr("Save Source File"), QString(),
                                                tr("8085 Assembly Source Files (*.asm);;All Files (*.*)"));
    if(name.isNull()) return; //operation cancelled.
    currentlyOpenedFile = QFileInfo(name);
    QFile file(currentlyOpenedFile.absoluteFilePath());
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream(&file)<<ui->source->toPlainText();
    file.close();
    setWindowTitle(currentlyOpenedFile.fileName() + tr(": QTSimulator8085"));
    ui->source->document()->setModified(false); fileModified(false);
}
void MainWindow::accumulatorChanged() {
    ui->accumulatorFull->setText(getHex8(processor->getAccumulator()));
    ui->accumulator7->setText(getBinDigit(processor->getAccumulator(), 7));
    ui->accumulator6->setText(getBinDigit(processor->getAccumulator(), 6));
    ui->accumulator5->setText(getBinDigit(processor->getAccumulator(), 5));
    ui->accumulator4->setText(getBinDigit(processor->getAccumulator(), 4));
    ui->accumulator3->setText(getBinDigit(processor->getAccumulator(), 3));
    ui->accumulator2->setText(getBinDigit(processor->getAccumulator(), 2));
    ui->accumulator1->setText(getBinDigit(processor->getAccumulator(), 1));
    ui->accumulator0->setText(getBinDigit(processor->getAccumulator(), 0));
    ui->programStatusWord->setText(getHex16(processor->getProgramStatusWord()));
}
void MainWindow::registerBChanged() {
    ui->registerBFull->setText(getHex8(processor->getBRegister()));
    ui->registerB7->setText(getBinDigit(processor->getBRegister(), 7));
    ui->registerB6->setText(getBinDigit(processor->getBRegister(), 6));
    ui->registerB5->setText(getBinDigit(processor->getBRegister(), 5));
    ui->registerB4->setText(getBinDigit(processor->getBRegister(), 4));
    ui->registerB3->setText(getBinDigit(processor->getBRegister(), 3));
    ui->registerB2->setText(getBinDigit(processor->getBRegister(), 2));
    ui->registerB1->setText(getBinDigit(processor->getBRegister(), 1));
    ui->registerB0->setText(getBinDigit(processor->getBRegister(), 0));
    ui->registerBC->setText(getHex16(processor->getBCRegisterPair()));
}
void MainWindow::registerCChanged() {
    ui->registerCFull->setText(getHex8(processor->getCRegister()));
    ui->registerC7->setText(getBinDigit(processor->getCRegister(), 7));
    ui->registerC6->setText(getBinDigit(processor->getCRegister(), 6));
    ui->registerC5->setText(getBinDigit(processor->getCRegister(), 5));
    ui->registerC4->setText(getBinDigit(processor->getCRegister(), 4));
    ui->registerC3->setText(getBinDigit(processor->getCRegister(), 3));
    ui->registerC2->setText(getBinDigit(processor->getCRegister(), 2));
    ui->registerC1->setText(getBinDigit(processor->getCRegister(), 1));
    ui->registerC0->setText(getBinDigit(processor->getCRegister(), 0));
    ui->registerBC->setText(getHex16(processor->getBCRegisterPair()));
}
void MainWindow::registerDChanged() {
    ui->registerDFull->setText(getHex8(processor->getDRegister()));
    ui->registerD7->setText(getBinDigit(processor->getDRegister(), 7));
    ui->registerD6->setText(getBinDigit(processor->getDRegister(), 6));
    ui->registerD5->setText(getBinDigit(processor->getDRegister(), 5));
    ui->registerD4->setText(getBinDigit(processor->getDRegister(), 4));
    ui->registerD3->setText(getBinDigit(processor->getDRegister(), 3));
    ui->registerD2->setText(getBinDigit(processor->getDRegister(), 2));
    ui->registerD1->setText(getBinDigit(processor->getDRegister(), 1));
    ui->registerD0->setText(getBinDigit(processor->getDRegister(), 0));
    ui->registerDE->setText(getHex16(processor->getDERegisterPair()));
}
void MainWindow::registerEChanged() {
    ui->registerEFull->setText(getHex8(processor->getERegister()));
    ui->registerE7->setText(getBinDigit(processor->getERegister(), 7));
    ui->registerE6->setText(getBinDigit(processor->getERegister(), 6));
    ui->registerE5->setText(getBinDigit(processor->getERegister(), 5));
    ui->registerE4->setText(getBinDigit(processor->getERegister(), 4));
    ui->registerE3->setText(getBinDigit(processor->getERegister(), 3));
    ui->registerE2->setText(getBinDigit(processor->getERegister(), 2));
    ui->registerE1->setText(getBinDigit(processor->getERegister(), 1));
    ui->registerE0->setText(getBinDigit(processor->getERegister(), 0));
    ui->registerDE->setText(getHex16(processor->getDERegisterPair()));
}
void MainWindow::flagsChanged() {
    ui->sign->setText(getBinDigit(processor->getFlags(), 7));
    ui->zero->setText(getBinDigit(processor->getFlags(), 6));
    ui->auxiliaryCarry->setText(getBinDigit(processor->getFlags(), 4));
    ui->parity->setText(getBinDigit(processor->getFlags(), 2));
    ui->carry->setText(getBinDigit(processor->getFlags(), 0));
    ui->programStatusWord->setText(getHex16(processor->getProgramStatusWord()));
}
void MainWindow::registerHChanged() {
    ui->registerHFull->setText(getHex8(processor->getHRegister()));
    ui->registerH7->setText(getBinDigit(processor->getHRegister(), 7));
    ui->registerH6->setText(getBinDigit(processor->getHRegister(), 6));
    ui->registerH5->setText(getBinDigit(processor->getHRegister(), 5));
    ui->registerH4->setText(getBinDigit(processor->getHRegister(), 4));
    ui->registerH3->setText(getBinDigit(processor->getHRegister(), 3));
    ui->registerH2->setText(getBinDigit(processor->getHRegister(), 2));
    ui->registerH1->setText(getBinDigit(processor->getHRegister(), 1));
    ui->registerH0->setText(getBinDigit(processor->getHRegister(), 0));
    ui->registerHL->setText(getHex16(processor->getHLRegisterPair()));
}
void MainWindow::registerLChanged() {
    ui->registerLFull->setText(getHex8(processor->getLRegister()));
    ui->registerL7->setText(getBinDigit(processor->getLRegister(), 7));
    ui->registerL6->setText(getBinDigit(processor->getLRegister(), 6));
    ui->registerL5->setText(getBinDigit(processor->getLRegister(), 5));
    ui->registerL4->setText(getBinDigit(processor->getLRegister(), 4));
    ui->registerL3->setText(getBinDigit(processor->getLRegister(), 3));
    ui->registerL2->setText(getBinDigit(processor->getLRegister(), 2));
    ui->registerL1->setText(getBinDigit(processor->getLRegister(), 1));
    ui->registerL0->setText(getBinDigit(processor->getLRegister(), 0));
    ui->registerHL->setText(getHex16(processor->getHLRegisterPair()));
}
#include <algorithm>
void MainWindow::programCounterChanged() {
    ui->programCounter->setText(getHex16(processor->getProgramCounter()));
    Instruction dummy; dummy.address = processor->getProgramCounter();
    //upper_bound can also be used. Both are guaranteed to be O(log n).
    std::vector<Instruction>::iterator location = std::lower_bound(currentDebugTableModel->list.begin(),
                                                                     currentDebugTableModel->list.end(),
                                                                     dummy, InstructionAddressComparator());
    if(location == currentDebugTableModel->list.end()) currentDebugTableModel->setHighlightedIndex(-1);
    else currentDebugTableModel->setHighlightedIndex(location - currentDebugTableModel->list.begin());
}
void MainWindow::stackPointerChanged() {ui->stackPointer->setText(getHex16(processor->getStackPointer()));}
void MainWindow::MChanged() {
    ui->MFull->setText(getHex8(processor->getM()));
    ui->M7->setText(getBinDigit(processor->getM(), 7));
    ui->M6->setText(getBinDigit(processor->getM(), 6));
    ui->M5->setText(getBinDigit(processor->getM(), 5));
    ui->M4->setText(getBinDigit(processor->getM(), 4));
    ui->M3->setText(getBinDigit(processor->getM(), 3));
    ui->M2->setText(getBinDigit(processor->getM(), 2));
    ui->M1->setText(getBinDigit(processor->getM(), 1));
    ui->M0->setText(getBinDigit(processor->getM(), 0));
}
void MainWindow::serialOutput() {ui->sod->setText(QString(processor->serialOutputDataLatch() ? "1" : "0"));}
void MainWindow::restart7_5RequestStatusChanged() {ui->r7_5->setChecked(processor->isRestart7_5Requested());}
void MainWindow::maskRestart7_5Changed() {ui->m7_5->setText(QString(processor->maskRestart7_5() ? "1" : "0"));}
void MainWindow::maskRestart6_5Changed() {ui->m6_5->setText(QString(processor->maskRestart6_5() ? "1" : "0"));}
void MainWindow::maskRestart5_5Changed() {ui->m5_5->setText(QString(processor->maskRestart5_5() ? "1" : "0"));}
void MainWindow::interruptEnableStatusChanged() {ui->ie->setText(QString(processor->isInterruptEnabled() ? "1" : "0"));}
void MainWindow::interruptAcknowledgeStatusChanged() {ui->inta->setText(tr(processor->isInterruptAcknowledged() ? "Yes" : "No"));}
