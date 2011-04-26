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

#import "PAServerInfo.h"


@implementation PAServerInfo

@synthesize userName;
@synthesize hostName;
@synthesize serverName;
@synthesize version;
@synthesize sampleSpec;
@synthesize channelMap;
@synthesize defaultSinkName;
@synthesize defaultSourceName;
@synthesize cookie;

- (NSDictionary *) dictionary
{
	NSMutableDictionary *d = [NSMutableDictionary dictionaryWithCapacity: 0];
	
	[d setObject: userName
	      forKey: @"User Name"];
	[d setObject: hostName
	      forKey: @"Host Name"];
	[d setObject: version
	      forKey: @"Server Version"];
	[d setObject: sampleSpec
	      forKey: @"Sample Spec"];
	[d setObject: channelMap
	      forKey: @"Channel Map"];
	[d setObject: defaultSinkName
	      forKey: @"Default Sink Name"];
	[d setObject: defaultSourceName
	      forKey: @"Default Source Name"];

	return d;
}


@end
