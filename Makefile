BUILD_PLUGIN = \
	cd $(1) && \
	xcodebuild clean -alltargets -configuration $(3) && \
	xcodebuild build -target $(2) -configuration $(3) 

DEFAULT_CONFIG = "Development (10.5 Compatible)"

all: ne500-plugin sidewinder-plugin hid-plugin drifting-grating-stimulus-plugin

hid-plugin:
	$(call BUILD_PLUGIN,HIDPlugin,HIDPlugin,$(DEFAULT_CONFIG))

sidewinder-plugin:
	$(call BUILD_PLUGIN,SidewinderPlugAndPlayGamepadPlugin,MSSWGamepadPlugin,$(DEFAULT_CONFIG))

ne500-plugin:
	$(call BUILD_PLUGIN,NE500,Everything,$(DEFAULT_CONFIG))

drifting-grating-stimulus-plugin:
	$(call BUILD_PLUGIN,DriftingGratingStimulus,DriftingGratingStimulusPlugin,"Debug (10.5 Compatible)")
