// 
// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _OrbitFileReader_h_
#define _OrbitFileReader_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OrbitData;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class OrbitFileReader
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OrbitFileReader();

   void setFileName(const UtlString& fileName);

   //:Default constructor
   ~OrbitFileReader();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    // Look up a user name in the list of orbits.
    // If found, return a pointer to the orbit data for the user.
    OrbitData* findInOrbitList(const UtlString& user);

    // Retrieve the "music on hold" file name.
    void getMusicOnHoldFile(UtlString& file);


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    
//    OrbitFileReader(const OrbitFileReader& rOrbitFileReader);
     //:Copy constructor

//    OrbitFileReader& operator=(const OrbitFileReader& rOrbitFileReader);
     //:Assignment operator
     
    // The full name of the orbit.xml file, or "" if there is none.
    UtlString mOrbitFileName;

    // A hash map that has as keys all the call parking orbit users, and
    // as values OrbitData objects containing the information for the orbits.
    UtlHashMap mOrbitList;

    // The last time we checked the modification time of mOrbitFileName.
    unsigned long mOrbitFileLastModTimeCheck;

    // The last known modification time of mOrbitFileName, or OS_INFINITY
    // if it did not exist.
    OsTime mOrbitFileModTime;

    // The file containing the "music on hold" audio.
    UtlString mMusicOnHoldFile;

/* //////////////////////////// PRIVATE /////////////////////////////////// */

    void refresh();

    OsStatus parseOrbitFile(UtlString& fileName);

};

// Object to contain the information in an orbit.xml entry.

class OrbitData : public UtlContainable
{

public:

   // The length of time in seconds before a parked call should be transferred
   // back to the parker.  NO_TIMEOUT means do not time out.
   int mTimeout;

   // The audio file to play for parked calls.
   UtlString mAudio;

   // The keycode for escaping from a parking orbit.
   // RFC 2833 code (as returned by enableDtmfEvent), or NO_KEYCODE
   // for no keycode.
   int mKeycode;

   // The maximum number of calls to handle in the orbit, or UNLIMITED_CAPACITY
   // if there is to be no limit.
   int mCapacity;

   virtual UtlContainableType getContainableType() const
      {
         return OrbitData::TYPE ;
      };

   virtual unsigned int hash() const
      {
         return (unsigned) this; 
      };

   virtual int compareTo(const UtlContainable* inVal) const
      {
         int result; 
   
         if (inVal->isInstanceOf(OrbitData::TYPE))
         {
            result = ((unsigned) this) - ((unsigned) ((OrbitData*) inVal));
         }
         else
         {
            result = -1; 
         }

         return result;
      };

   /** Class type used for runtime checking */
   static UtlContainableType TYPE;

   // Negative value used to indicate no mTimeout value is present.
   static const int NO_TIMEOUT;

   // Negative value used to indicate no mKeycode value is present.
   static const int NO_KEYCODE;

   // Very large positive value used to indicate there is no limit to
   // the number of calls to be handled by the orbit.
   static const int UNLIMITED_CAPACITY;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OrbitFileReader_h_
