#!/bin/sh
macdeployqt Nuclei.app
cp -r /opt/local/lib/Resources/qt_menu.nib Nuclei.app/Contents/Resources
install_name_tool -change /usr/lib/libakk.1.dylib @executable_path/../Frameworks/libakk.1.dylib Nuclei.app/Contents/MacOS/nuclei
sed -e "s/com.yourcompany/de.uni-goettingen/g" Nuclei.app/Contents/Info.plist > tmp.plist
mv tmp.plist Nuclei.app/Contents/Info.plist
rm tmp.plist
hdiutil create ./Nuclei.dmg -srcfolder ./Nuclei.app/ -ov
