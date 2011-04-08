#include "PA_Device.h"
#include "CAAudioBufferList.h"

#include <pulse/pulseaudio.h>

#define super PA_Object

PA_Device::PA_Device()
{
	ioProcList = CFArrayCreateMutable(NULL, 0, NULL);
	ioProcListMutex = new CAMutex("ioProcListMutex");
}

PA_Device::~PA_Device()
{
	for (SInt32 i = 0; CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		pa_xfree(io);
	}

	CFRelease(ioProcList);
	ioProcList = NULL;
	
	delete ioProcListMutex;
}

IOProcTracker *
PA_Device::FindIOProc(AudioDeviceIOProc inProc)
{
	for (SInt32 i = 0; CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io->proc == inProc)
			return io;
	}

	return NULL;
}

IOProcTracker *
PA_Device::FindIOProcByID(AudioDeviceIOProcID inProcID)
{
	for (SInt32 i = 0; CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io == (IOProcTracker *) inProcID)
			return io;
	}
	
	return NULL;
}

void
PA_Device::Initialize()
{
	deviceName = CFStringCreateWithCString(NULL, "PulseAudio", kCFStringEncodingASCII);
	deviceManufacturer = CFStringCreateWithCString(NULL, "PulseAudio", kCFStringEncodingASCII);
}

void
PA_Device::Teardown()
{
	CFRelease(deviceName);
	CFRelease(deviceManufacturer);
	
	deviceName = NULL;
	deviceManufacturer = NULL;
}

PA_Stream *
PA_Device::GetStreamById(AudioObjectID inObjectID)
{
	SInt32 i;
	
	for (i = 0; i < CFArrayGetCount(streams); i++) {
		PA_Stream *stream = (PA_Stream *) CFArrayGetValueAtIndex(streams, i);
		if (stream->GetObjectID() == inObjectID)
			return stream;
	}
	
	return NULL;
}

PA_Object *
PA_Device::findObjectById(AudioObjectID searchID)
{
	if (GetObjectID() == searchID)
		return this;

	SInt32 i;
	PA_Object *o = NULL;
	
	for (i = 0; i < CFArrayGetCount(streams); i++) {
		PA_Stream *stream = (PA_Stream *) CFArrayGetValueAtIndex(streams, i);
		o = stream->findObjectById(searchID);

		if (o)
			break;
	}
	
	return o;
}

OSStatus
PA_Device::CreateIOProcID(AudioDeviceIOProc inProc,
			  void *inClientData,
			  AudioDeviceIOProcID *outIOProcID)
{
	ioProcListMutex->Lock();

	if (FindIOProc(inProc)) {
		DebugLog("IOProc has already been added");
		ioProcListMutex->Unlock();
		return kAudioHardwareIllegalOperationError;
	}
	
	IOProcTracker *io = pa_xnew0(IOProcTracker, 1);
	io->proc = inProc;
	io->clientData = inClientData;
	
	if (outIOProcID)
		*outIOProcID = (AudioDeviceIOProcID) io;

	CFArrayAppendValue(ioProcList, io);
	ioProcListMutex->Unlock();
	
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::DestroyIOProcID(AudioDeviceIOProcID inIOProcID)
{
	ioProcListMutex->Lock();

	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io == (IOProcTracker *) inIOProcID) {
			pa_xfree(io);
			CFArrayRemoveValueAtIndex(ioProcList, i);
			ioProcListMutex->Unlock();
			return kAudioHardwareNoError;
		}
	}
	
	ioProcListMutex->Unlock();
	DebugLog("IOProc has not been added");
	return kAudioHardwareIllegalOperationError;
}

OSStatus
PA_Device::AddIOProc(AudioDeviceIOProc inProc, 
		     void *inClientData)
{
	return CreateIOProcID(inProc, inClientData, NULL);
}

OSStatus
PA_Device::RemoveIOProc(AudioDeviceIOProc inProc)
{
	ioProcListMutex->Lock();
	
	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		if (io->proc == inProc) {
			pa_xfree(io);
			CFArrayRemoveValueAtIndex(ioProcList, i);
			ioProcListMutex->Unlock();
			return kAudioHardwareNoError;
		}
	}
	
	ioProcListMutex->Unlock();
	DebugLog("IOProc has not been added");
	return kAudioHardwareIllegalOperationError;
}

OSStatus
PA_Device::Start(AudioDeviceIOProc inProc)
{
	return StartAtTime(inProc, NULL, 0);
}

OSStatus
PA_Device::StartAtTime(AudioDeviceIOProc inProc,
		       AudioTimeStamp *ioRequestedStartTime,
		       UInt32 inFlags)
{
	ioProcListMutex->Lock();
	IOProcTracker *io = FindIOProc(inProc);
	
	if (io) {
		io->enabled = true;
		io->startTimeFlags = inFlags;

		if (ioRequestedStartTime)
			memcpy(&io->startTime, ioRequestedStartTime, sizeof(*ioRequestedStartTime));
	}

	ioProcListMutex->Unlock();
	
	if (io)
		return kAudioHardwareNoError;
	else {
		DebugLog("IOProc has not been added");
		return kAudioHardwareIllegalOperationError;
	}
}

OSStatus
PA_Device::Stop(AudioDeviceIOProc inProc)
{
	ioProcListMutex->Lock();
	IOProcTracker *io = FindIOProc(inProc);
	
	if (io)
		io->enabled = false;
	
	ioProcListMutex->Unlock();
	
	if (io)
		return kAudioHardwareNoError;
	else {
		DebugLog("IOProc has not been added");
		return kAudioHardwareIllegalOperationError;
	}
}

OSStatus
PA_Device::Read(const AudioTimeStamp *inStartTime,
		AudioBufferList *outData)
{
	// FIXME
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::GetCurrentTime(AudioTimeStamp *outTime)
{
	memset(outTime, 0, sizeof(*outTime));
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::TranslateTime(const AudioTimeStamp *inTime,
			 AudioTimeStamp *outTime)
{
	memset(outTime, 0, sizeof(*outTime));
	return kAudioHardwareNoError;
}

OSStatus
PA_Device::GetNearestStartTime(AudioTimeStamp *ioRequestedStartTime,
			       UInt32 inFlags)
{
	memset(ioRequestedStartTime, 0, sizeof(*ioRequestedStartTime));
	return kAudioHardwareNoError;
}

void
PA_Device::EnableAllIOProcs(Boolean enabled)
{
	ioProcListMutex->Lock();

	for (SInt32 i = 0; i < CFArrayGetCount(ioProcList); i++) {
		IOProcTracker *io = (IOProcTracker *) CFArrayGetValueAtIndex(ioProcList, i);
		io->enabled = enabled;
	}

	ioProcListMutex->Unlock();
}

void
PA_Device::SetBufferSize(UInt32 size)
{
}

#pragma mark ### properties ###

Boolean
PA_Device::HasProperty(const AudioObjectPropertyAddress *inAddress)
{	
	switch (inAddress->mSelector) {
		case kAudioObjectPropertyName:
		case kAudioObjectPropertyManufacturer:
		case kAudioObjectPropertyElementName:
		case kAudioObjectPropertyElementCategoryName:
		case kAudioObjectPropertyElementNumberName:
		case kAudioDevicePropertyConfigurationApplication:
		case kAudioDevicePropertyDeviceUID:
		case kAudioDevicePropertyModelUID:
		case kAudioDevicePropertyTransportType:
		case kAudioDevicePropertyRelatedDevices:
		case kAudioDevicePropertyClockDomain:
		case kAudioDevicePropertyDeviceIsAlive:
		case kAudioDevicePropertyDeviceHasChanged:
		case kAudioDevicePropertyDeviceIsRunning:
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
		case kAudioDeviceProcessorOverload:
		case kAudioDevicePropertyHogMode:
		case kAudioDevicePropertyLatency:
		case kAudioDevicePropertyBufferFrameSize:
		case kAudioDevicePropertyBufferFrameSizeRange:
		case kAudioDevicePropertyStreams:
		case kAudioDevicePropertySafetyOffset:
		case kAudioDevicePropertyStreamConfiguration:
		case kAudioDevicePropertyIOProcStreamUsage:
		case kAudioDevicePropertyActualSampleRate:
		case kAudioDevicePropertyDeviceName:
		case kAudioDevicePropertyDeviceManufacturer:
		case kAudioDevicePropertyChannelName:
		case kAudioDevicePropertyChannelCategoryName:
		case kAudioDevicePropertyChannelNumberName:
		case kAudioDevicePropertyBufferSize:
		case kAudioDevicePropertyBufferSizeRange:
		//case kAudioDevicePropertyIcon:
		case kAudioDevicePropertyIsHidden:
			return true;
			
	}
	
	return super::HasProperty(inAddress);
}

OSStatus
PA_Device::IsPropertySettable(const AudioObjectPropertyAddress *inAddress,
			      Boolean *outIsSettable)
{
	switch (inAddress->mSelector) {
		case kAudioDevicePropertyDeviceIsRunning:
		case kAudioDevicePropertyBufferFrameSize:
		case kAudioDevicePropertyBufferSize:
			return true;
	}
	
	return super::IsPropertySettable(inAddress, outIsSettable);
}

OSStatus
PA_Device::GetPropertyDataSize(const AudioObjectPropertyAddress *inAddress,
			       UInt32 inQualifierDataSize,
			       const void *inQualifierData,
			       UInt32 *outDataSize)
{
	switch (inAddress->mSelector) {
		case kAudioObjectPropertyName:
		case kAudioObjectPropertyManufacturer:
		case kAudioObjectPropertyElementName:
		case kAudioObjectPropertyElementCategoryName:
		case kAudioObjectPropertyElementNumberName:
		case kAudioDevicePropertyConfigurationApplication:
		case kAudioDevicePropertyDeviceUID:
		case kAudioDevicePropertyModelUID:
			return sizeof(CFStringRef);

		case kAudioDevicePropertyRelatedDevices:
			return sizeof(AudioObjectID);
			
		case kAudioDevicePropertyTransportType:
		case kAudioDevicePropertyClockDomain:
		case kAudioDevicePropertyDeviceIsAlive:
		case kAudioDevicePropertyDeviceHasChanged:
		case kAudioDevicePropertyDeviceIsRunning:
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
		case kAudioDeviceProcessorOverload:
		case kAudioDevicePropertyLatency:
		case kAudioDevicePropertyBufferFrameSize:
		case kAudioDevicePropertySafetyOffset:
		case kAudioDevicePropertyBufferSize:
		case kAudioDevicePropertyIsHidden:
			return sizeof(UInt32);
			
		case kAudioDevicePropertyHogMode:
			return sizeof(pid_t);

		case kAudioDevicePropertyBufferFrameSizeRange:
		case kAudioDevicePropertyBufferSizeRange:
			return sizeof(AudioValueRange);
			
		case kAudioDevicePropertyStreams:
			//theAnswer = SizeOf32(AudioStreamID) * GetNumberStreams(isInput);
			return 0;
			
		case kAudioDevicePropertyStreamConfiguration:
			return CAAudioBufferList::CalculateByteSize(nInputStreams);

		case kAudioDevicePropertyIOProcStreamUsage:
			//theAnswer = SizeOf32(void*) + SizeOf32(UInt32) + (GetNumberStreams(isInput) * SizeOf32(UInt32));
			return 0;

		case kAudioDevicePropertyActualSampleRate:
			return sizeof(Float64);
			
		case kAudioDevicePropertyDeviceName:
			return CFStringGetLength(deviceName);
			
		case kAudioDevicePropertyDeviceManufacturer:
			return CFStringGetLength(deviceManufacturer);
			
		case kAudioDevicePropertyChannelName:
		case kAudioDevicePropertyChannelCategoryName:
		case kAudioDevicePropertyChannelNumberName:
			return 0;

		//case kAudioDevicePropertyIcon:
		//	return sizeof(CFURLRef);
	}
	
	return super::GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData, outDataSize);
}

OSStatus
PA_Device::GetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 *ioDataSize,
			   void *outData)
{
	return super::GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus
PA_Device::SetPropertyData(const AudioObjectPropertyAddress *inAddress,
			   UInt32 inQualifierDataSize,
			   const void *inQualifierData,
			   UInt32 inDataSize,
			   const void *inData)
{
	switch (inAddress->mSelector) {
		case kAudioDevicePropertyDeviceIsRunning:
			EnableAllIOProcs(!!(*(UInt32 *) inData));
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferFrameSize:
			SetBufferSize(*(UInt32 *) inData);
			return kAudioHardwareNoError;

		case kAudioDevicePropertyBufferSize:
			SetBufferSize((*(UInt32 *) inData) * 8);
			return kAudioHardwareNoError;
	}
	
	return super::SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

#pragma mark ### properties (legacy interface) ###

OSStatus
PA_Device::GetPropertyInfo(UInt32 inChannel,
			   Boolean isInput,
			   AudioDevicePropertyID inPropertyID,
			   UInt32 *outSize,
			   Boolean *outWritable)
{
	AudioObjectPropertyAddress addr;
	OSStatus ret;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = isInput ? kAudioDevicePropertyScopeInput :
				kAudioDevicePropertyScopeOutput;
	
	ret = IsPropertySettable(&addr, outWritable);
	if (ret != kAudioHardwareNoError)
		return ret;
	
	return GetPropertyDataSize(&addr, 0, NULL, outSize);
}

OSStatus
PA_Device::GetProperty(UInt32 inChannel,
		       Boolean isInput,
		       AudioDevicePropertyID inPropertyID,
		       UInt32* ioPropertyDataSize,
		       void* outPropertyData)
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = isInput ? kAudioDevicePropertyScopeInput :
				kAudioDevicePropertyScopeOutput;

	return GetPropertyData(&addr, 0, NULL, ioPropertyDataSize, outPropertyData);
}

OSStatus
PA_Device::SetProperty(const AudioTimeStamp * /* inWhen */,
		       UInt32 inChannel,
		       Boolean isInput,
		       AudioDevicePropertyID inPropertyID,
		       UInt32 inPropertyDataSize,
		       const void *inPropertyData)
{
	AudioObjectPropertyAddress addr;
	
	addr.mSelector = inPropertyID;
	addr.mElement = inChannel;
	addr.mScope = isInput ? kAudioDevicePropertyScopeInput :
				kAudioDevicePropertyScopeOutput;
	
	return SetPropertyData(&addr, 0, NULL, inPropertyDataSize, inPropertyData);
}

