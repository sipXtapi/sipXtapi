//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsAssert_h_
#define _OsAssert_h_

/* SYSTEM INCLUDES */

/* APPLICATION INCLUDES */

#ifdef __cplusplus
extern "C" {
#endif

/* DEFINES */

#define OS_ASSERT_REBOOT 47
#define OS_ASSERT_SUSPEND 17
#define OS_ASSERT_ALWAYS_SUSPEND 4717

/* MACROS */
/* EXTERNAL FUNCTIONS */

/*
 * OsAssert_SetFailureAction(boolean): 
 *   Set action on assert() failure:  reboot system, or suspend calling task
 */
#ifdef _VXWORKS /* [ */
extern int OsAssert_SetFailureAction(int assertFailureAction);
#else /* _VXWORKS ] [ */
#define OsAssert_SetFailureAction(assertFailureAction)
#endif /* _VXWORKS ] */

/*
 * assertCount():  Return number of assertion failures reported.
 */
extern int assertCount(void);

/* EXTERNAL VARIABLES */
/* CONSTANTS */
/* STRUCTS */
/* TYPEDEFS */
/* FORWARD DECLARATIONS */

#ifdef __cplusplus
}
#endif
#endif  /* _OsAssert_h_ */

