VARIABLES_PLUGIN=VariablesWindow
BEHAVIOR_PLUGIN=BehaviorWindow

all: variables-window behavior-window

variables-window:
	cd $(VARIABLES_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target Everything -configuration Debug

behavior-window:
	cd $(BEHAVIOR_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target Everything -configuration Debug