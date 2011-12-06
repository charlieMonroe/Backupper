//
//  FCOptionValue.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCOptionValue_h
#define Backupper_FCOptionValue_h

#include "FCOption.h"
#include "FCTypes.h"
#include "FCRelease.h"

struct __FCOptionValue;
typedef struct __FCOptionValue* FCOptionValueRef;

// if releaseValue == YES and releaseFunction == NULL, FCRelease is used; FCOptionRef is *not* released.
FCOptionValueRef FCOptionValueCreateWithValue(FCOptionRef option, void *value, 
						BOOL releaseValue, FCReleaseFunction releaseFunction);
FCOptionRef FCOptionValueGetOption(FCOptionValueRef optionValue);
void *FCOptionValueGetValue(FCOptionValueRef optionValue);
void FCOptionValueSetValue(FCOptionValueRef optionValue, void *value);
void FCOptionValueRelease(FCOptionValueRef optionValue);

#endif
