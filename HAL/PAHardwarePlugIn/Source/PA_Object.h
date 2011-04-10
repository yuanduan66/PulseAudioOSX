#ifndef PA_OBJECT_H_
#define PA_OBJECT_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/AudioHardwarePlugIn.h>
#include <CoreAudio/AudioHardware.h>
#include "CAMutex.h"

#ifndef CLASS_NAME
#error CLASS_NAME undefined
#endif

#define DebugLog(format, args...) printf("%s::%s(), line %d: " format "\n", \
					 CLASS_NAME, __func__, __LINE__, ## args);

class PA_Object
{
private:
	AudioObjectID objectID;
	CFMutableArrayRef properties;
	CAMutex *mutex;

public:
	
	PA_Object();
	~PA_Object();
	
	AudioObjectID	GetObjectID();
	void		SetObjectID(AudioObjectID i);
	
#pragma mark ### plugin interface ###

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
	
	void Show();
	
#pragma mark ### mutex ###

	void Lock();
	void Unlock();
	
	virtual PA_Object *FindObjectByID(AudioObjectID searchID) = 0;
};

#endif // PA_OBJECT_H_
