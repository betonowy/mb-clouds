#!/bin/bash

rm -r package

mkdir -p package/mb_clouds/bin &&
  ./build.sh &&
  cp build/mb_clouds package/mb_clouds/bin/mb_clouds &&
  cp -r res package/mb_clouds/

FINDLIBS="$(ldd build/mb_clouds | grep -i mingw | awk '{printf "%s ", $1}')"

for LIB in $FINDLIBS; do
    FILELOCATION=$(which $LIB)
    cp -v $FILELOCATION package/mb_clouds/bin/
done

cd package || exit 1

zip -rq mb_clouds.zip mb_clouds
