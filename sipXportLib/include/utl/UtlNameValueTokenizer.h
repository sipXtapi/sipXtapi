//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlNameValueTokenizer_h_
#define _UtlNameValueTokenizer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"

// DEFINES
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'
#define CARRIAGE_RETURN_NEWLINE "\r\n"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Parses name value pairs from multiple lines of text.
class UtlNameValueTokenizer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// Constructor.
   UtlNameValueTokenizer(const char* multiLineText, int textLength = -1);

     /// Destructor.
   virtual
   ~UtlNameValueTokenizer();

/* ============================ MANIPULATORS ============================== */

     /// Finds the index to the next line terminator.
   static
   int findNextLineTerminator(const char* text, int length,
                              int* nextLineIndex);
     /**<
     *  @param[in] text - The char array in which to search for the
     *             terminator.
     *  @param[in] length - The length of the text array.
     *  @param[in] nextLineIndex - The index to the beginning of the next
     *             line.  This may be -1 if the end of the string is encountered.
     *
     *  @note The line terminator may be 1 or 2 characters.
     *
     *  @returns Index into the text char array to the line terminator.
     */

   static
   UtlBoolean getSubField(const char* textField,
                          int subfieldIndex,
                          const char* subfieldSeparator,
                          UtlString* subfieldText,
                          int* lastCharIndex = NULL);

   static
   UtlBoolean getSubField(const char* textField,
                          int textFieldLength,
                          int subfieldIndex,
                          const char* subfieldSeparators,
                          const char*& subfieldPtr,
                          int& subFieldLength,
                          int* lastCharIndex);

   UtlBoolean getNextPair(char separator, UtlString* name, UtlString* value);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    UtlBoolean isAtEnd();
    int getProcessedIndex();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    const char* textPtr;
    int textLen;
    int bytesConsumed;

      /// Disabled Copy constructor
    UtlNameValueTokenizer(const UtlNameValueTokenizer& rNameValueTokenizer);

      /// Disabled Assignment operator
    UtlNameValueTokenizer& operator=(const UtlNameValueTokenizer& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _UtlNameValueTokenizer_h_
