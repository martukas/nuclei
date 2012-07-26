#!/bin/sh
macdeployqt nuclei.app
mkdir nuclei.app/Contents/Frameworks
cp ../../libakk/src/libakk.1.dylib nuclei.app/Contents/Frameworks/
install_name_tool -change /usr/lib/libakk.1.dylib @executable_path/../Frameworks/libakk.1.dylib nuclei.app/Contents/MacOS/nuclei
cp ../../quazip/quazip/libquazip.1.dylib nuclei.app/Contents/Frameworks/
