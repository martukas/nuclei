#!/bin/sh
macdeployqt Nuclei.app
chmod 755 Nuclei.app/Contents/Frameworks/libcrypto.1.0.0.dylib
chmod 755 Nuclei.app/Contents/Frameworks/libssl.1.0.0.dylib
cp -r /opt/local/lib/Resources/qt_menu.nib Nuclei.app/Contents/Resources
install_name_tool -change /usr/local/lib/libakk.2.dylib @executable_path/../Frameworks/libakk.2.dylib Nuclei.app/Contents/MacOS/nuclei
cp /usr/local/lib/libakk.2.dylib Nuclei.app/Contents/Frameworks/
install_name_tool -id @executable_path/../Frameworks/libakk.2.dylib Nuclei.app/Contents/Frameworks/libakk.2.dylib
cp /opt/local/lib/libgsl.0.dylib Nuclei.app/Contents/Frameworks/
install_name_tool -id @executable_path/../Frameworks/libgsl.0.dylib Nuclei.app/Contents/Frameworks/libgsl.0.dylib
install_name_tool -change /opt/local/lib/libgsl.0.dylib @executable_path/../Frameworks/libgsl.0.dylib Nuclei.app/Contents/Frameworks/libakk.2.dylib
install_name_tool -change /opt/local/lib/libcrypto.1.0.0.dylib @executable_path/../Frameworks/libcrypto.1.0.0.dylib Nuclei.app/Contents/Frameworks/libcrypto.1.0.0.dylib
install_name_tool -change /opt/local/lib/libz.1.dylib @executable_path/../Frameworks/libz.1.dylib Nuclei.app/Contents/Frameworks/libcrypto.1.0.0.dylib
install_name_tool -change /opt/local/lib/libcrypto.1.0.0.dylib @executable_path/../Frameworks/libcrypto.1.0.0.dylib Nuclei.app/Contents/Frameworks/libssl.1.0.0.dylib
install_name_tool -change /opt/local/lib/libssl.1.0.0.dylib @executable_path/../Frameworks/libssl.1.0.0.dylib Nuclei.app/Contents/Frameworks/libssl.1.0.0.dylib
install_name_tool -change /opt/local/lib/libz.1.dylib @executable_path/../Frameworks/libz.1.dylib Nuclei.app/Contents/Frameworks/libssl.1.0.0.dylib

sed -e "s/com.yourcompany/de.uni-goettingen/g" Nuclei.app/Contents/Info.plist > tmp.plist
mv tmp.plist Nuclei.app/Contents/Info.plist
hdiutil create ./Nuclei.dmg -srcfolder ./Nuclei.app/ -ov
