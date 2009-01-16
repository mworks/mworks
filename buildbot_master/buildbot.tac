
from twisted.application import service
from buildbot.master import BuildMaster

basedir = r'/Users/davidcox/Repositories/monkeyworks/mw_build/buildbot_master'
configfile = r'master.cfg'

application = service.Application('buildmaster')
BuildMaster(basedir, configfile).setServiceParent(application)

