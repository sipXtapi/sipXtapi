// 
// Copyright (C) 2007 Jaroslav Libak
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef OsNameDbInit_h__
#define OsNameDbInit_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES

// Be sure to include all dependencies in the right order!
#include "utl/UtlInit.h"   // OsNameDb uses Utl

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

/**
*  @brief OsNameDB initializer class. Solves the problem with ordering of
*  constructors & destructors.
* 
*  The h file needs to be included in:
*  - all cpp files whose static members are initialized in OsNameDBInit
*  constructor, in order to make sure they are initialized properly whenever
*  they are linked into program.
*  - it also has to be included as the first include in cpp files, that
*  initialize static members whose constructor or destructor tries to use any
*  static members we initialize here.
*/
class OsNameDBInit
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

   /// Constructor
   OsNameDBInit(void);

   /// Destructor
   ~OsNameDBInit(void);
//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static unsigned int msCount; ///< Class instance counter.
};

/*
*  This fixes constructor/destructor ordering by declaring a static instance
*  of this Init class in every cpp file where this .h file is included.
*  Thus constructor of this class will be run first, and destructor as the last.
*/
static OsNameDBInit gOsNameDBInit;

#endif // OsNameDbInit_h__
