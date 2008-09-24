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
                   int minOutputs, int maxOutputs);

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
                                     int samplesPerFrame,
                                     int samplesPerSecond
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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MpAudioResource(const MpAudioResource& rMpResource);

     /// Assignment operator (not implemented for this class)
   MpAudioResource& operator=(const MpAudioResource& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAudioResource_h_
