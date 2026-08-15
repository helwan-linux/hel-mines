# Maintainer: Saeed Badreldin <helwanlinux@gmail.com>
pkgname=hel-mines
pkgver=1.1
pkgrel=2
pkgdesc="Professional Minesweeper with Data Decay mechanics for Helwan Linux"
arch=('x86_64')
url="https://github.com/helwan-linux/hel-mines"
license=('GPL3')
depends=('gtk3' 'gdk-pixbuf2')
makedepends=('gcc' 'pkgconf' 'git')
source=("git+${url}.git")
sha256sums=('SKIP')

build() {
  # الدخول للمجلد الفرعي Mines الموجود داخل المستودع 
  cd "${srcdir}/${pkgname}/Mines"
  
  # تجميع البرنامج باستخدام المكتبات المطلوبة 
  gcc main.c logic.c ui.c -o ${pkgname} $(pkg-config --cflags --libs gtk+-3.0 gdk-pixbuf-2.0)
}

package() {
  # الدخول للمجلد الفرعي Mines لتثبيت الملفات منه 
  cd "${srcdir}/${pkgname}/Mines"

  # 1. تثبيت الملف التنفيذي
  install -Dm755 "${pkgname}" "${pkgdir}/usr/bin/${pkgname}"

  # 2. تثبيت الأيقونة
  install -Dm644 "icon.png" "${pkgdir}/usr/share/pixmaps/${pkgname}.png"

  # 3. تثبيت ملف الـ Desktop
  install -Dm644 "helwan-mines.desktop" "${pkgdir}/usr/share/applications/${pkgname}.desktop"

  # 4. تثبيت ملفات الثيمات الثلاثة (Classic, Nordic, Matrix)
  install -Dm644 "style_classic.css" "${pkgdir}/usr/share/${pkgname}/style_classic.css"
  install -Dm644 "style_nord.css" "${pkgdir}/usr/share/${pkgname}/style_nord.css"
  install -Dm644 "style_matrix.css" "${pkgdir}/usr/share/${pkgname}/style_matrix.css"
}
