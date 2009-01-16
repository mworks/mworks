#!/bin/sh

if [ -z $1 ]
    then
    echo "usage: "
    echo "  uploadPackages.sh <VERSION_NUMBER>"
    exit -1;
fi

MONKEYWORKS_VERSION_NUMBER_TEMP=$1


WEBSERVER_DIRECTORY=admin@dicarloserver2.mit.edu:/Library/WebServer/Documents/MonkeyWorks/
DOWNLOADS_HTML_PATH=$WEBSERVER_DIRECTORY/MonkeyWorks/


if [ -e Archive/MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP.mpkg.zip ]
    then
    scp Archive/MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP.mpkg.zip $WEBSERVER_DIRECTORY
    
    if [ $MONKEYWORKS_VERSION_NUMBER_TEMP != "NIGHTLY" ]
    ## auto-generate the downloads file
	then
	rm -f Downloads.html
	perl template_file.pl Downloads.html.template VERSION_NUMBER $MONKEYWORKS_VERSION_NUMBER_TEMP Downloads.html
	scp edu.mit.MonkeyWorks*.plist  $WEBSERVER_DIRECTORY
	scp Downloads.html $DOWNLOADS_HTML_PATH
    fi
else
    echo "Archive/MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP.mpkg.zip doesn't exist"
    exit -1
fi


