//
//  main.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "FCString.h"
#include "FCOption.h"
#include "FCUserDefaults.h"
#include "FCBackupperClient.h"
#include "FCBackupperServer.h"

static void print_usage(void){
	printf("Usage:\n");
	printf("\tbackupper preferences - opens preferences for setup.\n");
	printf("\tbackupper client (default server is localhost on 3233)\n");
	printf("\tbackupper server (default port in 3233)\n");
}

int main (int argc, const char * argv[]) {
	const char *task = NULL;
	const char *server = NULL;
	
	if (argc != 2 || (strcmp(argv[1], "client") == 0 && argc != 3)){
		print_usage();
		return 1;
	}
	
	FCArrayRef options = FCArrayCreateWithCapacity(2);
	FCArrayAddItem(options, FCOptionCreateWithName("server", FCOptionTypeString));
	FCArrayAddItem(options, FCOptionCreateWithName("port", FCOptionTypeInt));
	FCArrayAddItem(options, FCOptionCreateWithName("files", FCOptionTypeArray));
	FCArrayAddItem(options, FCOptionCreateWithName("debug", FCOptionTypeBool));
	FCArrayAddItem(options, FCOptionCreateWithName("directory", FCOptionTypeString));
	
	char *fileString = FCStringByExpandingTildeInPath("~/.backupper_config");
	FCUserDefaultsRef defaults = FCUserDefaultsCreateWithFile(fileString, options);
	free(fileString); // It gets copied to user defaults
	
	FCOptionValueRef debugValue = FCUserDefaultsValueForOptionWithName(defaults, "debug");
	if (debugValue){
		FCRunInDebugMode = (BOOL)FCOptionValueGetValue(debugValue);
	}
	
	FCUserDefaultsRunMain(defaults);
	//FCBackupperServerRunMain(defaults);
	//FCBackupperClientRunMain(defaults);
	return 0;
	
	task = argv[1];
	if (FCStringsEqual(task, "client")){
		if (argc > 2){
			server = argv[2];
		}
	}else if (FCStringsEqual(task, "server")){
		FCBackupperServerRunMain(defaults);
	}else if (FCStringsEqual(task, "preferences")){
		return FCUserDefaultsRunMain(defaults);
	}else{
		print_usage();
		return 1;
	}
	
	return 0;
}

