# Build script for Arch Linux (AUR)
# Maintainer: Chirantan Nath <chirantannath@gmail.com>

pkgname=qtsimulator8085-git
gitdir=QTsimulator8085
pkgver=v0.1.1alpha
pkgrel=11
pkgdesc="An 8085 simulator developed using C++11/Qt5"
arch=('x86_64' 'i686')
url="https://github.com/chirantannath/QTsimulator8085"
license=('MIT') 
source=(git+https://github.com/chirantannath/QTsimulator8085.git)
#groups=()
depends=('qt5-base')
makedepends=('git' 'make' 'gcc' 'qt5-base')
provides=("${pkgname}")
conflicts=("${pkgname}")
replaces=("${pkgname}")
backup=()
options=()
noextract=()
md5sums=('SKIP')

#pkgver() {
#	cd $srcdir/$gitdir
#	"$(git describe --long | sed 's/\([^-]*-\)g/r\1/;s/-/./g')" 
#	printf ".%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
#}

prepare() {
	cd $srcdir/$gitdir
	git checkout tags/$pkgver	
}

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
