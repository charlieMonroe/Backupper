//
//  FCServer.c
//  Backupper
//
//  Created by Charlie Monroe on 05.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "FCServer.h"
#include "FCRelease.h"
#include "FCUserDefaults.h"

static const int kFCServerBufferSize = 1024;

struct __FCServer{
	FCServerCallbackFunction _callbackFunction;
	FCServerErrorCallbackFunction _errorCallbackFunction;
	void *_ctx;
	struct sockaddr_in _socketStruct;
	socklen_t socketLength;
	int _port;
	int _socket;
	int _currentClient;
	BOOL _running;
};

FCServerRef FCServerCreateWithPort(int port, FCServerCallbackFunction callbackFunction, 
				   FCServerErrorCallbackFunction errorCallback, void *ctx){
	if (callbackFunction == NULL){
		// There'd be no way to communicate
		return NULL;
	}
	
	FCServerRef server = calloc(1, sizeof(struct __FCServer));
	server->_callbackFunction = callbackFunction;
	server->_errorCallbackFunction = errorCallback;
	server->_ctx = ctx;
	server->_port = port;
	
	return server;
}
ssize_t FCServerReceive(FCServerRef server, char *buffer, size_t bufferSize){
	ssize_t n = read(server->_currentClient, buffer, bufferSize);
	if (n <= 0){
		if (server->_errorCallbackFunction){
			server->_errorCallbackFunction(server, "Cannot read from socket.", server->_ctx);
		}
	}
	return n;
}
void FCServerRun(FCServerRef server){
	if (server->_running){
		return;
	}
	
	server->_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server->_socket < 0) {
		if (server->_errorCallbackFunction){
			server->_errorCallbackFunction(server, "Cannot open socket.", server->_ctx);
		}
		return;
	}
	
	bzero((char *) &server->_socketStruct, sizeof(server->_socketStruct));
	server->_socketStruct.sin_family = AF_INET;
	server->_socketStruct.sin_addr.s_addr = INADDR_ANY;
	server->_socketStruct.sin_port = htons(server->_port);
	
	if (bind(server->_socket, (struct sockaddr *)&server->_socketStruct, sizeof(server->_socketStruct)) < 0){
		if (server->_errorCallbackFunction){
			server->_errorCallbackFunction(server, "Cannot bind.", server->_ctx);
		}
		
		shutdown(server->_socket, SHUT_RDWR);
		return;
	}
	
	server->_running = YES;
	
	listen(server->_socket, 5);
	struct sockaddr_in client;
	socklen_t clientSize = sizeof(client);
	
	char buffer[1024];
	
	while (server->_running){
		if (FCRunInDebugMode){
			printf("%s - waiting for new connection.\n", __FUNCTION__);
		}
		
		int newSockFD = accept(server->_socket, (struct sockaddr*)&client, &clientSize);
		server->_currentClient = newSockFD;
		
		if (FCRunInDebugMode){
			printf("%s - accepted new connection (%i).\n", __FUNCTION__, newSockFD);
		}
		
		while (YES){
			if (newSockFD < 0){
				if (server->_errorCallbackFunction){
					server->_errorCallbackFunction(server, "Cannot open new socket.", server->_ctx);
				}
				break;
			}
			
			bzero(buffer, kFCServerBufferSize);
			ssize_t n = read(newSockFD, buffer, kFCServerBufferSize);
			if (n <= 0){
				if (server->_errorCallbackFunction){
					server->_errorCallbackFunction(server, "Cannot read from socket.", server->_ctx);
				}
				break;
			}
			
			if (!server->_callbackFunction(server, buffer, n, server->_ctx)){
				break;
			}
		}
		
		server->_currentClient = -1;
		shutdown(newSockFD, SHUT_RDWR);
	}
	
	shutdown(server->_socket, SHUT_RDWR);
}
void FCServerSendMessageToCurrentClient(FCServerRef server, const char *msg, size_t msg_len){
	int socket = server->_currentClient;
	send(socket, msg, msg_len, 0);
//	write(socket, msg, msg_len);
}
void FCServerStop(FCServerRef server){
	server->_running = NO;
}

void FCServerRelease(FCServerRef server){
	if (server->_running){
		FCServerStop(server);
	}
	FCRelease(server);
}
void FCServerSendStringToCurrentClient(FCServerRef server, const char *msg){
	FCServerSendMessageToCurrentClient(server, msg, strlen(msg) + 1);
}



