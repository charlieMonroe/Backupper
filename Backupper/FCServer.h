//
//  FCServer.h
//  Backupper
//
//  Created by Charlie Monroe on 05.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCServer_h
#define Backupper_FCServer_h

#include "FCTypes.h"

// Opaque type
struct __FCServer;
typedef struct __FCServer* FCServerRef;

typedef BOOL(*FCServerCallbackFunction)(FCServerRef server, const char *msg, size_t msg_len, void *ctx); // Return NO to close connection
typedef void(*FCServerErrorCallbackFunction)(FCServerRef server, char *errorString, void *ctx);

// Ctx is not copied and not released
FCServerRef FCServerCreateWithPort(int port, FCServerCallbackFunction callbackFunction, 
				   FCServerErrorCallbackFunction errorCallbackFunction, void *ctx);

ssize_t FCServerReceive(FCServerRef server, char *buffer, size_t bufferSize);

void FCServerRun(FCServerRef server);
void FCServerStop(FCServerRef server);

void FCServerRelease(FCServerRef server);

void FCServerSendMessageToCurrentClient(FCServerRef server, const char *msg, size_t msg_len);
void FCServerSendStringToCurrentClient(FCServerRef server, const char *msg);

#endif
