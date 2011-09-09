#!/bin/sh

APP_SUPPORT_DIR="/Library/Application Support/MWorks/Developer/Xcode/"

mkdir -p "$APP_SUPPORT_DIR"
cp *.xcconfig WARNING.txt "$APP_SUPPORT_DIR"
