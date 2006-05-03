//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlRandom_h_
#define _UtlRandom_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A simple utility class that provides random number functionality.  
 * Generally, you should create a UtlRandom class for each of your thread
 * contexts.  The implementation uses rand() and srand().
 */
class UtlRandom
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructors, seeds the random number generator with the current
    * time.
    */
   UtlRandom();

   /**
    * Constructor accepting a seed for the random number generator.
    */
   UtlRandom(int seed);
     
   /**
    * Destructor
    */
   virtual ~UtlRandom();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   /**
    * Generate a pseudo-random number between 0 and RAND_MAX (defined in 
    * stdlib.h)
    */
   int rand() ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _UtlRandom_h_
