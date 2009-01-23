//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpPlcBase_h_
#define _MpPlcBase_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include "mp/MpTypes.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  Base class for all PLC algorithms.
*
*  To create concrete class you could directly instantiate it or use
*  MpPlcBase::createPlc() static method for greater flexibility.
*
*  @nosubgrouping
*/
class MpPlcBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Factory method for PLC algorithms creation.
   static MpPlcBase *createPlc(const UtlString &name = "");
     /**<
     *  @param[in] name - name of PLC algorithm to use. Use empty string
     *             to get default algorithm.
     *
     *  @returns Method never returns NULL. If appropriate PLC algorithm is
     *           not found, default one is returned.
     */

     /// Initialize PLC with given sample rate and frame size.
   virtual OsStatus init(int samplesPerSec) = 0;
     /**<
     *  Should be called before any other class methods.
     */

     /// Virtual base destructor.
   virtual ~MpPlcBase() {};

     /// Reset
   virtual void reset() = 0;

     /// Full reset followed by init call
   virtual void fullReset() = 0;

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Update PLC history with late/future frame.
   virtual OsStatus insertToHistory(int frameNum,
                                    const MpSpeechParams &speechParams,
                                    MpAudioSample* pBuf,
                                    unsigned inSamplesNum) = 0;
     /**<
     * @param[in] frameNum - position of passed data relative to current moment.
     *            E.g. \p frameNum<0 means late frame, \p frameNum>0 means
     *            future frame and \p frameNum==0 means current frame. Passing
     *            late and future frames may improve PLC quality if algorithm
     *            supports this. Passing late and future frames they should
     *            be in bounds set by of getMaxFutureFramesNum() and
     *            getMaxDelayedFramesNum().
     * @param[in] speechParams - various parameters of speech.
     * @param[in,out] pBuf - buffer with input data.
     * @param[in] inSamplesNum - number of samples of actual data, passed to
     *            this function.
     */

     /// Process next frame - do PLC and/or adjustment if needed.
   virtual OsStatus processFrame(MpSpeechParams &speechParams,
                                 MpAudioSample* pBuf,
                                 unsigned bufferSize,
                                 unsigned inSamplesNum,
                                 unsigned outSamplesNum,
                                 int wantedAdjustment,
                                 int &madeAdjustment) = 0;
     /**<
     * @param[in] speechParams - various parameters of speech.
     * @param[in,out] pBuf - buffer with input data and place for output data.
     *            Data is read from this buffer if \p numSamples>0. If any
     *            processing is needed, i.e. PLC or adjustment is requested,
     *            output data will also be written to this buffer.
     * @param[in] bufferSize - size of \p pBuf buffer. This value is always
     *            greater or equal to \p numSamples. When algorithm needs to
     *            write data to buffer it must take this value into account.
     * @param[in] inSamplesNum - number of samples of actual data, passed to
     *            this function. If we've got no data and PLC is needed,
     *            this parameter should be equal to 0. If \p inSamplesNum is
     *             not 0, it is equal to \p outSamplesNum.
     * @param[in] outSamplesNum - output frame size. Together with
     *            \p wantedAdjustment this value sets bounds in which we want
     *            to fit output data length. See \p wantedAdjustment for more
     *            details. If \p inSamplesNum is not 0, \p outSamplesNum is
     *            equal to it.
     * @param[in] wantedAdjustment - number of samples by which we want to
     *            adjust audio stream. Output number of samples must be in range
     *            [\p outSamplesNum; \p outSamplesNum + \p wantedAdjustment].
     *            Output lengths closer to \p outSamplesNum + \p wantedAdjustment
     *            are desired, but not required. Note, that output lengths
     *            not in this range are not allowed and may confuse upper levels.
     *            Positive values request stream extension (samples should be
     *            added), negative values request stream reduction (samples
     *            should be took off).
     * @param[out] madeAdjustment - length of made adjustment. Shows how many
     *            samples were added to stream (if positive) or removed from
     *            stream (if negative).
     */

     /// Set algorithm to be used by default.
   static void setDefaultAlgorithm(const UtlString& name);
     /**<
     *  Initially default algorithm is defined at compile time. Using this
     *  function you can change default algorithm at run-time or switch back to
     *  compile-time default.
     *
     *  @param[in] name - name of algorithm to use by default. Reverts to
     *             compile-time default if empty.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return maximum offset for delayed frame with respect to current
   virtual int getMaxDelayedFramesNum() const = 0;
     /**<
     *  If zero is returned, late packets are not allowed.
     */

     /// Return maximum offset for future frame with respect to current
   virtual int getMaxFutureFramesNum() const = 0;
     /**<
     *  If zero is returned, future packets are not allowed.
     */

     /// Return algorithmic delay in samples, should be called after init
   virtual int getAlgorithmicDelay() const = 0;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   static UtlString smDefaultAlgorithm; ///< Name of algorithm to be used by default.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif //_MpPlcBase_h_
