/***
 This file is part of the PulseAudio HAL plugin project
 
 Copyright 2010,2011 Daniel Mack <pulseaudio@zonque.de>
 
 The PulseAudio HAL plugin project is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 The PulseAudio HAL plugin project is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with PulseAudio; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA.
 ***/

#import <Foundation/Foundation.h>

@interface PASinkInfo : NSObject {
	NSString *name;
	NSString *description;
	NSString *sampleSpec;
	NSString *channelMap;
	NSString *driver;
	
	UInt32 latency;
	UInt32 configuredLatency;
	UInt32 nVolumeSteps;
	UInt32 volume;
	
	NSDictionary *properties;
}

@property (assign) NSString *name;
@property (assign) NSString *description;
@property (assign) NSString *sampleSpec;
@property (assign) NSString *channelMap;
@property (assign) NSString *driver;
@property (assign) UInt32 latency;
@property (assign) UInt32 configuredLatency;
@property (assign) UInt32 nVolumeSteps;
@property (assign) UInt32 volume;
@property (assign) NSDictionary *properties;

@end
