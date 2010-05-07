# To use this configuration, create a symbolic link to this file named
# "local_config.py"

buildbot_hostname = 'monkeyworks.coxlab.org'
buildbot_admin = 'Dave Cox <cox@rowland.harvard.edu>'
webstatus_port = 7349

from slave_pass import slave_pass
slave_port = 7355

installer_destination_path = '/home/coxlab/webapps/public'
installer_download_base_url = 'http://public.coxlab.org/mw'
