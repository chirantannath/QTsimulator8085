# Build script for Arch Linux (AUR)
# Maintainer: Chirantan Nath <chirantannath@gmail.com>

pkgname=qtsimulator8085-git
gitdir=QTsimulator8085
pkgver=.24.8116ff8
pkgrel=1
pkgdesc="An 8085 simulator developed using C++/Qt"
arch=('x86_64' 'i686')
url="https://github.com/chirantannath/QTsimulator8085"
license=('MIT') 
source=(git+https://github.com/chirantannath/QTsimulator8085.git)
#groups=()
depends=('qt5-base' 'qt5-multimedia')
makedepends=('git' 'make' 'gcc' 'qt5-base' 'qt5-multimedia')
provides=("${pkgname}")
conflicts=("${pkgname}")
replaces=("${pkgname}")
backup=()
options=()
noextract=()
md5sums=('SKIP')

pkgver() {
	cd $srcdir/$gitdir
#	"$(git describe --long | sed 's/\([^-]*-\)g/r\1/;s/-/./g')" 
	printf ".%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

#prepare() {
#	
#}

build() {
	cd $srcdir/$gitdir
	qmake "CONFIG+=release"
	make -j
	make -j clean
}

#check() {
#
#}

package() {
	cd $srcdir/$gitdir
	mkdir $pkgdir/usr
	mkdir $pkgdir/usr/bin
	install -Dm555 QTsimulator8085 $pkgdir/usr/bin/$pkgname
}