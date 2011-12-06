//
//  FCOption.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCOption_h
#define Backupper_FCOption_h

#include "FCTypes.h"

typedef enum {
	FCOptionTypeBool,
	FCOptionTypeInt,
	FCOptionTypeString,
	FCOptionTypeArray
} FCOptionType;

/**** WARNING: Option name must be 32 or less characters. ****/

// Opaque type
struct __FCOption;
typedef struct __FCOption* FCOptionRef;

// Name is copied
FCOptionRef FCOptionCreateWithName(const char *name, FCOptionType type);
const char *FCOptionName(FCOptionRef option);
FCOptionType FCOptionGetType(FCOptionRef option);

#endif
