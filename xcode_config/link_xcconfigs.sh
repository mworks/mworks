#!/bin/sh

echo "It doesn't look like Xcode will you edit a linked file (symbolic or hard); oh well..."
exit

APP_SUPPORT_DIR="/Library/Application Support/MWorks/Developer/Xcode"

for filename in *.xcconfig
do
  # unfortunately, Xcode refuses to deal with a symbolic link
  echo "Hard linking " $filename
  ln $filename "$APP_SUPPORT_DIR/$filename"
done;

ln WARNING.txt "$APP_SUPPORT_DIR/WARNING.txt"

