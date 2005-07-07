//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Derived from:
// regex.hpp 1.0 Copyright (c) 2003 Peter Petersen (pp@on-time.de)
// Simple C++ wrapper for PCRE
//
// This source file is freeware. You may use it for any purpose without
// restriction except that the copyright notice as the top of this file as
// well as this paragraph may not be removed or altered.
//
//
// The bottom of this file contains an example using class RegEx. It's
// the simplest version of grep I could come with. You can compile it by
// defining REGEX_DEMO on the compiler command line.
//

#include "utl/UtlRegex.h"


/////////////////////////////////
RegEx::RegEx(const char * regex, int options)
{
   const char * error = __FILE__ ": unknown PCRE error in RegEx(char*)";
   int          erroffset;

   // compile and study the expression
   re = pcre_compile(regex, options, &error, &erroffset, NULL);
   if (re == NULL)
   {
      throw error;
   }
   pe = pcre_study(re, 0, &error);

   // save the compilation block sizes for the copy constructor.
   pcre_fullinfo(re, pe, PCRE_INFO_SIZE, &re_size);
   pcre_fullinfo(re, pe, PCRE_INFO_STUDYSIZE, &pe_size);

   // allocate space for match results based on how many substrings
   // there are in the expression (+1 for the entire match)
   pcre_fullinfo(re, pe, PCRE_INFO_CAPTURECOUNT, &substrcount);
   substrcount++;
   ovector = new int[3*substrcount];
   matchlist = NULL;
};

/////////////////////////////////
RegEx::RegEx(const RegEx& regex)
{
   const char * error = __FILE__ ": unknown PCRE error in RegEx(RegEx)";

   // allocate memory for the compiled regular expression information
   re = (pcre*)pcre_malloc(regex.re_size); 
   if (re)
   {
      // copy the compiled regular expression information
      memcpy(re, regex.re, regex.re_size);
      re_size = regex.re_size;
         
      if (   (NULL != regex.pe) // did pcre_study return anything?
          && (0 < regex.pe_size)
          )
      {
         // allocate memory for the extra information
         pe = (pcre_extra*)pcre_malloc(sizeof(pcre_extra));
         if (pe)
         {
            // clear the interface part of the extra structure
            memset(pe, 0, sizeof(pcre_extra)) ;
            // allocate memory as needed for study data
            pe->study_data = pcre_malloc(regex.pe_size);
            if (pe->study_data)
            {
               // copy the study data
               pe_size = regex.pe_size;
               memcpy(pe->study_data, regex.pe->study_data, pe_size);
               pe->flags = PCRE_EXTRA_STUDY_DATA;
            }
            else
            {               
               pcre_free(pe);
               pe = NULL;
               pe_size = 0;
            }
         }
         else
         {
            pe_size = 0;
         }
      }
      else
      {
         pe = NULL;
         pe_size = 0;
      }
      substrcount = regex.substrcount;
      ovector = new int[3*substrcount];
      matchlist = NULL;
   }
   else
   {
      throw error;
   }
};

/////////////////////////////////
RegEx::~RegEx()
{
  ClearMatchList();
  if (ovector != NULL)
  {
     delete [] ovector;
  }
  if (pe)
  {
    pcre_free(pe);
  }
  pcre_free(re);
}

/////////////////////////////////
int RegEx::SubStrings(void) const
{
  return substrcount;
}

/////////////////////////////////
bool RegEx::Search(const char * subject, int len, int options)
{
  ClearMatchList();
  subjectStr = subject;
  lastStart   = 0;
  subjectLen = (len >= 0) ? len : strlen(subject);
  lastMatches = pcre_exec(re, pe, subjectStr, subjectLen, 0, options, ovector, 3*substrcount);
  return lastMatches > 0;
}

/////////////////////////////////
bool RegEx::SearchAgain(int options)
{
  ClearMatchList();
  bool matched;
  lastStart = ovector[1];
  if (lastStart < subjectLen)
  {
     lastMatches = pcre_exec(re, pe, subjectStr, subjectLen, lastStart, options, ovector, 3*substrcount);
     matched = lastMatches > 0;
  }
  else
  {
     // The last search matched the entire subject string
     // If the pattern allows a nuss string to match, then another call to pcre_exec
     // would return that match, so don't do that.
     // Instead, return no match to prevent an infinite loop.
     matched = false; 
  }
  return matched;
}

////////////////////////////////////////////////////////////////
bool RegEx::BeforeMatchString(UtlString* before)
{
   bool hadBefore = false; // assume no match
   
   if (lastMatches) // any matches in the last search?
   {
      int startOffset = ovector[0]; // start of all of most recent match
      if (lastStart < startOffset) // anything before the last match?
      {
         int length = startOffset - lastStart;
         if (NULL!=before)
         {
            before->append(subjectStr+lastStart, length);
         }
         hadBefore = true;
      }
   }
   return hadBefore;      
}

bool RegEx::AfterMatchString(UtlString* after)
{
   bool hadAfter = false; // assume no match
   
   if (lastMatches) // any matches in the last search?
   {
      int endOffset = ovector[1]; // end of all of most recent match
      if (endOffset < subjectLen) // anything after the last match?
      {
         int length = subjectLen - endOffset;
         if (NULL!=after)
         {
            after->append(subjectStr+endOffset, length);
         }
         hadAfter = true;
      }
   }
   return hadAfter;      
}

bool RegEx::MatchString(UtlString* matched, int i)
{
   bool hadMatch = false; // assume no match
   
   if (i < lastMatches) // enough matches in the last search?
   {
      if (-1 == i) // return entire subject string
      {
         if (NULL!=matched)
         {
            matched->append(subjectStr, subjectLen);
         }
         hadMatch = true;
      }
      else
      {
         int startOffset = ovector[i*2];
         if (0 <= startOffset) // did ith string match?
         {
            int length = ovector[(i*2)+1] - startOffset;
            if (0<length)
            {
               if (NULL!=matched)
               {
                  matched->append(subjectStr+startOffset, length);
               }
            }
            else
            {
               // the matched substring was a null string - can happen
            }
            hadMatch = true;
         }
      }
   }
   return hadMatch;      
}

/////////////////////////////////
const char * RegEx::Match(int i)
{
   // use of this routine is discouraged for efficiency reasons - use MatchString instead
   if (i >= 0)
   {
      if (matchlist == NULL)
      {
         pcre_get_substring_list(subjectStr, ovector, substrcount, &matchlist);
      }
      return matchlist[i];
   }
   else
   {
      return subjectStr;
   }
}

// PRIVATE METHODS

void RegEx::ClearMatchList(void)
{
   if (matchlist)
   {
      pcre_free_substring_list(matchlist);
      matchlist = NULL;
   }
}

// Below is a little demo/test program using class RegEx

#ifdef REGEX_DEMO

#include <stdio.h>
#include "regex.hpp"

///////////////////////////////////////
int main(int argc, char * argv[])
{
  if (argc != 2)
    {
      fprintf(stderr, "Usage: grep pattern\n\n"
              "Reads stdin, searches 'pattern', writes to stdout\n");
      return 2;
    }
  try
    {
      RegEx Pattern(argv[1]);
      int count = 0;
      char buffer[1024];

      while (fgets(buffer, sizeof(buffer), stdin))
        if (Pattern.Search(buffer))
          fputs(buffer, stdout),
            count++;
      return count == 0;
    }
  catch (const char * ErrorMsg)
    {
      fprintf(stderr, "error in regex '%s': %s\n", argv[1], ErrorMsg);
      return 2;
    }
}

#endif // REGEX_DEMO



