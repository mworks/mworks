DEFAULT_CONFIG = "Development (10.5 Compatible)"

BUILD_PLUGIN = \
	cd $(1) && \
	xcodebuild clean -alltargets -configuration $(DEFAULT_CONFIG) && \
	xcodebuild build -target Everything -configuration $(DEFAULT_CONFIG) 

all: variables-window behavior-window python-bridge

variables-window:
	$(call BUILD_PLUGIN,VariablesWindow)

behavior-window:
	$(call BUILD_PLUGIN,BehavioralWindow)

python-bridge:
	$(call BUILD_PLUGIN,PythonBridgePlugin)
