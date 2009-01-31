//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpPackedResourceMsg_h_
#define _MpPackedResourceMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#define UTL_SERIALIZED_SIZE   1024
#define UTL_SERIALIZED_ALIGN  4

class UtlSerialized
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   inline UtlSerialized()
   : mpEnd(mData)
   , mSize(0)
   {
   };

     /// Copy constructor
   inline UtlSerialized(const UtlSerialized& rhs)
   : mpEnd(mData+(rhs.mpEnd-rhs.mData))
   , mSize(rhs.mSize)
   {
      memcpy(mData, rhs.mData, rhs.getSize());
   };

     /// Destructor
   virtual ~UtlSerialized() 
   {
   };

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

#define SERIALIZE_POD_DEFINE(type)        \
   inline OsStatus serialize(type val)    \
   {                                      \
      if (sizeof(type) > getFreeSize())   \
      {                                   \
         return OS_LIMIT_REACHED;         \
      }                                   \
      *(type*)mpEnd = val;                \
      mpEnd += sizeof(type);              \
      mSize += sizeof(type);              \
      return OS_SUCCESS;                  \
   }                                      \
   inline OsStatus deserialize(type &val) \
   {                                      \
      if (sizeof(type) > getSize() - (mpEnd-mData))     \
      {                                   \
         return OS_NO_MORE_DATA;          \
      }                                   \
      val = *(type*)mpEnd;                \
      mpEnd += sizeof(type);              \
      return OS_SUCCESS;                  \
   }

   SERIALIZE_POD_DEFINE(char);
   SERIALIZE_POD_DEFINE(short);
   SERIALIZE_POD_DEFINE(int);
   SERIALIZE_POD_DEFINE(long);
   SERIALIZE_POD_DEFINE(unsigned char);
   SERIALIZE_POD_DEFINE(unsigned short);
   SERIALIZE_POD_DEFINE(unsigned int);
   SERIALIZE_POD_DEFINE(unsigned long);

   SERIALIZE_POD_DEFINE(void *);
#undef SERIALIZE_POD_DEFINE

   inline OsStatus serialize(const UtlString &val)
   {
      size_t length = val.length();
      if (sizeof(int) + length > getFreeSize())
      {
         return OS_LIMIT_REACHED;
      }
      serialize(length);
      memcpy(mpEnd, val.data(), length);
      mpEnd += length;
      mSize += length;
      return OS_SUCCESS;
   }

   inline OsStatus deserialize(UtlString &val)
   {
      OsStatus result;
      size_t length;
      result = deserialize(length);
      if (result != OS_SUCCESS)
      {
         return result;
      }
      if (length > getSize() - (mpEnd-mData))
      {
         // Something is wrong - length of the string is bigger then available data.
         return OS_FAILED;
      }
      val.resize(length, FALSE);
      memcpy((void*)val.data(), mpEnd, length);
      mpEnd += length;
      return OS_SUCCESS;
   }

     /// Prepare for de-serialization.
   inline void finishSerialize()
   {
      mpEnd = mData;
   }

     /// Assignment operator
   inline UtlSerialized& operator=(const UtlSerialized& rhs)
   {
      if(&rhs == this)
      {
         return(*this);
      }

      memcpy(mData, rhs.mData, rhs.getSize());
      mpEnd = mData+(rhs.mpEnd-rhs.mData);
      mSize = rhs.mSize;

      return *this;
   }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

      /// Get size of available space (in bytes).
   inline size_t getMaxSize() const {return UTL_SERIALIZED_SIZE;}

     /// Get size of stored data (in bytes).
   inline size_t getSize() const {return mSize;}

     /// Get free size in storage (in bytes).
   inline size_t getFreeSize() const {return getMaxSize()-getSize();}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   uint8_t *mpEnd;           ///< Pointer to the byte right after the end of data.
   uint8_t  mData[UTL_SERIALIZED_SIZE]; ///< Array to serialize data to.
   size_t   mSize;           ///< Size of the serialized data.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/// Message used pass arbitrary data to a resource.
class MpPackedResourceMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpPackedResourceMsg(MpResourceMsgType msgType,
                       const UtlString& targetResourceName)
   : MpResourceMsg(msgType, targetResourceName)
   {
   };

     /// Copy constructor
   MpPackedResourceMsg(const MpPackedResourceMsg& resourceMsg)
   : MpResourceMsg(resourceMsg)
   , mData(resourceMsg.mData)
   {
   };

     /// @copydoc OsMsg::createCopy()
   OsMsg* createCopy() const 
   {
      return new MpPackedResourceMsg(*this); 
   }

     /// Destructor
   ~MpPackedResourceMsg() 
   {
   };

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpPackedResourceMsg& operator=(const MpPackedResourceMsg& rhs)
   {
      if(&rhs == this)
      {
         return(*this);
      }

      MpResourceMsg::operator=(rhs);
      mData = rhs.mData;
      return *this;
   }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get stored data.
   inline UtlSerialized &getData();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlSerialized    mData;     ///< Input or output port (row or column number)
};

/* ============================ INLINE METHODS ============================ */

UtlSerialized &MpPackedResourceMsg::getData()
{
   return mData;
}
#endif  // _MpPackedResourceMsg_h_
