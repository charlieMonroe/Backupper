//
//  FCFileListing.h
//  Backupper
//
//  Created by Charlie Monroe on 05.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCFileListing_h
#define Backupper_FCFileListing_h

#include "FCFile.h"

typedef enum {
	FCFileListingTypeRegularFile,
	FCFileListingTypeDirectory,
	FCFileListingTypeSymlink
} FCFileListingType;

struct __FCFileListing;
typedef struct __FCFileListing* FCFileListingRef;

FCFileListingRef FCFileListingCreateWithFile(FCFileRef file);
FCFileListingRef FCFileListingCreateWithPath(const char* path);
unsigned long long FCFileListingGetModificationDate(FCFileListingRef listing);
const char *FCFileListingGetPath(FCFileListingRef listing);
unsigned long long FCFileListingGetSize(FCFileListingRef listing);
const char *FCFileListingGetSymlinkPath(FCFileListingRef listing);
FCFileListingType FCFileListingGetType(FCFileListingRef listing);
void FCFileListingRelease(FCFileListingRef listing);
void FCFileListingSetModificationDate(FCFileListingRef listing, unsigned long long date);
void FCFileListingSetSize(FCFileListingRef listing, unsigned long long size);
void FCFileListingSetSymlinkPath(FCFileListingRef listing, const char *symlink);
void FCFileListingSetType(FCFileListingRef listing, FCFileListingType type);

#endif
