/*

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

class DirectDisplay
{
	CGDisplayCount num_displays;
	CGDirectDisplayID* display_ids;

	long get_value(CFDictionaryRef values, CFStringRef key)
	{
		CFNumberRef number_value = (CFNumberRef) CFDictionaryGetValue(values, key);
	
		if (!number_value)
			return -1;
			
		long int_value;
	
		if (!CFNumberGetValue(number_value, kCFNumberLongType, &int_value))
			return -1;
	
		return int_value;
	}

	void DumpDisplayModeValues(CFDictionaryRef values)
	{
		printf("   ----- Display Mode Info for %ld -----\n", get_value(values, kCGDisplayMode));
		printf("   Bounds = %ld x %ld\n", get_value(values, kCGDisplayWidth), get_value(values, kCGDisplayHeight));
		printf("   bpp = %ld, hz = %ld\n", get_value(values, kCGDisplayBitsPerPixel), get_value(values,  kCGDisplayRefreshRate));
	}

public:

	DirectDisplay()
	{
		num_displays = 0;
		display_ids = 0;
	}
	
	int Init()
	{
		 if (CGGetActiveDisplayList(0, NULL, &num_displays) != CGDisplayNoErr || num_displays == 0)
			return 0;
		
		display_ids = new CGDirectDisplayID[num_displays];

		if (CGGetActiveDisplayList(num_displays, display_ids, &num_displays) != CGDisplayNoErr)
			return false;

		return num_displays;
	}
	
	CGRect GetDisplayBounds(CGDisplayCount display_num)
	{
		if (display_num >= num_displays)
			return CGRectMake(0,0,0,0);

		return CGDisplayBounds(display_ids[display_num]);
	}

	u_int32_t GetOpenGLDisplayID(CGDisplayCount display_num)
	{
		if (display_num >= num_displays)
			return 0;
			
		return CGDisplayIDToOpenGLDisplayMask(display_ids[display_num]);
	}

	void DumpCurrentDisplayMode(CGDisplayCount display_num)
	{
		if (display_num >= num_displays)
			return;

		CFDictionaryRef display_mode_values = CGDisplayCurrentMode(display_ids[display_num]);

		DumpDisplayModeValues(display_mode_values);
	}

	void DumpDisplayModes(CGDisplayCount display_num)
	{
		if (display_num >= num_displays)
			return;

		CFArrayRef display_modes = CGDisplayAvailableModes(display_ids[display_num]);
		
		CFIndex num_modes = CFArrayGetCount(display_modes);

		printf("+ %d Display Modes for Display %d (OpenGL mask = %x)\n", (int)num_modes, display_num, CGDisplayIDToOpenGLDisplayMask(display_ids[display_num]));
		
		for (int i = 0; i < num_modes; i++)
			DumpDisplayModeValues((CFDictionaryRef) CFArrayGetValueAtIndex(display_modes, i));
	}
	
	bool SetDisplayMode(CGDisplayCount display_num, const CGSize& size, size_t bpp, CGRefreshRate hz)
	{
		if (display_num >= num_displays)
			return false;

		CFDictionaryRef display_mode_values = CGDisplayBestModeForParametersAndRefreshRate(display_ids[display_num], bpp, (size_t) size.width, (size_t) size.height, hz, NULL);
	
		int display_hz = get_value(display_mode_values,  kCGDisplayRefreshRate);		

		printf("Setting Display Mode to %.0f x %.0f @%dhz\n", size.width, size.height, display_hz);

		CGDisplayErr err = CGDisplaySwitchToMode(display_ids[display_num], display_mode_values);

		return err == CGDisplayNoErr;
	}

	bool Capture(CGDisplayCount display_num)
	{
		if (display_num >= num_displays || CGDisplayIsCaptured(display_ids[display_num]))
			return false;

		return CGDisplayCapture(display_ids[display_num]) == CGDisplayNoErr;
	}
	
	bool Release(CGDisplayCount display_num = 0)
	{
		if (display_num >= num_displays || !CGDisplayIsCaptured(display_ids[display_num]))
			return false;

		return CGDisplayRelease(display_ids[display_num]) == CGDisplayNoErr;
	}
};

