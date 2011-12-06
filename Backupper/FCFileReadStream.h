//
//  FCFileReadStream.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCFileReadStream_h
#define Backupper_FCFileReadStream_h

// Opaque type
struct __FCFileReadStream;
typedef struct __FCFileReadStream* FCFileReadStreamRef;

FCFileReadStreamRef FCFileReadStreamCreateWithFile(const char *path); // Will return NULL if file doesn't exist

size_t FCFileReadStreamReadData(FCFileReadStreamRef stream, char *buffer, size_t maxLen);

// Dynamically grows an internal buffer until the line fits in and then
// returns a pointer to that buffer itself. Doesn't include the \n char.
const char *FCFileReadStreamReadLine(FCFileReadStreamRef stream);

void FCFileReadStreamRelease(FCFileReadStreamRef stream);

#endif
