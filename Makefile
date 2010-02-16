DEFAULT_CONFIG = "Development (10.5 Compatible)"

all: ne500-plugin sidewinder-plugin hid-plugin drifting-grating-stimulus-plugin

hid-plugin:
	cd HIDPlugin; \
	xcodebuild clean -alltargets -configuration $(DEFAULT_CONFIG); \
	xcodebuild build -target HIDPlugin -configuration $(DEFAULT_CONFIG)

sidewinder-plugin:
	cd SidewinderPlugAndPlayGamepadPlugin; \
	xcodebuild clean -alltargets -configuration $(DEFAULT_CONFIG); \
	xcodebuild build -target MSSWGamepadPlugin -configuration $(DEFAULT_CONFIG)

ne500-plugin:
	cd NE500; \
	xcodebuild clean -alltargets -configuration $(DEFAULT_CONFIG); \
	xcodebuild build -target Everything -configuration $(DEFAULT_CONFIG)

drifting-grating-stimulus-plugin:
	cd DriftingGratingStimulus; \
	xcodebuild clean -alltargets -configuration "Debug (10.5 Compatible)"; \
	xcodebuild build -target DriftingGratingStimulusPlugin -configuration "Debug (10.5 Compatible)"
