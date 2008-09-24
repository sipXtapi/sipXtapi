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


// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlNameValueTokenizer.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlNameValueTokenizer::UtlNameValueTokenizer(const char* multiLineText, int length)
{
   if (multiLineText && length < 0)
   {
      length = strlen(multiLineText);
   }

   textPtr = multiLineText;
   bytesConsumed = 0;
   textLen = length;
}

// Destructor
UtlNameValueTokenizer::~UtlNameValueTokenizer()
{
}

/* ============================ MANIPULATORS ============================== */

int UtlNameValueTokenizer::findNextLineTerminator(const char* text, int length, int* nextLineIndex)
{
   int byteIndex = 0;
   int terminatorIndex = -1;
   *nextLineIndex = -1;

   while (byteIndex < length)
   {
      if (text[byteIndex] == NEWLINE || text[byteIndex] == CARRIAGE_RETURN)
      {
         terminatorIndex = byteIndex;
         // Check for NL after CR
         if (byteIndex < length - 1 && text[byteIndex + 1] == NEWLINE &&
             text[byteIndex] == CARRIAGE_RETURN)
         {
            *nextLineIndex = terminatorIndex + 2;
         }
         else
         {
            *nextLineIndex = terminatorIndex + 1;
         }
         break;
      }
      byteIndex++;
   }
   return(terminatorIndex);
}

UtlBoolean UtlNameValueTokenizer::getSubField(const char* textField,
                                           int textFieldLength,
                                           int subFieldIndex,
                                           const char* subFieldSeparators,
                                           const char*& subFieldPtr,
                                           int& subFieldLength,
                                           int* lastCharIndex)
{
   UtlBoolean found = FALSE;
   if (textField)
   {
      int subFieldI = -1;
      int subFieldBegin = 0;
      int separatorIndex = -1;
      int numSeparators = strlen(subFieldSeparators);

      for (int charIndex = 0; subFieldI < subFieldIndex; charIndex++)
      {
         if ((textFieldLength >= 0 && charIndex >= textFieldLength) ||
            textField[charIndex] == '\0')
         {
            subFieldI++;
            subFieldBegin = separatorIndex + 1;
            separatorIndex = charIndex;
            break;
         }

         // If we found a separator character
         else if (// 1 separator check it directly, it is much faster
                  (numSeparators == 1 &&
                   subFieldSeparators[0] == textField[charIndex]) ||
                  // 2 separator characters, check both:
                  (numSeparators == 2 &&
                   (subFieldSeparators[0] == textField[charIndex] ||
                    subFieldSeparators[1] == textField[charIndex])) ||
                  // 3 or more separator characters, do it the slow way:
                  (numSeparators > 2 &&
                   strchr(subFieldSeparators, textField[charIndex])))
         {
            subFieldBegin = separatorIndex + 1;
            separatorIndex = charIndex;

            // Ignore empty subfields (i.e. they do not count
            if (subFieldBegin != separatorIndex)
            {
               subFieldI++;
            }
         }
      }

      if (subFieldI == subFieldIndex)
      {
         found = TRUE;
         subFieldPtr = &(textField[subFieldBegin]);
         subFieldLength = separatorIndex - subFieldBegin;

         if (lastCharIndex)
         {
            *lastCharIndex = separatorIndex;
         }
      }
   }

   if (!found)
   {
      subFieldPtr = NULL;
      subFieldLength = 0;

      if (lastCharIndex)
      {
         *lastCharIndex = 0;
      }
   }

   return found;
}

UtlBoolean UtlNameValueTokenizer::getSubField(const char* textField,
                                           int subFieldIndex,
                                           const char* subFieldSeparators,
                                           UtlString* subFieldText,
                                           int* lastCharIndex)
{
   int subFieldLength = 0;
   const char* subFieldPtr = NULL;

   UtlBoolean found = getSubField(textField,
                                  -1, // stop at null (i.e. '\0')
                                  subFieldIndex,
                                  subFieldSeparators,
                                  subFieldPtr,
                                  subFieldLength,
                                  lastCharIndex);

   if (subFieldPtr && subFieldLength > 0)
   {
      subFieldText->replace(0, subFieldLength, subFieldPtr, subFieldLength);
      subFieldText->remove(subFieldLength);
   }
   else
   {
      subFieldText->remove(0);
   }

   return found;
}

UtlBoolean UtlNameValueTokenizer::getNextPair(char separator, UtlString* name,
                                           UtlString* value)
{
   UtlBoolean nameFound = 0;
   name->remove(0);
   value->remove(0);

   int nextLineOffset;

   // Find the end of the line and the begining of the next
   int lineLength = findNextLineTerminator(&textPtr[bytesConsumed],
                                           textLen - bytesConsumed,
                                           &nextLineOffset);

   // Did not find an end of line, assume the rest of the text is the line
   if (lineLength < 0)
   {
      lineLength = textLen - bytesConsumed;
   }

   // Add a header field
   if (lineLength > 0)
   {
      // Find the name value delimiter
      int nameEnd = 0;
      while (nameEnd < lineLength &&
             textPtr[bytesConsumed + nameEnd] != separator)
      {
         nameEnd++;
      }

      if (nameEnd > 0)
      {
         name->append(&textPtr[bytesConsumed], nameEnd);
         nameFound = 1;
      }

      // Get rid of the white space
      while (nameEnd + 1 < lineLength &&
             (textPtr[bytesConsumed + nameEnd + 1] == ' ' ||
              textPtr[bytesConsumed + nameEnd + 1] == '\t'))
      {
         nameEnd++;
      }

      nameEnd++;
      if (nameEnd < lineLength)
      {
         value->append(&textPtr[bytesConsumed + nameEnd],
                       lineLength - nameEnd);
      }
   }

   if (nextLineOffset > 0)
   {
      bytesConsumed += nextLineOffset;
   }
   else
   {
      bytesConsumed += lineLength;
   }

   return nameFound;
}

/* ============================ ACCESSORS ================================= */

int UtlNameValueTokenizer::getProcessedIndex()
{
   return bytesConsumed;
}

/* ============================ INQUIRY =================================== */
UtlBoolean UtlNameValueTokenizer::isAtEnd()
{
   return bytesConsumed >= textLen;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
