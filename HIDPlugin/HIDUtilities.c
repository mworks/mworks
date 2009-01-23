/*
	File:		HID Manager Test.c

	Contains:	Basic HID Manager Test Code
    
	DRI: George Warner

	Copyright:	Copyright © 2002 Apple Computer, Inc., All Rights Reserved

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




// project includes ---------------------------------------------------------

#include "HIDUtilities.h"

// prototypes ---------------------------------------------------------------

void PrintErrMsgIfIOErr (long expr, char * msg);
void PrintErrMsgIfErr (long expr, char * msg);


// statics/globals (internal only) ------------------------------------------

static CFIndex gNestingLevel = 0;
static CFIndex gElementIndex = 0;
static IOHIDElementCookie gXAxisCookie = 0;
static IOHIDElementCookie gButton1Cookie = 0;
static IOHIDElementCookie gButton2Cookie = 0;
static IOHIDElementCookie gButton3Cookie = 0;

// functions (internal/private) ---------------------------------------------

void PrintErrMsgIfIOErr (long expr, char * msg)
{
    IOReturn err = (expr);
    if (err != kIOReturnSuccess)
    {
        fprintf (stderr, "%s - %s(%x,%d)\n",
                 msg, mach_error_string (err), err, err & 0xffffff);
        fflush(stderr);
        DebugStr ("\pExecution Halted");
    }
}

// --------------------------------------------------------------------------

void PrintErrMsg (char * msg)
{
	fprintf (stderr, "%s\n", msg);
	fflush (stderr);
    DebugStr ("\pExecution Halted");
}

// --------------------------------------------------------------------------

 CFMutableDictionaryRef MySetUpHIDMatchingDictionary (UInt32 usagePage, UInt32 usage)
{
    CFNumberRef refUsage = NULL, refUsagePage = NULL;
    CFMutableDictionaryRef refHIDMatchDictionary = NULL;

    // Set up a matching dictionary to search I/O Registry by class name for all HID class devices.
    refHIDMatchDictionary = IOServiceMatching (kIOHIDDeviceKey);
    if (refHIDMatchDictionary != NULL)
    {
        // Add key for device type (joystick, in this case) to refine the matching dictionary.
        refUsagePage = CFNumberCreate (kCFAllocatorDefault, kCFNumberIntType, &usagePage);
        refUsage = CFNumberCreate (kCFAllocatorDefault, kCFNumberIntType, &usage);
        CFDictionarySetValue (refHIDMatchDictionary, CFSTR (kIOHIDPrimaryUsagePageKey), refUsagePage);
        CFDictionarySetValue (refHIDMatchDictionary, CFSTR (kIOHIDPrimaryUsageKey), refUsage);
    }
    else
        PrintErrMsg ("Failed to get HID CFMutableDictionaryRef via IOServiceMatching.");
    return refHIDMatchDictionary;
}

// --------------------------------------------------------------------------

 io_iterator_t MyFindHIDDevices (const mach_port_t masterPort, UInt32 usagePage, UInt32 usage)
{
    CFMutableDictionaryRef hidMatchDictionary = NULL;
    IOReturn ioReturnValue = kIOReturnSuccess;
    io_iterator_t hidObjectIterator;

    // Set up matching dictionary to search the I/O Registry for HID devices we are interested in. Dictionary reference is NULL if error.
    hidMatchDictionary = MySetUpHIDMatchingDictionary (usagePage, usage);
    if (NULL == hidMatchDictionary)
        PrintErrMsg ("Couldn’t create a matching dictionary.");

    // Now search I/O Registry for matching devices.
    ioReturnValue = IOServiceGetMatchingServices (masterPort, hidMatchDictionary, &hidObjectIterator);
	// If error, print message and hang (for debugging purposes).
	if ((ioReturnValue != kIOReturnSuccess) | (hidObjectIterator == 0))
        PrintErrMsg ("Couldn’t create a HID object iterator.");

    // IOServiceGetMatchingServices consumes a reference to the dictionary, so we don't need to release the dictionary ref.
    hidMatchDictionary = NULL;
    return hidObjectIterator;
}

// --------------------------------------------------------------------------

 void MyIncrementElementIndex(CFIndex increment)
{
    gElementIndex += increment;
}

// --------------------------------------------------------------------------

 void MyIndent ()
{
    int i;
    printf("\n");
    for (i = 0; i < gNestingLevel; i++)
        printf ("   ");
}
// --------------------------------------------------------------------------

 void MyStartBlock (Boolean printIndex)
{
    if (printIndex)
    {
        MyIndent ();
        printf ("[%ld]", MyGetElementIndex());
    }
	gNestingLevel++;
}

// --------------------------------------------------------------------------

 void MyEndBlock ()
{
    gNestingLevel--;
}

// --------------------------------------------------------------------------

 CFIndex MyGetElementIndex ()
{
    return gElementIndex;
}

// --------------------------------------------------------------------------

 void MySetElementIndex (CFIndex newIndex)
{
    gElementIndex = newIndex;
}
// --------------------------------------------------------------------------

 void MyCFArrayShow(CFArrayRef object)
{
    CFRange range = {0, CFArrayGetCount (object)};
    CFIndex savedIndex = MyGetElementIndex ();

    //Show an element array containing one or more element dictionaries
    MySetElementIndex (0); //Reset index to zero
    MyStartBlock (FALSE);
    CFArrayApplyFunction (object, range, MyCFArrayShowApplier, 0);
    MyEndBlock ();
    MySetElementIndex (savedIndex); //Restore original index
}

// --------------------------------------------------------------------------

 void MyCFArrayShowApplier (const void * value, void * parameter)
{
    if (CFGetTypeID (value) != CFDictionaryGetTypeID ())
        return;
    MyCFObjectShow(value);
}

// --------------------------------------------------------------------------

 void MyCFBooleanShow(CFBooleanRef object)
{
    printf(CFBooleanGetValue(object) ? "true" : "false");
}

// --------------------------------------------------------------------------

 void MyStoreImportantCookies (CFDictionaryRef element)
{
    CFTypeRef object;
    long number;
    IOHIDElementCookie cookie;
    long usage;
    long usagePage;

    //Get cookie
    object = CFDictionaryGetValue (element, CFSTR(kIOHIDElementCookieKey));
    if (object == 0 || CFGetTypeID (object) != CFNumberGetTypeID ())
        return;
    if(!CFNumberGetValue ((CFNumberRef) object, kCFNumberLongType, &number))
        return;
    cookie = (IOHIDElementCookie) number;

    //Get usage
    object = CFDictionaryGetValue (element, CFSTR(kIOHIDElementUsageKey));
    if (object == 0 || CFGetTypeID (object) != CFNumberGetTypeID ())
        return;
    if (!CFNumberGetValue ((CFNumberRef) object, kCFNumberLongType, &number))
        return;
    usage = number;

    //Get usage page
    object = CFDictionaryGetValue (element,CFSTR(kIOHIDElementUsagePageKey));
    if (object == 0 || CFGetTypeID (object) != CFNumberGetTypeID ())
        return;
    if (!CFNumberGetValue ((CFNumberRef) object, kCFNumberLongType, &number))
        return;
    usagePage = number;

    //Check for x axis
    if (usage == kHIDUsage_GD_X && usagePage == kHIDPage_GenericDesktop)
        gXAxisCookie = cookie;
    //Check for buttons
    else if (usage == kHIDUsage_Button_1 && usagePage == kHIDPage_Button)
        gButton1Cookie = cookie;
    else if (usage == kHIDUsage_Button_2 && usagePage == kHIDPage_Button)
        gButton2Cookie = cookie;
    else if (usage == kHIDUsage_Button_3 && usagePage == kHIDPage_Button)
        gButton3Cookie = cookie;
}

// --------------------------------------------------------------------------

 void MyShowTypeElement (CFDictionaryRef dictionary)
{
    CFStringRef keyType = CFSTR(kIOHIDElementTypeKey);
    CFTypeRef objectType = CFDictionaryGetValue (dictionary, keyType);
    long valueType = 0;

    if (objectType)
    {
        if (CFNumberGetValue ((const __CFNumber*)objectType, kCFNumberLongType, &valueType))
        {
            MyIndent ();
            switch (valueType)
            {
                case kIOHIDElementTypeInput_Misc:
                    printf("Miscellaneous Input Type (%ld), ", valueType);
                    break;
                case kIOHIDElementTypeInput_Button:
                    printf("Button Input Type (%ld), ", valueType);
                    break;
                case kIOHIDElementTypeInput_Axis:
                    printf("Axis Input Type (%ld), ", valueType);
                    break;
                case kIOHIDElementTypeInput_ScanCodes:
                    printf("Scan Code Input Type (%ld), ", valueType);
                    break;
                case kIOHIDElementTypeOutput:
                    printf("Output Type (%ld), ", valueType);
                    break;
                case kIOHIDElementTypeFeature:
                    printf("Feature Type (%ld), ", valueType);
                    break;
                case kIOHIDElementTypeCollection:
                    printf("Collection Type (%ld), ", valueType);
                    break;
            }
        }
	}
}

// --------------------------------------------------------------------------

 void MyShowUsageAndPageElement (CFDictionaryRef dictionary)
{
	CFStringRef keyUsagePage = CFSTR(kIOHIDElementUsagePageKey);
	CFStringRef keyUsage = CFSTR(kIOHIDElementUsageKey);
	CFTypeRef objectUsagePage = CFDictionaryGetValue (dictionary, keyUsagePage);
	CFTypeRef objectUsage = CFDictionaryGetValue (dictionary, keyUsage);
	long valueUsage = 0, valueUsagePage = 0;

	if (objectUsagePage && objectUsage)
	{
		if (CFNumberGetValue ((const __CFNumber*)objectUsage, kCFNumberLongType, &valueUsage) && CFNumberGetValue ((const __CFNumber*)objectUsagePage, kCFNumberLongType, &valueUsagePage))
		{
			MyIndent ();
			printf ("Usage: ");

			switch (valueUsagePage)
			{
				case kHIDPage_Undefined:
					printf("Undefined Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default:
							printf("Undefined Usage (0x%lx)", valueUsage);
							break;
					}
						break;
				case kHIDPage_GenericDesktop:
					printf("Generic Desktop (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						case kHIDUsage_GD_Pointer: printf("Pointer (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Mouse: printf("Mouse (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Joystick: printf("Joystick (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_GamePad: printf("GamePad (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Keyboard: printf("Keyboard (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Keypad: printf("Keypad (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_MultiAxisController: printf("Multi-Axis Controller (0x%lx)", valueUsage); break;

						case kHIDUsage_GD_X: printf("X-Axis (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Y: printf("Y-Axis (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Z: printf("Z-Axis (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Rx: printf("X-Rotation (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Ry: printf("Y-Rotation (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Rz: printf("Z-Rotation (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Slider: printf("Slider (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Dial: printf("Dial (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Wheel: printf("Wheel (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Hatswitch: printf("Hatswitch (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_CountedBuffer: printf("Counted Buffer (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_ByteCount: printf("Byte Count (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_MotionWakeup: printf("Motion Wakeup (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Start: printf("Start (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Select: printf("Select (0x%lx)", valueUsage); break;

						case kHIDUsage_GD_Vx: printf("X-Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vy: printf("Y-Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vz: printf("Z-Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vbrx: printf("X-Rotation Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vbry: printf("Y-Rotation Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vbrz: printf("Z-Rotation Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vno: printf("Vno (0x%lx)", valueUsage); break;

						case kHIDUsage_GD_SystemControl: printf("System Control (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemPowerDown: printf("System Power Down (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemSleep: printf("System Sleep (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemWakeUp: printf("System Wake Up (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemContextMenu: printf("System Context Menu (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMainMenu: printf("System Main Menu (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemAppMenu: printf("System App Menu (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuHelp: printf("System Menu Help (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuExit: printf("System Menu Exit (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenu: printf("System Menu (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuRight: printf("System Menu Right (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuLeft: printf("System Menu Left (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuUp: printf("System Menu Up (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuDown: printf("System Menu Down (0x%lx)", valueUsage); break;

						case kHIDUsage_GD_DPadUp: printf("DPad Up (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_DPadDown: printf("DPad Down (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_DPadRight: printf("DPad Right (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_DPadLeft: printf("DPad Left (0x%lx)", valueUsage); break;

						case kHIDUsage_GD_Reserved: printf("Reserved (0x%lx)", valueUsage); break;

						default: printf("Undefined Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Simulation:
					printf("Simulation Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_VR:
					printf("VR Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Sport:
					printf("Sport Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Game:
					printf("Game Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_KeyboardOrKeypad:
					printf("Keyboard or Keypad Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Button:
					printf("Button Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Button #%ld (0x%lx)", valueUsage, valueUsage); break;
					}
						break;
				case kHIDPage_Ordinal:
					printf("Ordinal Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Telephony:
					printf("Telephony Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Consumer:
					printf("Consumer Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Digitizer:
					printf("Digitizer Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_PID:
					printf("PID Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Unicode:
					printf("Unicode Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_AlphanumericDisplay:
					printf("Alphanumeric Display Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_BarCodeScanner:
					printf("Bar Code Scanner Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Scale:
					printf("Scale Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_CameraControl:
					printf("Camera Control Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Arcade:
					printf("Arcade Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				default:
					printf("Unknown Page (0x%lx), ", valueUsagePage);
					printf("Usage (0x%lx)", valueUsage);
					break;
			}
		}
	}
}

// --------------------------------------------------------------------------

 void MyCFDictionaryShow(CFDictionaryRef object)
{
    MyStartBlock(TRUE);
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementCookieKey));
    MyShowTypeElement (object);
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementCollectionTypeKey));
    MyShowUsageAndPageElement (object);
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementMinKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementMaxKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementScaledMinKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementScaledMaxKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementSizeKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementIsRelativeKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementIsWrappingKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementIsNonLinearKey));
#ifdef kIOHIDElementHasPreferredStateKey
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementHasPreferredStateKey));
#else
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementHasPreferedStateKey));
#endif
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementHasNullStateKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementVendorSpecificKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementUnitKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementUnitExponentKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementNameKey));
    MyShowDictionaryElement (object, CFSTR(kIOHIDElementKey));
    fflush (stdout);
    MyStoreImportantCookies (object);
    //Store specific elements in global variables for use in queue handling functions.
    MyEndBlock ();
    MyIncrementElementIndex (1);
}

// --------------------------------------------------------------------------

 void MyCFNumberShow (CFNumberRef object)
{
    long number;
    if (CFNumberGetValue (object, kCFNumberLongType, &number))
        printf("0x%lx (%ld)", number, number);
}

// --------------------------------------------------------------------------

 void MyCFStringShow(CFStringRef object)
{
    const char * c = CFStringGetCStringPtr (object, CFStringGetSystemEncoding ());
    if (c)
        printf ("%s", c);
    else
    {
        CFIndex bufferSize = CFStringGetLength (object) + 1;
        char * buffer = (char *)malloc (bufferSize);
        if (buffer)
        {
            if (CFStringGetCString (object, buffer, bufferSize, CFStringGetSystemEncoding ()))
                printf ("%s", buffer);
            free(buffer);
        }
    }
}

// --------------------------------------------------------------------------

 void MyCFObjectShow (CFTypeRef object)
{
    CFTypeID type = CFGetTypeID(object);
    if (type == CFArrayGetTypeID())
        MyCFArrayShow((const __CFArray*)object);
    else if (type == CFBooleanGetTypeID())
        MyCFBooleanShow((const __CFBoolean*)object);
    else if (type == CFDictionaryGetTypeID())
        MyCFDictionaryShow((const __CFDictionary*)object);
    else if (type == CFNumberGetTypeID())
        MyCFNumberShow((const __CFNumber*)object);
    else if (type == CFStringGetTypeID())
        MyCFStringShow((const __CFString*)object);
    else
        printf("<unknown object>");
}

// --------------------------------------------------------------------------

 void MyShowProperty(const void * key, const void * value)
{
    //Both parameters are references to CF objects
    MyIndent ();
    MyCFStringShow ((const __CFString*)key);
    printf (" = ");
    MyCFObjectShow (value);
}

// --------------------------------------------------------------------------

 Boolean MyShowDictionaryElement (CFDictionaryRef dictionary, CFStringRef key)
{
    CFTypeRef object = CFDictionaryGetValue (dictionary, key);
    if (object)
        MyShowProperty (key,object);
    return (object != NULL);
}

// --------------------------------------------------------------------------

 void MyShowHIDProperties (io_registry_entry_t hidDevice)
{
    kern_return_t result;
    CFMutableDictionaryRef properties = 0;
    char path[512];

    result = IORegistryEntryGetPath (hidDevice, kIOServicePlane, path);
    if (result == KERN_SUCCESS)
        printf("IO Registry Path: [ %s ]\n", path);

	//Create a CF dictionary representation of the I/O Registry entry’s properties
    result = IORegistryEntryCreateCFProperties (hidDevice, &properties, kCFAllocatorDefault, kNilOptions);
    if ((result == KERN_SUCCESS) && properties)
    {
        printf ("- Device Properties -\n");
        MyShowDictionaryElement (properties, CFSTR(kIOHIDTransportKey));
        MyShowDictionaryElement (properties, CFSTR(kIOHIDVendorIDKey));
        MyShowDictionaryElement (properties, CFSTR(kIOHIDProductIDKey));
        MyShowDictionaryElement (properties, CFSTR(kIOHIDVersionNumberKey));
        MyShowDictionaryElement (properties, CFSTR(kIOHIDManufacturerKey));
        MyShowDictionaryElement (properties, CFSTR(kIOHIDProductKey));
        MyShowDictionaryElement (properties, CFSTR(kIOHIDSerialNumberKey));
        if (!MyShowDictionaryElement (properties, CFSTR(kIOHIDLocationIDKey)))
            MyShowDictionaryElement (properties, CFSTR(kUSBDevicePropertyLocationID));
        MyShowDictionaryElement (properties, CFSTR(kIOHIDPrimaryUsageKey));
        MyShowDictionaryElement (properties, CFSTR(kIOHIDPrimaryUsagePageKey));

		MyShowDictionaryElement (properties, CFSTR("idVendor"));
		MyShowDictionaryElement (properties, CFSTR("USB Product Name"));
		MyShowDictionaryElement (properties, CFSTR("idProduct"));


        fflush (stdout);
        printf ("- Device Element Properties -\n");
        MyShowDictionaryElement (properties, CFSTR(kIOHIDElementKey));

        //Release the properties dictionary
        CFRelease (properties);
    }
    else
        PrintErrMsg ("Failed to create properties via IORegistryEntryCreateCFProperties.");
    printf ("\n\n");
}

// --------------------------------------------------------------------------

 IOHIDDeviceInterface ** MyCreateHIDDeviceInterface (io_object_t hidDevice)
{
    io_name_t className;
    IOCFPlugInInterface **plugInInterface = NULL;
    HRESULT plugInResult = S_OK;
    SInt32 score = 0;
    IOReturn ioReturnValue = kIOReturnSuccess;
    IOHIDDeviceInterface ** pphidDeviceInterface = NULL;

    ioReturnValue = IOObjectGetClass (hidDevice, className);
    PrintErrMsgIfIOErr (ioReturnValue, "Failed to get class name.");

    printf ("Creating interface for device of class %s\n\n", className);

    ioReturnValue = IOCreatePlugInInterfaceForService (hidDevice, kIOHIDDeviceUserClientTypeID,
                                                       kIOCFPlugInInterfaceID, &plugInInterface, &score);
    if (ioReturnValue == kIOReturnSuccess)
    {
        // Call a method of the intermediate plug-in to create the device interface
        plugInResult = (*plugInInterface)->QueryInterface (plugInInterface,
                                                           CFUUIDGetUUIDBytes (kIOHIDDeviceInterfaceID), (void **) &pphidDeviceInterface);
        if (plugInResult != S_OK)
            PrintErrMsg ("Couldn’t query HID class device interface from plugInInterface");
        (*plugInInterface)->Release (plugInInterface);
    }
    else
        PrintErrMsg ("Failed to create **plugInInterface via IOCreatePlugInInterfaceForService.");
    return pphidDeviceInterface;
}

// --------------------------------------------------------------------------
#define TEST_EVENT_CALLBACK TRUE

#if TEST_EVENT_CALLBACK
static UInt32 gCallback_count = 0;
 void MyIOHIDCallbackFunction(void * target, IOReturn result, void * refcon, void * sender)
{
	gCallback_count++;
	//if (0 == (gCallback_count % 32))
	{
		printf("MyIOHIDCallbackFunction - count = %ld.\n",gCallback_count);
		fflush(stdout);
	}

	result = kIOReturnSuccess;
}
#endif TEST_EVENT_CALLBACK
// --------------------------------------------------------------------------

 void MyTestEventInterface (IOHIDDeviceInterface **hidDeviceInterface)
{
    HRESULT result;
    IOHIDQueueInterface ** queue;
    Boolean hasElement;
    IOHIDEventStruct event;

    queue = (*hidDeviceInterface)->allocQueue (hidDeviceInterface);
    printf("Queue allocated: %lx\n", (long) queue);
    if (queue)
    {
        //create the queue
        result = (*queue)->create (queue, 0, /* flag?? */ 8);/* depth: maximum number of elements
		in queue before oldest elements in
		queue begin to be lost*/
        printf ("Queue create result: %lx\n", result);

        //add elements to the queue
        result = (*queue)->addElement (queue, gXAxisCookie, 0);
        printf ("Queue added x axis result: %lx\n", result);
        result = (*queue)->addElement (queue, gButton1Cookie, 0);
        printf ("Queue added button 1 result: %lx\n", result);
        result = (*queue)->addElement (queue, gButton2Cookie, 0);
        printf ("Queue added button 2 result: %lx\n", result);
        result = (*queue)->addElement (queue, gButton3Cookie, 0);
        printf ("Queue added button 3 result: %lx\n", result);

        //check to see if button 3 is in queue
        hasElement = (*queue)->hasElement (queue,gButton3Cookie);
        printf ("queue has button 3 result: %s\n", hasElement ? "true" : "false");

        //remove button 3 from queue
        result = (*queue)->removeElement (queue, gButton3Cookie);
        printf ("queue remove button 3 result: %lx\n",result);

#if TEST_EVENT_CALLBACK
		{
			CFRunLoopSourceRef tCFRunLoopSourceRef = NULL;

			result = (*queue)->createAsyncEventSource(queue, &tCFRunLoopSourceRef);
			if (kIOReturnSuccess != result)
				printf ("Failed to createAsyncEventSource, error: %ld.\n", result);

			// if we have one now…
			if (NULL != tCFRunLoopSourceRef)
			{
				CFRunLoopRef tCFRunLoopRef = (CFRunLoopRef) GetCFRunLoopFromEventLoop(GetMainEventLoop());

				// and it's not already attached to our runloop…
				if (!CFRunLoopContainsSource(tCFRunLoopRef, tCFRunLoopSourceRef, kCFRunLoopDefaultMode))
					// then attach it now.
					CFRunLoopAddSource(tCFRunLoopRef, tCFRunLoopSourceRef, kCFRunLoopDefaultMode);
			}

			// now install our callback
			result = (*queue)->setEventCallout(queue, MyIOHIDCallbackFunction, queue, queue);
			if (kIOReturnSuccess != result)
				printf ("Failed to setEventCallout, error: %ld.\n", result);
#endif TEST_EVENT_CALLBACK

			//start data delivery to queue
			result = (*queue)->start (queue);
			printf ("queue start result: %lx\n", result);

			//check queue a few times to see accumulated events
			printf ("Testing Event Interface (Mouse button to continue)...\n");
			fflush (stdout);
			while (Button()) {}
			while (!Button())
			{
				AbsoluteTime zeroTime = {0,0};
				result = (*queue)->getNextEvent (queue, &event, zeroTime, 0);
				if (result == kIOReturnUnderrun)
				{
					// print nothing queue empty
				}
				else if (result != kIOReturnSuccess)
					printf ("queue getNextEvent result error: %lx\n", result);
				else
					printf ("queue: event:[%lx] %ld\n", (unsigned long) event.elementCookie, event.value);
				fflush (stdout);
#if TEST_EVENT_CALLBACK
				CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25f, true);
#endif TEST_EVENT_CALLBACK
			}

			//stop data delivery to queue
			result = (*queue)->stop (queue);
			printf ("queue stop result: %lx\n", result);

#if TEST_EVENT_CALLBACK
			// see if we have an Async event source for this queue…
			tCFRunLoopSourceRef = (*queue)->getAsyncEventSource(queue);
			if (NULL != tCFRunLoopSourceRef)	// if so then…
			{
				CFRunLoopRef tCFRunLoopRef = (CFRunLoopRef) GetCFRunLoopFromEventLoop(GetMainEventLoop());

				// if it's attached to our runloop…
				if (CFRunLoopContainsSource(tCFRunLoopRef, tCFRunLoopSourceRef, kCFRunLoopDefaultMode))
					// then remove it
					CFRunLoopRemoveSource(tCFRunLoopRef, tCFRunLoopSourceRef, kCFRunLoopDefaultMode);
				// now release it.
				CFRelease(tCFRunLoopSourceRef);
			}
		}
#endif TEST_EVENT_CALLBACK

        //dispose of queue
        result = (*queue)->dispose (queue);

        //release the queue we allocated
        result = (*queue)->Release (queue);
        printf ("queue release result: %lx\n", result);

		fflush (stdout);
    }
}

// --------------------------------------------------------------------------

 void MyTestPollingInterface (IOHIDDeviceInterface ** hidDeviceInterface)
{
    HRESULT result;
    IOHIDEventStruct hidEvent;

	printf ("Testing Polling Interface (Mouse button to continue)...\n");
    printf("X Axis (%lx), Button 1 (%lx), Button 2 (%lx), Button 3 (%lx)\n",
           (long) gXAxisCookie, (long) gButton1Cookie, (long) gButton2Cookie, (long) gButton3Cookie);
	while (Button()) {}
	while (!Button())
    {
        long xAxis, button1, button2, button3;

        //Get x axis
        result = (*hidDeviceInterface)->getElementValue(hidDeviceInterface, gXAxisCookie, &hidEvent);
        if (result)
            printf("getElementValue error = %lx", result);
        xAxis = hidEvent.value;

        //Get button 1
        result = (*hidDeviceInterface)->getElementValue(hidDeviceInterface, gButton1Cookie, &hidEvent);
        if (result)
            printf("getElementValue error = %lx", result);
        button1 = hidEvent.value;

        //Get button 2
        result = (*hidDeviceInterface)->getElementValue(hidDeviceInterface, gButton2Cookie, &hidEvent);
        if (result)
            printf("getElementValue error = %lx", result);
        button2 = hidEvent.value;

        //Get button 3
        result = (*hidDeviceInterface)->getElementValue (hidDeviceInterface, gButton3Cookie, &hidEvent);
        if (result)
            printf ("getElementValue error = %lx", result);
        button3 = hidEvent.value;

        //Print values
        printf ("xAxis: %ld\t Buttons:%s%s%s\n",
                xAxis, button1 ? " 1 " : "", button2 ? " 2 " : "", button3 ? " 3 " : "");
		fflush (stdout);
		sleep (1);
    }
}

// functions (external/public) ----------------------------------------------

void MyStartHIDDeviceInterfaceTest (void)
{
    mach_port_t masterPort = NULL;
    io_iterator_t hidObjectIterator = NULL;
    IOReturn ioReturnValue = kIOReturnSuccess;

    // Get a Mach port to initiate communication with I/O Kit.
    ioReturnValue = IOMasterPort (bootstrap_port, &masterPort);
    PrintErrMsgIfIOErr (ioReturnValue, "Couldn’t create a master I/O Kit Port.");

    printf("\n\n--- Finding GamePads ---\n");
    hidObjectIterator = MyFindHIDDevices (masterPort, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
    if (hidObjectIterator != 0)
    {
		io_object_t hidDevice = NULL;
		IOHIDDeviceInterface ** pphidDeviceInterface = NULL;
		IOReturn ioReturnValue = kIOReturnSuccess;
		short numDevice = 0;

		while ((hidDevice = IOIteratorNext (hidObjectIterator)))
		{
			printf ("\n--- Device %d ---\n", numDevice++);
			MyShowHIDProperties (hidDevice);
			pphidDeviceInterface = MyCreateHIDDeviceInterface (hidDevice);
			// Release the device
			ioReturnValue = IOObjectRelease (hidDevice);
			PrintErrMsgIfIOErr (ioReturnValue, "Error releasing HID device");
			if (*pphidDeviceInterface != NULL)
			{
				// open the device
				ioReturnValue = (*pphidDeviceInterface)->open (pphidDeviceInterface, 0);
				printf ("Open result = %d\n", ioReturnValue);

				//test the event interface
				MyTestEventInterface (pphidDeviceInterface);

				//test the polling interface
				MyTestPollingInterface (pphidDeviceInterface);

				//close the device
				if (ioReturnValue == KERN_SUCCESS)
				{
					ioReturnValue = (*pphidDeviceInterface)->close (pphidDeviceInterface);
					printf ("Close result = %d\n", ioReturnValue);
				}
				// Release the device interface
				(*pphidDeviceInterface)->Release (pphidDeviceInterface);
			}
		}
		IOObjectRelease (hidObjectIterator);
    }
    fflush (stdout);

    //Free master port if we created one.
    if (masterPort)
        mach_port_deallocate (mach_task_self (), masterPort);
}





void hidUsageLookup(CFTypeRef objectUsagePage, CFTypeRef objectUsage, char **usagePageString, char **usageString)
{
	
	long valueUsage = 0, valueUsagePage = 0;

	if (objectUsagePage && objectUsage){
	
		if (CFNumberGetValue ((const __CFNumber*)objectUsage, kCFNumberLongType, &valueUsage) && CFNumberGetValue ((const __CFNumber*)objectUsagePage, kCFNumberLongType, &valueUsagePage)){

			switch (valueUsagePage){
				case kHIDPage_Undefined:
					*usagePageString = "Undefined_Page";
					switch (valueUsage)
					{
						default:
							*usageString = "Undefined_Usage";
							break;
					}
						break;
				case kHIDPage_GenericDesktop:
					*usagePageString = "Generic_Desktop";

					switch (valueUsage)
					{
						case kHIDUsage_GD_Pointer: *usageString = "Pointer"; break;
						case kHIDUsage_GD_Mouse: *usageString = "Mouse"; break;
						case kHIDUsage_GD_Joystick: *usageString = "Joystick"; break;
						case kHIDUsage_GD_GamePad: *usageString = "Gamepad"; break;
						case kHIDUsage_GD_Keyboard: *usageString = "Keyboard"; break;
						case kHIDUsage_GD_Keypad: *usageString = "Keypad"; break;
						case kHIDUsage_GD_MultiAxisController: *usageString = "Multi-Axis_Controller"; break;

						case kHIDUsage_GD_X: *usageString = "X-Axis"; break;
						case kHIDUsage_GD_Y: *usageString = "Y-Axis"; break;
						case kHIDUsage_GD_Z: *usageString = "Z-Axis"; break;
						
						case kHIDUsage_GD_Rx: *usageString = "X-Rotation"; break;
						case kHIDUsage_GD_Ry: *usageString = "Y-Rotation"; break;
						case kHIDUsage_GD_Rz: *usageString = "Z-Rotation"; break;
						case kHIDUsage_GD_Slider: *usageString = "Slider"; break;
						case kHIDUsage_GD_Dial: *usageString = "Dial"; break;
						case kHIDUsage_GD_Wheel: *usageString = "Wheel"; break;
						case kHIDUsage_GD_Hatswitch: *usageString = "Hat_Switch"; break;
						case kHIDUsage_GD_CountedBuffer: *usageString = "Counted_Buffer"; break;
						case kHIDUsage_GD_ByteCount: *usageString = "Byte_Count"; break;
						case kHIDUsage_GD_MotionWakeup: *usageString = "Motion_Wakeup"; break;
						case kHIDUsage_GD_Start: *usageString = "Start"; break;
						case kHIDUsage_GD_Select: *usageString = "Select"; break;

						/*case kHIDUsage_GD_Vx: printf("X-Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vy: printf("Y-Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vz: printf("Z-Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vbrx: printf("X-Rotation Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vbry: printf("Y-Rotation Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vbrz: printf("Z-Rotation Velocity (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_Vno: printf("Vno (0x%lx)", valueUsage); break;

						case kHIDUsage_GD_SystemControl: printf("System Control (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemPowerDown: printf("System Power Down (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemSleep: printf("System Sleep (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemWakeUp: printf("System Wake Up (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemContextMenu: printf("System Context Menu (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMainMenu: printf("System Main Menu (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemAppMenu: printf("System App Menu (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuHelp: printf("System Menu Help (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuExit: printf("System Menu Exit (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenu: printf("System Menu (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuRight: printf("System Menu Right (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuLeft: printf("System Menu Left (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuUp: printf("System Menu Up (0x%lx)", valueUsage); break;
						case kHIDUsage_GD_SystemMenuDown: printf("System Menu Down (0x%lx)", valueUsage); break;*/

						case kHIDUsage_GD_DPadUp: *usageString = "DPad_Up"; break;
						case kHIDUsage_GD_DPadDown: *usageString = "DPad_Down"; break;
						case kHIDUsage_GD_DPadRight: *usageString = "DPad_Right"; break;
						case kHIDUsage_GD_DPadLeft: *usageString = "DPad_Left"; break;

						//case kHIDUsage_GD_Reserved: printf("Reserved (0x%lx)", valueUsage); break;

						default: *usageString = "Undefined"; break;
					}
						break;
				/*case kHIDPage_Simulation:
					printf("Simulation Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_VR:
					printf("VR Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Sport:
					printf("Sport Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Game:
					printf("Game Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_KeyboardOrKeypad:
					printf("Keyboard or Keypad Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;*/
						
				case kHIDPage_Button:
					*usagePageString = "Button";
					//printf("Button Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default:
							*usageString = (char *)calloc(80, sizeof(char));
							sprintf(*usageString, "Button_%ld", valueUsage); break;
					}
						break;
				/*case kHIDPage_Ordinal:
					printf("Ordinal Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Telephony:
					printf("Telephony Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Consumer:
					printf("Consumer Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Digitizer:
					printf("Digitizer Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_PID:
					printf("PID Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Unicode:
					printf("Unicode Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_AlphanumericDisplay:
					printf("Alphanumeric Display Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_BarCodeScanner:
					printf("Bar Code Scanner Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Scale:
					printf("Scale Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_CameraControl:
					printf("Camera Control Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;
				case kHIDPage_Arcade:
					printf("Arcade Page (0x%lx), ", valueUsagePage);
					switch (valueUsage)
					{
						default: printf("Usage (0x%lx)", valueUsage); break;
					}
						break;*/
				default:
					*usagePageString = "Unknown";
					//printf("Unknown Page (0x%lx), ", valueUsagePage);
					*usageString = "Unknown";
					//printf("Usage (0x%lx)", valueUsage);
					break;
			}
		}
	}
}	



void hidTypeLookup (CFTypeRef objectType, char **type_string)
{

    long valueType = 0;

    if (objectType)
    {
        if (CFNumberGetValue ((const __CFNumber*)objectType, kCFNumberLongType, &valueType))
        {
            MyIndent ();
            switch (valueType)
            {
                case kIOHIDElementTypeInput_Misc:
					*type_string = "Miscellaneous";
                    break;
                case kIOHIDElementTypeInput_Button:
					*type_string = "Button";
                    break;
                case kIOHIDElementTypeInput_Axis:
                    *type_string = "Axis";
                    break;
					
               /* case kIOHIDElementTypeInput_ScanCodes:
                    printf("Scan Code Input Type (%ld), ", valueType);
                    break;
                case kIOHIDElementTypeOutput:
                    printf("Output Type (%ld), ", valueType);
                    break;
                case kIOHIDElementTypeFeature:
                    printf("Feature Type (%ld), ", valueType);
                    break;*/
                case kIOHIDElementTypeCollection:
                    *type_string = "Collection";
                    break;
            }
        }
	}
}
