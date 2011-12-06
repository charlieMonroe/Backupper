//
//  FCOption.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "FCOption.h"
#include "FCTypes.h"

static const int kFCOptionMaxOptionNameLength = 32;

struct __FCOption{
	char _name[kFCOptionMaxOptionNameLength + 1];
	FCOptionType _type;
};

// Name is copied
FCOptionRef FCOptionCreateWithName(const char *name, FCOptionType type){
	if (strlen(name) > kFCOptionMaxOptionNameLength){
		printf("%s - Can't create an option with name longer than %i chars!\n", 
					__FUNCTION__, kFCOptionMaxOptionNameLength);
		return NULL;
	}
	
	FCOptionRef option = malloc(sizeof(struct __FCOption));
	strcpy(option->_name, name);
	option->_type = type;
	return option;
}
const char *FCOptionName(FCOptionRef option){
	return option->_name;
}
FCOptionType FCOptionGetType(FCOptionRef option){
	return option->_type;
}
