#include "mainwindow.h"
#include "processor.h"
#include "opcodes.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    processor_h::registerHeaderMetaTypes();
    OPCODES_H_registerHeaderMetaTypes();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
