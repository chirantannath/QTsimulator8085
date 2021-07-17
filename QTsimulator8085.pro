QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050000    # disables all the APIs deprecated before Qt 5.0.0

SOURCES += \
    assembler.cpp \
    commdefs.cpp \
    debugtable.cpp \
    editor.cpp \
    iomodel.cpp \
    main.cpp \
    mainwindow.cpp \
    memorymodel.cpp \
    opcodes.cpp \
    processor.cpp \
    syntaxhighlighter.cpp

HEADERS += \
    assembler.h \
    commdefs.h \
    debugtable.h \
    editor.h \
    iomodel.h \
    mainwindow.h \
    memorymodel.h \
    opcodes.h \
    processor.h \
    syntaxhighlighter.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
