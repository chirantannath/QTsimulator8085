#include "mainwindow.h"
#include "processor.h"
#include "opcodes.h"
#include "assembler.h"

#include <QApplication>

#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

int main(int argc, char *argv[])
{
    processor_h::registerHeaderMetaTypes();
    OPCODES_H_registerHeaderMetaTypes();
    ASSEMBLER_H_registerHeaderMetaTypes();
#ifdef Q_OS_WIN
    qt_ntfs_permission_lookup++; //This enables file stat check on NTFS. This is disabled by default on Qt.
#endif
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
