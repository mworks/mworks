## MonkeyWorks Core Plugin Template

Copy the MWorks folder to /Developer/Library/Xcode/Project\ Templates/

Works with Xcode 3.1+.  Choose "New Project" and then select MWorks instead of one of the usual options

## Editing the Template

A quick and dirty python script, "sporulate.py," is included to ease editing of the template.  The principal problem is that the UTF-8 characters used by Xcode's templating engine are incompatible with editing of the project.  An editable version of the template (EditableProject.xcodeproj) is included, and running:
	python sporulate.py
from within the template folder will take care of all of the UTF-8 find and replace to repackage the editable version into a working template

Note that the product of the "sporulate.py" script will not be directly editable in Xcode (indeed, this is why this step is necessary).  You'll need to put it in the above directory and open it as a template for a new project.  You can do this with the command:

	cp -R MWorks /Developer/Library/Xcode/Project\ Templates/