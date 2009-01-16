
from twisted.application import service
from buildbot.slave.bot import BuildSlave

basedir = r'/Users/davidcox/Repositories/monkeyworks/mw_build/buildbot_slave/tmp'
buildmaster_host = 'localhost'
port = 9990
slavename = 'i386-OSX-10.5'
passwd = 'davidcox'
keepalive = 600
usepty = 1
umask = None

application = service.Application('buildslave')
s = BuildSlave(buildmaster_host, port, slavename, passwd, basedir,
               keepalive, usepty, umask=umask)
s.setServiceParent(application)

