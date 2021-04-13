#include "mainwindow.h"
#include "processor.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    processor_h::registerHeaderMetaTypes();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
