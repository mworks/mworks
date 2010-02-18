# dicarlolab plugins


from buildbot.buildslave import BuildSlave
from buildbot.scheduler import Scheduler, Nightly, Triggerable

from buildbot.process import factory
from buildbot.steps import source, shell
from buildbot.steps.trigger import Trigger
from buildbot.steps.python_twisted import Trial

from local_config import repository_base_url

def get_plugins(**kwargs):
    
    # defaults
    current_branch = "master"
    checkout_type = "update"
    
    if("current_branch" in kwargs):
        current_branch = kwargs["current_branch"]
    
    if("checkout_type" in kwargs):
        checkout_type = kwargs["checkout_type"]
        
    if("standard_mac_arch" in kwargs):
        standard_mac_arch = kwargs["standard_mac_arch"]
    else:
        standard_mac_arch = "i386-OSX-10.5"
    
    if("standard_linux_arch" in kwargs):
        standard_linux_arch = kwargs["standard_linux_arch"]
    else:
        standard_linux_arch = "a64-debian"

    
    # lists to build up and return
    builders = []
    schedulers = []

    # ======================
    # DiCarlo Lab Core Plugins
    # ======================
        
    builder_name = "dicarlolab_core_plugins"    
    core_plugins_scheduler = Triggerable(name=builder_name, builderNames=[builder_name])
    
    schedulers.append(core_plugins_scheduler)
    
    core_plugins_factory = factory.BuildFactory();
    core_plugins_factory.addStep(source.Git(repository_base_url + "dicarlolab_mwcore_plugins.git", current_branch, mode=checkout_type))
    core_plugins_factory.addStep(shell.ShellCommand(command=["make", "all"],
                                                        descriptionDone=["built"],
                                                        description=["building"]))


    core_plugins_builder = {'name': builder_name,
          'slavename': standard_mac_arch,
          'builddir': builder_name,
          'factory': core_plugins_factory,
          }

    builders.append(core_plugins_builder)
    
    
    # ========================
    # DiCarlo Lab Client Plugins
    # ========================
    
    builder_name = "dicarlolab_client_plugins"    
    client_plugins_scheduler = Triggerable(name=builder_name, builderNames=[builder_name])
    
    schedulers.append(client_plugins_scheduler)
    
    client_plugins_factory = factory.BuildFactory();
    client_plugins_factory.addStep(source.Git(repository_base_url + "dicarlolab_mwclient_plugins.git", current_branch, mode=checkout_type))
    client_plugins_factory.addStep(shell.ShellCommand(command=["make", "all"],
                                                        descriptionDone=["built"],
                                                        description=["building"]))


    client_plugins_builder = {'name': builder_name,
          'slavename': standard_mac_arch,
          'builddir': builder_name,
          'factory': client_plugins_factory,
          }

    builders.append(client_plugins_builder)

    
    return (schedulers, builders)
