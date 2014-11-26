//
// Copyright (C) 2008-2014 SIPez LLC. All rights reserved.
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpVadBase_h_
#define _MpVadBase_h_

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
*  Base class for all VAD algorithms.
*
*  To create concrete class you could directly instantiate it or use
*  MpVadBase::createVad() static method for greater flexibility.
*
*  @nosubgrouping
*/

class MpVadBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Initialize VAD with given sample rate
   virtual OsStatus init(int samplesPerSec) = 0;
     /**<
     *  Should be called before any other class methods.
     */

     /// Factory method for VAD algorithms creation.
   static MpVadBase *createVad(const UtlString &name = "");
     /**<
     *  @param[in] name - name of VAD algorithm to use. Use empty string
     *             to get default algorithm.
     *
     *  @returns Method never returns NULL. If appropriate VAD algorithm is
     *           not found, default one is returned.
     */

     /// Destructor
   virtual ~MpVadBase() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Detect speech presence
   virtual MpSpeechType processFrame(uint32_t packetTimeStamp,
                                     const MpAudioSample* pBuf,
                                     unsigned inSamplesNum,
                                     const MpSpeechParams &speechParams,
                                     UtlBoolean calcEnergyOnly = FALSE) = 0;
     /**<
     * @param[in] packetTimeStamp - RTP timestamp of packet.
     * @param[in] pBuf - buffer with input data.
     * @param[in] inSamplesNum - number of samples of actual data, passed to
     *            this function.
     * @param[in] speechParams - various parameters of speech.
     * @param[in] calcEnergyOnly - if TRUE, VAD should calculate energy only.
     * @returns Method returns MP_SPEECH_ACTIVE or MP_SPEECH_SILENT mainly,
     *          but if algorithm doesn't handle some situations return value
     *          may be MP_SPEECH_UNKNOWN
     */

     /// Set algorithm parameter
   virtual OsStatus setParam(const char* paramName, void* value) = 0;
     /**<
     * @param[in] paramName - name of parameter
     * @param[in] value - value, can be any type, before call cast it to void*
     * @returns Method returns OS_SUCCESS if parameter has been set,
     *          otherwise OS_FAILED
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

     /// Prepare to process other unrelated audio stream.
   virtual void reset() = 0;
     /**<
     *  Only initialized algorithm can be reseted. Calling reset() should
     *  bring algorithm to the original state as it was right after init().
     *  So init() should NOT be called after reset().
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
     /// Return weighted energy of current frame
   virtual int getEnergy() const = 0;

   /// Set name
   void setName(const UtlString& name);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   static UtlString smDefaultAlgorithm; ///< Name of algorithm to be used by default.

   UtlString mName; ///< Name useful for debugging to distiguish multiple instances

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // _MpVadBase_h_
