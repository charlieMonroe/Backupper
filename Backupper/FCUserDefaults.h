//
//  FCUserDefaults.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCUserDefaults_h
#define Backupper_FCUserDefaults_h

#include "FCArray.h"
#include "FCOptionValue.h"
#include "FCTypes.h"

// Opaque type
struct __FCUserDefaults;
typedef struct __FCUserDefaults* FCUserDefaultsRef;

// Allowed options are released on FCUserDefaultsRelease, and it must be an array of FCOption
FCUserDefaultsRef FCUserDefaultsCreateWithFile(const char *path, FCArrayRef allowedOptions);
int FCUserDefaultsRunMain(FCUserDefaultsRef defaults);
void FCUserDefaultsRelease(FCUserDefaultsRef defaults);
void FCUserDefaultsSave(FCUserDefaultsRef defaults);
FCOptionValueRef FCUserDefaultsValueForOption(FCUserDefaultsRef defaults, FCOptionRef option);
FCOptionValueRef FCUserDefaultsValueForOptionWithName(FCUserDefaultsRef defaults, const char *name);

// FCUserDefaults supplies the variable, but you need to set it.
extern BOOL FCRunInDebugMode;

#endif
