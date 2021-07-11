#include "mainwindow.h"
#include "commdefs.h"
#include "opcodes.h"
#include "assembler.h"

#include <QApplication>

#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

//void tests();

int main(int argc, char *argv[])
{
    commdefs_h::registerHeaderMetaTypes();
    OPCODES_H_registerHeaderMetaTypes();
    ASSEMBLER_H_registerHeaderMetaTypes();

    //TODO: remove the following code in release version
    //tests();

#ifdef Q_OS_WIN
    qt_ntfs_permission_lookup++; //This enables file stat check on NTFS. This is disabled by default on Qt.
#endif
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
/*
//TODO: remove the following code in release
#include <sstream>
#include <fstream>
#include <iostream>
void tests() {
    std::stringstream in("STAX D");
    std::ifstream file("/home/chiru/8085 Assembly Projects/multiply.asm");
    Tokenizer tok(file);
    LineTranslator tr(tok);
    try {
        do {
            Instruction i = tr.translateOneLine();
            std::cout<<i.lineNumber<<": ";
            if(i.isEmpty()) std::cout<<"Comment line";
            else std::cout<<i.label<<"\t"<<i.code->name<<"\t"<<i.toLabel<<"\t"<<std::hex<<i.operand;
            std::cout<<std::endl;
        } while (tok.ttype != Tokenizer::END_OF_FILE);
    } catch (SyntaxError ex) {
        std::cerr<<ex.what<<" at line "<<ex.lineNumber<<" and col "<<ex.columnNumber<<std::endl;
    }

    //std::cout<<std::hex<<(unsigned)tok.peekNextChar()<<std::endl;
}
*/
