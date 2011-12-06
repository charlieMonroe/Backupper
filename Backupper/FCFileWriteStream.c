//
//  FCFileWriteStream.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "FCFileWriteStream.h"
#include "FCRelease.h"

struct __FCFileWriteStream{
	FILE *_file;
};

FCFileWriteStreamRef FCFileWriteStreamCreateWithFile(const char *path){
	FILE *file = fopen(path, "w");
	if (file == NULL){
		return NULL;
	}
	
	FCFileWriteStreamRef stream = calloc(1, sizeof(struct __FCFileWriteStream));
	stream->_file = file;
	return stream;
}
void FCFileWriteStreamWriteArray(FCFileWriteStreamRef stream, FCArrayRef arr){
	for (int i = 0; i < FCArrayCount(arr); ++i){
		char *strValue = FCArrayItemAtIndex(arr, i);
		FCFileWriteStreamWriteString(stream, strValue);
		if (i != FCArrayCount(arr) - 1){
			FCFileWriteStreamWriteString(stream, ";");
		}
	}
}
void FCFileWriteStreamWriteBOOL(FCFileWriteStreamRef stream, BOOL boolValue){
	if (boolValue){
		FCFileWriteStreamWriteString(stream, "YES");
	}else{
		FCFileWriteStreamWriteString(stream, "NO");
	}
}
void FCFileWriteStreamWriteData(FCFileWriteStreamRef stream, const char *data, size_t length){
	fwrite(data, length, 1, stream->_file);
}
void FCFileWriteStreamWriteFormat(FCFileWriteStreamRef stream, const char *format, ...){
	va_list argList;
	va_start(argList, format);
	vfprintf(stream->_file, format, argList);
	va_end(argList);
}
void FCFileWriteStreamWriteInt(FCFileWriteStreamRef stream, int intValue){
	fprintf(stream->_file, "%i", intValue);
}
void FCFileWriteStreamWriteString(FCFileWriteStreamRef stream, const char *string){
	fprintf(stream->_file, "%s", string);
}

void FCFileWriteStreamRelease(FCFileWriteStreamRef stream){
	fclose(stream->_file);
	FCRelease(stream);
}

