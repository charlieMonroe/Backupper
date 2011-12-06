//
//  FCClient.c
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
#include <netdb.h>

#include "FCRelease.h"
#include "FCUserDefaults.h"

#include "FCClient.h"
#include "FCString.h"

struct __FCClient{
	char *_server;
	FCClientCallbackFunction _callbackFunction;
	FCClientErrorCallbackFunction _errorCallbackFunction;
	void *_ctx;
	struct sockaddr_in _socketStruct;
	int _port;
	int _socket;
	BOOL _connected;
};

FCClientRef FCClientCreate(const char *server, int port, FCClientCallbackFunction callbackFunction, 
			   FCClientErrorCallbackFunction errorCallbackFunction, void *ctx){
	if (server == NULL || port == 0 || callbackFunction == NULL){
		return NULL;
	}
	
	FCClientRef client = calloc(1, sizeof(struct __FCClient));
	client->_server = FCStringCopy(server);
	client->_port = port;
	client->_callbackFunction = callbackFunction;
	client->_errorCallbackFunction = errorCallbackFunction;
	client->_ctx = ctx;
	
	return client;
}

BOOL FCClientConnect(FCClientRef client){
	if (client->_connected){
		return YES;
	}
	
	client->_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client->_socket < 0) {
		if (client->_errorCallbackFunction){
			client->_errorCallbackFunction(client, "Cannot open socket.", client->_ctx);
		}
		return NO;
	}
	
	struct hostent *server = gethostbyname(client->_server);
	if (server == NULL) {
		if (client->_errorCallbackFunction){
			client->_errorCallbackFunction(client, "Cannot resolve host.", client->_ctx);
		}
		shutdown(client->_socket, SHUT_RDWR);
		return NO;
	}
	
	struct sockaddr_in *serverSocket = &client->_socketStruct;
	bzero((char *)serverSocket, sizeof(*serverSocket));
	serverSocket->sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverSocket->sin_addr.s_addr, server->h_length);
	serverSocket->sin_port = htons(client->_port);
	
	if (connect(client->_socket, (struct sockaddr*)serverSocket, sizeof(*serverSocket)) < 0){
		if (client->_errorCallbackFunction){
			client->_errorCallbackFunction(client, "Cannot connect to host.", client->_ctx);
		}
		shutdown(client->_socket, SHUT_RDWR);
		return NO;
	}
	
	client->_connected = YES;
	
	return YES;
}
void FCClientDisconnect(FCClientRef client){
	if (!client->_connected){
		return;
	}
	shutdown(client->_socket, SHUT_RDWR);
	client->_connected = NO;
}
ssize_t FCClientReceive(FCClientRef client, char *buffer, size_t bufferSize){
	ssize_t n = read(client->_socket, buffer, bufferSize);
	if (n <= 0){
		if (client->_errorCallbackFunction){
			client->_errorCallbackFunction(client, "Cannot read from socket.", client->_ctx);
		}
	}
	return n;
}
void FCClientRelease(FCClientRef client){
	FCClientDisconnect(client);
	FCRelease(client->_server);
	FCRelease(client);
}
void FCClientSendMessageToServer(FCClientRef client, const char *msg, size_t msg_len){
	int socket = client->_socket;
//	write(socket, msg, msg_len);
	send(socket, msg, msg_len, 0);
}
void FCClientSendStringToServer(FCClientRef client, const char *msg){
	FCClientSendMessageToServer(client, msg, strlen(msg) + 1);
}








