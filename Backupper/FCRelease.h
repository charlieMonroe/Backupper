//
//  FCRelease.h
//  Backupper
//
//  Created by Charlie Monroe on 04.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef Backupper_FCRelease_h
#define Backupper_FCRelease_h

#include <stdlib.h>

typedef void(*FCReleaseFunction)(void*);

static inline void FCRelease(void *memory){
	if (memory != NULL){
		free(memory);
	}
	#if DEBUG
	else{
		printf("****WARNING - trying to release NULL memory.\n");
	}
	#endif
}

#endif
