//
//  FCString.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCString_h
#define Backupper_FCString_h

#include "FCTypes.h"

typedef const char * FCStringRef;

char *FCStringCopy(const char *string);

// Needs to be freed by caller
char *FCStringLastPathComponent(const char *path);

// Needs to be freed by caller
char *FCStringByAppendingPathComponent(const char *path, const char *pathComponent);

// Needs to be freed by caller
char *FCStringByAppendingString(const char *str1, const char *str2);

// Needs to be freed by caller
char *FCStringByExpandingTildeInPath(const char *path);

// Gets a line from stdin
// Need to be freed by caller
char *FCStringGetLine(void);

BOOL FCStringHasPrefix(const char *str, const char *prefix);

unsigned long long FCStringUnsignedLongLongValue(const char *str);

BOOL FCStringsEqual(const char *str1, const char *str2);

#endif
