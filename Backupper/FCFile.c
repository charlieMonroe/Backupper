//
//  FCFileRef.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

#include "FCFile.h"
#include "FCRelease.h"
#include "FCTypes.h"
#include "FCString.h"

struct __FCFile{
	char *_path; // +1 for \0 termination
	char *_originalPath;
	struct stat _stat;
	FCArrayRef _directoryContents;
};

static void _FCFileCacheDirectoryListing(FCFileRef file){
	assert(FCFileIsDirectory(file));
	
	const char *source = file->_path;
	DIR *sourceDIR = opendir(file->_path);
	if (!sourceDIR){
		printf("%s - Source %s could not be opened.", __FUNCTION__, source);
		return;
	}
	
	FCArrayRef files = FCArrayCreate();
	
	struct dirent *pent;
	while ((pent = readdir(sourceDIR)) != NULL){
		char *fileName = pent->d_name;
		if (strcmp(".", fileName) == 0 || strcmp("..", fileName) == 0 || strlen(fileName) == 0){
			// The dot or double-dot entry, ignore
			continue;
		}
		
		char *fullFilePath = FCStringByAppendingPathComponent(source, fileName);
		FCFileRef newFileRef = FCFileCreateWithPath(fullFilePath);
		if (newFileRef != NULL){
			FCArrayAddItem(files, newFileRef);
		}
		#if DEBUG
		else{
			printf("%s - *** WARNING, skipping file %s\n", __FUNCTION__, fullFilePath);
		}
		#endif
		
		free(fullFilePath);
	}
	
	file->_directoryContents = files;
}


FCFileRef FCFileCreateWithPath(const char* path){
	FCFileRef fileRef = calloc(1, sizeof(struct __FCFile));
	fileRef->_path = FCStringCopy(path);
	
	if (stat(path, &fileRef->_stat) != 0){
		// Failure, need to deallocate and return NULL
		FCFileRelease(fileRef);
		return NULL;
	}
	
	return fileRef;
}
FCArrayRef FCFileDirectoryListing(FCFileRef file){
	if (!file){
		return NULL;
	}
	
	if (!file->_directoryContents){
		_FCFileCacheDirectoryListing(file);
	}
	
	// Cached contents
	return file->_directoryContents;
}
unsigned long long FCFileGetModificationDate(FCFileRef file){
	return file->_stat.st_mtimespec.tv_sec;
}
unsigned long long FCFileGetSize(FCFileRef file){
	return file->_stat.st_size;
}
BOOL FCFileIsDirectory(FCFileRef file){
	return S_ISDIR(file->_stat.st_mode);
}
BOOL FCFileIsSymlink(FCFileRef file){
	return S_ISLNK(file->_stat.st_mode);
}
BOOL FCFileMakeDirectoryWithIntermediateDirectories(const char *dir){
	char tmp[256];
	char *p = NULL;
	size_t len;
	
	snprintf(tmp, sizeof(tmp),"%s",dir);
	len = strlen(tmp);
	if(tmp[len - 1] == '/')
		tmp[len - 1] = 0;
		for(p = tmp + 1; *p; p++)
			if(*p == '/') {
				*p = 0;
				mkdir(tmp, 0777);
				*p = '/';
			}
	return mkdir(tmp, 0777) == 0;
}
const char *FCFileOriginalPath(FCFileRef file){
	if (!FCFileIsSymlink(file)){
		return NULL;
	}
	if (file->_originalPath){
		return file->_originalPath;
	}
	
	file->_originalPath = malloc(513);
	readlink(file->_path, file->_originalPath, 512);
	
	return file->_originalPath;
}
const char *FCFilePath(FCFileRef file){
	return file->_path;
}
void FCFileRelease(FCFileRef file){
	FCRelease(file->_path);
	FCRelease(file->_originalPath);
	FCRelease(file);
}
void FCFileSetModificationDate(FCFileRef file, unsigned long long modificationDate){
	struct stat *statStr = &file->_stat;
	struct utimbuf new_times;
	new_times.modtime = modificationDate;
	new_times.actime = statStr->st_atimespec.tv_sec;
	statStr->st_mtimespec.tv_sec = modificationDate;
	
	utime(file->_path, &new_times);
	
}

