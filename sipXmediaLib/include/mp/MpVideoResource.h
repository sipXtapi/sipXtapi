//  
// Copyright (C) 2006-2011 SIPez LLC. All rights reserved
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpVideoResource_h_
#define _MpVideoResource_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpResource.h>
#include <mp/MpBuf.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Abstract base class for all video processing objects.
/**
*  This class adds doProcessFrame() function which should be
*  defined in all derived classes.
*/

class MpVideoResource : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpVideoResource(const UtlString& rName,
                   int minInputs, int maxInputs,
                   int minOutputs, int maxOutputs);

     /// Destructor
   virtual ~MpVideoResource();

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
                                     UtlBoolean isEnabled
                                    ) = 0;
     /**<
     *  @param inBufs (in) array of pointers to input buffers for the resource
     *  @param outBufs (out) array of pointers to output buffers produce by the resource
     *  @param inBufsSize (in) size of the inBufs array
     *  @param outBufsSize (in) size of the outBufs array
     *  @param isEnabled (in) indicates whether this resource has been enabled
     * 
     *  @returns TRUE if successful, FALSE otherwise.
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MpVideoResource(const MpVideoResource& rMpResource);

     /// Assignment operator (not implemented for this class)
   MpVideoResource& operator=(const MpVideoResource& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpVideoResource_h_
