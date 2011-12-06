//
//  FCUserDefaults.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "FCUserDefaults.h"
#include "FCRelease.h"
#include "FCOption.h"
#include "FCString.h"
#include "FCFileReadStream.h"
#include "FCFileWriteStream.h"
#include "FCOptionValue.h"

static const int kFCUserDefaultsMaxFilePath = 512;
BOOL FCRunInDebugMode = NO;

static void _FCPrintOptionValue(FCUserDefaultsRef defaults, FCOptionValueRef optionValue);
static FCOptionRef _FCUserDefaultsOptionForName(FCUserDefaultsRef defaults, const char *name);

struct __FCUserDefaults{
	char *_path;
	FCArrayRef _allowedOptions;
	FCArrayRef _loadedOptions;
};


static void _FCAddOptionValue(FCUserDefaultsRef defaults, char *line){
	// We need to parse this first
	char *optionName = line;
	char *value = NULL;
	size_t lineLen = strlen(line);
	
	int i;
	for (i = 0; i < lineLen; ++i){
		if (line[i] == ' '){
			// Hello, space
			line[i] = '\0';
			value = line + i + 1;
			break;
		}
	}
	
	if (i == lineLen){
		printf("Invalid syntax: add <key> <value>.\n");
		return;
	}
	
	FCOptionRef option = _FCUserDefaultsOptionForName(defaults, optionName);
	if (option == NULL){
		printf("Invalid key name. Such a key is not allowed.\n");
		return;
	}
	
	
	if (FCOptionGetType(option) != FCOptionTypeArray){
		printf("The add command is allowed only for array values.\n");
		return;
	}
	
	FCOptionValueRef optionValue = FCUserDefaultsValueForOption(defaults, option);
	if (optionValue == NULL){
		// Need to add a new optionValue
		optionValue = FCOptionValueCreateWithValue(option, FCArrayCreateFromString(value), YES, (FCReleaseFunction)FCArrayRelease);
		FCArrayAddItem(defaults->_loadedOptions, optionValue);
		return;
	}
	
	FCArrayRef arr = FCOptionValueGetValue(optionValue);
	FCArrayAddItem(arr, FCStringCopy(value));
	
}
static void _FCPrintAllOptionValues(FCUserDefaultsRef defaults){
	for (int i = 0; i < FCArrayCount(defaults->_loadedOptions); ++i){
		FCOptionValueRef optionValue = FCArrayItemAtIndex(defaults->_loadedOptions, i);
		_FCPrintOptionValue(defaults, optionValue);
	}
}

static void _FCPrintAvailableKeys(FCUserDefaultsRef defaults){
	printf("\tAvailable keys: \n");
	for (int i = 0; i < FCArrayCount(defaults->_allowedOptions); ++i){
		printf("\t\t");
		FCOptionRef option = FCArrayItemAtIndex(defaults->_allowedOptions, i);
		printf("%s", FCOptionName(option));
		printf("\t\t[");
		
		switch (FCOptionGetType(option)) {
			case FCOptionTypeBool:
				printf("BOOL (either YES or NO)");
				break;
			case FCOptionTypeInt:
				printf("Int");
				break;
			case FCOptionTypeString:
				printf("String");
				break;
			case FCOptionTypeArray:
				printf("Array (individual entries are separated by a semicolon)");
				break;
			default:
				break;
		}
		
		printf("]\n");
	}
}
static void _FCPrintHelp(FCUserDefaultsRef defaults){
	printf("Available commands:\n");
	printf("\thelp - displays this help\n");
	printf("\texit - exits\n");
	printf("\n");
	printf("\tprint <key> - prints value\n");
	printf("\tprintall - prints all values\n");
	printf("\tremove <key> <value> - removes value\n");
	printf("\tset <key> <value> - sets value for key\n");
	printf("\tadd <key> <value> - adds a value for key (only if the option is an array)\n");
	printf("\n");
	_FCPrintAvailableKeys(defaults);
	printf("\n");
}
static void _FCPrintOptionValue(FCUserDefaultsRef defaults, FCOptionValueRef optionValue){
	FCOptionRef option = FCOptionValueGetOption(optionValue);
	const char *name = FCOptionName(option);
	
	if (optionValue == NULL){
		printf("Unknown option '%s'.\n", name);
		return;
	}
	
	printf("%s = ", name);
	
	void *value = FCOptionValueGetValue(optionValue);
	switch (FCOptionGetType(option)) {
		case FCOptionTypeBool:
			printf("%s", value ? "YES" : "NO");
			break;
		case FCOptionTypeInt:
			printf("%i", (int)value);
			break;
		case FCOptionTypeString:
			printf("%s", value);
			break;
		case FCOptionTypeArray:
			FCArrayPrint(value);
			break;
		default:
			printf("Unknown value type %i", FCOptionGetType(option));
			break;
	}
	printf("\n");
	fflush(stdin);
}
static void _FCSetOptionValue(FCUserDefaultsRef defaults, char *line){
	// We need to parse this first
	char *optionName = line;
	char *value = NULL;
	size_t lineLen = strlen(line);
	
	int i;
	for (i = 0; i < lineLen; ++i){
		if (line[i] == ' '){
			// Hello, space
			line[i] = '\0';
			value = line + i + 1;
			break;
		}
	}
	
	if (i == lineLen){
		printf("Invalid syntax: set <key> <value>.\n");
		return;
	}
	
	FCOptionRef option = _FCUserDefaultsOptionForName(defaults, optionName);
	if (option == NULL){
		printf("Invalid key name. Such a key is not allowed.\n");
		return;
	}
	
	FCOptionValueRef optionValue = FCUserDefaultsValueForOption(defaults, option);
	if (optionValue == NULL){
		// Need to add a new optionValue
		FCOptionType type = FCOptionGetType(option);
		optionValue = FCOptionValueCreateWithValue(option, NULL, (type == FCOptionTypeString || type == FCOptionTypeArray), 
							   (type == FCOptionTypeArray) ? (FCReleaseFunction)FCArrayRelease : NULL);
		FCArrayAddItem(defaults->_loadedOptions, optionValue);
	}
	
	void *newValue = NULL;
	switch (FCOptionGetType(option)) {
		case FCOptionTypeBool:
			newValue = (void*)FCStringsEqual(value, "YES");
			break;
		case FCOptionTypeInt:
			newValue = (void*)atoi(value);
			break;
		case FCOptionTypeString:
			newValue = FCStringCopy(value);
			break;
		case FCOptionTypeArray:
			newValue = FCArrayCreateFromString(value);
			break;
		default:
			printf("%s - ***WARNING - unknown option type %i!\n", __FUNCTION__, FCOptionGetType(option));
			break;
	}
	
	FCOptionValueSetValue(optionValue, newValue);
	
}
static void _FCPrintOptionValueWithName(FCUserDefaultsRef defaults, char *name){
	FCOptionValueRef optionValue = FCUserDefaultsValueForOptionWithName(defaults, name);
	_FCPrintOptionValue(defaults, optionValue);
}
static void _FCRemoveOptionValue(FCUserDefaultsRef defaults, char *name){
	FCOptionValueRef optionValue = FCUserDefaultsValueForOptionWithName(defaults, name);
	if (optionValue == NULL){
		printf("Cannot remove option '%s' because it is not set.\n", name);
		return;
	}
	
	FCArrayRemoveItem(defaults->_loadedOptions, optionValue);
	FCOptionValueRelease(optionValue);
}
static FCOptionRef _FCUserDefaultsOptionForName(FCUserDefaultsRef defaults, const char *name){
	for (int i = 0; i < FCArrayCount(defaults->_allowedOptions); ++i){
		FCOptionRef option = FCArrayItemAtIndex(defaults->_allowedOptions, i);
		if (FCStringsEqual(name, FCOptionName(option))){
			return option;
		}
	}
	return NULL;
}

static void _FCUserDefaultsLoadFromFile(FCUserDefaultsRef defaults, const char *path){
	FCFileReadStreamRef stream = FCFileReadStreamCreateWithFile(path);
	if (stream == NULL){
		// File doesn't exist yet
		return;
	}
	
	const char *line;
	while ((line = FCFileReadStreamReadLine(stream)) != NULL){
		size_t lineLength = strlen(line);
		if (lineLength == 0){
			// Empty line
			continue;
		}
		if (line[0] == '#'){
			// Comment
			continue;
		}
		
		char *lineValue = FCStringCopy(line);
		char *paramName = strsep(&lineValue, "=");
		
		if (paramName == NULL || strlen(paramName) == 0){
			printf("%s - ***WARNING malformed line in defaults, skipping (%s)\n", __FUNCTION__, line);
			continue;
		}
		
		FCOptionRef option = _FCUserDefaultsOptionForName(defaults, paramName);
		if (option == NULL){
			printf("%s - ***WARNING non-registered option, skipping (%s)\n", __FUNCTION__, line);
			continue;
		}
		
		if (FCUserDefaultsValueForOption(defaults, option) != NULL){
			printf("%s - ***WARNING option already loaded, skipping (%s)\n", __FUNCTION__, line);
			continue;
		}
		
		FCOptionValueRef value;
		switch (FCOptionGetType(option)) {
			case FCOptionTypeBool:
				value = FCOptionValueCreateWithValue(option, (void*)FCStringsEqual(lineValue, "YES"), NO, NULL);
				break;
			case FCOptionTypeInt:
				value = FCOptionValueCreateWithValue(option, (void*)atoi(lineValue), NO, NULL);
				break;
			case FCOptionTypeString:
				value = FCOptionValueCreateWithValue(option, FCStringCopy(lineValue), YES, NULL);
				break;
			case FCOptionTypeArray:
				value = FCOptionValueCreateWithValue(option, FCArrayCreateFromString(lineValue), YES, (FCReleaseFunction)FCArrayRelease);
				break;
			default:
				printf("%s - ***WARNING: Unknown type %i\n", __FUNCTION__, FCOptionGetType(option));
				break;
		}
		 
		FCArrayAddItem(defaults->_loadedOptions, value);
		
		free(paramName);
	}
	
	FCFileReadStreamRelease(stream);
}

FCUserDefaultsRef FCUserDefaultsCreateWithFile(const char *path, FCArrayRef allowedOptions){
	FCUserDefaultsRef defs = malloc(sizeof(struct __FCUserDefaults));
	defs->_path = FCStringCopy(path);
	defs->_allowedOptions = allowedOptions;
	defs->_loadedOptions = FCArrayCreate();
	_FCUserDefaultsLoadFromFile(defs, path);
	
	return defs;
}
int FCUserDefaultsRunMain(FCUserDefaultsRef defaults){
	printf("Welcome to backupper preferences. Type help for available commands.\n");

	while (YES){
		printf("> ");
		char *line = FCStringGetLine();
		if (line == NULL || strlen(line) == 0){
			continue;
		}
		
		if (FCStringsEqual("exit", line)){
			FCUserDefaultsSave(defaults);
			free(line);
			break;
		}else if (FCStringsEqual("help", line)){
			_FCPrintHelp(defaults);
		}else if (FCStringsEqual("printall", line)){
			_FCPrintAllOptionValues(defaults);
		}else if (FCStringHasPrefix(line, "print ")){
			char *optionName = line + strlen("print ");
			_FCPrintOptionValueWithName(defaults, optionName);
		}else if (FCStringHasPrefix(line, "remove ")){
			char *optionName = line + strlen("remove ");
			_FCRemoveOptionValue(defaults, optionName);
		}else if (FCStringHasPrefix(line, "set ")){
			char *optionName = line + strlen("set ");
			_FCSetOptionValue(defaults, optionName);
		}else if (FCStringHasPrefix(line, "add ")){
			char *optionName = line + strlen("add ");
			_FCAddOptionValue(defaults, optionName);
		}else{
			printf("Unknown command. \n");
		}
		
		free(line);
	}
	
	return 0;
}
void FCUserDefaultsRelease(FCUserDefaultsRef defaults){
	FCRelease(defaults->_path);
	FCArrayRelease(defaults->_allowedOptions);
	FCArrayReleaseWithOptions(defaults->_loadedOptions, YES, (FCReleaseFunction)FCOptionValueRelease);
	FCRelease(defaults);
}
void FCUserDefaultsSave(FCUserDefaultsRef defaults){
	FCFileWriteStreamRef stream = FCFileWriteStreamCreateWithFile(defaults->_path);
	if (stream == NULL){
		printf("******* ERROR saving user defaults *******\n");
		return;
	}
	
	for (int i = 0; i < FCArrayCount(defaults->_loadedOptions); ++i){
		FCOptionValueRef optionValue = FCArrayItemAtIndex(defaults->_loadedOptions, i);
		FCOptionRef option = FCOptionValueGetOption(optionValue);
		void *value = FCOptionValueGetValue(optionValue);
		
		FCFileWriteStreamWriteFormat(stream, "%s=", FCOptionName(option));
		
		switch (FCOptionGetType(option)) {
			case FCOptionTypeBool:
				FCFileWriteStreamWriteBOOL(stream, (BOOL)value);
				break;
			case FCOptionTypeInt:
				FCFileWriteStreamWriteInt(stream, (int)value);
				break;
			case FCOptionTypeString:
				FCFileWriteStreamWriteString(stream, value);
				break;
			case FCOptionTypeArray:
				FCFileWriteStreamWriteArray(stream, value);
				break;
			default:
				printf("%s - ****WARNING: unknown option type %i\n", __FUNCTION__, FCOptionGetType(option));
				break;
		}
		
		FCFileWriteStreamWriteString(stream, "\n");
	}
}
FCOptionValueRef FCUserDefaultsValueForOption(FCUserDefaultsRef defaults, FCOptionRef option){
	for (int i = 0; i < FCArrayCount(defaults->_loadedOptions); ++i){
		FCOptionValueRef value = FCArrayItemAtIndex(defaults->_loadedOptions, i);
		if (FCOptionValueGetOption(value) == option){
			return value;
		}
	}
	return NULL;
}
FCOptionValueRef FCUserDefaultsValueForOptionWithName(FCUserDefaultsRef defaults, const char *name){
	FCOptionRef option = _FCUserDefaultsOptionForName(defaults, name);
	return FCUserDefaultsValueForOption(defaults, option);
}

