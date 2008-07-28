// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MimeBodyPart_h_
#define _MimeBodyPart_h_

// SYSTEM INCLUDES
//#include <...>
#include "utl/UtlDList.h"

// APPLICATION INCLUDES
#include <net/HttpBody.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:One part of a multipart Mime body
// This is a child part of a multipart MIME body
class MimeBodyPart : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MimeBodyPart(const HttpBody* parent = NULL,
                int parentBodyStartIndex = 0,
                int rawBodyLength = 0);
     //:Default constructor

   //! Construct a MimeBodyPart from an HttpBody and a list of parameters.
   MimeBodyPart(const HttpBody& httpBody,
                //< Provides the bytes of the body.
                const UtlDList& parameters
                //< Provides the parameters.
      );
   /**< Does not attach the MimeBodyPart to a parent, or set the members
    *   showing its location in the parent.  For that you need attach().
    */

   MimeBodyPart(const MimeBodyPart& rMimeBodyPart);
     //:Copy constructor

   virtual
   ~MimeBodyPart();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   MimeBodyPart& operator=(const MimeBodyPart& rhs);
     //:Assignment operator

   /** Update the members that locate this MimeBodyPart within its parent
    *  HttpBody.
    */
   void attach(HttpBody* parent,
               int rawPartStart, int rawPartLength,
               int partStart, int partLength);

/* ============================ ACCESSORS ================================= */

   // Get the various indexes from the object.
   int getRawStart() const;
   int getRawLength() const;
   int getStart() const;
   virtual int getLength() const;

   virtual void getBytes(const char** bytes, int* length) const;

   UtlBoolean getPartHeaderValue(const char* headerName,
                                 UtlString& headerValue) const;

   UtlDList* getParameters();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlDList mNameValues;
    const HttpBody* mpParentBody;
    int mParentBodyRawStartIndex;
    int mRawBodyLength;
    int mParentBodyStartIndex;
    int mBodyLength;

};

/* ============================ INLINE METHODS ============================ */

inline int MimeBodyPart::getRawStart() const
{
   return mParentBodyRawStartIndex;
}

inline int MimeBodyPart::getRawLength() const
{
   return mRawBodyLength;
}

inline int MimeBodyPart::getStart() const
{
   return mParentBodyStartIndex;
}

inline int MimeBodyPart::getLength() const
{
   return mBodyLength;
}

inline UtlDList* MimeBodyPart::getParameters()
{
   return &mNameValues;
}

#endif  // _MimeBodyPart_h_
