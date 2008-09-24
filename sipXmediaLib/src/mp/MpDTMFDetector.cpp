//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
#include <math.h>

// APPLICATION INCLUDES
#include "mp/MpDTMFDetector.h"


// Class data allocation
double MpDtmfDetector::sFreqs_to_detect[] = 
{ 
   697, 770, 852, 941, // DTMF key row
   1209, 1336, 1477, 1633 // DTMF key column
};
uint8_t MpDtmfDetector::snFreqsToDetect = sizeof(MpDtmfDetector::sFreqs_to_detect)/sizeof(double);


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpDtmfDetector::MpDtmfDetector(const unsigned samplesPerSec, const unsigned numProcessSamples)
: mSamplesPerSec(samplesPerSec)
, mNumProcessSamples(numProcessSamples)
{
   mQ1 = new double[snFreqsToDetect];
   mQ2 = new double[snFreqsToDetect];
   mR = new double[snFreqsToDetect];
   mCoefs = new double[snFreqsToDetect];
   reset();
}

// Destructor
MpDtmfDetector::~MpDtmfDetector()
{
   delete[] mQ1;
   delete[] mQ2;
   delete[] mR;
   delete[] mCoefs;
}

/* ============================ MANIPULATORS ============================== */

void MpDtmfDetector::reset()
{
   mSampleCount = 0;
   mLastDetectedDTMF = 0;
   int i;
   for(i=0; i< snFreqsToDetect; i++)
   {
      mQ1[i] = 0;
      mQ2[i] = 0;
      mR[i] = 0;
      mCoefs[i] = 0;
   }

   // Now calculate new coefficients 
   calcCoeffs();
}

void MpDtmfDetector::setSamplesPerSec(const unsigned samplesPerSec)
{
   mSamplesPerSec = samplesPerSec;
   reset();
}

void MpDtmfDetector::setNumProcessSamples(const unsigned numProcessSamples)
{
   mNumProcessSamples = numProcessSamples; 
}

UtlBoolean MpDtmfDetector::processSample(const MpAudioSample sample)
{
   UtlBoolean ret = FALSE;
   double q0;
   uint32_t i;

   mSampleCount++;
   for ( i=0; i<snFreqsToDetect; i++ )
   {
      q0 = mCoefs[i] * mQ1[i] - mQ2[i] + sample;
      mQ2[i] = mQ1[i];
      mQ1[i] = q0;
   }

   if (mSampleCount == mNumProcessSamples)
   {
      for ( i=0; i<snFreqsToDetect; i++ )
      {
         mR[i] = (mQ1[i] * mQ1[i]) + (mQ2[i] * mQ2[i]) - (mCoefs[i] * mQ1[i] * mQ2[i]);
         mQ1[i] = 0.0;
         mQ2[i] = 0.0;
      }
      dtmfValidation();
      mSampleCount = 0;
      ret = TRUE;
   }
   return ret;
}

/* ============================ ACCESSORS ================================= */

unsigned MpDtmfDetector::getSamplesPerSec() const
{
   return mSamplesPerSec;
}

unsigned MpDtmfDetector::getNumProcessSamples() const
{
   return mNumProcessSamples;
}

char MpDtmfDetector::getLastDetectedDTMF() const
{
   return mLastDetectedDTMF;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void MpDtmfDetector::calcCoeffs()
{
   int n;

   for(n = 0; n < snFreqsToDetect; n++)
   {
      mCoefs[n] = 2.0 * cos(2.0 * 3.141592654 * sFreqs_to_detect[n] / mSamplesPerSec);
   }
}

void MpDtmfDetector::dtmfValidation()
{
   int row, col, passed_tests;
   int peak_count, max_index;
   double maxval, t;
   int i;
   char row_col_ascii_codes[4][4] = 
   {
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
   };

   // Find the largest in the row group.
   row = 0;
   maxval = 0.0;
   for ( i=0; i<4; i++ )
   {
      if ( mR[i] > maxval )
      {
         maxval = mR[i];
         row = i;
      }
   }

   // Find the largest in the column group.
   col = 4;
   maxval = 0.0;
   for ( i=4; i<8; i++ )
   {
      if ( mR[i] > maxval )
      {
         maxval = mR[i];
         col = i;
      }
   }

   // Check for minimum energy
   if ( mR[row] < 4.0e5 )   // 2.0e5 ... 1.0e8 no change
   {
      // row frequency energy is not high enough
   }
   else if ( mR[col] < 4.0e5 )
   {
      // column frequency energy is not high enough
   }
   else
   {
      passed_tests = TRUE;

      // Twist check
      // CEPT => twist < 6dB
      // AT&T => forward twist < 4dB and reverse twist < 8dB
      //  -ndB < 10 log10( v1 / v2 ), where v1 < v2
      //  -4dB < 10 log10( v1 / v2 )
      //  -0.4  < log10( v1 / v2 )
      //  0.398 < v1 / v2
      //  0.398 * v2 < v1
      if ( mR[col] > mR[row] )
      {
         // Normal twist
         max_index = col;
         if ( mR[row] < (mR[col] * 0.398) )    // twist > 4dB results in error
         {
            passed_tests = FALSE;
         }
      }
      else // if ( r[row] > r[col] )
      {
         // Reverse twist
         max_index = row;
         if ( mR[col] < (mR[row] * 0.158) )    // twist > 8db results in error
         {
            passed_tests = FALSE;
         }
      }

      // Signal to noise test
      // AT&T states that the noise must be 16dB down from the signal.
      // Here we count the number of signals above the threshold and
      // there ought to be only two.
      if ( mR[max_index] > 1.0e9 )
      {
         t = mR[max_index] * 0.158;
      }
      else
      {
         t = mR[max_index] * 0.010;
      }

      peak_count = 0;
      for ( i=0; i<8; i++ )
      {
         if ( mR[i] > t )
            peak_count++;
      }
      if ( peak_count > 2 )
      {
         passed_tests = FALSE;
      }

      // Set the last detected DTMF tone.
      mLastDetectedDTMF = passed_tests ? row_col_ascii_codes[row][col-4] : NULL;
   }
}

/* ============================ FUNCTIONS ================================= */
