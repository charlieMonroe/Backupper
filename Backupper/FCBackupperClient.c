//
//  FCBackupperClient.c
//  Backupper
//
//  Created by Charlie Monroe on 05.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "FCBackupperClient.h"
#include "FCClient.h"
#include "FCRelease.h"
#include "FCFileListing.h"
#include "FCArray.h"
#include "FCString.h"
#include "FCFile.h"
#include "FCFileReadStream.h"

// In current implementation, this buffer size should be enough
static const int kFCBackupperClientBufferSize = 512;

typedef enum {
	FCBackupperClientStateInitial = 0,
	FCBackupperClientStateClientListing,
	FCBackupperClientStateClientSendingFiles,
	FCBackupperClientStateEnded
} FCBackupperClientState;

struct __FCBackupperClient {
	FCClientRef _client;
	FCUserDefaultsRef _defaults;
	char _buffer[kFCBackupperClientBufferSize];
	FCArrayRef _fileListing; // Of FCFileListing
	FCBackupperClientState _state;
};
typedef struct __FCBackupperClient* FCBackupperClientRef;


static BOOL _FCBackupperClientHandleFileListing(FCBackupperClientRef client);
static BOOL _FCBackupperClientHandleFileTransfer(FCBackupperClientRef client);
static BOOL _FCBackupperClientHandleHandshake(FCBackupperClientRef client);
static void _FCBackupperClientListFiles(FCBackupperClientRef client);
static void _FCBackupperClientListFilesInDirectory(FCBackupperClientRef client, FCFileRef directory);


static BOOL _FCBackupperClientCallback(FCClientRef client, const char *msg, size_t msg_len, void *ctx){
	
	return YES;
}
static void _FCBackuperClientErrorCallback(FCClientRef client, char *errorString, void *ctx){
	printf("***SERVER ERROR: %s\n", errorString);
}

static void _FCBackupperClientHandleBackup(FCBackupperClientRef client){
	if (!_FCBackupperClientHandleHandshake(client)){
		return;
	}
	if (!_FCBackupperClientHandleFileListing(client)){
		return;
	}
	
	_FCBackupperClientHandleFileTransfer(client);
}
static BOOL _FCBackupperClientHandleFileListing(FCBackupperClientRef client){
	_FCBackupperClientListFiles(client);
	
	char buffer[1024]; // Shouldn't be longer
	
	// Send them
	for (int i = 0; i < FCArrayCount(client->_fileListing); ++i){
		bzero(buffer, 1024);
		
		FCFileListingRef listing = FCArrayItemAtIndex(client->_fileListing, i);
		switch (FCFileListingGetType(listing)) {
			case FCFileListingTypeDirectory:
				sprintf(buffer, "%s:d", FCFileListingGetPath(listing));
				break;
			case FCFileListingTypeSymlink:
				sprintf(buffer, "%s:s:%s", FCFileListingGetPath(listing), FCFileListingGetSymlinkPath(listing));
				break;
			case FCFileListingTypeRegularFile:
				sprintf(buffer, "%s:f:%qu:%qu", FCFileListingGetPath(listing), FCFileListingGetSize(listing),
					FCFileListingGetModificationDate(listing));
				break;
			default:
				printf("***WARNING - a file of unknown type (skipping): %i\n", FCFileListingGetType(listing));
				break;
		}
		
		if (FCRunInDebugMode){
			//printf("%s - Sending to server: %s\n", __FUNCTION__, buffer);
		}
		
		FCClientSendStringToServer(client->_client, buffer);
		
		ssize_t n = FCClientReceive(client->_client, client->_buffer, kFCBackupperClientBufferSize);
		if (n <= 0){
			printf("ERROR*** Haven't received confirmation from server (%s).\n", __FUNCTION__);
			return NO;
		}
		
		if (!FCStringHasPrefix(client->_buffer, "OK")){
			printf("ERROR*** Server responded: %s\n", client->_buffer);
			return NO;
		}
	}
	
	FCClientSendStringToServer(client->_client, "END");
	return YES;
}
static BOOL _FCBackupperClientHandleFileTransfer(FCBackupperClientRef client){
	char *buffer = NULL;
	size_t bufferSize = 0;
	
	while (YES){
		while (YES) {
			ssize_t bytesReceived = FCClientReceive(client->_client, client->_buffer, kFCBackupperClientBufferSize);
			
			printf("%s\n", client->_buffer);
			
			if (bytesReceived <= 0){
				printf("ERROR*** Haven't received any command from server (%s), aborting.\n", __FUNCTION__);
				goto fail;
			}
			
			if (!FCStringHasPrefix(client->_buffer, "SEND ")){
				if (!FCStringHasPrefix(client->_buffer, "WARNING ")){
					printf("ERROR*** Server responded: %s\n", client->_buffer);
					goto fail;
				}else{
					printf("WARNING from server - %s - %s\n", __FUNCTION__, client->_buffer);
					FCClientSendStringToServer(client->_client, "CONTINUE");
					continue;
				}
			}
			break;
		}
		    
		char *bufferSizeString = client->_buffer;
		strsep(&bufferSizeString, " ");
		char *filePath = bufferSizeString;
		strsep(&filePath, " ");
		
		size_t requiredBufferSize = atoi(bufferSizeString);
		if (requiredBufferSize == 0){
			printf("ERROR*** Server requested a 0 size buffer (%s), aborting.\n", __FUNCTION__);
			goto fail;
		}
		
		if (requiredBufferSize > bufferSize){
			if (buffer != NULL){
				free(buffer);
			}
			
			buffer = malloc(requiredBufferSize);
			bufferSize = requiredBufferSize;
		}
		
		FCFileReadStreamRef stream = FCFileReadStreamCreateWithFile(filePath);
		if (stream == NULL){
			// Couldn't open stream.
			printf("ERROR*** Cannot open stream for %s (%s), aborting.\n", filePath, __FUNCTION__);
			goto fail;
		}
		
		while (YES) {
			bzero(buffer, bufferSize);
			ssize_t bytesRead = FCFileReadStreamReadData(stream, buffer, bufferSize);
			if (bytesRead <= 0){
				printf("WARNING*** Can no longer read from %s (%s), ending transfer.\n", filePath, __FUNCTION__);
			}
			
			FCClientSendMessageToServer(client->_client, buffer, bytesRead);
			
			FCClientReceive(client->_client, client->_buffer, kFCBackupperClientBufferSize);
			printf("%s\n", client->_buffer);
			if (!FCStringHasPrefix(client->_buffer, "OK")){
				if (!FCStringHasPrefix(client->_buffer, "WARNING ")){
					printf("ERROR*** Server responded: %s\n", client->_buffer);
					FCFileReadStreamRelease(stream);
					goto fail;
				}else{
					printf("WARNING from server - %s - %s\n", __FUNCTION__, client->_buffer);
					FCClientSendStringToServer(client->_client, "CONTINUE");
					break;
				}
			}
			
			if (bytesRead < bufferSize){
				break;
			}
		}
		
		FCFileReadStreamRelease(stream);
		FCClientSendStringToServer(client->_client, "END");
	}
	
	return YES;
	
fail:
	if (buffer){
		free(buffer);
	}
	
	return NO;
}
static BOOL _FCBackupperClientHandleHandshake(FCBackupperClientRef client){
	char *name = getenv("LOGNAME");
	if (name == NULL){
		printf("ERROR*** Backupper needs a sane environment that has LOGNAME in its environment variables.\n");
		return NO;
	}
	FCClientSendStringToServer(client->_client, name);
	ssize_t n = FCClientReceive(client->_client, client->_buffer, kFCBackupperClientBufferSize);
	if (n <= 0){
		printf("ERROR*** Server doesn't respond to a shakehand.\n");
		return NO;
	}
	
	if (!FCStringsEqual(client->_buffer, "OK")){
		printf("ERROR*** Server responded: %s\n", client->_buffer);
		return NO;
	}
	
	return YES;
}
static void _FCBackupperClientListFiles(FCBackupperClientRef client){
	FCOptionValueRef filesValue = FCUserDefaultsValueForOptionWithName(client->_defaults, "files");
	FCArrayRef arr = FCOptionValueGetValue(filesValue);
	
	if (client->_fileListing == NULL){
		client->_fileListing = FCArrayCreate();
	}
	
	for (int i = 0; i < FCArrayCount(arr); ++i){
		char *filePath = FCArrayItemAtIndex(arr, i);
		FCFileRef file = FCFileCreateWithPath(filePath);
		if (FCFileIsDirectory(file)){
			_FCBackupperClientListFilesInDirectory(client, file);
		}else{
			FCArrayAddItem(client->_fileListing, FCFileListingCreateWithFile(file));
		}
	}
}
static void _FCBackupperClientListFilesInDirectory(FCBackupperClientRef client, FCFileRef directory){
	FCArrayAddItem(client->_fileListing, FCFileListingCreateWithFile(directory));
	FCArrayRef arr = FCFileDirectoryListing(directory);
	for (int i = 0; i < FCArrayCount(arr); ++i){
		FCFileRef file = FCArrayItemAtIndex(arr, i);
		if (FCFileIsDirectory(file)){
			_FCBackupperClientListFilesInDirectory(client, file);
		}else{
			FCArrayAddItem(client->_fileListing, FCFileListingCreateWithFile(file));
		}
	}
}
void FCBackupperClientRunMain(FCUserDefaultsRef defaults){
	if (FCUserDefaultsValueForOptionWithName(defaults, "files") == NULL){
		printf("No directories to backup set. Please, run in preferences mode and set the directories.\n");
		exit(1);
		return;
	}
	
	FCOptionValueRef serverValue = FCUserDefaultsValueForOptionWithName(defaults, "server");
	char *server = "localhost";
	if (serverValue){
		server = (char*)FCOptionValueGetValue(serverValue);
	}
	
	FCOptionValueRef portValue = FCUserDefaultsValueForOptionWithName(defaults, "port");
	int port = 3233;
	if (portValue){
		port = (int)FCOptionValueGetValue(portValue);
	}
	
	FCBackupperClientRef backupperClient = calloc(1, sizeof(struct __FCBackupperClient));
	FCClientRef client = FCClientCreate(server, port, _FCBackupperClientCallback, _FCBackuperClientErrorCallback, backupperClient);
	
	backupperClient->_client = client;
	backupperClient->_defaults = defaults;
	backupperClient->_state = FCBackupperClientStateInitial;
	
	if (FCClientConnect(client)){
		_FCBackupperClientHandleBackup(backupperClient);
	}else{
		printf("***ERROR connecting to the server.\n");
	}
	
	// Releasing
	FCArrayReleaseWithOptions(backupperClient->_fileListing, YES, (FCReleaseFunction)FCFileListingRelease);
	FCClientRelease(backupperClient->_client);
	FCRelease(backupperClient);
}

