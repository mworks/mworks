Installation
============


macOS
-----


Install or Upgrade
^^^^^^^^^^^^^^^^^^

To install or upgrade MWorks on macOS, first `download <https://mworks.github.io/downloads/>`_ an installer disk image.  Double click the disk image to mount it, then double click the *MWorks.pkg* icon to start the installer.  The installer will guide you through the rest of the installation process.

If the installer detects an existing MWorks or MonkeyWorks installation, it will move all the existing components to a folder called *MWorks Backup* on your desktop.  To remove this backup, simply drag the folder to the trash.


Uninstall or Revert
^^^^^^^^^^^^^^^^^^^

The MWorks installer disk image includes a script that will completely remove any current or legacy release of MWorks or MonkeyWorks from a macOS system.

To uninstall, open the disk image (*.dmg* file) you installed from (or `download <https://mworks.github.io/downloads/>`_ a more recent one), and double click the *Uninstall MWorks* icon.  A terminal window will open, and you will be prompted to enter your macOS password.  Once you do so, the uninstaller will move all MWorks components to a folder called *Uninstalled MWorks* in the trash. To permanently delete the uninstalled components, simply empty the trash.

If the uninstaller finds a previous MWorks version in the *MWorks Backup* folder on your desktop (which it will if you upgraded an existing MWorks installation and have not moved or deleted the backup), then after uninstalling the current version, it will re-install the old one. To uninstall the re-installed version, simply run *Uninstall MWorks* again.


iOS
---

To install MWorks on iOS, `download it from the App Store <https://itunes.apple.com/us/app/mworks/id1389408331?ls=1&mt=8>`_.  To uninstall, simply delete the *MWorks* application.


.. _configuration:

Configuration
=============

If this is your first time installing MWorks, or you are upgrading from MonkeyWorks, you will need to configure your installation before running experiments.


macOS
-----


Configuration File
^^^^^^^^^^^^^^^^^^

To configure MWorks, you assign values to a special set of `system variables <config vars>`.  The desired values are stored in a file called ``setup_variables.xml``.   On macOS, this file can reside in one of two directories:

* For all users: ``/Library/Application Support/MWorks/Configuration``
* For a specific user: ``$HOME/Library/Application Support/MWorks/Configuration``, where ``$HOME`` is the path to the user's home directory (e.g. ``/Users/chris``)

The user-specific configuration file takes precedence over the system-wide one, which allows individual users to customize their setup.

MWorks is distributed with two example configuration files, both of which are installed in ``/Library/Application Support/MWorks/Configuration/examples``:

* ``setup_variables_two_monitors.xml``, appropriate for a two-monitor system where stimuli are displayed fullscreen on the secondary monitor
* ``setup_variables_macbook.xml``, appropriate for a laptop or other single-monitor system where stimuli are displayed in a non-fullscreen window

To use either of these examples, you must copy the file to one of the aforementioned directories and rename it ``setup_variables.xml``.

During installation, the MWorks installer checks for an existing configuration file.  If none is found, it installs a copy of ``setup_variables_macbook.xml`` for system-wide use.


Changing Configuration Options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The easiest way to modify configuration variables and save the changes to the configuration file is via MWServer's preferences window, which provides a graphical interface to all settings controlled by ``setup_variables.xml``.  Configuration changes made via the preferences window are stored in the user-specific configuration file.

Alternatively, since the configuration file is an XML document, it can be edited with any text editor (TextEdit, Emacs, Vim, etc.).


iOS
---

To configure MWorks for iOS, use the system *Settings* app.  You can find all of MWorks' settings in the section titled "MWorks".

The iOS version of MWorks supports most of the `configuration options <config vars>` available in the macOS version.  At a minimum, you should change the display width, height, and distance parameters to match your device and experimental setup.  If you leave the server name blank, MWorks will use the device name specified in Settings → General → About → Name.
