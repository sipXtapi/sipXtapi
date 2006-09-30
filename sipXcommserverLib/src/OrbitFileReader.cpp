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

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include <os/OsFS.h>
#include <os/OsDateTime.h>
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"
#include <utl/UtlHashMapIterator.h>
#include "OrbitFileReader.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

UtlContainableType OrbitData::TYPE = "OrbitData" ;

const int OrbitData::NO_TIMEOUT = -1;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OrbitFileReader::OrbitFileReader()
{
}

void OrbitFileReader::setFileName(const UtlString& fileName)
{
   // Set the file name.
   mOrbitFileName = fileName;
   OsSysLog::add(FAC_DB, PRI_INFO, "OrbitFileReader:: "
                 "Orbit file is '%s'", mOrbitFileName.data());

   // Initialize the state variables.
   // 0 is never returned as a value of OsDateTime::getSecsSinceEpoch()..
   mOrbitFileLastModTimeCheck = 0;
   // OS_INFINITY is never a valid time.
   // We use it as a dummy meaning "file does not exist".
   mOrbitFileModTime = OsTime::OS_INFINITY;

   // Force the caching scheme to read in the orbit file, so that
   // if there are any failures, they are reported near the start
   // of the application's log file.
   UtlString dummy("dummy value");
   findInOrbitList(dummy);
}

// Destructor
OrbitFileReader::~OrbitFileReader()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Return pointer to the OrbitData structure if the argument is an
// orbit name listed in the orbits.xml file.
OrbitData* OrbitFileReader::findInOrbitList(const UtlString& user)
{
   OrbitData* ret;

   // Refresh mOrbitList if necessary.
   refresh();

   // Check to see if 'user' is in it.  If so, return a pointer to its
   // data.
   ret = dynamic_cast <OrbitData*> (mOrbitList.findValue(&user));

   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitFileReader::findInOrbitList "
                 "user = '%s', ret = %p",
                 user.data(), ret);
   return ret;
}

// Retrieve the "music on hold" file name.
void OrbitFileReader::getMusicOnHoldFile(UtlString& file)
{
   // Refresh mMusicOnHoldFile if necessary.
   refresh();

   // Get the value.
   file = mMusicOnHoldFile;

   OsSysLog::add(FAC_PARK, PRI_DEBUG,
                 "OrbitFileReader::getMusicOnHoldFile "
                 "file = '%s'",
                 file.data());
   return;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Refresh the data structures if the contents of the file has changed.
// This function takes some care to avoid re-reading orbits.xml when it has
// not changed since the last call.
// The strategy is to check the modification time of the orbits.xml file,
// and only re-read orbits.xml if the modification time has changed since
// the last time we checked it.
// But checking the modification time is relatively slow, and we do not want
// to do it on all calls in a high-usage system.  So we check the clock time
// instead, and if it has been 1 second since the last time we checked
// the modification time of orbits.xml, we check it again.
// Checking the clock time is fast (about 1.6 microseconds on a 2GHz
// processor), and checking the modification time of orbits.xml once a
// second is acceptable.
// Any process which changes orbits.xml should wait 1 second before reporting
// that it has succeeded, and before doing any further changes to orbits.xml.
void OrbitFileReader::refresh()
{
   // If there is no orbit file name, just return, as there is no file
   // to read..
   if (!mOrbitFileName.isNull())
   {
      // Check to see if 1 second has elapsed since the last time we checked
      // the modification time of orbits.xml.
      unsigned long current_time = OsDateTime::getSecsSinceEpoch();
      if (current_time != mOrbitFileLastModTimeCheck)
      {
         // It has been.
         mOrbitFileLastModTimeCheck = current_time;

         // Check to see if orbits.xml has a different modification time than
         // the last time we checked.
         OsFile orbitFile(mOrbitFileName);
         OsFileInfo fileInfo;
         OsTime mod_time;
         if (orbitFile.getFileInfo(fileInfo) == OS_SUCCESS) {
            // If the file exists, use its modification time.
            fileInfo.getModifiedTime(mod_time);
         }
         else
         {
            // If the file does not exist, use OS_INFINITY as a dummy value.
            mod_time = OsTime::OS_INFINITY;
         }

         // Check to see if the modification time of orbits.xml is different
         // than the last time we checked.
         if (mod_time != mOrbitFileModTime)
         {
            // It is different.
            mOrbitFileModTime = mod_time;

            // Clear the list of the previous orbit names.
            mOrbitList.destroyAll();
            // Forget the music-on-hold file.
            mMusicOnHoldFile.remove(0);

            if (mOrbitFileModTime != OsTime::OS_INFINITY)
            {
               // The file exists, so we should read and parse it.
               OsStatus status = parseOrbitFile(mOrbitFileName);
               OsSysLog::add(FAC_PARK, PRI_INFO,
                             "OrbitFileReader::findInOrbitList "
                             "Called parseOrbitFile('%s') returns %s",
                             mOrbitFileName.data(),
                             status == OS_SUCCESS ? "SUCCESS" : "FAILURE");
            }
            else
            {
               // The file does not exist, that is not an error.
               // Take no further action.
               OsSysLog::add(FAC_PARK, PRI_INFO,
                             "OrbitFileReader::findInOrbitList "
                             "Orbit file '%s' does not exist",
                             mOrbitFileName.data());
            }
         }
      }
   }
}

// Read and parse the orbits.xml file into the data structures.
OsStatus OrbitFileReader::parseOrbitFile(UtlString& fileName)
{
   // Initialize Tiny XML document object.
   TiXmlDocument document;
   TiXmlNode* orbits_element;
   if (
      // Load the XML into it.
      document.LoadFile(fileName.data()) &&
      // Find the top element, which should be an <orbits>.
      (orbits_element = document.FirstChild("orbits")) != NULL &&
      orbits_element->Type() == TiXmlNode::ELEMENT)
   {
      // Find all the <orbit> elements.
      for (TiXmlNode* orbit_element = 0;
           (orbit_element = orbits_element->IterateChildren("orbit",
                                                            orbit_element));
         )
      {
         // Process each <orbit> element.
         bool orbit_valid = true;

         // Process the <extension> element.
         TiXmlNode* extension_element =
            orbit_element->FirstChild("extension");
         UtlString extension;
         if (extension_element)
         {
            textContentShallow(extension, extension_element->ToElement());
            if (extension.isNull())
            {
               // Extension had zero length
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "Extension was null.");
               orbit_valid = false;
            }
         }
         else
         {
            // Extension was missing.
            OsSysLog::add(FAC_PARK, PRI_ERR,
                          "OrbitFileReader::parseOrbitFile "
                          "Extension was missing.");
            orbit_valid = false;
         }

         // Process the <background-audio> element.
         TiXmlNode* audio_element =
            orbit_element->FirstChild("background-audio");
         UtlString audio;
         if (audio_element)
         {
            textContentShallow(audio, audio_element->ToElement());
            if (audio.isNull())
            {
               // Extension had zero length
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "Background-audio was null for extension '%s'",
                             extension.data());
               orbit_valid = false;
            }
         }
         else
         {
            // Background-audio was missing.
            OsSysLog::add(FAC_PARK, PRI_ERR,
                          "OrbitFileReader::parseOrbitFile "
                          "Background-audio was missing for extension '%s'",
                          extension.data());
            orbit_valid = false;
         }

         // Process the <timeout> element to set mTimeout.
         int timeout = OrbitData::NO_TIMEOUT;	// Assume no value present.
         TiXmlNode* timeout_element =
            orbit_element->FirstChild("timeout");
         if (timeout_element)
         {
            UtlString temp;
            textContentShallow(temp, timeout_element->ToElement());
            char *endptr;
            timeout = strtol(temp.data(), &endptr, 10);
            if (temp.isNull() ||
                endptr - temp.data() != temp.length())
            {
               // Timeout was null or unparsable.
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "Timeout '%s' was null or unparsable for extension '%s'",
                             temp.data(), extension.data());
               orbit_valid = false;
            }
         }

         // If no errors were found, create the values to insert into
         // mOrbitList.
         if (orbit_valid)
         {
            // Allocate the objects and assign their values.
            UtlString* extension_heap = new UtlString;
            *extension_heap = extension;
            OrbitData* orbit_data_heap = new OrbitData;
            orbit_data_heap->mTimeout = timeout;
            orbit_data_heap->mAudio = audio;

            // Attempt to insert the user into the orbit list.
            if (mOrbitList.insertKeyAndValue(extension_heap, orbit_data_heap))
            {
               // Insertion succeeded.
               // *extension_heap and *orbit_data_heap are now owned
               // by mOrbitList.
            }
            else
            {
               // Insertion failed, presumably because the extension was
               // already in there.
               OsSysLog::add(FAC_PARK, PRI_ERR,
                             "OrbitFileReader::parseOrbitFile "
                             "Inserting extension '%s' failed -- specified as an orbit twice?",
                             extension_heap->data());
               // mOrbitList does not own the objects, so we must delete them.
               delete extension_heap;
               delete orbit_data_heap;
            }
         }
      }

      if (OsSysLog::willLog(FAC_PARK, PRI_DEBUG))
      {
         // Output the list of orbits.
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitFileReader::parseOrbitFile "
                       "Valid orbits are:");
         UtlHashMapIterator itor(mOrbitList);
         while (itor())
         {
            OsSysLog::add(FAC_PARK, PRI_DEBUG,
                          "OrbitFileReader::parseOrbitFile "
                          "Orbit '%s', mTimeout = %d, mAudio = '%s'",
                          (dynamic_cast<UtlString*> (itor.key()))->data(),
                          (dynamic_cast<OrbitData*> (itor.value()))->mTimeout,
                          (dynamic_cast<OrbitData*> (itor.value()))->
                            mAudio.data());
         }
         OsSysLog::add(FAC_PARK, PRI_DEBUG,
                       "OrbitFileReader::parseOrbitFile "
                       "End of list");
      }

      // Find the <music-on-hold> element.
      TiXmlNode *groupNode = orbits_element->FirstChild("music-on-hold");
      if (groupNode != NULL)
      {
         TiXmlNode* audioNode = groupNode->FirstChild("background-audio");
         if ((audioNode != NULL)
             && (audioNode->FirstChild() != NULL))
         {
            mMusicOnHoldFile = (audioNode->FirstChild())->Value();
         }
      }
      OsSysLog::add(FAC_PARK, PRI_DEBUG,
                    "OrbitFileReader::parseOrbitFile "
                    "mMusicOnHoldFile = '%s'",
                    mMusicOnHoldFile.data());

      // In any of these cases, attempt to do call retrieval.
      return OS_SUCCESS;
   }
   else
   {
      // Report error parsing file.
      OsSysLog::add(FAC_PARK, PRI_CRIT,
                    "OrbitFileReader::parseOrbitFile "
                    "Orbit file '%s' could not be parsed.", fileName.data());
      // No hope of doing call retrieval.
      return OS_FAILED;
   } 
}
