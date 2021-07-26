#include "mainwindow.h"
#include "commdefs.h"
#include "opcodes.h"
#include "assembler.h"

#include <QApplication>

#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

int main(int argc, char *argv[])
{
    commdefs_h::registerHeaderMetaTypes(); //Register some types for interop with Qt.
    OPCODES_H_registerHeaderMetaTypes();
    ASSEMBLER_H_registerHeaderMetaTypes();

//Enable file stat check if on Windows
#ifdef Q_OS_WIN
    qt_ntfs_permission_lookup++; //This enables file stat check on NTFS. This is disabled by default on Qt.
#endif

    QApplication a(argc, argv); //create application
    QApplication::setApplicationName("QTsimulator8085");
    QApplication::setApplicationDisplayName("QTsimulator8085");
    QApplication::setApplicationVersion("v0.2.0alpha"); //Try replicating Git tag name
    QApplication::setOrganizationName("Jadavpur University"); //I'm out of options LOL
    QApplication::setOrganizationDomain("jaduniv.edu.in");
    MainWindow w; //create main window
    w.show(); //show main window
    return a.exec(); //enter GUI event loop
}
