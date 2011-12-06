//
//  FCOptionValue.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "FCOptionValue.h"
#include "FCRelease.h"

struct __FCOptionValue{
	FCOptionRef _option;
	void *_value;
	FCReleaseFunction _releaseFunction;
	BOOL _releaseValue;
};

	
FCOptionValueRef FCOptionValueCreateWithValue(FCOptionRef option, void *value, BOOL releaseValue, FCReleaseFunction releaseFunction){
	FCOptionValueRef optionValue = malloc(sizeof(struct __FCOptionValue));
	optionValue->_option = option;
	optionValue->_value = value;
	optionValue->_releaseFunction = releaseFunction ? releaseFunction : FCRelease;
	optionValue->_releaseValue = releaseValue;
	return optionValue;
}
FCOptionRef FCOptionValueGetOption(FCOptionValueRef optionValue){
	return optionValue->_option;
}
void *FCOptionValueGetValue(FCOptionValueRef optionValue){
	return optionValue->_value;
}
void FCOptionValueSetValue(FCOptionValueRef optionValue, void *value){
	if (optionValue->_releaseValue){
		optionValue->_releaseFunction(optionValue->_value);
	}
	optionValue->_value = value;
}
void FCOptionValueRelease(FCOptionValueRef optionValue){
	if (optionValue->_releaseValue){
		optionValue->_releaseFunction(optionValue->_value);
	}
	FCRelease(optionValue);
}

