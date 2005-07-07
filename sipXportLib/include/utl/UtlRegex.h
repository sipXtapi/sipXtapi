//
// Copyright (C) 2004 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
/**
 * A simple and small C++ wrapper for PCRE.
 * PCRE (or libprce) is the Perl Compatible Regular Expression library.
 * http://www.pcre.org/
 *
 * Adapted for the sipXportLib from the regex.hpp wrapper:
 *
 * regex.hpp 1.0 Copyright (c) 2003 Peter Petersen (pp@on-time.de)
 * Simple C++ wrapper for PCRE
 *
 *   This source file is freeware. You may use it for any purpose without
 *   restriction except that the copyright notice as the top of this file as
 *   well as this paragraph may not be removed or altered.
 *
 * @Author Peter Petersen
 */

#ifndef _REGEX_H
#define _REGEX_H

#include <string.h>
#include <pcre.h>
#include "utl/UtlString.h"

class RegEx
{
 public:
   /// Compile a regular expression to create the matching object.
  RegEx( const char * regex, //< the regular expression
         int options = 0     //< any sum of PCRE options bits
         );
  /**<
   * If compiling the regular expression fails, an error message string is
   * thrown as an exception.
   * For options documentation, see 'man pcre'
   */

  /// Construct from a constant regex to save compilation time. 
  RegEx( const RegEx& );
  /**<
   * If you are using the same constant regular expression frequently, you can
   * use this constructor to save the time to compile and study it.  First, declare
   * a private constant copy of your expression - this will be compiled by PCRE
   * just once when it is instantiated:
   * \code
   * static const RegEx FooNumbers("foo([0-9]+)");
   * \endcode
   * Then in your method, construct a copy of it to use when matching strings:
   * \code
   *    RegEx fooNumbers(FooNumbers);
   *    fooNumbers.Search(someString);
   * \endcode
   * Constructing this copy does not require a PCRE call to compile the expression.
   */


  ~RegEx();

  /// Count the number of possible substrings returned by this expression
  int SubStrings(void) const;
  /**
   *   SubStrings() @returns the number of substrings defined by
   *   the regular expression. The match of the entire expression is also
   *    considered a substring, so the return value will always be >= 1.
   */


  /// Search a string for matches to this regular expression
  bool Search( const char * subject,  ///< the string to be searched for a match
               int len = -1,          ///< the length of the subject string
               int options = 0        ///< sum of any PCRE options flags
               );
  /**<
   *    Method Search() applies the regular expression to parameter subject.
   *    Optional parameter len can be used to pass the subject's length to
   *    Search(). If not specified (or less than 0), strlen() is used
   *    internally to determine the length. Parameter options can contain
   *    any combination of options; for options documentation, see 'man pcre'
   *    @returns true if a match is found.
   */

  /// Repeat the last Search, starting immediately after the previous match
  bool SearchAgain( int options = 0        ///< sum of any PCRE options flags
                   );
  /**<
   *    SearchAgain() applies the regular expression to the same
   *    subject last passed to Search, but restarts the search after the last match.
   *    Subsequent calls to SearchAgain() will find all matches in the subject.
   *    @returns true if a further match is found.
   *    Example:
   *    \code
   *       RegEx Pattern("A[0-9]");
   *       const char* value = "xyzA1abcA2def";
   *       for (matched = Pattern.Search(value); matched; matched = Pattern.SearchAgain())
   *       {
   *          printf("%s\n", Pattern.Match());
   *       }
   *    \endcode
   *    Would print "A1\n" and then "A2\n".
   *
   * (Note: prefer MatchString over the less efficient Match)
   */


  /// Append a match from the last Search or SearchAgain to a UtlString.
  bool MatchString(UtlString* matched, /**< string to append the match to -
                                        * may be NULL, in which case no string is returned,
                                        * but the return code still indicates whether or not
                                        * this substring was matched.
                                        */
                   int i = 0           /**< which substring to append from the last search
                                        * - Match(-1) returns the last searched subject.
                                        * - Match(0) returns the match of the complete regular expression.
                                        * - Match(i>0) returns $i
                                        */
                   );
  /**<
   * May only be called after a successful call to Search() or SearchAgain()
   * and applies to the results of that call. 
   * @returns true if there was an ith match, false if not
   *
   * Example:@code
   * RegEx matchBs("((B)B+)");
   * UtlString getB;
   * UtlString getBs;
   * if (matchB.Search("xxaBBBBcyy"))
   * {
   *   matchB.MatchString(&getBs,0);
   *   matchB.MatchString(&getB,2);
   * }
   * @endcode
   * would set the UtlStrings
   *  - getBs to "BBBB"
   *  - getB  to "B"
   */
  
  /// Append string preceeding the most recently matched value to a UtlString.
  bool BeforeMatchString(UtlString* before /**< string to append to -
                                            * may be NULL, in which case no string is returned,
                                            * but the return code still indicates whether or not
                                            * there was some string preceeding the last match.
                                            */
                         );
  /**<
   * May only be called after a successful call to Search() or SearchAgain() and applies to
   * the results of that call. This is equivalent to the Perl $` variable.
   *
   * @returns true if there was a string before the match, false if not 
   * Example:@code
   * RegEx matchB("B");
   * UtlString getBefore;
   * if (matchB.Search("xxaBcyy"))
   * {
   *   matchB.BeforeMatchString(&getBefore);
   * }
   * @endcode
   *
   * would set the UtlString getBefore to "xxa".
   */
  
  /// Append string following the most recently matched value to a UtlString.
  bool AfterMatchString(UtlString* before /**< string to append to -
                                            * may be NULL, in which case no string is returned,
                                            * but the return code still indicates whether or not
                                            * there was some string following the last match.
                                            */
                         );
  /**<
   * May only be called after a successful call to Search() or SearchAgain() and applies to
   * the results of that call. This is equivalent to the Perl $' variable.
   *
   * @returns true if there was a string following the match, false if not 
   * Example:@code
   * RegEx matchB("B");
   * UtlString getAfter;
   * if (matchB.Search("xxaBcyy"))
   * {
   *   matchB.AfterMatchString(&getAfter);
   * }
   * @endcode
   *
   * would set the UtlString getAfter to "cyy".
   */
  

  /// Get a string matched by a previous Search or SearchAgain call.
  const char * Match(int i = 0 /**< must be < SubStrings() */ );
  /**<
   * Note - this does more memory allocation and data copying than MatchString - use MatchString when possible.
   *
   * May only be called after a successful
   * call to Search() or SearchAgain() and applies to the results of
   * that call. Parameter i must be less than
   * SubStrings().
   * - Match(-1) returns the last searched subject.
   * - Match(0) returns the match of the complete regular expression.
   * - Match(1) returns $1, etc.
   * @returns a pointer to the ith matched substring.
   */

 private:
  /*
   * Use the copy constructor above instead of the = operator.
   */
  RegEx& operator=(const char *);

  void ClearMatchList(void);

  pcre * re;
  size_t re_size;
  pcre_extra * pe;
  size_t pe_size;
  int substrcount;         // maximum substrings in pattern
  const char * subjectStr; // original subject
  int subjectLen;          // original length
  int lastStart;           // offset of start for most recent Search or SearchAgain
  int lastMatches;         // pcre_exec return for most recent Search or SearchAgain
  int * ovector;           // results from (and workspace for) pcre_exec
  const char * * matchlist;// string cache for Match
};

#endif // _REGEX_H

