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
  const char * error = __FILE__ ": unknown PCRE error";
  int          erroffset;

  re = pcre_compile(regex, options, &error, &erroffset, NULL);
  if (re == NULL)
    throw error;
  pe = pcre_study(re, 0, &error);
  pcre_fullinfo(re, pe, PCRE_INFO_CAPTURECOUNT, &substrcount);
  substrcount++;
  ovector = new int[3*substrcount];
  matchlist = NULL;
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
  return pcre_exec(re, pe, lastsubject = subject, slen = (len >= 0) ? len : strlen(subject), 0, options, ovector, 3*substrcount) > 0;
}

/////////////////////////////////
bool RegEx::SearchAgain(int options)
{
  ClearMatchList();
  return pcre_exec(re, pe, lastsubject, slen, ovector[1], options, ovector, 3*substrcount) > 0;
}

/////////////////////////////////
const char * RegEx::Match(int i)
{
  if (i < 0)
    return lastsubject;
  if (matchlist == NULL)
    pcre_get_substring_list(lastsubject, ovector, substrcount, &matchlist);
  return matchlist[i];
}

void RegEx::ClearMatchList(void)
{
  if (matchlist)
    pcre_free_substring_list(matchlist),
      matchlist = NULL;
}

pcre * re;
pcre_extra * pe;
int substrcount;
int * ovector;
const char * lastsubject;
int slen;
const char * * matchlist;

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



