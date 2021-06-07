#include <QChar>
#include "mainwindow.h"
#include "ui_mainwindow.h"

const char HEX_DIGITS[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '\0'};
QString getHex8(data8_t value) {
    QString text; text.resize(2);
    text[0] = QChar(HEX_DIGITS[(value >> 4) & 0xF]);
    text[1] = QChar(HEX_DIGITS[value & 0xF]);
    return text;
}
QString getHex16(data16_t value) {
    QString text; text.resize(4);
    text[0] = QChar(HEX_DIGITS[(value >> 12) & 0xF]);
    text[1] = QChar(HEX_DIGITS[(value >> 8) & 0xF]);
    text[2] = QChar(HEX_DIGITS[(value >> 4) & 0xF]);
    text[3] = QChar(HEX_DIGITS[value & 0xF]);    
    return text;
}
QString getBinDigit(data8_t value, int position) {
    const QChar bit[1] = {(value & (1 << position)) ? QChar('1') : QChar('0')};
    return QString(bit, 1);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    processor = new Processor(this);
    
    //connect events
    connect(processor, &Processor::accumulatorChanged, this, &MainWindow::accumulatorChanged);
    connect(processor, &Processor::registerBChanged, this, &MainWindow::registerBChanged);
    connect(processor, &Processor::registerCChanged, this, &MainWindow::registerCChanged);
    connect(processor, &Processor::registerDChanged, this, &MainWindow::registerDChanged);
    connect(processor, &Processor::registerEChanged, this, &MainWindow::registerEChanged);
    connect(processor, &Processor::flagsChanged, this, &MainWindow::flagsChanged);
    connect(processor, &Processor::registerHChanged, this, &MainWindow::registerHChanged);
    connect(processor, &Processor::registerLChanged, this, &MainWindow::registerLChanged);
    
    //fire events to initialize
    accumulatorChanged();
    registerBChanged();
    registerCChanged();
    registerDChanged();
    registerEChanged();
    flagsChanged();
    registerHChanged();
    registerLChanged();
}

MainWindow::~MainWindow()
{
    delete ui;
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
