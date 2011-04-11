/***
 This file is part of PulseConsole
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 PulseConsole is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 PulseConsole is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#ifndef PA_DEVICE_H_
#define PA_DEVICE_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/AudioHardware.h>
#include <CarbonCore/Multiprocessing.h>

#include "PA_Object.h"

class PA_Plugin;
class PA_Stream;
class PA_DeviceBackend;
class PA_DeviceControl;

typedef struct IOProcTracker
{
	AudioDeviceIOProc proc;
	AudioTimeStamp startTime;
	UInt32 startTimeFlags;
	void *clientData;
	Boolean enabled;
	
	// for one-shot DeviceRead() listeners
	MPSemaphoreID semaphore;
	AudioBufferList *bufferList;
} IOProcTracker;

class PA_Device : public PA_Object
{
private:
	CFMutableArrayRef ioProcList;
	
	CFStringRef deviceName, deviceManufacturer, deviceUID;
	UInt32 nInputStreams, nOutputStreams;
	PA_Stream **inputStreams, **outputStreams;
	
	CAMutex *ioProcListMutex;
	
	UInt32 bufferFrameSize;
	PA_Plugin *plugin;
	
	PA_DeviceBackend *deviceBackend;
	PA_DeviceControl *deviceControl;
	
	Float64 sampleRate;
	
	Boolean isRunning;

public:
	PA_Device(PA_Plugin *inPlugin);
	~PA_Device();
	
	void Initialize();
	void Teardown();

	PA_DeviceBackend *	GetBackend()			{ return deviceBackend; };
	UInt32			GetIOBufferFrameSize()		{ return bufferFrameSize; };
	Float64			GetSampleRate()			{ return sampleRate; };
	CFStringRef		CopyDeviceName()		{ return (CFStringRef) CFRetain(deviceName); };
	Boolean			IsRunning()			{ return isRunning; };
	
	UInt32			CountEnabledIOProcs();

	CFArrayRef		LockIOProcList();
	void			UnlockIOProcList();
	
	AudioStreamBasicDescription streamDescription;
	AudioStreamRangedDescription physicalFormat;
	
#pragma mark ### plugin interface ###

	PA_Stream *GetStreamById(AudioObjectID inObjectID);
	IOProcTracker *FindIOProc(AudioDeviceIOProc inProc);
	IOProcTracker *FindIOProcByID(AudioDeviceIOProcID inProcID);
	
	OSStatus CreateIOProcID(AudioDeviceIOProc inProc,
				void *inClientData,
				AudioDeviceIOProcID *outIOProcID);
	
	OSStatus DestroyIOProcID(AudioDeviceIOProcID inIOProcID);
	
	OSStatus AddIOProc(AudioDeviceIOProc inProc, 
			   void *inClientData);
	
	OSStatus RemoveIOProc(AudioDeviceIOProc inProc);
	
	OSStatus Start(AudioDeviceIOProc inProc);
	
	OSStatus StartAtTime(AudioDeviceIOProc inProc,
			     AudioTimeStamp *ioRequestedStartTime,
			     UInt32 inFlags);
	
	OSStatus Stop(AudioDeviceIOProc inProc);
	
	OSStatus Read(const AudioTimeStamp *inStartTime,
		      AudioBufferList *outData);
	
	OSStatus GetCurrentTime(AudioTimeStamp* outTime);
	
	OSStatus TranslateTime(const AudioTimeStamp *inTime,
			       AudioTimeStamp *outTime);
	
	OSStatus GetNearestStartTime(AudioTimeStamp *ioRequestedStartTime,
				     UInt32 inFlags);
	
	OSStatus GetPropertyInfo(UInt32 inChannel,
				 Boolean isInput,
				 AudioDevicePropertyID
				 inPropertyID,
				 UInt32 *outSize,
				 Boolean *outWritable);
	
	OSStatus GetProperty(UInt32 inChannel,
			     Boolean isInput,
			     AudioDevicePropertyID inPropertyID,
			     UInt32* ioPropertyDataSize,
			     void* outPropertyData);

	OSStatus SetProperty(const AudioTimeStamp *inWhen,
			     UInt32 inChannel,
			     Boolean isInput,
			     AudioDevicePropertyID inPropertyID,
			     UInt32 inPropertyDataSize,
			     const void *inPropertyData);
	
#pragma mark ### properties ###
	virtual Boolean	HasProperty(const AudioObjectPropertyAddress *inAddress);
	
	virtual OSStatus IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
					    Boolean *outIsSettable);
	
	virtual OSStatus GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
					     UInt32 inQualifierDataSize,
					     const void *inQualifierData,
					     UInt32 *outDataSize);
	
	virtual OSStatus GetPropertyData(const AudioObjectPropertyAddress *inAddress,
					 UInt32 inQualifierDataSize,
					 const void *inQualifierData,
					 UInt32 *ioDataSize,
					 void *outData);
	
	virtual OSStatus SetPropertyData(const AudioObjectPropertyAddress *inAddress,
					 UInt32 inQualifierDataSize,
					 const void *inQualifierData,
					 UInt32 inDataSize,
					 const void *inData);

#pragma mark ### internal stuff ###
	void		EnableAllIOProcs(Boolean enable);
	void		SetBufferSize(UInt32 size);
	OSStatus	RegisterObjects();
	UInt32		GetFrameSize();

	PA_Object *FindObjectByID(AudioObjectID searchID);
};

#endif // PA_DEVICE_H_
