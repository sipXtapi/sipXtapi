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

#ifndef _MpSpeakerSelectBase_h_
#define _MpSpeakerSelectBase_h_

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
*  Base class for all Speaker Selection (SS) algorithms.
*
*  To create concrete class you could directly instantiate it or use
*  MpSpeakerSelectBase::createInstance() static method for greater flexibility.
*
*  @nosubgrouping
*/

class MpSpeakerSelectBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Initialize SS to initial state.
   virtual OsStatus init(int maxParticipants, int maxActive) = 0;
     /**<
     *  @param[in] maxParticipants - maximum number of participants in a conference.
     *  @param[in] maxActive - maximum number of participants to be detected 
     *             as active speech, -1 lead to no constrains. Correct setting
     *             of \p maxActive helps SS algorithm to minimize number of
     *             flips between least ranked speaker inside \p maxActive
     *             speakers and most ranked speaker outside of this number.
     *
     *  Should be called before any other class methods. All participants
     *  after initialization are disabled, to enable it call
     *  enableParticipant().
     */

     /// Factory method for SS algorithms creation.
   static MpSpeakerSelectBase *createInstance(const UtlString &name = "");
     /**<
     *  @param[in] name - name of SS algorithm to use. Use empty string
     *             to get default algorithm.
     *
     *  @note To date we have no available SS algorithms in open-source,
     *        so NULL is always returned.
     *
     *  @returns If appropriate SS algorithm is not found, default one is
     *           returned.
     */

     /// Destructor
   virtual ~MpSpeakerSelectBase() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Reset algorithm state to initial and prepare for processing of new data.
   virtual void reset() = 0;
     /**<
     *  It's supposed that init() will not be called after reset(). So reset()
     *  must turn algorithm to the state as right after calling init().
     *  Maximum number of participants intentionally is not changed, to prevent
     *  memory reallocation.
     */

     /// Enable/disable processing of a selected participant.
   virtual OsStatus enableParticipant(int num, UtlBoolean newState) = 0;
     /**<
     *  Initially all participants are disabled. Use this method to enable
     *  processing for newly added participants and disable processing for
     *  removed participants. Data from disabled participants are just ignored.
     *
     *  @param[in] num - number of participant, starting from zero
     *  @param[in] newState - pass TRUE to enable processing of this participant,
     *             FALSE to disable.
     *
     *  @retval OS_SUCCESS if participant is successfully enabled
     *          or disabled.
     *  @retval OS_INVALID_ARGUMENT if participant number refers to non existing
     *          participant.
     */

     /// Compute speaker ranks.
   virtual OsStatus processFrame(MpSpeechParams* speechParams[],
                                 int frameSize) = 0;
     /**<
     *  @param[in] speechParams - parameters of bridges
     *  @param[in] frameSize - number of milliseconds in frame
     *
     *  @retval OS_SUCCESS if processing is ok.
     */

     /// Set algorithm parameter
   virtual OsStatus setParam(const char* paramName, void* value) = 0;
     /**<
     *  @param[in] paramName - name of parameter.
     *  @param[in] value - pointer to a value.
     *
     *  @retval OS_SUCCESS if parameter has been set.
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

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// Is processing enabled for a given participant?
   virtual OsStatus isParticipantEnabled(int num, UtlBoolean &enabled) = 0;
     /**<
     *  @param[in]  num - number of a participant, starting from zero.
     *  @param[out] enabled - variable is set to TRUE processing of this
     *              participant is enabled, FALSE otherwise.
     *
     *  @retval OS_SUCCESS if value is returned.
     *  @retval OS_INVALID_ARGUMENT if participant number refers to non existing
     *          participant.
     */

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   static UtlString smDefaultAlgorithm; ///< Name of algorithm to be used by default.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // _MpSpeakerSelectBase_h_
