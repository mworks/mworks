#!/bin/sh

if [ -z $1 ]
    then
    echo "usage: "
    echo "  buildAllPackages.sh <VERSION_NUMBER>"
    exit -1;
fi

MONKEYWORKS_VERSION_NUMBER_TEMP=$1


echo "Packaging version number $MONKEYWORKS_VERSION_NUMBER_TEMP"
echo "Building MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP..."
echo

PACKAGE_MAKER=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker

MONKEYWORKSINSTALL_DIR=`dirname $0`
cd $MONKEYWORKSINSTALL_DIR


#INSTALL_DIR=/tmp/MonkeyWorksInstall
INSTALL_DIR=$MONKEYWORKSINSTALL_DIR/MonkeyWorksInstall
APPS_INSTALL_DIR=$INSTALL_DIR/Apps
APP_SUPPORT_INSTALL_DIR=$INSTALL_DIR/Application_Support
RESOURCES_INSTALL_DIR=$INSTALL_DIR/Resources
FRAMEWORK_INSTALL_DIR=$INSTALL_DIR/Frameworks
SUPPORT_INSTALL_DIR=$INSTALL_DIR/Support
DEVELOPER_INSTALL_DIR=$SUPPORT_INSTALL_DIR/Developer/SDKs/MacOSX10.5.sdk

echo "removing existing install directory: $INSTALL_DIR"
chmod -R 777 $INSTALL_DIR
rm -rf $INSTALL_DIR

echo "creating install directory structure"
echo "  creating: $INSTALL_DIR/..."
mkdir -p $INSTALL_DIR
echo "  creating: $APPS_INSTALL_DIR/..."
mkdir -p $APPS_INSTALL_DIR
echo "  creating: $APP_SUPPORT_INSTALL_DIR/..."
mkdir -p $APP_SUPPORT_INSTALL_DIR
echo "  creating: $RESOURCES_INSTALL_DIR/..."
mkdir -p $RESOURCES_INSTALL_DIR
echo "  creating: $FRAMEWORK_INSTALL_DIR/..."
mkdir -p $FRAMEWORK_INSTALL_DIR
echo "  creating: $SUPPORT_INSTALL_DIR/..."
mkdir -p $SUPPORT_INSTALL_DIR
echo "  creating: $DEVELOPER_INSTALL_DIR/..."
mkdir -p $DEVELOPER_INSTALL_DIR
echo "  creating: $DEVELOPER_INSTALL_DIR/Library/..."
mkdir -p $DEVELOPER_INSTALL_DIR/Library
echo "  creating: $DEVELOPER_INSTALL_DIR/System/..."
mkdir -p $DEVELOPER_INSTALL_DIR/System
echo "  creating: $DEVELOPER_INSTALL_DIR/System/Library/..."
mkdir -p $DEVELOPER_INSTALL_DIR/System/Library

echo "copying applications to install directory"
cp -rf /Applications/MonkeyWorksServer.app $APPS_INSTALL_DIR
cp -rf /Applications/MWClient.app $APPS_INSTALL_DIR
cp -rf /Applications/MWEditor.app $APPS_INSTALL_DIR


echo "copying frameworks to install directory"
cp -rf /Library/Frameworks/Sparkle.framework $FRAMEWORK_INSTALL_DIR
cp -rf /Library/Frameworks/MonkeyWorksCore.framework $FRAMEWORK_INSTALL_DIR
cp -rf /Library/Frameworks/MonkeyWorksCocoa.framework $FRAMEWORK_INSTALL_DIR

echo "copying MonkeyWorks library to install directory"
cp -rf /Library/MonkeyWorks $RESOURCES_INSTALL_DIR
INSTALLER_DATE=`date "+%y%m%d-%H%M"`
# add a info file to the resources directory
printf "MonkeyWorks Installer - $MONKEYWORKS_VERSION_NUMBER_TEMP\nBuilt on: $INSTALLER_DATE\nby: `whoami`\n" > $RESOURCES_INSTALL_DIR/MonkeyWorks/local/MonkeyWorksInstaller.info

chmod -R 777 $RESOURCES_INSTALL_DIR/MonkeyWorks/Experiments
chmod -R 777 $RESOURCES_INSTALL_DIR/MonkeyWorks/DataFiles
rm -f $RESOURCES_INSTALL_DIR/MonkeyWorks/local/setup_variables.xml
rm -rf $RESOURCES_INSTALL_DIR/MonkeyWorks/tests
rm -rf $RESOURCES_INSTALL_DIR/MonkeyWorks/ExampleExperiments
cp -r $MONKEYWORKSINSTALL_DIR/../MonkeyWorksExperimentXML/Experiments/ExampleExperiments $RESOURCES_INSTALL_DIR/MonkeyWorks
rm -rf `find $RESOURCES_INSTALL_DIR/MonkeyWorks/ExampleExperiments -name ".svn" -type d`

echo "copying Application Support files to install directory"
cp -rf /Library/Application\ Support/NewClient $APP_SUPPORT_INSTALL_DIR
cp -rf /Library/Application\ Support/NewEditor $APP_SUPPORT_INSTALL_DIR

echo "creating symbolic links for Development"
#ln -s /Library/MonkeyWorks $DEVELOPER_INSTALL_DIR/Library/MonkeyWorks
#ln -s /Library/Frameworks $DEVELOPER_INSTALL_DIR/Library/Frameworks


echo "Updating version plists..."

# get the plist files out of the apps before we package them
#cp -f $APPS_INSTALL_DIR/MonkeyWorksServer.app/Contents/Info.plist server.plist.template
#cp -f $APPS_INSTALL_DIR/MonkeyWorksClient.app/Contents/Info.plist client.plist.template
#cp -f $APPS_INSTALL_DIR/MonkeyWorksEditor.app/Contents/Info.plist editor.plist.template

#perl template_file.pl  server.plist.template "<key>CFBundleVersion</key>\\s*<string>.*?</string>" "<key>CFBundleVersion</key><string>$MONKEYWORKS_VERSION_NUMBER_TEMP</string>"  server.plist
#perl template_file.pl  client.plist.template "<key>CFBundleVersion</key>\\s*<string>.*?</string>" "<key>CFBundleVersion</key><string>$MONKEYWORKS_VERSION_NUMBER_TEMP</string>"  client.plist
#perl template_file.pl  editor.plist.template "<key>CFBundleVersion</key>\\s*<string>.*?</string>" "<key>CFBundleVersion</key><string>$MONKEYWORKS_VERSION_NUMBER_TEMP</string>"  editor.plist


# copy the plists back into the apps (now with the latest release number
#cp -f server.plist $APPS_INSTALL_DIR/MonkeyWorksServer.app/Contents/Info.plist
#cp -f client.plist $APPS_INSTALL_DIR/MonkeyWorksClient.app/Contents/Info.plist
#cp -f editor.plist $APPS_INSTALL_DIR/MonkeyWorksEditor.app/Contents/Info.plist


# template in the new version number into files that will go on the web server
#rm -f edu.mit.MonkeyWorksServer.plist
#rm -f edu.mit.MonkeyWorksClient.plist
#rm -f edu.mit.MonkeyWorksEditor.plist

#perl template_file.pl  edu.mit.MonkeyWorksServer.plist.template VERSION_NUMBER $MONKEYWORKS_VERSION_NUMBER_TEMP edu.mit.MonkeyWorksServer.plist
#perl template_file.pl  edu.mit.MonkeyWorksClient.plist.template VERSION_NUMBER $MONKEYWORKS_VERSION_NUMBER_TEMP edu.mit.MonkeyWorksClient.plist
#perl template_file.pl  edu.mit.MonkeyWorksEditor.plist.template VERSION_NUMBER $MONKEYWORKS_VERSION_NUMBER_TEMP edu.mit.MonkeyWorksEditor.plist


## build the subpackages
echo "Building subpackages..."



echo " apps subpackage..."
$PACKAGE_MAKER -build -proj MonkeyWorksAppsPackager.pmproj -v -p "Subcomponent Installers/MonkeyWorksApps.pkg"

echo "application support subpackage..."
$PACKAGE_MAKER -build -proj MonkeyWorksAppSupportPackager.pmproj -v -p "Subcomponent Installers/MonkeyWorksAppSupport.pkg"

echo " frameworks subpackage..."
$PACKAGE_MAKER -build -proj MonkeyWorksFrameworksPackager.pmproj -v -p "Subcomponent Installers/MonkeyWorksFrameworks.pkg"

echo "resource subpackage..."
$PACKAGE_MAKER -build -proj MonkeyWorksResourcesPackager.pmproj -v -p "Subcomponent Installers/MonkeyWorksResources.pkg"

echo "support subpackage..."
$PACKAGE_MAKER -build -proj MonkeyWorksSupportPackager.pmproj -v -p "Subcomponent Installers/MonkeyWorksSupport.pkg"


# New Improved Hack!
#   I'm using an existing installer's framework and just copying in the new packages.

echo "removing old installer"
rm -rf Installer.mpkg
echo "copying template"
cp -rf Installer.mpkg.template Installer.mpkg
echo "removing SVN data from installer"

echo "inserting packages into installer"
cp -r Subcomponent\ Installers/MonkeyWorksApps.pkg Installer.mpkg/Contents/Packages/
cp -r Subcomponent\ Installers/MonkeyWorksAppSupport.pkg Installer.mpkg/Contents/Packages/
cp -r Subcomponent\ Installers/MonkeyWorksFrameworks.pkg Installer.mpkg/Contents/Packages/
cp -r Subcomponent\ Installers/MonkeyWorksResources.pkg Installer.mpkg/Contents/Packages/
cp -r Subcomponent\ Installers/MonkeyWorksSupport.pkg Installer.mpkg/Contents/Packages/

rm -rf `find Installer.mpkg -name ".svn" -type d`

#echo 
#echo 
echo "Compressing and preparing the installer..."

if [ $MONKEYWORKS_VERSION_NUMBER_TEMP == "NIGHTLY" ]
    then
    INSTALLER_NAME="MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP-$INSTALLER_DATE.mpkg"
else
    INSTALLER_NAME="MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP.mpkg"
fi

mv Installer.mpkg $INSTALLER_NAME
zip -r MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP.mpkg.zip $INSTALLER_NAME


## Get rid of intermediates
rm -Rf MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP.mpkg

mv MonkeyWorks-$MONKEYWORKS_VERSION_NUMBER_TEMP.mpkg.zip Archive/


