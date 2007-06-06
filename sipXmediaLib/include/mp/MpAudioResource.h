//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpAudioResource_h_
#define _MpAudioResource_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResource.h"
#include "mp/MpAudioBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Abstract base class for all audio processing objects.
/**
*  This class adds doProcessFrame() function which should be
*  defined in all derived classes.
*/

class MpAudioResource : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpAudioResource(const UtlString& rName,
                   int minInputs, int maxInputs,
                   int minOutputs, int maxOutputs,
                   int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual ~MpAudioResource();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Wrapper around <i>doProcessFrame()</i>.
   virtual UtlBoolean processFrame(void);
     /**< 
     *  Returns TRUE if successful, FALSE otherwise.
     *  This method prepares the input buffers before calling
     *  <i>doProcessFrame()</i> and distributes the output buffers to the
     *  appropriate downstream resources after <i>doProcessFrame()</i>
     *  returns.
     */

     /// Sets the number of samples expected per frame.
   virtual UtlBoolean setSamplesPerFrame(int samplesPerFrame);
     /**< @returns FALSE if the specified rate is not supported, TRUE otherwise. */

     /// Sets the number of samples expected per second.
   virtual UtlBoolean setSamplesPerSec(int samplesPerSec);
     /**< Returns FALSE if the specified rate is not supported, TRUE otherwise. */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// @brief This method does the real work for the media processing resource and 
    /// must be defined in each class derived from this one.
   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000
                                    ) = 0;
     /**<
     *  @param inBufs (in) array of pointers to input buffers for the resource
     *  @param outBufs (out) array of pointers to output buffers produce by the resource
     *  @param inBufsSize (in) size of the inBufs array
     *  @param outBufsSize (in) size of the outBufs array
     *  @param isEnabled (in) indicates whether this resource has been enabled
     *  @param samplesPerFrame (in) samples to produce per frame processing interval
     *  @param samplesPerSecond (in) samples to produce per second
     * 
     *  @returns TRUE if successful, FALSE otherwise.
     */

     /// @brief Handles an incoming flowgraph message for this media processing object.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& fgMsg);
     /**< @returns TRUE if the message was handled, otherwise FALSE. */

     /// @brief Handles an incoming resource message for this media processing object.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);
     /**< @returns TRUE if the message was handled, otherwise FALSE. */

     /// Return number of samples per frame
   int getSamplesPerFrame();

     /// return number of samples per second
   int getSamplesPerSec();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int          mSamplesPerFrame;   ///< number of samples per frame
   int          mSamplesPerSec;     ///< number of samples per second

     /// Copy constructor (not implemented for this class)
   MpAudioResource(const MpAudioResource& rMpResource);

     /// Assignment operator (not implemented for this class)
   MpAudioResource& operator=(const MpAudioResource& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAudioResource_h_
