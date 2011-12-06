//
//  FCString.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "FCTypes.h"
#include "FCString.h"

char *FCStringByAppendingPathComponent(const char *path, const char *pathComponent){
	size_t pathLen = strlen(path);
	if (pathLen == 0){
		return FCStringCopy(pathComponent);
	}
	
	size_t componentLen = strlen(pathComponent);
	
	BOOL addSlash = (path[pathLen - 1] != '/');
	
	size_t fullPathLen = (pathLen + componentLen + (addSlash ? 2 : 1)); // 2 = '/' + NULL
	char *fullPath = malloc(sizeof(char) * fullPathLen); 
	strcpy(fullPath, path);
	if (addSlash){
		fullPath[pathLen] = '/';
	}
	strcpy(fullPath + pathLen + (addSlash ? 1 : 0), pathComponent);
	return fullPath;
}
char *FCStringByAppendingString(const char *str1, const char *str2){
	size_t str1Len = strlen(str1);
	if (str1 == 0){
		return FCStringCopy(str1);
	}
	
	size_t str2Len = strlen(str2);
	if (str2 == 0){
		return FCStringCopy(str2);
	}
	
	char *buffer = calloc(1, str1Len + str2Len + 1);
	strcpy(buffer, str1);
	strcpy(buffer + str1Len, str2);
	
	return buffer;
}
char *FCStringByExpandingTildeInPath(const char *path){
	if (path[0] != '~'){
		return FCStringCopy(path);
	}
	
	char *homeDir = FCStringCopy(getenv ("HOME"));
	if (strlen(path) == 1){
		// Just the tilde
		return homeDir;
	}
	char *result = FCStringByAppendingPathComponent(homeDir, path + 2);
	free(homeDir);
	return result;
}
char *FCStringCopy(const char *string){
	char *newString = malloc((strlen(string) + 1) * sizeof(char));
	strcpy(newString, string);
	return newString;
}
char *FCStringGetLine(void){
	size_t currentSize = 32;
	size_t ptr = 0;
	char *line = malloc(currentSize);
	
	
	while (YES) {
		int c = fgetc(stdin);
		if (c == EOF || c == '\n'){
			break;
		}
		
		line[ptr] = c;
		++ptr;
		
		if (ptr >= currentSize){
			currentSize *= 2;
			line = realloc(line, currentSize);
			if (line == NULL){
				free(line);
				return NULL;
			}
		}
	}
	
	line[ptr] = '\0';
	
	return line;
}
BOOL FCStringHasPrefix(const char *str, const char *prefix){
	size_t prefixLen = strlen(prefix);
	if (prefixLen > strlen(str)){
		return NO;
	}
	
	int counter = 0;
	while (counter < prefixLen && (*str++ == *prefix++)){
		++counter;
	}
	
	return counter == prefixLen;
}
char *FCStringLastPathComponent(const char *path){
	size_t pathLen = strlen(path);
	for (int i = (int)pathLen - 1; i >= 0; --i){
		if (path[i] == '/'){
			// Break
			char *lpc = malloc(sizeof(char) * (pathLen - i) + 1);
			strcpy(lpc, path + i);
			return lpc;
			break;
		}
	}
	char *lpc = malloc(sizeof(char) * pathLen + 1);
	strcpy(lpc, path);
	return lpc;
}
unsigned long long FCStringUnsignedLongLongValue(const char *str){
	unsigned long long value = 0;
	size_t strLen = strlen(str);
	for (int i = 0; i < strLen; ++i){
		char c = str[i];
		if (c < '0' || c > '9'){
			break;
		}
		
		value *= 10;
		value += c - '0';
	}
	
	return value;
}
BOOL FCStringsEqual(const char *str1, const char *str2){
	return strcmp(str1, str2) == 0;
}


