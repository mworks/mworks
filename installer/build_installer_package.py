#!/usr/bin/env python

# Builds an installer package (e.g. *.pkg) based on the currently installed version on the system
# This could be used to "replicate" a given development workstation's environment, or it could
# be used by a nightly buildbot to package the results of a fresh build
#
# TODO: add a mechanism for dynamically including plugin installers (which would be "checkable" options
# in the meta-installer)
#
# TODO: remove shell commands and replace with Python os calls
import os
import sys

if(len(sys.argv) != 2):
    raise Exception('This script takes a single argument specifying the version string')

version_string = sys.argv[1]

# definitions
package_maker = "/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker"
project_id = "edu.harvard.rowland"

def quote(str):
    return "\"" + str + "\""

def build_package(root, version, target_path, output_path):
    print("Building: %s" % pkg)    
    cmd = "%s --root %s/ --id %s --version %s --install-to %s --out %s" % (package_maker, root, project_id, version, target_path, output_path)
    print(cmd)
    os.system(cmd)


def build_metapackage(doc, version, output_path):
    print("Building: %s" % pkg)    
    cmd = "%s --doc %s --id %s --version %s  --out %s" % (package_maker, doc, project_id, version, output_path)
    print(cmd)
    os.system(cmd)

# directories
install_root="/tmp/mw_installer"

# Important directories in the live system
mw_applications_dir = "/Applications"
mw_application_support_dir = "/Library/Application Support/MonkeyWorks"
mw_frameworks_dir = "/Library/Frameworks"
mw_configuration_dir = mw_application_support_dir + "/Configuration"
mw_developer_dir = mw_application_support_dir + "/Developer"

mw_installer_info_file = "mw_installer.info"
mw_installer_info_path = mw_configuration_dir + "/" + mw_installer_info_file

mw_subcomponent_package_path = install_root + "/subcomponents"

# Package roots for various sub-project installers
applications_package_root = install_root + "/applications"
application_support_package_root = install_root + "/application_support"
frameworks_package_root = install_root + "/frameworks"
developer_package_root = install_root + "/developer"
package_roots = [applications_package_root,
                 application_support_package_root,
                 frameworks_package_root,
                 developer_package_root]

# Package names
packages = ["applications",
            "application_support",
            "frameworks",
            "developer"]


# Applications
applications = ["MWEditor.app", "MWServer.app", "MWClient.app"]

# Frameworks
frameworks = ["MonkeyWorksCocoa.framework", "MonkeyWorksCore.framework"]

# Remove the current install root
os.system("rm -rf %s" % quote(install_root))

# Create the package root directories
for pkg_root in package_roots:
    os.system("mkdir -p %s" % quote(pkg_root))


# Copy stuff into the appropriate directories

# applications
applications_install_dir = applications_package_root + mw_applications_dir
os.system("mkdir -p %s" % quote(applications_install_dir))
for app in applications:
    os.system("cp -rf %s %s" % (quote(mw_applications_dir + "/" + app), quote(applications_install_dir)))

# application support
application_support_install_dir = application_support_package_root + mw_application_support_dir
os.system("mkdir -p %s" % quote(application_support_install_dir))
os.system("rsync -a --exclude Developer --exclude setup_variables.xml %s %s" % (quote(mw_application_support_dir + "/"), 
                                                   quote(application_support_install_dir)))
# frameworks
frameworks_install_dir = frameworks_package_root + mw_frameworks_dir
os.system("mkdir -p %s" % quote(frameworks_install_dir))
for fw in frameworks:
    os.system("cp -rf %s %s" % (quote(mw_frameworks_dir + "/" + fw), quote(frameworks_install_dir)))

# developer support
developer_install_dir = developer_package_root + mw_developer_dir
os.system("mkdir -p %s" % quote(developer_install_dir))
os.system("cp -rf %s %s" % (quote(mw_developer_dir), quote(developer_install_dir)))


os.system("mkdir -p %s" % quote(mw_subcomponent_package_path))

mw_version = version_string

for pkg in packages:
    build_package(install_root + "/" + pkg , mw_version, "/", "%s.pkg" % (pkg))
    os.system("mv %s.pkg %s/" % (pkg, quote(mw_subcomponent_package_path + "/")))

build_metapackage(os.path.dirname(__file__) + "/mw_installer.pmdoc", mw_version, install_root + "/MonkeyWorksInstaller.pkg")
