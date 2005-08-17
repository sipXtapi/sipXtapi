//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _CpMediaInterfaceFactoryFactory_h_
#define _CpMediaInterfaceFactoryFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cp/CpMediaInterfaceFactory.h>
#include <cp/CpMediaInterfaceFactoryImpl.h>
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

/**
 * Well known function that is called to create a CpMediaInterfaceFactory object,
 * which owns a CpMediaInterfaceFactoryImpl object, which this function also
 * creates.
 * This function must be implemented for all "plug-in" static sipx media processing
 * libraries. 
 */
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb);

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#endif
