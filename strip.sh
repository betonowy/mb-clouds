#!/bin/sh

# strips/compresses executable to reduce it's size
# requires strip (part of GNU binary utilities)
# requires upx (executable compression tool)
# optional sstrip (similar to strip, but more aggressive - easily available on Linux)

# check if all tools are available

FAIL=0
NOSTRIP=0
NOSSTRIP=0
NOUPX=0

echo "Checking for strip, sstrip and upx"

which strip > /dev/null 2> /dev/null   # basic stripper
RES=$?
if [ $RES -eq 1 ]; then FAIL=1; NOSTRIP=1; fi

which sstrip > /dev/null 2> /dev/null  # aggressive stripper (linux only)
RES=$?
if [ $RES -eq 1 ]; then NOSSTRIP=1; fi

which upx > /dev/null 2> /dev/null     # executable compression
RES=$?
if [ $RES -eq 1 ]; then FAIL=1; NOUPX=1; fi


echo
# if those are unavailable there's nothing to do here
if [ $FAIL -eq 1 ]; then
    echo "At least one necessary tool is unavailable."
fi


# recognize platform
if [ -f mb_clouds.exe ]; then
    echo "It's a Windows build"
    EXECUTABLE="mb_clouds.exe"
elif [ -f mb_clouds ]; then
    echo "It's a Linux build"
    EXECUTABLE="mb_clouds"
else
    echo "Couldn't find executable. Bailing out."
    exit 1
fi


echo
echo "Downsizing $EXECUTABLE"
# remember size before stripping and compression
PRESTRIP=$(ls -lh $EXECUTABLE | awk '{printf $5}')
PRESTRIPBYTES=$(stat -c%s $EXECUTABLE)


# strip
if [ $NOSTRIP -eq 0 ]; then
    strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment \
          --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag \
          $EXECUTABLE
else
    echo "Strip not available"
fi

POSTSTRIP=$(ls -lh $EXECUTABLE | awk '{printf $5}')
POSTSTRIPBYTES=$(stat -c%s $EXECUTABLE)


# aggressive strip
if [ $NOSSTRIP -eq 0 ]; then
    sstrip -z $EXECUTABLE
else
    echo "Sstrip not available"
fi

POSTSSTRIP=$(ls -lh $EXECUTABLE | awk '{printf $5}')
POSTSSTRIPBYTES=$(stat -c%s $EXECUTABLE)


# lzma compression
if [ $NOUPX -eq 0 ]; then
    upx --lzma $EXECUTABLE
else
    echo "Upx not available"
fi

POSTUPX=$(ls -lh $EXECUTABLE | awk '{printf $5}')
POSTUPXBYTES=$(stat -c%s $EXECUTABLE)


# post compression aggressive strip
if [ $NOSSTRIP -eq 0 ]; then
    sstrip -z $EXECUTABLE
fi

POSTUPXSSTRIP=$(ls -lh $EXECUTABLE | awk '{printf $5}')
POSTUPXSSTRIPBYTES=$(stat -c%s $EXECUTABLE)

if [ $NOSTRIP -eq 1 ]; then
    NOSTRIP="Strip is not available"
else
    NOSTRIP=""
fi

if [ $NOSSTRIP -eq 1 ]; then
    NOSSTRIP="Sstrip is not available. That's okay."
else
    NOSSTRIP=""
fi

if [ $NOUPX -eq 1 ]; then
    NOUPX="Upx is not available"
else
    NOUPX=""
fi


# print results
echo
printf "| %-20s | %9s | %7s |\n" "Stage" "KiB" "bytes"
printf "%s\n" "----------------------------------------------"
printf "| %-20s | %9s | %7s |\n" "Before" "$PRESTRIP" "$PRESTRIPBYTES"
printf "| %-20s | %9s | %7s | %s\n" "After strip" "$POSTSTRIP" "$POSTSTRIPBYTES" "$NOSTRIP"
printf "| %-20s | %9s | %7s | %s\n" "After sstrip" "$POSTSSTRIP" "$POSTSSTRIPBYTES" "$NOSSTRIP"
printf "| %-20s | %9s | %7s | %s\n" "After upx" "$POSTUPX" "$POSTUPXBYTES" "$NOUPX"
printf "| %-20s | %9s | %7s | %s\n" "After upx + sstrip" "$POSTUPXSSTRIP" "$POSTUPXSSTRIPBYTES" "$NOSSTRIP"
echo
