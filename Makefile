DEFAULT_CONFIG = "Development (10.5 Compatible)"

all: variables-window behavior-window python-bridge

variables-window:
	cd VariablesWindow; \
	xcodebuild clean -alltargets -configuration $(DEFAULT_CONFIG); \
	xcodebuild build -target Everything -configuration $(DEFAULT_CONFIG)

behavior-window:
	cd BehavioralWindow; \
	xcodebuild clean -alltargets -configuration $(DEFAULT_CONFIG); \
	xcodebuild build -target Everything -configuration $(DEFAULT_CONFIG)

python-bridge:
	cd PythonBridgePlugin; \
	xcodebuild clean -alltargets -configuration $(DEFAULT_CONFIG); \
	xcodebuild build -target Everything -configuration $(DEFAULT_CONFIG)
