#!/bin/bash
read -r -d '' PRO_FILE <<'EOF'
TEMPLATE=subdirs
SUBDIRS=libakk nuclei
EOF

read -r -d '' README_LINUX <<'EOF'
Installation instructions for linux distributions:

Simply execute the following commands from the directory containing this file:
qmake PREFIX=/usr/local
make
sudo make install

If problems occur do not hesitate to file a bug on the project page http://nuclei.sf.net
EOF

set -e

while getopts ":n:" opt; do
    case $opt in
        n)
            CODEDIR=nuclei_$OPTARG
            mkdir $CODEDIR
            svn export http://svn.code.sf.net/p/nuclei/svn/tags/$OPTARG $CODEDIR/nuclei

            # \todo MUST BE FIXED as soon as libakk is checked in at sf.net!!
            cp -r ../libakk $CODEDIR/
            echo "$PRO_FILE" > $CODEDIR/nuclei.pro
            echo "$README_LINUX" > $CODEDIR/README.linux

            tar -cjf nuclei_$OPTARG.tar.bz2 $CODEDIR
            rm -r nuclei_$OPTARG

            echo "Successfully created nuclei_$OPTARG.tar.bz2."

            exit 0
            ;;
        \?)
            ;;
        :)
            ;;
    esac
done

echo "Missing Parameter: You need to supply a version like this: makeTarBall.sh -n 2.3" >&2
exit 1