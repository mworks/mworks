HID_PLUGIN=HIDPlugin
NE500_PLUGIN=NE500
SIDEWINDER_PLUGIN=SidewinderPlugAndPlayGamepadPlugin

all: ne500-plugin  sidewinder-plugin hid-plugin 

hid-plugin:
	cd $(HID_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target Everything -configuration Debug

sidewinder-plugin:
	cd $(SIDEWINDER_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target MSSWGamepadPlugin -configuration Debug

ne500-plugin:
	cd $(NE500_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target Everything -configuration Debug


