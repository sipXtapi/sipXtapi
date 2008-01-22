//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _PlgStaff_h_
#define _PlgStaff_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlVoidPtr.h>
#include <utl/UtlString.h>
#include <mp/codecs/PlgDefsV1.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Very simple single-linked list implementation, used by codecs' loading
*         infrastructure.
*
*  We do not use Utl* lists to avoid excessive locking and complexity they
*  introduce.
*/
class MpStaticCodecSimpleList
{
public:
/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Default Constructor
   inline
   MpStaticCodecSimpleList();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Add this element head of the list and return pointer to it.
   inline
   MpStaticCodecSimpleList* bound(MpStaticCodecSimpleList* newItem);

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

     /// Return pointer to next element.
   inline
   MpStaticCodecSimpleList* getNext() const;

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   MpStaticCodecSimpleList* mNext; ///< Pointer to next element in list.
};

/**
*  @brief Object-oriented wrapper for codec.
*
*  This class is used to provide object-oriented access to codecs, which are
*  represented as a set of plain functions originally.
*/
class MpCodecCallInfoV1 : protected UtlVoidPtr, public MpStaticCodecSimpleList
{
public:
/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor.
   inline
   MpCodecCallInfoV1(const char* moduleName,
                     const char* codecModuleName,
                     dlPlgInitV1_1 plgInit,
                     dlPlgGetInfoV1_1 plgGetInfo,
                     dlPlgDecodeV1 plgDecode,
                     dlPlgEncodeV1 plgEncode,
                     dlPlgFreeV1 plgFree,
                     dlPlgGetSignalingDataV1 plgSignaling,
                     UtlBoolean bStatic = TRUE);

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

     /// Return pointer to next codec in the list.
   inline
   MpCodecCallInfoV1* getNext() const;

     /// Return module name.
   inline
   const UtlString& getModuleName() const;

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{

     /// Is codec registered as static or dynamic?
   inline
   const UtlBoolean isStatic() const;

//@}

/* =============================== WRAPPERS =============================== */
///@name Wrappers
/// Pointers to actual functions, defined for this codec.
//@{

   const dlPlgInitV1_1 mPlgInit;
   const dlPlgGetInfoV1_1 mPlgGetInfo;
   const dlPlgDecodeV1 mPlgDecode;
   const dlPlgEncodeV1 mPlgEncode;
   const dlPlgFreeV1 mPlgFree;
   const dlPlgGetSignalingDataV1 mPlgSignaling;

//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:
   UtlBoolean mbStatic;   ///< Is codec compiled-in or dynamically loaded?
   UtlString mModuleName; ///< Dynamic module name. Empty string for compiled-in codecs.

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

/* ======================== MpStaticCodecSimpleList ======================= */

MpStaticCodecSimpleList::MpStaticCodecSimpleList()
: mNext(NULL)
{}

MpStaticCodecSimpleList* MpStaticCodecSimpleList::bound(MpStaticCodecSimpleList* newItem)
{
   mNext = newItem;
   return this;
}

MpStaticCodecSimpleList* MpStaticCodecSimpleList::getNext() const
{
   return mNext;
}

/* =========================== MpCodecCallInfoV1 ========================== */

MpCodecCallInfoV1::MpCodecCallInfoV1(const char* moduleName,
                                     const char* codecModuleName,
                                     dlPlgInitV1_1 plgInit,
                                     dlPlgGetInfoV1_1 plgGetInfo,
                                     dlPlgDecodeV1 plgDecode,
                                     dlPlgEncodeV1 plgEncode,
                                     dlPlgFreeV1 plgFree,
                                     dlPlgGetSignalingDataV1 plgSignaling,
                                     UtlBoolean bStatic)
: mbStatic(bStatic)
, mModuleName(moduleName)
, mPlgInit(plgInit)
, mPlgGetInfo(plgGetInfo)
, mPlgDecode(plgDecode)
, mPlgEncode(plgEncode)
, mPlgFree(plgFree)
, mPlgSignaling(plgSignaling)
{}

const UtlBoolean MpCodecCallInfoV1::isStatic() const
{
   return mbStatic;
}

MpCodecCallInfoV1* MpCodecCallInfoV1::getNext() const
{
   return (MpCodecCallInfoV1*)MpStaticCodecSimpleList::getNext();
}

const UtlString& MpCodecCallInfoV1::getModuleName() const
{
   return mModuleName;
}

#endif //_PlgStaff_h_
