//
//  FCFile.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCFile_h
#define Backupper_FCFile_h

#include "FCArray.h"
#include "FCTypes.h"

// Opaque file type
struct __FCFile;
typedef struct __FCFile* FCFileRef;

/** Path is copied, no need to hold it in memory. */
FCFileRef FCFileCreateWithPath(const char* path);

/** Do *NOT* release. Is cached within the __FCFile struct 
 *   and will be released with the file itself. 
 *   Is an array of FCFileRef.
 */
FCArrayRef FCFileDirectoryListing(FCFileRef file);

unsigned long long FCFileGetModificationDate(FCFileRef file);
unsigned long long FCFileGetSize(FCFileRef file);
BOOL FCFileIsDirectory(FCFileRef file);
BOOL FCFileIsSymlink(FCFileRef file);
BOOL FCFileMakeDirectoryWithIntermediateDirectories(const char *dir);
const char *FCFileOriginalPath(FCFileRef file); // Only works on symlinks.
const char *FCFilePath(FCFileRef file);
void FCFileRelease(FCFileRef file);
void FCFileSetModificationDate(FCFileRef file, unsigned long long modificationDate);


#endif
