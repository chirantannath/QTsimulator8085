# QTsimulator8085

This is an 8085 simulator developed using C++11/Qt5.

## Screenshots

![Startup screenshot on Openbox/Manjaro](/assets/screenshots/startup.png)

## Installation instructions

Only building from source is supported as of now.

For Arch Linux users, a PKGBUILD is included for convenience. Such users need only download/copy the PKGBUILD and run 
	makepkg -si
to install the application. (Do not clone the whole repository in this case because _makepkg_ will clone this repository anyway.)

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

