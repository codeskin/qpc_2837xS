/*
	Copyright (c) 2018 by CodeSkin
	All rights reserved.

	NOTICE: This software embodies unpublished proprietary
	and trade secret information of CodeSkin.  Any copying,
	disclosure or transmission not expressly authorized by
	the owner is expressly prohibited. CodeSkin retains exclusive
	ownership of all intellectual property rights in this software,
	including all copyright, trade secret and patent rights.
*/

#ifndef qf_port_h
#define qf_port_h

/* The maximum number of active objects in the application, see NOTE1 */
#define QF_MAX_ACTIVE           32

/* The maximum number of system clock tick rates */
#define QF_MAX_TICK_RATE        2

#include "qep_port.h" /* QEP port */

extern uint16_t DisableInt();
extern void RestoreInt(uint16_t stat0);

extern uint16_t IntStat;

#define QF_INT_DISABLE() do {\
		IntStat = DisableInt();\
	} while(0)

#define QF_INT_ENABLE() do {\
		RestoreInt(IntStat);\
	} while(0)

#define QF_CRIT_ENTRY(dummy) QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)  QF_INT_ENABLE()

#include "qv_port.h"  /* QV cooperative kernel port */

#include "qf.h"       /* QF platform-independent public interface */

#endif /* qf_port_h */

