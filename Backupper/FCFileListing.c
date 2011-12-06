//
//  FCFileListing.c
//  Backupper
//
//  Created by Charlie Monroe on 05.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "FCString.h"
#include "FCFileListing.h"
#include "FCRelease.h"

struct __FCFileListing{
	char *_path;
	char *_symlinkPath;
	unsigned long long _size;
	unsigned long long _modificationDate;
	FCFileListingType _type;
};

FCFileListingRef FCFileListingCreateWithFile(FCFileRef file){
	FCFileListingRef listing = FCFileListingCreateWithPath(FCFilePath(file));
	if (FCFileIsDirectory(file)){
		listing->_type = FCFileListingTypeDirectory;
	}else if (FCFileIsSymlink(file)){
		listing->_type = FCFileListingTypeSymlink;
		FCFileListingSetSymlinkPath(listing, FCFileOriginalPath(file));
	}else{
		listing->_size = FCFileGetSize(file);
		listing->_modificationDate = FCFileGetModificationDate(file);
	}
	
	return listing;
}
FCFileListingRef FCFileListingCreateWithPath(const char* path){
	FCFileListingRef listing = calloc(1, sizeof(struct __FCFileListing));
	listing->_path = FCStringCopy(path);
	return listing;
}
unsigned long long FCFileListingGetModificationDate(FCFileListingRef listing){
	return listing->_modificationDate;
}
const char *FCFileListingGetPath(FCFileListingRef listing){
	return listing->_path;
}
unsigned long long FCFileListingGetSize(FCFileListingRef listing){
	return listing->_size;
}
const char *FCFileListingGetSymlinkPath(FCFileListingRef listing){
	return listing->_symlinkPath;
}
FCFileListingType FCFileListingGetType(FCFileListingRef listing){
	return listing->_type;
}
void FCFileListingRelease(FCFileListingRef listing){
	FCRelease(listing->_path);
	FCRelease(listing->_symlinkPath);
	FCRelease(listing);
}
void FCFileListingSetModificationDate(FCFileListingRef listing, unsigned long long date){
	listing->_modificationDate = date;
}
void FCFileListingSetSize(FCFileListingRef listing, unsigned long long size){
	listing->_size = size;
}
void FCFileListingSetSymlinkPath(FCFileListingRef listing, const char *symlink){
	FCRelease(listing->_symlinkPath);
	listing->_symlinkPath = FCStringCopy(symlink);
}
void FCFileListingSetType(FCFileListingRef listing, FCFileListingType type){
	listing->_type = type;
}

