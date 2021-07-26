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
	- C++ compiler which supports at least the ISO C++ 2011 standard. QMake itself should require this as a dependency. (On Windows, either MinGW or MSVC can be used. On a side note, Windows 10 Insider builds and Windows 11 should also include a command-line package manager.)
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
8085 program counter register is pointing to (and this is the instruction which will be executed next by this simulator). The program counter is automatically set to the least possible address of the instructions given in your source code.
