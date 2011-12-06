//
//  FCBackupperServer.c
//  Backupper
//
//  Created by Charlie Monroe on 05.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>

#include "FCBackupperServer.h"
#include "FCServer.h"
#include "FCTypes.h"
#include "FCString.h"
#include "FCArray.h"
#include "FCFileListing.h"
#include "FCFile.h"
#include "FCFileReadStream.h"
#include "FCFileWriteStream.h"

#pragma mark typedefs

typedef enum {
	FCBackupperServerStateInitial = 0,
	FCBackupperServerStateClientListing,
	FCBackupperServerStateClientSendingFiles,
	FCBackupperServerStateEnded
} FCBackupperServerState;

struct __FCBackupperServer {
	FCServerRef _server;
	FCUserDefaultsRef _defaults;
	FCArrayRef _remoteFileListing; // Of FCFileListing
	char *_username;
	char *_previousBackupDirectory;
	char *_currentBackupDirectory;
	FCBackupperServerState _state;
};
typedef struct __FCBackupperServer* FCBackupperServerRef;

// When you change this, don't forget to change it 
// in _FCBackupperServerHandleFileTransferForRegularFile as well
static const int kFCBackupperServerFileTransferBufferSize = 4098;

#pragma mark -
#pragma mark static method declarations

static BOOL _FCBackupperServerHandleFileListing(FCBackupperServerRef server, const char *fileLine);
static BOOL _FCBackupperServerHandleFileTransfer(FCBackupperServerRef server);
static BOOL _FCBackupperServerHandleFileTransferForListing(FCBackupperServerRef server, FCFileListingRef listing);
static BOOL _FCBackupperServerHandleFileTransferForRegularFile(FCBackupperServerRef server, FCFileListingRef listing);
static BOOL _FCBackupperServerHandleFileTransferForSymlink(FCBackupperServerRef server, FCFileListingRef listing);
static BOOL _FCBackupperServerHandleUsername(FCBackupperServerRef server, const char *name);

#pragma mark -
#pragma mark implementation

static BOOL _FCBackupperServerCallback(FCServerRef server, const char *msg, size_t msg_len, void *ctx){
	FCBackupperServerRef backupperServer = ctx;
	
	if (backupperServer->_state != FCBackupperServerStateInitial) {
		printf("%s - ***SERVER ERROR - shoudln't be getting a callback in this state. (%i)", __FUNCTION__, backupperServer->_state);
		return NO;
	}
	
	if (!_FCBackupperServerHandleUsername(backupperServer, msg)){
		return NO;
	}
	
	char *buffer = calloc(1, 1024);
	
	while (YES){
		ssize_t n = FCServerReceive(server, buffer, 1024);
		if (n <= 0){
			printf("%s - Couldn't receive any more data, disconnecting.\n", __FUNCTION__);
			return NO;
		}
		
		if (FCStringsEqual(buffer, "END")){
			// End of file listing
			if (FCRunInDebugMode){
				printf("%s - Received file listing END\n", __FUNCTION__);
			}
			backupperServer->_state = FCBackupperServerStateClientSendingFiles;
			break;
		}
		
		if (!_FCBackupperServerHandleFileListing(backupperServer, buffer)){
			return NO;
		}
	}
	
	if (!_FCBackupperServerHandleFileTransfer(backupperServer)){
		return NO;
	}
	
	FCServerSendStringToCurrentClient(server, "Backup successful.");
	
	return NO; //We're done, no reason not to close the connection
}
static void _FCBackuperServerErrorCallback(FCServerRef server, char *errorString, void *ctx){
	printf("***SERVER ERROR: %s\n", errorString);
}
BOOL _FCBackupperServerHandleFileListing(FCBackupperServerRef server, const char *fileLine){
	if (server->_remoteFileListing == NULL){
		server->_remoteFileListing = FCArrayCreate();
	}
		
	if (FCRunInDebugMode){
		//printf("%s - Received %s\n", __FUNCTION__, fileLine);
	}
	
	char *toFree = FCStringCopy(fileLine);
	char *type = toFree;
	char *filePath = strsep(&type, ":");
	
	if (strlen(filePath) == 0){
		FCServerSendStringToCurrentClient(server->_server, "ERROR Invalid file path (empty).");
		FCRelease(toFree);
		return NO;
	}
	
	FCFileListingRef listing = FCFileListingCreateWithPath(filePath);
	if (type[0] == 'd'){
		// Directory, nothing else, really
		FCFileListingSetType(listing, FCFileListingTypeDirectory);
	}else if (type[0] == 's'){
		// Just followed by symlink file
		FCFileListingSetType(listing, FCFileListingTypeSymlink);
		strsep(&type, ":");
		FCFileListingSetSymlinkPath(listing, type);
	}else if (type[0] == 'f'){
		// Regular file:
		char *fileSize = type;
		strsep(&fileSize, ":");
		char *modDate = fileSize;
		strsep(&modDate, ":");
		
		if (strlen(fileSize) == 0 || strlen(modDate) == 0){
			FCServerSendStringToCurrentClient(server->_server, "ERROR Wrong file description format for regular file. Use path:f:fileSize:modDate.");
			FCRelease(toFree);
			return NO;
		}
		
		FCFileListingSetSize(listing, FCStringUnsignedLongLongValue(fileSize));
		FCFileListingSetModificationDate(listing, FCStringUnsignedLongLongValue(modDate));
	}else{
		FCServerSendStringToCurrentClient(server->_server, "ERROR Unknown file type.");
		FCRelease(toFree);
		return NO;
	}
	
	FCRelease(toFree);
	FCArrayAddItem(server->_remoteFileListing, listing);
	
	FCServerSendStringToCurrentClient(server->_server, "OK");
	
	return YES;
}
static BOOL _FCBackupperServerHandleFileTransfer(FCBackupperServerRef server){
	// Create a new folder, etc.
	FCOptionValueRef optionValue = FCUserDefaultsValueForOptionWithName(server->_defaults, "directory");
	const char *rootDir = FCOptionValueGetValue(optionValue);
	char newFolderName[20];
	
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	strftime(newFolderName, 20, "%Y-%m-%d-%H%M%S", timeinfo);
	
	FCRelease(server->_previousBackupDirectory);
	FCRelease(server->_currentBackupDirectory);
	
	char *userDir = FCStringByAppendingPathComponent(rootDir, server->_username);
	char *latest = FCStringByAppendingPathComponent(userDir, "latest");
	
	FCFileReadStreamRef readStream = FCFileReadStreamCreateWithFile(latest);
	const char *latestFolder = NULL;
	if (readStream){
		latestFolder = FCFileReadStreamReadLine(readStream);
		server->_previousBackupDirectory = FCStringByAppendingPathComponent(userDir, latestFolder);
		FCFileReadStreamRelease(readStream);
	}else{
		server->_previousBackupDirectory = NULL;
	}
	
	server->_currentBackupDirectory = FCStringByAppendingPathComponent(userDir, newFolderName);
	free(userDir);
	
	
	/**** Make backup directory */
	if (!FCFileMakeDirectoryWithIntermediateDirectories(server->_currentBackupDirectory)){
		FCServerSendStringToCurrentClient(server->_server, "ERROR Cannot create new folder for backup.");
		free(latest);
		return NO;
	}
	
	/**** Backup each file */
	for (int i = 0; i < FCArrayCount(server->_remoteFileListing); ++i){
		FCFileListingRef listing = FCArrayItemAtIndex(server->_remoteFileListing, i);
		if (!_FCBackupperServerHandleFileTransferForListing(server, listing)){
			// Continue;
			printf("WARNING - couldn't backup %s", FCFileListingGetPath(listing));
		}
	}
	
	// Save the latest backup
	FCFileWriteStreamRef writeStream = FCFileWriteStreamCreateWithFile(latest);
	FCFileWriteStreamWriteString(writeStream, newFolderName);
	FCFileWriteStreamRelease(writeStream);
	
	free(latest);
	
	server->_state = FCBackupperServerStateEnded;
	
	FCServerSendStringToCurrentClient(server->_server, "END");
	
	server->_state = FCBackupperServerStateInitial; // Back to initial state
	return YES;
}
static BOOL _FCBackupperServerHandleFileTransferForDirectory(FCBackupperServerRef server, FCFileListingRef listing){
	const char *originalPath = FCFileListingGetPath(listing);
	if (originalPath[0] == '/'){
		++originalPath;
	}
	char *path = FCStringByAppendingPathComponent(server->_currentBackupDirectory, originalPath);
	BOOL result = FCFileMakeDirectoryWithIntermediateDirectories(path);
	free(path);
	return result;
}
static BOOL _FCBackupperServerHandleFileTransferForListing(FCBackupperServerRef server, FCFileListingRef listing){
	switch (FCFileListingGetType(listing)) {
		case FCFileListingTypeDirectory:
			return _FCBackupperServerHandleFileTransferForDirectory(server, listing);
			break;
		case FCFileListingTypeSymlink:
			return _FCBackupperServerHandleFileTransferForSymlink(server, listing);
			break;
		case FCFileListingTypeRegularFile:
			return _FCBackupperServerHandleFileTransferForRegularFile(server, listing);
			break;
		default:
			printf("****ERROR - unknown file type %i (%s)\n", FCFileListingGetType(listing), FCFileListingGetPath(listing));
			FCServerSendStringToCurrentClient(server->_server, "ERROR Invalid file type.");
			return NO;
			break;
	}
	
	return NO;
}
static BOOL _FCBackupperServerHandleFileTransferForRegularFile(FCBackupperServerRef server, FCFileListingRef listing){
	const char *remotePath = FCFileListingGetPath(listing);
	if (remotePath[0] == '/'){
		++remotePath;
	}
	
	char *currentBackupPath = FCStringByAppendingPathComponent(server->_currentBackupDirectory, remotePath);
	if (server->_previousBackupDirectory != NULL){
		char *lastBackupPath = FCStringByAppendingPathComponent(server->_previousBackupDirectory, remotePath);
		FCFileRef file = FCFileCreateWithPath(lastBackupPath);
		
		if (file != NULL){
			if (FCFileGetSize(file) == FCFileListingGetSize(listing) 
			    && FCFileGetModificationDate(file) == FCFileListingGetModificationDate(listing)){
				// Just create a hardlink
				if (link(currentBackupPath, lastBackupPath) == 0){
					// Went well
					free(lastBackupPath);
					free(currentBackupPath);
					FCFileRelease(file);
					return YES;
				}
			}
		}
		
		free(lastBackupPath);
	}
	
	if (FCFileListingGetSize(listing) == 0){
		// No data to transfer!
		FCFileWriteStreamRef stream = FCFileWriteStreamCreateWithFile(FCFileListingGetPath(listing));
		if (stream){
			FCFileWriteStreamRelease(stream);
		}
		return YES;
	}
	
	char *request = FCStringByAppendingString("SEND 4098 ", FCFileListingGetPath(listing));
	FCServerSendStringToCurrentClient(server->_server, request);
	free(request);
	
	// Need to load the file and write it to currentBackupPath
	char *buffer = calloc(1, kFCBackupperServerFileTransferBufferSize);
	
	FCFileWriteStreamRef writeStream = FCFileWriteStreamCreateWithFile(currentBackupPath);
	
	while (YES){
		ssize_t n = FCServerReceive(server->_server, buffer, kFCBackupperServerFileTransferBufferSize);
		if (n <= 0){
			free(currentBackupPath);
			FCServerSendStringToCurrentClient(server->_server, "ERROR Couldn't read from socket.");
			return NO;
			break;
		}
		
		if (FCStringsEqual(buffer, "END")){
			break;
		}
		
		if (writeStream == NULL){
			free(currentBackupPath);
			FCServerSendStringToCurrentClient(server->_server, "WARNING Cannot create write stream.");
			FCServerReceive(server->_server, buffer, kFCBackupperServerFileTransferBufferSize); // Whether to continue or not, but it has no meaning here
			return NO;
			break;
		}
		
		FCFileWriteStreamWriteData(writeStream, buffer, n);
		FCServerSendStringToCurrentClient(server->_server, "OK");
		
		//if (n != kFCBackupperServerFileTransferBufferSize){
			// End of file, evidently
		//	break;
		//}
	}
	
	if (writeStream != NULL){
		FCFileWriteStreamRelease(writeStream);
	}
	
	FCFileRef file = FCFileCreateWithPath(currentBackupPath);
	FCFileSetModificationDate(file, FCFileListingGetModificationDate(listing));
	
	free(currentBackupPath);
	return YES;
}
static BOOL _FCBackupperServerHandleFileTransferForSymlink(FCBackupperServerRef server, FCFileListingRef listing){
	const char *originalPath = FCFileListingGetSymlinkPath(listing);
	
	for (int i = 0; i < FCArrayCount(server->_remoteFileListing); ++i){
		FCFileListingRef list = FCArrayItemAtIndex(server->_remoteFileListing, i);
		if (FCStringsEqual(originalPath, FCFileListingGetPath(list))){
			if (originalPath[0] == '/'){
				++originalPath;
			}
			char *originalPathLocal = FCStringByAppendingPathComponent(server->_currentBackupDirectory, originalPath);
			
			const char *symlinkPath = FCFileListingGetPath(listing);
			char *symlinkLocal = FCStringByAppendingPathComponent(server->_currentBackupDirectory, symlinkPath);
			
			BOOL result = symlink(symlinkLocal, originalPathLocal) == 0;
			if (!result){
				free(symlinkLocal);
				free(originalPathLocal);
				break;
			}
			
			free(symlinkLocal);
			free(originalPathLocal);
			
			return YES;
		}
	}
	
	FCServerSendStringToCurrentClient(server->_server, "WARNING Couldn't create symlink, ignoring.");
	char buffer[kFCBackupperServerFileTransferBufferSize];
	FCServerReceive(server->_server, buffer, kFCBackupperServerFileTransferBufferSize); // Whether to continue or not
	if (!FCStringHasPrefix(buffer, "CONTINUE")){
		// The client wants to stop
		return NO;
	}
	
	return YES;
}
static BOOL _FCBackupperServerHandleUsername(FCBackupperServerRef server, const char *name){
	// It has to be the user name, has to be less than 64 chars
	if (strlen(name) > 64){
		FCServerSendStringToCurrentClient(server->_server, "ERROR Username too long.");
		return NO;
	}else{
		FCRelease(server->_username);
		server->_username = FCStringCopy(name);
		FCServerSendStringToCurrentClient(server->_server, "OK");
		server->_state = FCBackupperServerStateClientListing;
		return YES;
	}
}

void FCBackupperServerRunMain(FCUserDefaultsRef defaults){
	if (FCUserDefaultsValueForOptionWithName(defaults, "directory") == NULL){
		printf("Backup directory not set. Please, run in preferences mode and set the directory.\n");
		exit(1);
		return;
	}
	
	FCOptionValueRef portValue = FCUserDefaultsValueForOptionWithName(defaults, "port");
	int port = 3233;
	if (portValue){
		port = (int)FCOptionValueGetValue(portValue);
	}
	
	FCBackupperServerRef backupperServer = calloc(1, sizeof(struct __FCBackupperServer));
	FCServerRef server = FCServerCreateWithPort(port, _FCBackupperServerCallback, _FCBackuperServerErrorCallback, backupperServer);
	
	backupperServer->_server = server;
	backupperServer->_defaults = defaults;
	backupperServer->_state = FCBackupperServerStateInitial;
	
	FCServerRun(server);
	
	// Releasing
	FCRelease(backupperServer->_username);
	FCRelease(backupperServer->_currentBackupDirectory);
	FCRelease(backupperServer->_previousBackupDirectory);
	FCArrayReleaseWithOptions(backupperServer->_remoteFileListing, YES, (FCReleaseFunction)FCFileListingRelease);
	FCServerRelease(backupperServer->_server);
	FCRelease(backupperServer);
}
