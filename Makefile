HID_PLUGIN=HIDPlugin
NE500_PLUGIN=NE500
SIDEWINDER_PLUGIN=SidewinderPlugAndPlayGamepadPlugin

all: ne500-plugin  sidewinder-plugin hid-plugin 

hid-plugin:
	cd $(HID_PLUGIN); \
	xcodebuild clean -alltargets -configuration "Development (10.5 Compatible)"; \
	xcodebuild build -target Everything -configuration "Development (10.5 Compatible)"

sidewinder-plugin:
	cd $(SIDEWINDER_PLUGIN); \
	xcodebuild clean -alltargets -configuration "Development (10.5 Compatible)"; \
	xcodebuild build -target MSSWGamepadPlugin -configuration "Development (10.5 Compatible)"

ne500-plugin:
	cd $(NE500_PLUGIN); \
	xcodebuild clean -alltargets -configuration "Development (10.5 Compatible)"; \
	xcodebuild build -target Everything -configuration "Development (10.5 Compatible)"


