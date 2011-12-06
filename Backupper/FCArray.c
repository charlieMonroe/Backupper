//
//  FCArray.c
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "FCArray.h"
#include "FCString.h"
#include "FCRelease.h"


// Opaque type
struct __FCArray{
	int _capacity;
	int _currentIndex;
	void **_array;
};

static void _FCArrayGrow(FCArrayRef arr){
	// Realloc should copy the memory
	arr->_capacity *= 2;
	arr->_array = realloc(arr->_array, sizeof(void*) * arr->_capacity);
	
	// Realloc doesn't zero the memory, need to do it manually
	for (int i = arr->_currentIndex + 1; i < arr->_capacity; ++i){
		arr->_array[i] = NULL;
	}
}

void FCArrayAddItem(FCArrayRef arr, void *item){
	if (!arr){
		return;
	}
	
	arr->_currentIndex++;
	if (arr->_currentIndex >= arr->_capacity){
		_FCArrayGrow(arr);
	}
	
	arr->_array[arr->_currentIndex] = item;
}
int FCArrayCapacity(FCArrayRef arr){
	if (arr){
		return arr->_capacity;
	}else{
		return 0;
	}
}
int FCArrayCount(FCArrayRef arr){
	if (arr){
		return arr->_currentIndex + 1;
	}else{
		return 0;
	}
}
FCArrayRef FCArrayCreate(){
	return FCArrayCreateWithCapacity(0);
}
FCArrayRef FCArrayCreateFromString(const char* string){
	FCArrayRef arr = FCArrayCreate();
	
	char *stringCopy = FCStringCopy(string);
	char *beginning = stringCopy;
	size_t strLen = strlen(stringCopy);
	
	for (int i = 0; i < strLen; ++i){
		if (stringCopy[i] == ';'){
			// New
			stringCopy[i] = '\0';
			
			if (strlen(beginning) > 0){
				FCArrayAddItem(arr, FCStringCopy(beginning));
			}
			
			++i;
			beginning = stringCopy + i;
		}
	}
	
	if (strlen(beginning) != 0){
		FCArrayAddItem(arr, FCStringCopy(beginning));
	}
	free(stringCopy);
	
	return arr;
}
FCArrayRef FCArrayCreateWithCapacity(int capacity){
	if (capacity == 0){
		capacity = 16;
	}
	
	FCArrayRef arr = malloc(sizeof(struct __FCArray));
	arr->_capacity = capacity;
	arr->_currentIndex = -1;
	arr->_array = calloc(1, sizeof(void*) * arr->_capacity);
	
	return arr;
}
void *FCArrayItemAtIndex(FCArrayRef arr, int index){
	assert(index < arr->_capacity && index >= 0);
	
	return arr->_array[index];
}
void FCArrayPrint(FCArrayRef arr){
	for (int i = 0; i < FCArrayCount(arr); ++i){
		if (i != FCArrayCount(arr) - 1){
			printf("%s,\n", arr->_array[i]);
		}else{
			printf("%s", arr->_array[i]);
		}
	}
}
void FCArrayRelease(FCArrayRef arr){
	if (arr == NULL){
		return;
	}
	
	free(arr->_array);
	FCRelease(arr);
}
void FCArrayReleaseWithOptions(FCArrayRef arr, BOOL releaseItems, void (releaseFunction)(void*)){
	if (arr == NULL){
		return;
	}
	
	if (!releaseItems){
		FCArrayRelease(arr);
		return;
	}
	
	if (releaseFunction == NULL){
		releaseFunction = FCRelease;
	}
	
	for (int i = 0; i < FCArrayCount(arr); ++i){
		releaseFunction(arr->_array[i]);
	}
	
	FCArrayRelease(arr);
}
void FCArrayRemoveItem(FCArrayRef arr, void *item){
	int index = 0;
	for (index = 0; index < FCArrayCount(arr); ++index){
		if (arr->_array[index] == item){
			break;
		}
	}
	
	if (index == FCArrayCount(arr)){
		#if DEBUG
			printf("%s - ***WARNING: removing an item that is not in the array - %p.\n", __FUNCTION__, item);
		#endif
		return;
	}
	
	if (index == FCArrayCount(arr) - 1){
		// Easy case, it's the last item, just wipe it and move the pointer
		arr->_array[index] = NULL;
		--arr->_currentIndex;
		return;
	}
	
	// Otherwise - take the last item and move it to the index slot
	arr->_array[index] = arr->_array[arr->_currentIndex];
	arr->_array[arr->_currentIndex] = NULL;
	--arr->_currentIndex;
	
}

