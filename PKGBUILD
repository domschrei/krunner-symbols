# Maintainer: Dominik Schreiber <dev@dominikschreiber.de>
pkgname=plasma5-runners-symbols
pkgver=1.0
pkgrel=1
pkgdesc="A little krunner plugin (Plasma 5) to retrieve unicode symbols, or any other string, based on a corresponding keyword"
arch=('any')
url="https://github.com/domschrei/krunner-symbols"
license=('GPL')
groups=()
depends=()
makedepends=('extra-cmake-modules')
optdepends=()
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=("https://raw.githubusercontent.com/domschrei/krunner-symbols/master/$pkgname-$pkgver.tar.gz")
noextract=()

prepare() {
  mkdir -p build
}

build() {
  cd build
  echo "Building $pkgname ..."
  cmake ../${pkgname}-${pkgver} -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` -DCMAKE_BUILD_TYPE=Release
  make
  echo "End of $pkgname build."
}

package() {
  echo "Installing $pkgname ..."
  cd build
  sudo make install
  cd ..
  cp ../${pkgname}-${pkgver}/krunner-symbols $HOME/.config/krunner-symbols
  echo "End of $pkgname installation."
}

md5sums=('3b63f5fcf1b680da6d41498f82ef454e')
