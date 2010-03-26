#!/usr/bin/env python
# coding=utf-8

import sys
import re
import os


editable_project_name = "EditableProject.xcodeproj"
final_project_name = "MWPlugin.xcodeproj"
project_name_as_identifier_token = u"«PROJECTNAMEASIDENTIFIER»"
project_name_token = u"«PROJECTNAME»"

identifier_replacement_token = "MWPROJECTNAMEASIDENTIFIER"
name_replacement_token = "MWPROJECTNAME"

current_dir = os.path.abspath(os.curdir)

input_filename = os.path.join(current_dir, editable_project_name, "project.pbxproj")
output_filename = os.path.join(current_dir, final_project_name, "project.pbxproj")

print("Processing: %s" % input_filename)
input_file = open(input_filename, "r")
input_data = "".join(input_file.readlines())
input_file.close()



id_pattern = re.compile(identifier_replacement_token)
name_pattern = re.compile(name_replacement_token)

output_data = project_name_as_identifier_token.join(id_pattern.split(input_data))
output_data = project_name_token.join(name_pattern.split(output_data))

# TODO: I'm just being lazy here, this should be done with python calls
os.system("rm -rf %s" % final_project_name)
os.system("cp -r %s %s" % (editable_project_name, final_project_name))
os.system("rm %s/project.pbxproj" % final_project_name)

import codecs
outfile = codecs.open(output_filename, 'w', 'utf-8')
outfile.write(output_data)
outfile.close()

# TODO: remove the "EditableProject.xcodeproj" directory from the template

print("Created template at: %s" % final_project_name)
print("Copy to /Developer/Library/Xcode/Project\ Templates/ to install")
print("e.g.: cp -R MWorks /Developer/Library/Xcode/Project\ Templates/")



