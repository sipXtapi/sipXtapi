//
// Copyright (C) 2004-2020 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <limits.h>

// APPLICATION INCLUDES
#include "utl/UtlHistogram.h"
#include "os/OsTime.h"
#include "os/OsDateTime.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlHistogram::UtlHistogram(unsigned int bins, int base, unsigned int size,
                           const char* outputFormat,
                           unsigned int outputWidth)
{
   if (bins == 0)
   {
      OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                    "UtlHistogram::UtlHistogram bins must be at least 1");
      bins = 1;
   }
   mNoOfBins = bins;

   mBase = base;

   if (size == 0)
   {
      OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                    "UtlHistogram::UtlHistogram size must be at least 1");
      size = 1;
   }
   mBinSize = size;

   mpBins = new unsigned int[mNoOfBins + 2];
   mCount = 0;

   mOutputFormat = outputFormat;
   mOutputWidth = outputWidth;
}

// Destructor
UtlHistogram::~UtlHistogram()
{
   delete[] mpBins;
}


/* ============================ MANIPULATORS ============================== */

unsigned int UtlHistogram::tally(int value)
{
   // Calculate the bin number.
   unsigned int bin = (value - mBase) / mBinSize;
   // Increment the appropriate bin.
   ++mpBins[bin < 0 ? 0 :
            bin > mNoOfBins ? mNoOfBins + 1 :
            bin + 1];
   // Increment the total count.
   mCount++;

   return mCount;
}

void UtlHistogram::clear()
{
   // Clear the bins.
   // Remember that there are mNoOfBins + 2 bins.
   for (unsigned int i = 0; i < mNoOfBins + 2; i++)
   {
      mpBins[i] = 0;
   }
   // Clear the total count.
   mCount = 0;
}

/* ============================ ACCESSORS ================================= */


/**
 * Get the number of bins.
 */
unsigned int UtlHistogram::getNoOfBins()
{
   return mNoOfBins;
}

/**
 * Get the lowest value for bin 0.
 */
unsigned int UtlHistogram::getBase()
{
   return mBase;
}

/**
 * Get the size of each bin.
 */
unsigned int UtlHistogram::getBinSize()
{
   return mBinSize;
}

/**
 * Get the total count.
 */
unsigned int UtlHistogram::getCount()
{
   return mCount;
}

/**
 * Get the count in bin i.
 */
unsigned int UtlHistogram::operator[](unsigned int i)
{
   // Normalize i to index mpBins[].
   i++;
   return
      i >= 0 && i < mNoOfBins+2 ?
      mpBins[i] :
      0;
}

/* ============================ INQUIRY =================================== */

/**
 * Get a string containing the formatted values from the bins.
 */
UtlString* UtlHistogram::show()
{
   unsigned int width = 0;
   UtlString* value = new UtlString();

   // Translate the values.
   for (unsigned int i = 0, j = 0; i < mNoOfBins + 2; i++, j += width)
   {
      value->appendFormat(mOutputFormat, mpBins[i]);
      width = value->length() - width;
      if (width > mOutputWidth)
      {
         OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                       "UtlHistogram::show output from format '%s' "
                       "had width %d != declared width %d",
                       mOutputFormat, width, mOutputWidth);
      }
   }

   return value;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


