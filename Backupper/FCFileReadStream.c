//
//  FCFileReadStream.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>

#include "FCFileReadStream.h"
#include "FCRelease.h"

static const int kFCFileReadStreamBufferSize = 512;

struct __FCFileReadStream{
	FILE *_file;
	char _buffer[kFCFileReadStreamBufferSize];
	int _readLineBufferSize;
	char *_readLineBuffer; // Allocated dynamically, can grow
};

FCFileReadStreamRef FCFileReadStreamCreateWithFile(const char *path){
	FILE *file = fopen(path, "r");
	if (file == NULL){
		return NULL;
	}
	
	FCFileReadStreamRef stream = calloc(1, sizeof(struct __FCFileReadStream));
	stream->_file = file;
	return stream;
}
size_t FCFileReadStreamReadData(FCFileReadStreamRef stream, char *buffer, size_t maxLen){
	size_t readBytes = fread(buffer, 1, maxLen, stream->_file);
	return readBytes;
}
const char *FCFileReadStreamReadLine(FCFileReadStreamRef stream){
	while (fgets(stream->_readLineBuffer, stream->_readLineBufferSize, stream->_file) == NULL){
		// Some kind of error
		if (feof(stream->_file)){
			// End of file -> return NULL
			return NULL;
		}
		
		if (ferror(stream->_file) != 0){
			// Some kind of error. When error is 0, then it's just a buffer too small
			return NULL;
		}
		
		stream->_readLineBufferSize *= 2;
		if (stream->_readLineBufferSize == 0){
			stream->_readLineBufferSize = kFCFileReadStreamBufferSize;
		}
		
		free(stream->_readLineBuffer);
		stream->_readLineBuffer = calloc(1, stream->_readLineBufferSize);
	}
	
	// Remove the trailing new line
	size_t lineLength = strlen(stream->_readLineBuffer);
	if (lineLength > 0 && stream->_readLineBuffer[lineLength - 1] == '\n'){
		stream->_readLineBuffer[lineLength - 1] = '\0';
	}
	return stream->_readLineBuffer;
}

void FCFileReadStreamRelease(FCFileReadStreamRef stream){
	if (stream == NULL){
		return;
	}
	fclose(stream->_file);
	if (stream->_readLineBuffer != NULL){
		free(stream->_readLineBuffer);
	}
	FCRelease(stream);
}


