// This include file is designed to contain items useful to all code files and projects.
#ifndef GLOBAL_H
#define GLOBAL_H

// global AVRLIB defines
#include "avrlibdefs.h"
// global AVRLIB types definitions
#include "avrlibtypes.h"

// project/system dependent defines

#ifndef F_CPU
	#define	F_CPU			16000000	// 16MHz processor
#endif

#define CYCLES_PER_US	((F_CPU+500000)/1000000)	// cpu cycles per microsecond
#define MY_DEBUG

#endif
