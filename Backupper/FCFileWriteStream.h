//
//  FCFileWriteStream.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCFileWriteStream_h
#define Backupper_FCFileWriteStream_h

#include "FCTypes.h"
#include "FCArray.h"

// Opaque type
struct __FCFileWriteStream;
typedef struct __FCFileWriteStream* FCFileWriteStreamRef;

FCFileWriteStreamRef FCFileWriteStreamCreateWithFile(const char *path); // Will return NULL if file doesn't exist

void FCFileWriteStreamWriteArray(FCFileWriteStreamRef stream, FCArrayRef arr); // Array mustn't have any other values but strings
void FCFileWriteStreamWriteBOOL(FCFileWriteStreamRef stream, BOOL boolValue);
void FCFileWriteStreamWriteData(FCFileWriteStreamRef stream, const char *data, size_t length);
void FCFileWriteStreamWriteInt(FCFileWriteStreamRef stream, int intValue);
void FCFileWriteStreamWriteFormat(FCFileWriteStreamRef stream, const char *format, ...);
void FCFileWriteStreamWriteString(FCFileWriteStreamRef stream, const char *string);

void FCFileWriteStreamRelease(FCFileWriteStreamRef stream);

#endif
