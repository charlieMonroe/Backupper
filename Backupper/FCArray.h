//
//  FCArray.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCArray_h
#define Backupper_FCArray_h

#include "FCTypes.h"

// Opaque type
struct __FCArray;
typedef struct __FCArray* FCArrayRef;


void FCArrayAddItem(FCArrayRef arr, void *item);
int FCArrayCapacity(FCArrayRef arr); // Number of items in array
FCArrayRef FCArrayCreate(void); // Uses default capacity
FCArrayRef FCArrayCreateFromString(const char* string); // The values are always string and are copied
FCArrayRef FCArrayCreateWithCapacity(int count);
int FCArrayCount(FCArrayRef arr); // Number of items in array
void *FCArrayItemAtIndex(FCArrayRef arr, int index);
void FCArrayPrint(FCArrayRef arr); // Only prints strings
void FCArrayRelease(FCArrayRef arr);
void FCArrayRemoveItem(FCArrayRef arr, void *item); // Item doesn't get released

/** If releaseItems == NO, just calls FCArrayRelease. If YES and releaseFunction == NULL, 
      then FCRelease is used on each item. */
void FCArrayReleaseWithOptions(FCArrayRef arr, BOOL releaseItems, void (releaseFunction)(void*));

#endif
