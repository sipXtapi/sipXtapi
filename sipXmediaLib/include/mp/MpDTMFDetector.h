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

#ifndef _MpDTMFDetector_h_
#define _MpDTMFDetector_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include <mp/MpTypes.h>
#include <utl/UtlDefs.h>  // UtlBoolean

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
*  @brief A simple DTMF detector class that uses the Goertzel algorithm
*
*  The MpDtmfDetector is a simple class that implements the goertzel algorithm
*  for detecting multiple frequencies.  It also incorporates some other checks
*  to make sure that the DTMF tones pass other tests that are needed for working
*  with bell standards (i.e. twist is in-range, etc).
*  
*  Full Disclosure:
*  This code is based off of the Goertzel example C code in wikipedia:
*  http://en.wikipedia.org/w/index.php?title=Goertzel_algorithm&oldid=179514279
*  It has been heavily re-worked by Keith Kyzivat, but important parts may be
*  identical.
*  
*  Usage:
*  To use this, construct it with your sample rate and an adequate value for 
*  number of samples to do the goertzel algorithm on.  
*  Then just call processSample repeatedly as you get samples.  It will return 
*  /p FALSE, until /p nProcessSamples samples have been processed, after 
*  which time it will return /p TRUE.
*  You then can get the detected DTMF using getLastDetectedDTMF(). If no DTMF 
*  was properly detected, NULL will be returned.
*  
*/
class MpDtmfDetector
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpDtmfDetector(const unsigned samplesPerSec, const unsigned nProcessSamples);

   /// Destructor
   virtual ~MpDtmfDetector();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Reset the state of the detector.
   void reset();
     /**<
     *  Reset all accumulators, last detected DTMF, and recalculate coefficients.
     */

     /// Set the sample rate.
   void setSamplesPerSec(const unsigned samplesPerSec);
     /**<
     *  @param[in] samplesPerSec - The new sample rate to use.
     *
     *  @NOTE This resets the object state and recalculates coefficients.
     */

     /// Set the number of samples to use to detect frequencies on.
   void setNumProcessSamples(const unsigned nProcessSamples);
     /**<
     *  This sets the number of samples that are collected before running the 
     *  goertzel algorithm.  The Goertzel algorithm will then be run on the
     *  collected samples.
     *
     *  @NOTE 92 is shown in wikipedia, 205 is mentioned in some mailing lists 
     *  as a good choice -- both are only for 8khz.
     *
     *  @param[in] nProcessSamples - the number of samples that are collected
     *  before running the goertzel algorithm.
     */

     /// Process a sample through the detector.
   UtlBoolean processSample(const MpAudioSample sample);
     /**<
     *  When getNumProcessSamples() samples are processed, a DTMF tone is either
     *  detected or not.  In either case, the stored last DTMF tone detected is
     *  overwritten with the current detection value.
     *  
     *  @see getLastDetectedDTMF()
     *  
     *  @return /p FALSE if this has not yet processed a multiple of getNumProcessSamples(), 
     *          /p TRUE if this has processed a multiple of getNumProcessSamples()
     *  
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get the sample rate.
   unsigned getSamplesPerSec() const;

     /// Get the number of samples that this detector uses to determine frequencies on.
   unsigned getNumProcessSamples() const;

     /// Get the last DTMF tone that the detector detected.
   char getLastDetectedDTMF() const;
     /**<
     *  When processSample() processes getNumProcessSamples() samples, 
     *  the last detected DTMF tone will be stored.  Use this to access the
     *  detected tone.
     *  
     *  @NOTE If getNumProcessSamples() samples have not been processed yet, or 
     *  if a tone was not detected during the last getNumProcessSamples(), then 
     *  NULL will be returned here.
     *
     *  @return the last detected DTMF tone.  If no tone has been detected,
     */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
     /// Calculate coefficients needed for the goertzel algorithm
   void calcCoeffs();
     /**<
     *  These coefficients are dependent on the sample rate, so new coefficients
     *  need to be calculated whenever the sample rate changes.
     *
     *  From Wikipedia:
     *
     * coef = 2.0 * cos( (2.0 * PI * k) / (float)GOERTZEL_N)) ;
     * Where k = (int) (0.5 + ((float)GOERTZEL_N * target_freq) / SAMPLING_RATE));
     *
     * More simply: coef = 2.0 * cos( (2.0 * PI * target_freq) / SAMPLING_RATE );
     */

     /// Validate the detected frequencies detected by processSample.
   void dtmfValidation();
     /**<
     *  This looks at the detected frequencies, and determines if it conforms
     *  to DTMF rules as specified by bell, and others -- i.e. can work well
     *  with PSTN.
     */

private:
   unsigned mSamplesPerSec;
   unsigned mNumProcessSamples;
   uint32_t mSampleCount;

   static double sFreqs_to_detect[]; 
   static uint8_t snFreqsToDetect;

   double* mQ1;
   double* mQ2;
   double* mR;

   double* mCoefs;

   char mLastDetectedDTMF;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDTMFDetector_h_
