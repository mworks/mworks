// Interface procedures for the Instrutech ITC18.

//

// Copyright (c) 1991, 1995 Instrutech Corporation, Great Neck, NY, USA

//

// Created by SKALAR Instruments, Seattle, WA, USA



#ifndef ITC18_H

#define ITC18_H



#ifdef __cplusplus

extern "C"

{

#endif



// ITC_CALL is a manifest constant used as a prefix to all function

// prototypes. If a particular prefix is needed (e.g. "PASCAL"), define

// ITC_CALL appropriately.



#ifndef ITC_CALL

#define ITC_CALL

#endif



// Application interface functions

//

// Functions that return a status value return zero if the function

// succeeds, and non-zero if the function fails. To interpret a

// status value, use ITC18_GetStatusText.





// Function ITC18_GetStatusText

//

// Translate a status value to a text string. The translated string

// is written into "text" with maximum length "length" characters.



ITC_CALL int ITC18_GetStatusText(void* device, int status, char* text, int length);





// Function ITC18_GetStructureSize

//

// Return the size of the structure that defines the device.



ITC_CALL int ITC18_GetStructureSize();



// ITC18_Open

//

// Open the device driver.



ITC_CALL int ITC18_Open(void* device, int device_number);





// ITC18_Close

//

// Close the device and free resources associated with it.



ITC_CALL void ITC18_Close(void* device);



// ITC18_Initialize

//

// Initialize the device hardware.



ITC_CALL int ITC18_Initialize(void* device);



// ITC18_InitializeCustom

//

// Initialize the device hardware.

// 'control_data' and 'isolated_data' are pointers to buffers containing

// LCA data. If a pointer is NULL, the corresponding default LCA is used.



ITC_CALL int ITC18_InitializeCustom(

	void* device,

	void* control_data,

	void* isolated_data

);



// Function ITC18_GetFIFOSize

//

// Return the size of the FIFO in samples.



ITC_CALL int ITC18_GetFIFOSize(void* device);



// The EEPROM total and sector sizes in bytes.



#define	ITC18_ROM_SIZE (32 * 1024)

#define ITC18_ROM_SECTOR_SIZE 128



// Function ITC18_ReadROM

//

// Read a ROM sector.

// 'address' is the starting address of the transfer, and must be a

// multiple of ITC18_SECTOR_SIZE in the range

// 0..ITC18_ROM_SIZE-ITC18_SECTOR_SIZE. 'data' is a pointer to a

// buffer of length ITC18_ROM_SECTOR_SIZE. The data in the ROM is

// written into the buffer.



ITC_CALL int ITC18_ReadROM(void* device, int address, void* data);



// Function ITC18_WriteROM

//

// Write a ROM sector.

// 'address' is the starting address of the transfer, and must be a

// multiple of ITC18_SECTOR_SIZE in the range

// 0..ITC18_ROM_SIZE-ITC18_SECTOR_SIZE. 'data' is a pointer to a

// buffer of length ITC18_ROM_SECTOR_SIZE. The data in the buffer

// is written to the ROM.



ITC_CALL int ITC18_WriteROM(void* device, int address, void* data);



// ITC18_SetSamplingInterval

//

// The sampling rate is represented in timer ticks of 1.25 usecs.

// Non zero 'external_clock' causes the ITC-18 to synchronous with

// an external clock.



#define ITC18_MINIMUM_TICKS 4

#define ITC18_MAXIMUM_TICKS 65535



ITC_CALL int ITC18_SetSamplingInterval(void* device, int timer_ticks, int external_clock);



// Function ITC18_WriteAuxiliaryDigitalOutput

//

// Write the auxiliary digital outputs asynchronously.



ITC_CALL int ITC18_WriteAuxiliaryDigitalOutput(void* device, int data);



// Function ITC18_SetDigitalInputMode

//

// Set the latch and inversion mode for the digital inputs.

// The default settings are non-latched and non-inverted.



ITC_CALL int ITC18_SetDigitalInputMode(void* device, int latch, int invert);



// ITC18_SetSequence

//

// An array of(length) 16 bit instructions is written out to the sequence RAM.

// Acquisition must be stopped to write to the sequence RAM.



// ITC18 Sequence memory parameters



#define ITC18_SEQUENCE_RAM_SIZE (4*1024)



#define ITC18_OUTPUT_UPDATE 0x8000

#define ITC18_INPUT_UPDATE 0x4000



#define ITC18_OUTPUT_DA0 0x0000

#define ITC18_OUTPUT_DA1 0x0800

#define ITC18_OUTPUT_DA2 0x1000

#define ITC18_OUTPUT_DA3 0x1800



#define ITC18_OUTPUT_DIGITAL0 0x2000

#define ITC18_OUTPUT_DIGITAL1 0x2800

#define ITC18_OUTPUT_SKIP	0x3800



#define ITC18_INPUT_AD0 0x0000

#define ITC18_INPUT_AD1 0x0080

#define ITC18_INPUT_AD2 0x0100

#define ITC18_INPUT_AD3 0x0180

#define ITC18_INPUT_AD4 0x0200

#define ITC18_INPUT_AD5 0x0280

#define ITC18_INPUT_AD6 0x0300

#define ITC18_INPUT_AD7 0x0380



#define ITC18_INPUT_DIGITAL 0x0400

#define ITC18_INPUT_SKIP 0x0780



ITC_CALL int ITC18_SetSequence(void* device, int length, int* instructions);





// ITC18_SetRange

//

// The range vector must be ITC18_AD_CHANNELS entries long.  Each entry

// contains an ITC18_AD_RANGE_ value.  SetRange may be called whether

// acquisition is in progress or not.



#define ITC18_AD_CHANNELS 8



#define ITC18_AD_RANGE_10V 0

#define ITC18_AD_RANGE_5V 1

#define ITC18_AD_RANGE_2V 2

#define ITC18_AD_RANGE_1V 3



ITC_CALL int ITC18_SetRange(void* device, int* range);



// Function ITC18_SetExternalTriggerMode

//

// Non zero 'transition' triggers on a transition only,

// 'invert' triggers low.

// By default the ITC-18 will trigger high without sensing

// transitions.



ITC_CALL int ITC18_SetExternalTriggerMode(void* device, int transition, int invert);



// ITC18_InitializeAcquisition

//

// This routine must be called before acquisition can be performed after power

// up, and after each call to ITC18_Stop.



ITC_CALL int ITC18_InitializeAcquisition(void* device);





// ITC18_Start

//

// Initiate acquisition.

//

// 'external_trigger' non-zero causes data acquisition to begin on an external

// trigger signal. 'output_enable' must be non-zero to activate the outputs,

// including the D/A converters.



ITC_CALL int ITC18_Start(void* device, int external_trigger, int output_enable);





// ITC18_GetFIFOWriteAvailable

//

// Return the number of FIFO entries available for writing.



ITC_CALL int ITC18_GetFIFOWriteAvailable(void* device, int* available);





// ITC18_WriteFIFO

//

// The buffer of length entries is written to the ITC18.

//

// Any value from 1 to the value returned by ITC18_GetFIFOWriteAvailable may

// be used as the length argument.



ITC_CALL int ITC18_WriteFIFO(void* device, int length, short* buffer);





// ITC18_GetFIFOReadAvailable

//

// Return the number of acquired samples not yet read out of the FIFO.



ITC_CALL int ITC18_GetFIFOReadAvailable(void* device, int* available);





// ITC18_ReadFIFO

//

// The buffer is filled with length entries from the ITC18.

//

// Any value from 1 to the value returned by ITC18_GetFIFOReadAvailable may

// be used as the length argument.



ITC_CALL int ITC18_ReadFIFO(void* device, int length, short* buffer);





// ITC18_IsClipping

//

// Set the "clipping" parameter to the state of the clipping line. If

// the clipping line is inactive, "clipping" is set to zero. If the

// clipping line is active, "clipping" is set to a non-zero value.

// The clipping latch is reset.



ITC_CALL int ITC18_IsClipping(void* device, int* clipping);





// ITC18_GetFIFOOverflow

//

// The 'overflow' parameter is set to zero if input FIFO overflow has

// not occurred, and non-zero if input FIFO overflow has occurred.



ITC_CALL int ITC18_GetFIFOOverflow(void* device, int* overflow);





// ITC18_Stop

//

// End acquisition immediately.



ITC_CALL int ITC18_Stop(void* device);





// ITC18_GetStatusOverflow

//

// Return the status value that indicates FIFO overflow.



ITC_CALL int ITC18_GetStatusOverflow(void* device);



#ifdef __cplusplus

}

#endif



#endif

