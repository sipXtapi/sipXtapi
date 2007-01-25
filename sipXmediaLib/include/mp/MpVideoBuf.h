//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPVIDEOBUF_H /* [ */
#define _INCLUDED_MPVIDEOBUF_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpDataBuf.h"
#include "mp/MpTypes.h"

// DEFINES
#ifdef SIPX_VIDEO // [
/// This enable color space conversations using FFMpeg's conversion routines.
#define DO_COLORSPACE_CONVERSION
#endif // SIPX_VIDEO ]

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
*  @todo Support line alignment in MpVideoBuf.
*/

///  Buffer for not compressed video frame.
/**
*  This is only the header for video frame. It contain some video frame related
*  parameters and pointer to external data (cause it is based on MpDataBuf).
*/
struct MpVideoBuf : public MpDataBuf
{
    friend class MpVideoBufPtr;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    typedef enum {
       MP_COLORSPACE_PACKED_BEGIN,  // interleaved, pixel-oriented
       MP_COLORSPACE_RGB24,
       MP_COLORSPACE_RGB32,
       MP_COLORSPACE_RGB555,
       MP_COLORSPACE_RGB565,

       MP_COLORSPACE_BGR24,
       MP_COLORSPACE_BGR32,
       MP_COLORSPACE_BGR555,
       MP_COLORSPACE_BGR565,

//       MP_COLORSPACE_YUV,
//       MP_COLORSPACE_YUV420,
//       MP_COLORSPACE_YUV411,
//       MP_COLORSPACE_YUV422,  // YUYV, YUY2

//       MP_COLORSPACE_GRAY,

       MP_COLORSPACE_PLANAR_BEGIN, // plane-oriented

//       MP_COLORSPACE_RGB24p,
       // MP_COLORSPACE_RGB32p,

//       MP_COLORSPACE_BGR24p,
       // MP_COLORSPACE_BGR32p,

//       MP_COLORSPACE_YUVp,
       MP_COLORSPACE_YUV420p,
//       MP_COLORSPACE_YUV411p,
//       MP_COLORSPACE_YUV422p,

       MP_COLORSPACE_END
    } ColorSpace;

    enum {
       MP_MAX_COLOR_PLANES = 3 ///< Maximum number of color planes in one video frame.
    };

    struct PlaneParameters {
       char     *mpBeginPointer; ///< Pointer to begin of plane data.
       unsigned  mpStep;         ///< Distance between beginnings of consecutive
                                 ///< lines in the source image (in bytes).
    };

    static MpBufPool *smpDefaultPool; ///< Default pool for this type of buffer

/* ============================ CREATORS ================================== */
///@name Creators
//@{


//@}

/* ============================ UTLILITES ================================== */

    /// Get pixel depth for given colorspace (in bits/pixel).
    static
    unsigned getColospacePixelDepth(ColorSpace colorspace);

    /// Get number of color planes for given colorspace.
    static
    unsigned getColospaceNumPlanes(ColorSpace colorspace);

    /// Get U and V planes width divider for given colorspace.
    static
    unsigned getColospaceUVWidthDivider(ColorSpace colorspace);

    /// Get U and V planes height divider for given colorspace.
    static
    unsigned getColospaceUVHeightDivider(ColorSpace colorspace);

    /// Is this colorspace in planar format?
    /**
    *  @returns <b>true</b> - if colorspace is in planar format.
    *           <b>false</b> - if colorspace is in interleaved format.
    */
    static
    bool isPlanar(ColorSpace colorspace)
    {return colorspace>=MP_COLORSPACE_PLANAR_BEGIN;}

    /// Get multiplier for line step for given colorspace (in bytes/pixel).
    /**
    *  It may then be used to calculate line step: <br>
    *  <i>step = StepMultiplier * width + alignment</i>
    *  
    *  @note This function does not care about line alignment!
    */
    static
    unsigned getColorspaceStepMultiplier(ColorSpace colorspace)
    {
       if (isPlanar(colorspace))
          // For planar colorspaces we should use plane depth.
          // It is always 8 bits/pixel now.
          return 8/8;
       else {
          // For packed colorspaces we may simply return pixel depth.
          return getColospacePixelDepth(colorspace)/8;
       }
    }

#ifdef DO_COLORSPACE_CONVERSION // [

     /// Get FFMpeg colorspace for given MpVideoBuf colorspace.
   static
   int getFFMpegColorspace(MpVideoBuf::ColorSpace colorspace);

     /// Convert this frame to other colorspace
   OsStatus convertToColorSpace(MpVideoBuf::ColorSpace colorspace, char *pBuffer, int bufSize);
   /**<
   *  convertToColorSpace convert this frame to provided buffer. Width and height
   *  are taken from this frame.
   *  
   *  @param colorspace - (in) Colorspace to convert to.
   *  @param pBuffer - (in) Buffer for converted data.
   *  @param bufSize - (in) Size of buffer pointed by pBuffer.
   */

     /// Convert from other colorspace to this frame
   OsStatus convertFromColorSpace(MpVideoBuf::ColorSpace colorspace, char *pBuffer, int bufSize);
   /**<
   *  convertFromColorSpace convert from provided buffer to this frame.
   *  
   *  @param colorspace - (in) Colorspace to convert from.
   *  @param pBuffer - (in) Buffer with data in "colorspace" format.
   *  @param bufSize - (in) Size of buffer pointed by pBuffer.
   */

#endif // DO_COLORSPACE_CONVERSION ]

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Set time code for this frame
    void setTimecode(unsigned timecode) {mTimecode=timecode;}

    /// Set colorspace of this frame.
    void setColorspace(ColorSpace colorspace);
    /**<
    *  This function set colorspace and adjusts number of planes, pixel depth.
    *  If planar colorspace is used number of planes will be set to 1.
    *
    *  @NOTE Frame width and height MUST be set before calling this function.
    */

    /// Set width of the frame (in pixels)
    void setFrameWidth(unsigned width)
    {
       mWidth = width;
    }

    /// Set height of the frame (in pixels)
    void setFrameHeight(unsigned height)
    {
       mHeight = height;
    }

    /// Set width and hight of the frame (in pixels)
    void setFrameSize(unsigned width, unsigned height)
    {
       setFrameWidth(width);
       setFrameHeight(height);
    }

    /// Resets line steps and pointers for planes to original values.
    /**
    *  May be useful to undo crop operations.
    *
    *  @NOTE Frame width and height MUST be set before calling this function.
    */
    void resetPlaneParameters();

    /// Move beginning of the frame.
    /**
    *  Beginning of the frame will be moved x pixels left and y pixels down.
    *  Width and height will remain unchanged.
    *
    *  @warning You should manually adjust frame width and height!
    */
    void indentFrame(int x, int y)
    {
       // First plane is always full width.
       mpPlaneParameters[0].mpBeginPointer
          += mpPlaneParameters[0].mpStep * y
           + getColorspaceStepMultiplier(mColorspace) * x;

       // Next planes may have less width.
       for (unsigned i=1; i<mNumPlanes; i++) {
          mpPlaneParameters[i].mpBeginPointer
             += mpPlaneParameters[i].mpStep * y
              + getColorspaceStepMultiplier(mColorspace) * x / mUVWidthDivider;
       }
    }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Get time code for this frame
    unsigned getTimecode() const {return mTimecode;}

    /// Get used colorspace
    ColorSpace getColorspace() const {return mColorspace;}

    /// Get number of color planes. For interleaved frames returns 1.
    unsigned getNumPlanes() const {return mNumPlanes;}

    /// Get width of the frame (in pixels)
    unsigned getFrameWidth() const {return mWidth;}

    /// Get height of the frame (in pixels)
    unsigned getFrameHeight() const {return mHeight;}

    /// Get divider of width of U and V planes.
    unsigned getUVWidthDivider() const {return mUVWidthDivider;}

    /// Get divider of height of U and V planes.
    unsigned getUVHeightDivider() const {return mUVHeightDivider;}

    /// @brief Get width of the given plane (in pixels). This function takes 
    /// care of U and V planes width divider.
    unsigned getPlaneWidth(unsigned plane) const
    {
       assert(plane<mNumPlanes);
       return (plane==0) ? mWidth : (mWidth/getUVWidthDivider());
    }

    /// @brief Get height of the given plane (in pixels). This function takes 
    /// care of U and V planes height divider.
    unsigned getPlaneHeight(unsigned plane) const
    {
       assert(plane<mNumPlanes);
       return (plane==0) ? mHeight : (mHeight/getUVHeightDivider());
    }

    /// Get line step for the given plane.
    unsigned getPlaneStep(unsigned plane) const
    {
       assert(plane<mNumPlanes);
       return mpPlaneParameters[plane].mpStep;
    }

    /// Get pointer to begin of the given plane.
    const void *getPlanePointer(unsigned plane) const
    {
       assert(plane<mNumPlanes);
       return mpPlaneParameters[plane].mpBeginPointer;
    }

    /// Get pointer to begin of the given plane.
    void *getWritePlanePointer(unsigned plane)
    {
       assert(plane<mNumPlanes);
       return mpPlaneParameters[plane].mpBeginPointer;
    }

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

    /// Does frame use planar format?
    /**
    *  @returns <b>true</b> - if planar format is used.
    *           <b>false</b> - if interleaved format is used.
    */
    bool isPlanar() const {return isPlanar(mColorspace);}

    /// Does frame use interleaved format?
    /**
    *  @returns <b>true</b> - if interleaved format is used.
    *           <b>false</b> - if planar format is used.
    */
    bool isInterleaved() const {return mColorspace<MP_COLORSPACE_PLANAR_BEGIN;}

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    unsigned   mTimecode;    ///< Time when this data is generated.
    ColorSpace mColorspace;  ///< Colorspace of this frame.
    unsigned   mNumPlanes;   ///< Number of color planes. If frame is in packed
                             ///<  color format then mNumPlanes=1.
    unsigned   mPixelDepth;  ///< Bits per pixel.
                             ///<  Calculated as (frame bytes*8/frame pixels).
    unsigned   mUVWidthDivider;  ///< Divider of width of U and V planes. For
                                 ///< RGB color mode equal to 1.
    unsigned   mUVHeightDivider; ///< Divider of height of U and V planes. For
                                 ///< RGB color mode equal to 1.
    unsigned   mWidth;       ///< Width of the buffer (in pixels).
    unsigned   mHeight;      ///< Height of the buffer (in pixels).
    PlaneParameters mpPlaneParameters[MP_MAX_COLOR_PLANES];
                             ///< Pointer to the begining of the color planes
                             ///<  and line step for each color plane.

    /// This is called in place of constructor.
    void init();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable copy (and other) constructor.
    MpVideoBuf(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpVideoBuf &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

///  Smart pointer to MpVideoBuf.
/**
*  You should only use this smart pointer, not #MpVideoBuf* itself.
*  The goal of this smart pointer is to care about reference counter and
*  buffer deallocation.
*/
class MpVideoBufPtr : public MpDataBufPtr {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor - construct invalid pointer.
    MPBUF_DEFAULT_CONSTRUCTOR(MpVideoBuf)

    /// This constructor owns MpBuf object.
    MPBUFDATA_FROM_BASE_CONSTRUCTOR(MpVideoBuf, MP_BUF_VIDEO, MpDataBuf)

    /// Copy object from base type with type check.
    MPBUF_TYPECHECKED_COPY(MpVideoBuf, MP_BUF_VIDEO, MpDataBuf)

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return pointer to MpVideoBuf.
    MPBUF_MEMBER_ACCESS_OPERATOR(MpVideoBuf)

    /// Return read only pointer to MpVideoBuf.
    MPBUF_CONST_MEMBER_ACCESS_OPERATOR(MpVideoBuf)

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif /* ] _INCLUDED_MPVIDEOBUF_H */
