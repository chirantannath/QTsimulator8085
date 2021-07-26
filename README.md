# QTsimulator8085

This is an 8085 simulator developed using C++11/Qt5.

## Screenshot

All screenshots where taken on an x86-64 based Manjaro Linux system running Openbox as the X11 window manager.

![Startup screenshot on Openbox/Manjaro](/assets/screenshots/startup.png "Startup screenshot on Openbox/Manjaro")

## Installation instructions

Only building from source is supported as of now.

For Arch Linux users, a PKGBUILD is included for convenience. Such users need only download/copy the PKGBUILD and run 
	makepkg -si
to install the application. (Do not clone the whole repository in this case because *makepkg* will clone this repository anyway.)

### Dependencies

- Qt libraries and the QMake utility. The way to install Qt libraries depends on platform. Consult the Qt website for details. Often the OS package manager will provide Qt libraries for installation (for example, 'qt5-base' on Arch Linux or 'qt5-default' and 'qt5-qmake' on Debian).
	- C++ compiler which supports at least the ISO C++ 2011 standard. QMake itself should require this as a dependency. (On Windows, either MinGW or MSVC can be used. On a side note, as of the time of writing this README, Windows 10 Insider builds and Windows 11 should also include
a command-line package manager.)
	- Make utility. This is labelled 'make' on most platforms. (On Windows with MinGW, 'mingw32-make' is used instead.)
- An operating system which has GUI support in place. (for example, on Linux/UNIX this means you have the X11 libraries and the X Window System server up and running. As of the time of writing this README, Wayland is not supported by Qt.)
	 
Only the Qt libraries (not QMake), GUI support, and any runtime libraries linked by the compiler system will be required during runtime.

When the above buildtime utilities are in place; clone/download this repository to a directory of your choice and run the following commands through your terminal or shell (assuming Bash/Zsh/Powershell/CMD):

	cd path_to_git_repo
	qmake "CONFIG+=release"
	make

Replace the last command with the Make utility command present in your system. Upon successful execution a binary named 'QTsimulator8085' must be present in the directory where the repository was cloned/downloaded.

## Short Usage Guide

This assumes a basic understanding of (assembly) programming in the 8085 as a prerequisite. This application provides a 64KiB buffer to model 8085 memory; and also assumes I/O ports to be memory-mapped (to a separate 256B buffer).

### Source tab

![Editing source](/assets/screenshots/source.png "Editing source")

Edit your 8085 assembly source code here. Basic syntax highlighting is supported. Click on the "Assemble" button or press F9 to assemble your source code. By default assembled instructions are placed starting from 8085 address 0 (unless you specifically instruct the assembler not to
do so; see below).

#### Assembly source syntax

Mnemonics as specified by Intel (INX, STA, RIM, etc.) are supported. *Everything is case insensitive* (even labels you can JMP or CALL).

Two pseudocode instructions are supported as of now:

	# ORG 2400H
	;.....
	# DATA 1000H 12H 23H FFH

Spaces between the hash sign and the pseudocode mnemonics are optional. **ORG** signifies that the following instructions (uptil another **ORG** statement) are to be placed starting from the given 8085 memory location (0x2400 in the given example).
**DATA** instructs the assembler to place some data bytes starting from the given address (ignoring any previous **ORG** statement). In the given example, 0x12 is placed at 8085 memory location 0x1000, 0x23 at 0x1001 and 0xFF at 0x1002.

### Assembled Code tab

![Assembled code](/assets/screenshots/assembled.png "Assembled code")

Your instructions (the ones that actually code for any action) are displayed in the table as shown and are sorted *according to their addresses in 8085 memory* (*not* in in the order in which they are present in the assembly source code). The highlighted row indicates where the 
8085 program counter register is pointing to (and this is the instruction which will be executed next by this simulator). The program counter is automatically set to the least possible address of the instructions given in your source code. If the address pointed to by the program
counter is not present in the table; no row is highlighted. The column headers should be self-explanatory; the B1, B2 and B3 fields show the opcode/operand values at byte 1, byte 2 and byte 3 of the instruction in hexadecimal.

The "Run Target" field accepts a 16-bit unsigned integer in hexadecimal. Input here the address from which you want execution to start. Hitting Enter/Return on this field immediately sets the 8085 program counter register to your entered address.

The "Step" button executes the instruction currently pointed to by the 8085 program counter register; advances the program counter (jumps if a jumping instruction is executed) and stops. The inbuilt processor still executes even if the instruction pointed to by the program counter
does not exist in the "Assembled Code" table (see "Memory" tab). 

"Run From Address in PC" starts continuous execution (imagine clicking on "Step" repeatedly) from the address pointed to by the current value in the program counter register. Execution should stop when the processor encounters the HLT instruction (0x76) in memory or if it is halted
externally (by clicking on "Halt"). "Run From Run Target" is similar except the program counter register is moved to the address present in the "Run Target" field before starting.

### Processor tab

![Processor tab](/assets/screenshots/processor.png "Processor tab")

This is a view of the inbuilt 8085 processor internals. Most of the data shown should be self-explanatory. The values displayed immediately below SID, TRAP, etc. (just above "Value Read by RIM") are clickable toggle buttons and can be used to send signals to the inbuilt processor
while running.

### Memory tab

![Memory tab](/assets/screenshots/memory.png "Memory tab")

This is an editable view of the 64KiB memory buffer attached to the internal 8085 processor. Each row corresponds to a 16-byte group (memory at address 0x243D is found at row 2430 and column D). Each cell is editable (select and type or double-click and type). 
A cell is highlighted: 

- Red if that location is currently pointed to by the program counter register.
- Grey-ish if that location is currently pointed to by the HL register pair (pseudo-register M).
- Blue-violet if that location is currently pointed to by the stack pointer register.
- Green if that location is currently pointed to by the BC register pair.
- Yellow if that location is currently pointed to by the DE register pair.

In case if a cell is pointed to by multiple registers; the above order is followed as a priority order for highlighting that cell.

Enter some address and press Enter/Return while typing in the "Find Address" field. The required address should be selected in the table. "Find PC", "Find HL" etc. make the cell selection jump to the location pointed to by the respective registers. If you check "Follow Program Counter",
the cell selection follows the address pointed to by the program counter register even if the said register changes its value (while executing instructions).

### I/O Ports tab

This editable view is very similar to the Memory tab view discussed previously. Each cell is similarly editable.

## (Very) Short Programming Guide

This is to help you get started on *understanging* this source code so that you can make your own modifications and improvements. You need the following prerequisites for this:

- How to program the 8085 and how the 8085 behaves to various internal and external conditions
- C++; including a knowledge of STL, pointers, pointers to functions, and lambda expressions
- The Qt C++ Library and API, including how to build Qt applications, usage of QMake, UIC and MOC, Qt GUI and Widgets, and the Qt signal-slot mechanism ([Qt Documentation](https://doc.qt.io/ Qt Documentation) helped me a lot in this regard)
- QT Creator (although not an absolute; this should ease the development process for this project. In the very least, use QT Designer or a similar suite to display the Qt UI files.)

Each symbol defined in the C++ source and header files contains a short description of its use. You will want to look at the following files (recommended in the given order):

1.  commdefs.h and commdefs.cpp. These have common definitions required by all other files.
2.  opcodes.h and opcodes.cpp. These have definitions (hardcoded data) for opcode mnemonics (mnemonic, short description, opcode value and number of bytes required).
3.  processor.h and processor.cpp. These have the code for the Processor class which models an 8085 processor. In particular, note the use of lambda expressions and STL features from this point onward. Do not be intimidated by the size of processor.cpp; the constructor is huge because
we needed to code for all 246 opcodes understood by the 8085.
4.  assembler.h and assembler.cpp. These have code that assemble 8085 source code into the memory of the Processor class mentioned above and also generate debugging information (the content for the "Assembled Code" tab).
5.  syntaxhighlighter.h and syntaxhighlighter.cpp. These have the code for the SyntaxHighlighter class which provides syntax highlighting to your 8085 source code.
6.  editor.h and editor.cpp. These have the code for the Editor and LineNumberArea classes which provide source code editing and line numbering respectively. (Also look at the [Qt Code Editor Example](https://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html "Qt Code Editor Example").)
7.  memorymodel.h, memorymodel.cpp, iomodel.h, and iomodel.cpp. These have the code for MemoryTableModel and IOTableModel classes which supply table models for QTableView. MemoryTableModel models the 64KiB 8085 memory buffer and IOTableModel models 256 I/O ports respectively.
8.  debugtable.h and debugtable.cpp. These have the code for the DebugTableModel class which displays debugging information (the table in the "Assembled Code" tab).
9.  mainwindow.ui, mainwindow.h and mainwindow.cpp. These have the UI definitions and code for the MainWindow class which displays the main window and integrates the various classes and functions in the previously mentioned files into a GUI interface.
10. finddialog.ui, finddialog.h and finddialog.cpp. These have the UI definitions and code for the FindDialog class; a classic "Find and Replace" dialog and functionality for the source code editor (because Qt doesn't supply one).
11. main.cpp. Entry point of this application. Performs initialization steps before creating an instance of the MainWindow class and then initializes MainWindow.
