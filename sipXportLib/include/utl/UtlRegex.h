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
//   This source file is freeware. You may use it for any purpose without
//   restriction except that the copyright notice as the top of this file as
//   well as this paragraph may not be removed or altered.
//
/// @Author Peter Petersen

#ifndef _REGEX_H
#define _REGEX_H

#include <string.h>
#include <pcre.h>

/**<
 * @Class RegEx
 * A simple and small C++ wrapper for PCRE.
 * PCRE (or libprce) is the Perl Compatible Regular Expression library.
 * http://www.pcre.org/
 */

class RegEx
{
 public:
  /**
   * The constructor compiles the regular expression.
   * If compiling the regular expression fails, an error message string is
   * thrown as an exception.
   * For options documentation, see 'man pcre'
   */
  RegEx( const char * regex, //< the regular expression
         int options = 0     //< any sum of PCRE options bits
         );

  ~RegEx();

  /**
   *   SubStrings() @returns the number of substrings defined by
   *   the regular expression. The match of the entire expression is also
   *    considered a substring, so the return value will always be >= 1.
   */
  int SubStrings(void) const;

  /**
   *    Method Search() applies the regular expression to parameter subject.
   *    Optional parameter len can be used to pass the subject's length to
   *    Search(). If not specified (or less than 0), strlen() is used
   *    internally to determine the length. Parameter options can contain
   *    any combination of options; for options documentation, see 'man pcre'
   *    @returns true if a match is found.
   */

  bool Search( const char * subject,  /// the string to be searched for a match
               int len = -1,          /// the length of the subject string
               int options = 0        /// sum of any PCRE options flags
               );

  /**
   *    SearchAgain() again applies the regular expression to parameter
   *    subject last passed to a successful call of Search().
   *    Subsequent calls to SearchAgain() will find all matches in subject.
   *    Example:
   *
   *       if (Pattern.Search(astring)) {
   *          do {
   *             printf("%s\n", Pattern.Match());
   *          } while (Pattern.SearchAgain());
   *       }
   *
   *    Parameter options is interpreted as for method Search().
   *    @returns true if a further match is found.
   */
  bool SearchAgain( int options = 0        /// sum of any PCRE options flags
                   );


  /**
   *    Match() may only be called after a successful
   *    call to Search() or SearchAgain() and applies to that last
   *    Search()/SearchAgain() call. Parameter i must be less than
   *    SubStrings(). Match(-1) returns the last searched subject.
   *    Match(0) returns the match of the complete regular expression.
   *    Match(1) returns $1, etc.
   *    @returns a pointer to the ith matched substring.
   */
  const char * Match(int i = 1);

 private:
  /*
   * Prevent operator= from being implied by the constructor
   * until we have figured out the implications for memory inside
   * PCRE
   */
  RegEx& operator=(const char *);

  /**
   * :TODO: http://track.sipfoundry.org/browse/XPL-3
   * Construct from another existing regex to save compilation time.
   */
  // RegEx( const RegEx& );

  void ClearMatchList(void);

  pcre * re;
  pcre_extra * pe;
  int substrcount;
  int * ovector;
  const char * lastsubject;
  int slen;
  const char * * matchlist;
};

#endif // _REGEX_H

