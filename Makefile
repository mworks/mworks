HID_PLUGIN=HIDPlugin
NE500_PLUGIN=NE500
PHIDGETS_PLUGIN=Phidgets
SIDEWINDER_PLUGIN=SidewinderPlugAndPlayGamepadPlugin

all: ne500-plugin phidgets-plugin sidewinder-plugin hid-plugin 

hid-plugin:
	cd $(HID_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target Everything -configuration Debug

sidewinder-plugin:
	cd $(SIDEWINDER_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target Everything -configuration Debug

ne500-plugin:
	cd $(NE500_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target Everything -configuration Debug

phidgets-plugin:
	cd $(PHIDGETS_PLUGIN); \
	xcodebuild clean -alltargets -configuration Debug; \
	xcodebuild build -target Everything -configuration Debug
