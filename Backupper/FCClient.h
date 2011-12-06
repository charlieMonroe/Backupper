//
//  FCClient.h
//  Backupper
//
//  Created by Charlie Monroe on 05.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCClient_h
#define Backupper_FCClient_h

#include "FCTypes.h"

// Opaque type
struct __FCClient;
typedef struct __FCClient* FCClientRef;

typedef BOOL(*FCClientCallbackFunction)(FCClientRef client, const char *msg, size_t msg_len, void *ctx); // Return NO to close connection
typedef void(*FCClientErrorCallbackFunction)(FCClientRef client, char *errorString, void *ctx);


FCClientRef FCClientCreate(const char *server, int port, FCClientCallbackFunction callbackFunction, 
			   FCClientErrorCallbackFunction errorCallbackFunction, void *ctx);

BOOL FCClientConnect(FCClientRef client);
void FCClientDisconnect(FCClientRef client);
ssize_t FCClientReceive(FCClientRef client, char *buffer, size_t bufferSize);
void FCClientRelease(FCClientRef client);
void FCClientSendMessageToServer(FCClientRef client, const char *msg, size_t msg_len);
void FCClientSendStringToServer(FCClientRef client, const char *msg);

#endif
