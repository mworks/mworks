To enable intelligent diffing of the XML files in `MWorks Installer.pmdoc`,
add the following two lines to `../.git/config`:

    [diff "pmdocxml"]
    	textconv = new_installer/format_pmdoc_xml
