# ==========================================================
# hid_factory
# ==========================================================

hid_factory = factory.BuildFactory();
hid_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabPlugins/HIDPlugin", 
                                    mode=checkout_type))
hid_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"],
                                          descriptionDone=["cleaned"],
                                          description=["cleaning"]))
hid_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"],
                                          descriptionDone=["cleaned"],
                                          description=["cleaning"]))
hid_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallHIDPlugin", "-configuration", "Debug"],
                                          descriptionDone=["uninstalled"],
                                          description=["uninstalling"]))
hid_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "HIDPlugin", "-configuration", "Debug"],
                                          descriptionDone=["built"],
                                          description=["building"]))


hid_builder = {'name': "HID plugin",
      'slavename': standard_mac_arch,
      'builddir': "HID_plugin",
      'factory': hid_factory,
      }

mssw_factory = factory.BuildFactory();
mssw_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabPlugins/SidewinderPlugAndPlayGamepadPlugin", 
                                    mode=checkout_type))
mssw_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"],
                                          descriptionDone=["cleaned"],
                                          description=["cleaning"]))
mssw_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"],
                                          descriptionDone=["cleaned"],
                                          description=["cleaning"]))
mssw_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallMSSWGamepadPlugin", "-configuration", "Debug"],
                                          descriptionDone=["uninstalled"],
                                          description=["uninstalling"]))
mssw_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "MSSWGamepadPlugin", "-configuration", "Debug"],
                                          descriptionDone=["built"],
                                          description=["building"]))


mssw_builder = {'name': "MSSW plugin",
      'slavename': standard_mac_arch,
      'builddir': "MSSW_plugin",
      'factory': mssw_factory,
      }

itc18_factory = factory.BuildFactory();
itc18_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabPlugins/ITC18Plugin", 
                                    mode=checkout_type))
itc18_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"],
                                          descriptionDone=["cleaned"],
                                          description=["cleaning"]))
itc18_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"],
                                          descriptionDone=["cleaned"],
                                          description=["cleaning"]))
itc18_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallITC18Plugin", "-configuration", "Debug"],
                                          descriptionDone=["uninstalled"],
                                          description=["uninstalling"]))
itc18_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "ITC18Plugin", "-configuration", "Debug"],
                                          descriptionDone=["built"],
                                          description=["building"]))


itc18_builder = {'name': "ITC18 plugin",
      'slavename': standard_mac_arch,
      'builddir': "ITC18_plugin",
      'factory': itc18_factory,
      }

fake_monkey_factory = factory.BuildFactory();
fake_monkey_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabPlugins/FakeMonkeyPlugin", 
                                    mode=checkout_type))
fake_monkey_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"],
                                          descriptionDone=["cleaned"],
                                          description=["cleaning"]))
fake_monkey_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"],
                                          descriptionDone=["cleaned"],
                                          description=["cleaning"]))
fake_monkey_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallFakeMonkeyPlugin", "-configuration", "Debug"],
                                          descriptionDone=["uninstalled"],
                                          description=["uninstalling"]))
fake_monkey_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "FakeMonkeyPlugin", "-configuration", "Debug"],
                                          descriptionDone=["built"],
                                          description=["building"]))


fake_monkey_builder = {'name': "Fake Monkey plugin",
      'slavename': standard_mac_arch,
      'builddir': "Fake_Monkey_plugin",
      'factory': fake_monkey_factory,
      }

movie_stimulus_factory = factory.BuildFactory();
movie_stimulus_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabPlugins/MoviePlugin", 
                                    mode=checkout_type))
movie_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"]))
movie_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
movie_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallMovieStimulusPlugin", "-configuration", "Debug"]))
movie_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "MovieStimulusPlugin", "-configuration", "Debug"]))

movie_stimulus_builder = {'name': "movie stimulus plugin",
      'slavename': standard_mac_arch,
      'builddir': "Movie_Stimulus_plugin",
      'factory': movie_stimulus_factory,
      }

rectangle_stimulus_factory = factory.BuildFactory();
rectangle_stimulus_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabPlugins/RectangleStimulus", 
                                    mode=checkout_type))
rectangle_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"]))
rectangle_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
rectangle_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallRectangleStimulusPlugin", "-configuration", "Debug"]))
rectangle_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "RectangleStimulusPlugin", "-configuration", "Debug"]))

rectangle_stimulus_builder = {'name': "rectangle stimulus plugin",
      'slavename': standard_mac_arch,
      'builddir': "Rectangle_Stimulus_plugin",
      'factory': rectangle_stimulus_factory,
      }

circle_stimulus_factory = factory.BuildFactory();
circle_stimulus_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabPlugins/CircleStimulus", 
                                    mode=checkout_type))
circle_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"]))
circle_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
circle_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallCircleStimulusPlugin", "-configuration", "Debug"]))
circle_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "CircleStimulusPlugin", "-configuration", "Debug"]))

circle_stimulus_builder = {'name': "circle stimulus plugin",
      'slavename': standard_mac_arch,
      'builddir': "Circle_Stimulus_plugin",
      'factory': circle_stimulus_factory,
      }

drifting_grating_stimulus_factory = factory.BuildFactory();
drifting_grating_stimulus_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabPlugins/DriftingGratingStimulus", 
                                    mode=checkout_type))
drifting_grating_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"]))
drifting_grating_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
drifting_grating_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallDriftingGratingStimulusPlugin", "-configuration", "Debug"]))
drifting_grating_stimulus_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "DriftingGratingStimulusPlugin", "-configuration", "Debug"]))

drifting_grating_stimulus_builder = {'name': "drifting grating stimulus plugin",
      'slavename': standard_mac_arch,
      'builddir': "Drifting_Grating_Stimulus_plugin",
      'factory': drifting_grating_stimulus_factory,
      }
      

      variables_window_factory = factory.BuildFactory();
      variables_window_factory.addStep(source.SVN(svnurl="https://svn.coxlab.org/other/code/MonkeyWorks/ClientPlugins/MonkeyWorksVariablesWindow", 
                                        mode=checkout_type))
      variables_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      variables_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "MonkeyWorksVariablesWindow", "-configuration", "Debug"]))

      variables_window_builder = {'name': "variables window",
            'slavename': standard_mac_arch,
            'builddir': "Variables_window",
            'factory': variables_window_factory,
            }


      behavior_window_factory = factory.BuildFactory();
      behavior_window_factory.addStep(source.SVN(svnurl="https://svn.coxlab.org/other/code/MonkeyWorks/ClientPlugins/MonkeyWorksBehavioralWindowNew", 
                                        mode=checkout_type))
      behavior_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      behavior_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "MonkeyWorksBehavioralWindow", "-configuration", "Debug"]))

      behavior_window_builder = {'name': "behavior window",
            'slavename': standard_mac_arch,
            'builddir': "Behavior_window",
            'factory': behavior_window_factory,
            }


      rat_behavior_window_factory = factory.BuildFactory();
      rat_behavior_window_factory.addStep(source.SVN(svnurl="https://svn.coxlab.org/other/code/MonkeyWorks/ClientPlugins/RatBehaviorControlPanelNew", 
                                        mode=checkout_type))
      rat_behavior_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      rat_behavior_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "RatBehaviorControlPanel", "-configuration", "Debug"]))

      rat_behavior_window_builder = {'name': "rat behavior window",
            'slavename': standard_mac_arch,
            'builddir': "Rat behavior_window",
            'factory': rat_behavior_window_factory,
            }


      calibrator_window_factory = factory.BuildFactory();
      calibrator_window_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabClientPlugins/CalibratorWindow", 
                                        mode=checkout_type))
      calibrator_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      calibrator_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "CalibratorWindow", "-configuration", "Debug"]))

      calibrator_window_builder = {'name': "calibrator window",
            'slavename': standard_mac_arch,
            'builddir': "Calibrator_window",
            'factory': calibrator_window_factory,
            }

      eye_window_factory = factory.BuildFactory();
      eye_window_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabClientPlugins/EyeWindow", 
                                        mode=checkout_type))
      eye_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      eye_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "MonkeyWorksEyeWindow", "-configuration", "Debug"]))

      eye_window_builder = {'name': "eye window",
            'slavename': standard_mac_arch,
            'builddir': "Eye_window",
            'factory': eye_window_factory,
            }

      MATLAB_window_factory = factory.BuildFactory();
      MATLAB_window_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabClientPlugins/MATLABWindow", 
                                        mode=checkout_type))
      MATLAB_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      MATLAB_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "MonkeyWorksMATLABWindow", "-configuration", "Debug"]))

      MATLAB_window_builder = {'name': "MATLAB window",
            'slavename': standard_mac_arch,
            'builddir': "MATLAB_window",
            'factory': MATLAB_window_factory,
            }

      reward_window_factory = factory.BuildFactory();
      reward_window_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/DLabClientPlugins/RewardWindow", 
                                        mode=checkout_type))
      reward_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      reward_window_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "MonkeyWorksRewardWindow", "-configuration", "Debug"]))

      reward_window_builder = {'name': "reward window",
            'slavename': standard_mac_arch,
            'builddir': "Reward_window",
            'factory': reward_window_factory,
            }

      data_file_indexer_factory = factory.BuildFactory();
      data_file_indexer_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/MonkeyWorksTools/DataFileIndexer", 
                                        mode=checkout_type))
      data_file_indexer_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"]))
      data_file_indexer_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      data_file_indexer_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "dfindexerUninstall", "-configuration", "Debug"]))
      data_file_indexer_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "dfindex", "-configuration", "Debug"]))

      data_file_indexer_builder = {'name': "data file indexer",
            'slavename': standard_mac_arch,
            'builddir': "Data_file_indexer",
            'factory': data_file_indexer_factory,
            }

      stream_utilities_factory = factory.BuildFactory();
      stream_utilities_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/MonkeyWorksTools/MonkeyWorksStreamUtilities", 
                                        mode=checkout_type))
      stream_utilities_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"]))
      stream_utilities_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      stream_utilities_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "UninstallMonkeyWorksStreamUtilities", "-configuration", "Debug"]))
      stream_utilities_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "libMonkeyWorksStreamUtilities", "-configuration", "Debug"]))

      stream_utilities_builder = {'name': "stream utilities",
            'slavename': standard_mac_arch,
            'builddir': "Stream_utilities",
            'factory': stream_utilities_factory,
            }

      MATLAB_data_reader_factory = factory.BuildFactory();
      MATLAB_data_reader_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/MonkeyWorksTools/MatlabDataReader", 
                                        mode=checkout_type))
      MATLAB_data_reader_factory.addStep(shell.ShellCommand(command=["make", "clean"]))
      MATLAB_data_reader_factory.addStep(shell.ShellCommand(command=["make", "all"]))

      MATLAB_data_reader_builder = {'name': "MATLAB data reader",
            'slavename': standard_mac_arch,
            'builddir': "MATLAB_data_reader",
            'factory': MATLAB_data_reader_factory,
            }

      MATLAB_data_reader_linux_factory = factory.BuildFactory();
      MATLAB_data_reader_linux_factory.addStep(shell.ShellCommand(command=["rm", "-rf", "./*"], timeout=3600))
      MATLAB_data_reader_linux_factory.addStep(shell.ShellCommand(command=["svn", "cleanup", "."], timeout=3600))
      MATLAB_data_reader_linux_factory.addStep(shell.ShellCommand(command=["svn", "checkout", "https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk", "--username", "labuser", "--password", "labuser", "."], timeout=3600))
      MATLAB_data_reader_linux_factory.addStep(shell.ShellCommand(command=["make", "-f", "Makefile.linux_analysis", "clean"]))
      MATLAB_data_reader_linux_factory.addStep(shell.ShellCommand(command=["make", "-f", "Makefile.linux_analysis", "install"]))

      MATLAB_data_reader_linux_builder = {'name': "MATLAB data reader: linux",
            'slavename': standard_linux_arch,
            'builddir': "MATLAB_data_reader_linux",
            'factory': MATLAB_data_reader_linux_factory,
            }

      data_file_reader_factory = factory.BuildFactory();
      data_file_reader_factory.addStep(source.SVN(svnurl="https://svn6.cvsdude.com/dicarlolab/MonkeyWorks/trunk/MonkeyWorksTools/DataFileReader", 
                                        mode=checkout_type))
      data_file_reader_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Release"]))
      data_file_reader_factory.addStep(shell.ShellCommand(command=["xcodebuild", "clean", "-alltargets", "-configuration", "Debug"]))
      data_file_reader_factory.addStep(shell.ShellCommand(command=["xcodebuild", "build", "-target", "mwdfr", "-configuration", "Debug"]))

      data_file_reader_builder = {'name': "data file reader",
            'slavename': standard_mac_arch,
            'builddir': "Data_file_reader",
            'factory': data_file_reader_factory,
            }

