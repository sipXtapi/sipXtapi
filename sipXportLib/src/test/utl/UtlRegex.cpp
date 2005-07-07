// 
//
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

/**
 * Unit test for RegEx
 *
 * The test points here are translations of the first set of test cases
 * in the PCRE unit tests.  Unfortunately, the templates in cppunit seem
 * to be causing a severe compilation time problem - on my system, even
 * with most cases commented out this module requires 3 minutes to compile,
 * and it seems to get worse in a non-linear way as cases are added.  So,
 * most of them are commented out.  Any suggestions on how to correct this
 * are most welcome.  - Scott Lawrence
 *
 */
#include "utl/UtlRegex.h"

class UtlRegExTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE(UtlRegExTest);
        CPPUNIT_TEST(testInput1);
        CPPUNIT_TEST(testCopy1);
        CPPUNIT_TEST(testInput2);
        CPPUNIT_TEST(testInput3);
        CPPUNIT_TEST(testInput4);
        CPPUNIT_TEST(testInput5);
        CPPUNIT_TEST(testInput6);
        CPPUNIT_TEST(testInput7);
        CPPUNIT_TEST(testInput8);
        CPPUNIT_TEST(testInput9);
        CPPUNIT_TEST(testInput10);
        CPPUNIT_TEST(testInput11);
        CPPUNIT_TEST(testInput12);
        CPPUNIT_TEST(testInput13);
        CPPUNIT_TEST(testInput14);
  CPPUNIT_TEST_SUITE_END();

# define EXPRESSION( expression, options ) \
           { if ( TheRegEx ) { delete TheRegEx; } \
             TheRegEx = new RegEx( expression, options ); \
             CPPUNIT_ASSERT_MESSAGE( "compiling expression /" expression "/", TheRegEx ); \
           }

# define SHOULD_MATCH( matches, in ) \
  CPPUNIT_ASSERT_MESSAGE( "'" in "' should have matched" #matches, \
                          TheRegEx->Search( in ) && ( TheRegEx->SubStrings() == matches ) \
                         );

# define MATCH( n, matchVal ) \
  CPPUNIT_ASSERT_MESSAGE( "'" matchVal "' should have been a match", \
                        ( strcmp( TheRegEx->Match(n), matchVal ) == 0 ) \
                        );

# define SHOULD_NOT_MATCH( in ) \
  CPPUNIT_ASSERT_MESSAGE( "'" in "' should not have matched", \
                           ! TheRegEx->Search( in )   \
                         );

public:

  void testInput1()
  {
    RegEx* TheRegEx = 0;

EXPRESSION( "the quick brown fox", 0 );
SHOULD_MATCH( 1, "the quick brown fox" );
MATCH( 0, "the quick brown fox" );
SHOULD_NOT_MATCH("The quick brown FOX");
SHOULD_MATCH( 1, "What do you know about the quick brown fox?" );
MATCH( 0, "the quick brown fox" );
SHOULD_NOT_MATCH("What do you know about THE QUICK BROWN FOX?");

delete TheRegEx;

  }

   void testCopy1()
      {
         static const RegEx ConstRegEx("^[a-z]+([0-9]+)$");
         RegEx* TheRegEx = new RegEx(ConstRegEx);

         SHOULD_MATCH(2,"foo35");
         MATCH(0,"foo35");
         MATCH(1,"35");

         SHOULD_NOT_MATCH( "abc" );
         SHOULD_NOT_MATCH( "Foo35" );
         SHOULD_NOT_MATCH( "12Foo35" );
         SHOULD_NOT_MATCH( "foo35 " );

         delete TheRegEx;
      }
   
void testInput2()
    {
        RegEx* TheRegEx = 0;

EXPRESSION( "The quick brown fox", PCRE_CASELESS );
SHOULD_MATCH( 1, "the quick brown fox" );
MATCH( 0, "the quick brown fox" );
SHOULD_MATCH( 1, "The quick brown FOX" );
MATCH( 0, "The quick brown FOX" );
SHOULD_MATCH( 1, "What do you know about the quick brown fox?" );
MATCH( 0, "the quick brown fox" );
SHOULD_MATCH( 1, "What do you know about THE QUICK BROWN FOX?" );
MATCH( 0, "THE QUICK BROWN FOX" );


// EXPRESSION( "abcd\t\n\r\f\a\e\\071\x3b\\$\\\?caxyz", 0 );
// SHOULD_MATCH( 1, "abcd\t\n\r\f\a\e9;\\$\\?caxyz" );
//:TBD: hex escape sequence out of range
// MATCH( 0, "abcd\x09\n\x0d\x0c\x07\x1b9;$\?caxyz" );


delete TheRegEx;

  }

void testInput3()
    {
        RegEx* TheRegEx = 0;


EXPRESSION( "a*abc?xyz+pqr{3}ab{2,}xy{4,5}pq{0,6}AB{0,}zz", 0 );
SHOULD_MATCH( 1, "abxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "abxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "abxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "abxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aabxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aabxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaabxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaabxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaaabxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaaabxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "abcxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "abcxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aabcxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aabcxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypAzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypAzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypqqAzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypqqAzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypqqqAzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypqqqAzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypqqqqAzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypqqqqAzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypqqqqqAzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypqqqqqAzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypqqqqqqAzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypqqqqqqAzz" );
SHOULD_MATCH( 1, "aaaabcxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaaabcxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "abxyzzpqrrrabbxyyyypqAzz" );
MATCH( 0, "abxyzzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aabxyzzzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aabxyzzzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaabxyzzzzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaabxyzzzzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaaabxyzzzzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaaabxyzzzzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "abcxyzzpqrrrabbxyyyypqAzz" );
MATCH( 0, "abcxyzzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aabcxyzzzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aabcxyzzzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaabcxyzzzzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaabcxyzzzzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaaabcxyzzzzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaaabcxyzzzzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaaabcxyzzzzpqrrrabbbxyyyypqAzz" );
MATCH( 0, "aaaabcxyzzzzpqrrrabbbxyyyypqAzz" );
SHOULD_MATCH( 1, "aaaabcxyzzzzpqrrrabbbxyyyyypqAzz" );
MATCH( 0, "aaaabcxyzzzzpqrrrabbbxyyyyypqAzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypABzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypABzz" );
SHOULD_MATCH( 1, "aaabcxyzpqrrrabbxyyyypABBzz" );
MATCH( 0, "aaabcxyzpqrrrabbxyyyypABBzz" );
SHOULD_MATCH( 1, ">>>aaabxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaabxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, ">aaaabxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "aaaabxyzpqrrrabbxyyyypqAzz" );
SHOULD_MATCH( 1, ">>>>abcxyzpqrrrabbxyyyypqAzz" );
MATCH( 0, "abcxyzpqrrrabbxyyyypqAzz" );
SHOULD_NOT_MATCH("abxyzpqrrabbxyyyypqAzz");
SHOULD_NOT_MATCH("abxyzpqrrrrabbxyyyypqAzz");
SHOULD_NOT_MATCH("abxyzpqrrrabxyyyypqAzz");
SHOULD_NOT_MATCH("aaaabcxyzzzzpqrrrabbbxyyyyyypqAzz");
SHOULD_NOT_MATCH("aaaabcxyzzzzpqrrrabbbxyyypqAzz");
SHOULD_NOT_MATCH("aaabcxyzpqrrrabbxyyyypqqqqqqqAzz");


delete TheRegEx;

  }

void testInput4()
    {
        RegEx* TheRegEx = 0;


EXPRESSION( "^(abc){1,2}zz", 0 );
SHOULD_MATCH( 2, "abczz" );
MATCH( 0, "abczz" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abcabczz" );
MATCH( 0, "abcabczz" );
MATCH( 1, "abc" );
SHOULD_NOT_MATCH("zz");
SHOULD_NOT_MATCH("abcabcabczz");
SHOULD_NOT_MATCH(">>abczz");


delete TheRegEx;

  }

void testInput5()
    {
        RegEx* TheRegEx = 0;


EXPRESSION( "^(b+?|a){1,2}?c", 0 );
SHOULD_MATCH( 2, "bc" );
MATCH( 0, "bc" );
MATCH( 1, "b" );
SHOULD_MATCH( 2, "bbc" );
MATCH( 0, "bbc" );
MATCH( 1, "b" );
SHOULD_MATCH( 2, "bbbc" );
MATCH( 0, "bbbc" );
MATCH( 1, "bb" );
SHOULD_MATCH( 2, "bac" );
MATCH( 0, "bac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbac" );
MATCH( 0, "bbac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aac" );
MATCH( 0, "aac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "abbbbbbbbbbbc" );
MATCH( 0, "abbbbbbbbbbbc" );
MATCH( 1, "bbbbbbbbbbb" );
SHOULD_MATCH( 2, "bbbbbbbbbbbac" );
MATCH( 0, "bbbbbbbbbbbac" );
MATCH( 1, "a" );
SHOULD_NOT_MATCH("aaac");
SHOULD_NOT_MATCH("abbbbbbbbbbbac");


delete TheRegEx;

  }

void testInput6()
    {
        RegEx* TheRegEx = 0;


EXPRESSION( "^(b+|a){1,2}c", 0 );
SHOULD_MATCH( 2, "bc" );
MATCH( 0, "bc" );
MATCH( 1, "b" );
SHOULD_MATCH( 2, "bbc" );
MATCH( 0, "bbc" );
MATCH( 1, "bb" );
SHOULD_MATCH( 2, "bbbc" );
MATCH( 0, "bbbc" );
MATCH( 1, "bbb" );
SHOULD_MATCH( 2, "bac" );
MATCH( 0, "bac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbac" );
MATCH( 0, "bbac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aac" );
MATCH( 0, "aac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "abbbbbbbbbbbc" );
MATCH( 0, "abbbbbbbbbbbc" );
MATCH( 1, "bbbbbbbbbbb" );
SHOULD_MATCH( 2, "bbbbbbbbbbbac" );
MATCH( 0, "bbbbbbbbbbbac" );
MATCH( 1, "a" );
SHOULD_NOT_MATCH("aaac");
SHOULD_NOT_MATCH("abbbbbbbbbbbac");


delete TheRegEx;

  }

void testInput7()
    {
        RegEx* TheRegEx = 0;



EXPRESSION( "^(b+|a){1,2}?bc", 0 );
SHOULD_MATCH( 2, "bbc" );
MATCH( 0, "bbc" );
MATCH( 1, "b" );


EXPRESSION( "^(b*|ba){1,2}?bc", 0 );
SHOULD_MATCH( 2, "babc" );
MATCH( 0, "babc" );
MATCH( 1, "ba" );
SHOULD_MATCH( 2, "bbabc" );
MATCH( 0, "bbabc" );
MATCH( 1, "ba" );
SHOULD_MATCH( 2, "bababc" );
MATCH( 0, "bababc" );
MATCH( 1, "ba" );
SHOULD_NOT_MATCH("bababbc");
SHOULD_NOT_MATCH("babababc");


delete TheRegEx;

  }

void testInput8()
    {
        RegEx* TheRegEx = 0;


EXPRESSION( "^(ba|b*){1,2}?bc", 0 );
SHOULD_MATCH( 2, "babc" );
MATCH( 0, "babc" );
MATCH( 1, "ba" );
SHOULD_MATCH( 2, "bbabc" );
MATCH( 0, "bbabc" );
MATCH( 1, "ba" );
SHOULD_MATCH( 2, "bababc" );
MATCH( 0, "bababc" );
MATCH( 1, "ba" );
SHOULD_NOT_MATCH("bababbc");
SHOULD_NOT_MATCH("babababc");


EXPRESSION( "^\\ca\\cA\\c[\\c{\\c:", 0 );
SHOULD_MATCH( 1, "\x01\x01\e;z" );
MATCH( 0, "\x01\x01\x1b;z" );


delete TheRegEx;

  }

void testInput9()
    {
        RegEx* TheRegEx = 0;


EXPRESSION( "^[ab\\]cde]", 0 );
SHOULD_MATCH( 1, "athing" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "bthing" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "]thing" );
MATCH( 0, "]" );
SHOULD_MATCH( 1, "cthing" );
MATCH( 0, "c" );
SHOULD_MATCH( 1, "dthing" );
MATCH( 0, "d" );
SHOULD_MATCH( 1, "ething" );
MATCH( 0, "e" );
SHOULD_NOT_MATCH("fthing");
SHOULD_NOT_MATCH("[thing");
SHOULD_NOT_MATCH("\\thing");


EXPRESSION( "^[]cde]", 0 );
SHOULD_MATCH( 1, "]thing" );
MATCH( 0, "]" );
SHOULD_MATCH( 1, "cthing" );
MATCH( 0, "c" );
SHOULD_MATCH( 1, "dthing" );
MATCH( 0, "d" );
SHOULD_MATCH( 1, "ething" );
MATCH( 0, "e" );
SHOULD_NOT_MATCH("athing");
SHOULD_NOT_MATCH("fthing");


delete TheRegEx;

  }

void testInput10()
    {
        RegEx* TheRegEx = 0;


EXPRESSION( "^[^ab\\]cde]", 0 );
SHOULD_MATCH( 1, "fthing" );
MATCH( 0, "f" );
SHOULD_MATCH( 1, "[thing" );
MATCH( 0, "[" );
SHOULD_MATCH( 1, "\\thing" );
MATCH( 0, "\\" );
SHOULD_NOT_MATCH("athing");
SHOULD_NOT_MATCH("bthing");
SHOULD_NOT_MATCH("]thing");
SHOULD_NOT_MATCH("cthing");
SHOULD_NOT_MATCH("dthing");
SHOULD_NOT_MATCH("ething");


EXPRESSION( "^[^]cde]", 0 );
SHOULD_MATCH( 1, "athing" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "fthing" );
MATCH( 0, "f" );
SHOULD_NOT_MATCH("]thing");
SHOULD_NOT_MATCH("cthing");
SHOULD_NOT_MATCH("dthing");
SHOULD_NOT_MATCH("ething");

delete TheRegEx;

  }

void testInput11()
    {
        RegEx* TheRegEx = 0;


#ifdef REGEX_FULL

EXPRESSION( "^Å", 0 );
SHOULD_MATCH( 1, "Å" );
MATCH( 0, "\x81" );

EXPRESSION( "^Åˇ", 0 );
SHOULD_MATCH( 1, "Åˇ" );
MATCH( 0, "Å\xff" );

#endif

EXPRESSION( "^[0-9]+$", 0 );
SHOULD_MATCH( 1, "0" );
MATCH( 0, "0" );
SHOULD_MATCH( 1, "1" );
MATCH( 0, "1" );
SHOULD_MATCH( 1, "2" );
MATCH( 0, "2" );
SHOULD_MATCH( 1, "3" );
MATCH( 0, "3" );
SHOULD_MATCH( 1, "4" );
MATCH( 0, "4" );
SHOULD_MATCH( 1, "5" );
MATCH( 0, "5" );
SHOULD_MATCH( 1, "6" );
MATCH( 0, "6" );
SHOULD_MATCH( 1, "7" );
MATCH( 0, "7" );
SHOULD_MATCH( 1, "8" );
MATCH( 0, "8" );
SHOULD_MATCH( 1, "9" );
MATCH( 0, "9" );
SHOULD_MATCH( 1, "10" );
MATCH( 0, "10" );
SHOULD_MATCH( 1, "100" );
MATCH( 0, "100" );
SHOULD_NOT_MATCH("abc");

delete TheRegEx;

  }

void testInput12()
    {
        RegEx* TheRegEx = 0;


#ifdef REGEX_FULL

EXPRESSION( "^.*nter", 0 );
SHOULD_MATCH( 1, "enter" );
MATCH( 0, "enter" );
SHOULD_MATCH( 1, "inter" );
MATCH( 0, "inter" );
SHOULD_MATCH( 1, "uponter" );
MATCH( 0, "uponter" );

#endif

EXPRESSION( "^xxx[0-9]+$", 0 );
SHOULD_MATCH( 1, "xxx0" );
MATCH( 0, "xxx0" );
SHOULD_MATCH( 1, "xxx1234" );
MATCH( 0, "xxx1234" );
SHOULD_NOT_MATCH("xxx");


delete TheRegEx;

  }

void testInput13()
    {
        RegEx* TheRegEx = 0;


EXPRESSION( "^.+[0-9][0-9][0-9]$", 0 );
SHOULD_MATCH( 1, "x123" );
MATCH( 0, "x123" );
SHOULD_MATCH( 1, "xx123" );
MATCH( 0, "xx123" );
SHOULD_MATCH( 1, "123456" );
MATCH( 0, "123456" );
SHOULD_NOT_MATCH("123");
SHOULD_MATCH( 1, "x1234" );
MATCH( 0, "x1234" );


EXPRESSION( "^.+?[0-9][0-9][0-9]$", 0 );
SHOULD_MATCH( 1, "x123" );
MATCH( 0, "x123" );
SHOULD_MATCH( 1, "xx123" );
MATCH( 0, "xx123" );
SHOULD_MATCH( 1, "123456" );
MATCH( 0, "123456" );
SHOULD_NOT_MATCH("123");
SHOULD_MATCH( 1, "x1234" );
MATCH( 0, "x1234" );

delete TheRegEx;

  }

void testInput14()
    {
        RegEx* TheRegEx = 0;


#ifdef REGEX_FULL

EXPRESSION( "^([^!]+)!(.+)=apquxz\\.ixr\\.zzz\\.ac\\.uk$", 0 );
SHOULD_MATCH( 3, "abc!pqr=apquxz.ixr.zzz.ac.uk" );
MATCH( 0, "abc!pqr=apquxz.ixr.zzz.ac.uk" );
MATCH( 1, "abc" );
MATCH( 2, "pqr" );
SHOULD_NOT_MATCH("!pqr=apquxz.ixr.zzz.ac.uk");
SHOULD_NOT_MATCH("abc!=apquxz.ixr.zzz.ac.uk");
SHOULD_NOT_MATCH("abc!pqr=apquxz:ixr.zzz.ac.uk");
SHOULD_NOT_MATCH("abc!pqr=apquxz.ixr.zzz.ac.ukk");


EXPRESSION( ":", 0 );
SHOULD_MATCH( 1, "Well, we need a colon: somewhere" );
MATCH( 0, ":" );
SHOULD_NOT_MATCH("*** Fail if we don't");


EXPRESSION( "([\\da-f:]+)$", PCRE_CASELESS );
SHOULD_MATCH( 2, "0abc" );
MATCH( 0, "0abc" );
MATCH( 1, "0abc" );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "abc" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "fed" );
MATCH( 0, "fed" );
MATCH( 1, "fed" );
SHOULD_MATCH( 2, "E" );
MATCH( 0, "E" );
MATCH( 1, "E" );
SHOULD_MATCH( 2, "::" );
MATCH( 0, "::" );
MATCH( 1, "::" );
SHOULD_MATCH( 2, "5f03:12C0::932e" );
MATCH( 0, "5f03:12C0::932e" );
MATCH( 1, "5f03:12C0::932e" );
SHOULD_MATCH( 2, "fed def" );
MATCH( 0, "def" );
MATCH( 1, "def" );
SHOULD_MATCH( 2, "Any old stuff" );
MATCH( 0, "ff" );
MATCH( 1, "ff" );
SHOULD_NOT_MATCH("0zzz");
SHOULD_NOT_MATCH("gzzz");
SHOULD_NOT_MATCH("fed\x20");
SHOULD_NOT_MATCH("Any old rubbish");

#endif

EXPRESSION( "^.*\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$", 0 );
SHOULD_MATCH( 4, ".1.2.3" );
MATCH( 0, ".1.2.3" );
MATCH( 1, "1" );
MATCH( 2, "2" );
MATCH( 3, "3" );
SHOULD_MATCH( 4, "A.12.123.0" );
MATCH( 0, "A.12.123.0" );
MATCH( 1, "12" );
MATCH( 2, "123" );
MATCH( 3, "0" );
SHOULD_NOT_MATCH(".1.2.3333");
SHOULD_NOT_MATCH("1.2.3");
SHOULD_NOT_MATCH("1234.2.3");

#ifdef REGEX_FULL

EXPRESSION( "^(\\d+)\\s+IN\\s+SOA\\s+(\\S+)\\s+(\\S+)\\s*\(\\s*$", 0 );
SHOULD_MATCH( 4, "1 IN SOA non-sp1 non-sp2(" );
MATCH( 0, "1 IN SOA non-sp1 non-sp2(" );
MATCH( 1, "1" );
MATCH( 2, "non-sp1" );
MATCH( 3, "non-sp2" );
SHOULD_MATCH( 4, "1    IN    SOA    non-sp1    non-sp2   (" );
MATCH( 0, "1    IN    SOA    non-sp1    non-sp2   (" );
MATCH( 1, "1" );
MATCH( 2, "non-sp1" );
MATCH( 3, "non-sp2" );
SHOULD_NOT_MATCH("1IN SOA non-sp1 non-sp2(");


EXPRESSION( "^[a-zA-Z\\d][a-zA-Z\\d\\-]*(\\.[a-zA-Z\\d][a-zA-z\\d\\-]*)*\\.$", 0 );
SHOULD_MATCH( 1, "a." );
MATCH( 0, "a." );
SHOULD_MATCH( 1, "Z." );
MATCH( 0, "Z." );
SHOULD_MATCH( 1, "2." );
MATCH( 0, "2." );
SHOULD_MATCH( 2, "ab-c.pq-r." );
MATCH( 0, "ab-c.pq-r." );
MATCH( 1, ".pq-r" );
SHOULD_MATCH( 2, "sxk.zzz.ac.uk." );
MATCH( 0, "sxk.zzz.ac.uk." );
MATCH( 1, ".uk" );
SHOULD_MATCH( 2, "x-.y-." );
MATCH( 0, "x-.y-." );
MATCH( 1, ".y-" );
SHOULD_NOT_MATCH("-abc.peq.");

#ifdef REGEX_FULL

EXPRESSION( "^\\*\\.[a-z]([a-z\\-\\d]*[a-z\\d]+)?(\\.[a-z]([a-z\\-\\d]*[a-z\\d]+)?)*$", 0 );
SHOULD_MATCH( 1, "*.a" );
MATCH( 0, "*.a" );
SHOULD_MATCH( 2, "*.b0-a" );
MATCH( 0, "*.b0-a" );
MATCH( 1, "0-a" );
SHOULD_MATCH( 3, "*.c3-b.c" );
MATCH( 0, "*.c3-b.c" );
MATCH( 1, "3-b" );
MATCH( 2, ".c" );
SHOULD_MATCH( 4, "*.c-a.b-c" );
MATCH( 0, "*.c-a.b-c" );
MATCH( 1, "-a" );
MATCH( 2, ".b-c" );
MATCH( 3, "-c" );
SHOULD_NOT_MATCH("*.0");
SHOULD_NOT_MATCH("*.a-");
SHOULD_NOT_MATCH("*.a-b.c-");
SHOULD_NOT_MATCH("*.c-a.0-c");


EXPRESSION( "^(?=ab(de))(abd)(e)", 0 );
SHOULD_MATCH( 4, "abde" );
MATCH( 0, "abde" );
MATCH( 1, "de" );
MATCH( 2, "abd" );
MATCH( 3, "e" );


EXPRESSION( "^(?!(ab)de|x)(abd)(f)", 0 );
SHOULD_MATCH( 4, "abdf" );
MATCH( 0, "abdf" );
MATCH( 1, "<unset>" );
MATCH( 2, "abd" );
MATCH( 3, "f" );


EXPRESSION( "^(?=(ab(cd)))(ab)", 0 );
SHOULD_MATCH( 4, "abcd" );
MATCH( 0, "ab" );
MATCH( 1, "abcd" );
MATCH( 2, "cd" );
MATCH( 3, "ab" );


EXPRESSION( "^[\\da-f](\\.[\\da-f])*$", PCRE_CASELESS );
SHOULD_MATCH( 2, "a.b.c.d" );
MATCH( 0, "a.b.c.d" );
MATCH( 1, ".d" );
SHOULD_MATCH( 2, "A.B.C.D" );
MATCH( 0, "A.B.C.D" );
MATCH( 1, ".D" );
SHOULD_MATCH( 2, "a.b.c.1.2.3.C" );
MATCH( 0, "a.b.c.1.2.3.C" );
MATCH( 1, ".C" );


EXPRESSION( "^\".*\"\\s*(;.*)?$", 0 );
SHOULD_MATCH( 1, "\"1234\"" );
MATCH( 0, "\"1234\"" );
SHOULD_MATCH( 2, "\"abcd\" ;" );
MATCH( 0, "\"abcd\"" );
MATCH( 1, ";" );
SHOULD_MATCH( 2, "\"\" ; rhubarb" );
MATCH( 0, "\"\" ; rhubarb" );
MATCH( 1, "; rhubarb" );
SHOULD_NOT_MATCH("\"1234\" : things");


EXPRESSION( "^$", 0 );
SHOULD_MATCH( 1, "" );
MATCH( 0, "" );


EXPRESSION( "   ^    a   (?# begins with a)  b\\sc (?# then b c) $ (?# then end)", PCRE_EXTENDED );
SHOULD_MATCH( 1, "ab c" );
MATCH( 0, "ab c" );
SHOULD_NOT_MATCH("abc");
SHOULD_NOT_MATCH("ab cde");


EXPRESSION( "(?x)   ^    a   (?# begins with a)  b\\sc (?# then b c) $ (?# then end)", 0 );
SHOULD_MATCH( 1, "ab c" );
MATCH( 0, "ab c" );
SHOULD_NOT_MATCH("abc");
SHOULD_NOT_MATCH("ab cde");


EXPRESSION( "a\\ b[c ]d", PCRE_EXTENDED );
SHOULD_MATCH( 1, "a bcd" );
MATCH( 0, "a bcd" );
SHOULD_MATCH( 1, "a b d" );
MATCH( 0, "a b d" );
SHOULD_NOT_MATCH("abcd");
SHOULD_NOT_MATCH("ab d");


EXPRESSION( "^(a(b(c)))(d(e(f)))(h(i(j)))(k(l(m)))$", 0 );
SHOULD_MATCH( 13, "abcdefhijklm" );
MATCH( 0, "abcdefhijklm" );
MATCH( 1, "abc" );
MATCH( 2, "bc" );
MATCH( 3, "c" );
MATCH( 4, "def" );
MATCH( 5, "ef" );
MATCH( 6, "f" );
MATCH( 7, "hij" );
MATCH( 8, "ij" );
MATCH( 9, "j" );
MATCH( 10, "klm" );
MATCH( 11, "lm" );
MATCH( 12, "m" );


EXPRESSION( "^(?:a(b(c)))(?:d(e(f)))(?:h(i(j)))(?:k(l(m)))$", 0 );
SHOULD_MATCH( 9, "abcdefhijklm" );
MATCH( 0, "abcdefhijklm" );
MATCH( 1, "bc" );
MATCH( 2, "c" );
MATCH( 3, "ef" );
MATCH( 4, "f" );
MATCH( 5, "ij" );
MATCH( 6, "j" );
MATCH( 7, "lm" );
MATCH( 8, "m" );

EXPRESSION( "^[\\w][\\W][\\s][\\S][\\d][\\D][\b][\n][\\c]][\\022]", 0 );
SHOULD_MATCH( 1, "a+ Z0+\x08\n\x1d\x12" );
MATCH( 0, "a+ Z0+\x08\n\x1d\x12" );


EXPRESSION( "^[.^$|()*+?{,}]+", 0 );
SHOULD_MATCH( 1, ".^\\$(*+)|{?,?}" );
MATCH( 0, ".^$(*+)|{?,?}" );


EXPRESSION( "^a*\\w", 0 );
SHOULD_MATCH( 1, "z" );
MATCH( 0, "z" );
SHOULD_MATCH( 1, "az" );
MATCH( 0, "az" );
SHOULD_MATCH( 1, "aaaz" );
MATCH( 0, "aaaz" );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "aa" );
MATCH( 0, "aa" );
SHOULD_MATCH( 1, "aaaa" );
MATCH( 0, "aaaa" );
SHOULD_MATCH( 1, "a+" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "aa+" );
MATCH( 0, "aa" );


EXPRESSION( "^a*?\\w", 0 );
SHOULD_MATCH( 1, "z" );
MATCH( 0, "z" );
SHOULD_MATCH( 1, "az" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "aaaz" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "aa" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "aaaa" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "a+" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "aa+" );
MATCH( 0, "a" );


EXPRESSION( "^a+\\w", 0 );
SHOULD_MATCH( 1, "az" );
MATCH( 0, "az" );
SHOULD_MATCH( 1, "aaaz" );
MATCH( 0, "aaaz" );
SHOULD_MATCH( 1, "aa" );
MATCH( 0, "aa" );
SHOULD_MATCH( 1, "aaaa" );
MATCH( 0, "aaaa" );
SHOULD_MATCH( 1, "aa+" );
MATCH( 0, "aa" );


EXPRESSION( "^a+?\\w", 0 );
SHOULD_MATCH( 1, "az" );
MATCH( 0, "az" );
SHOULD_MATCH( 1, "aaaz" );
MATCH( 0, "aa" );
SHOULD_MATCH( 1, "aa" );
MATCH( 0, "aa" );
SHOULD_MATCH( 1, "aaaa" );
MATCH( 0, "aa" );
SHOULD_MATCH( 1, "aa+" );
MATCH( 0, "aa" );


EXPRESSION( "^\\d{8}\\w{2,}", 0 );
SHOULD_MATCH( 1, "1234567890" );
MATCH( 0, "1234567890" );
SHOULD_MATCH( 1, "12345678ab" );
MATCH( 0, "12345678ab" );
SHOULD_MATCH( 1, "12345678__" );
MATCH( 0, "12345678__" );
SHOULD_NOT_MATCH("1234567");


EXPRESSION( "^[aeiou\\d]{4,5}$", 0 );
SHOULD_MATCH( 1, "uoie" );
MATCH( 0, "uoie" );
SHOULD_MATCH( 1, "1234" );
MATCH( 0, "1234" );
SHOULD_MATCH( 1, "12345" );
MATCH( 0, "12345" );
SHOULD_MATCH( 1, "aaaaa" );
MATCH( 0, "aaaaa" );
SHOULD_NOT_MATCH("123456");


EXPRESSION( "^[aeiou\\d]{4,5}?", 0 );
SHOULD_MATCH( 1, "uoie" );
MATCH( 0, "uoie" );
SHOULD_MATCH( 1, "1234" );
MATCH( 0, "1234" );
SHOULD_MATCH( 1, "12345" );
MATCH( 0, "1234" );
SHOULD_MATCH( 1, "aaaaa" );
MATCH( 0, "aaaa" );
SHOULD_MATCH( 1, "123456" );
MATCH( 0, "1234" );


EXPRESSION( "\\A(abc|def)=(\1){2,3}\\Z", 0 );
SHOULD_MATCH( 3, "abc=abcabc" );
MATCH( 0, "abc=abcabc" );
MATCH( 1, "abc" );
MATCH( 2, "abc" );
SHOULD_MATCH( 3, "def=defdefdef" );
MATCH( 0, "def=defdefdef" );
MATCH( 1, "def" );
MATCH( 2, "def" );
SHOULD_NOT_MATCH("abc=defdef");


EXPRESSION( "^(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)\11*(\3\4)\1(?#)2$", 0 );
SHOULD_MATCH( 13, "abcdefghijkcda2" );
MATCH( 0, "abcdefghijkcda2" );
MATCH( 1, "a" );
MATCH( 2, "b" );
MATCH( 3, "c" );
MATCH( 4, "d" );
MATCH( 5, "e" );
MATCH( 6, "f" );
MATCH( 7, "g" );
MATCH( 8, "h" );
MATCH( 9, "i" );
MATCH( 10, "j" );
MATCH( 11, "k" );
MATCH( 12, "cd" );
SHOULD_MATCH( 13, "abcdefghijkkkkcda2" );
MATCH( 0, "abcdefghijkkkkcda2" );
MATCH( 1, "a" );
MATCH( 2, "b" );
MATCH( 3, "c" );
MATCH( 4, "d" );
MATCH( 5, "e" );
MATCH( 6, "f" );
MATCH( 7, "g" );
MATCH( 8, "h" );
MATCH( 9, "i" );
MATCH( 10, "j" );
MATCH( 11, "k" );
MATCH( 12, "cd" );


EXPRESSION( "(cat(a(ract|tonic)|erpillar)) \1()2(3)", 0 );
SHOULD_MATCH( 6, "cataract cataract23" );
MATCH( 0, "cataract cataract23" );
MATCH( 1, "cataract" );
MATCH( 2, "aract" );
MATCH( 3, "ract" );
MATCH( 4, "" );
MATCH( 5, "3" );
SHOULD_MATCH( 6, "catatonic catatonic23" );
MATCH( 0, "catatonic catatonic23" );
MATCH( 1, "catatonic" );
MATCH( 2, "atonic" );
MATCH( 3, "tonic" );
MATCH( 4, "" );
MATCH( 5, "3" );
SHOULD_MATCH( 6, "caterpillar caterpillar23" );
MATCH( 0, "caterpillar caterpillar23" );
MATCH( 1, "caterpillar" );
MATCH( 2, "erpillar" );
MATCH( 3, "<unset>" );
MATCH( 4, "" );
MATCH( 5, "3" );



EXPRESSION( "^From +([^ ]+) +[a-zA-Z][a-zA-Z][a-zA-Z] +[a-zA-Z][a-zA-Z][a-zA-Z] +[0-9]?[0-9] +[0-9][0-9]:[0-9][0-9]", 0 );
SHOULD_MATCH( 2, "From abcd  Mon Sep 01 12:33:02 1997" );
MATCH( 0, "From abcd  Mon Sep 01 12:33" );
MATCH( 1, "abcd" );


EXPRESSION( "^From\\s+\\S+\\s+([a-zA-Z]{3}\\s+){2}\\d{1,2}\\s+\\d\\d:\\d\\d", 0 );
SHOULD_MATCH( 2, "From abcd  Mon Sep 01 12:33:02 1997" );
MATCH( 0, "From abcd  Mon Sep 01 12:33" );
MATCH( 1, "Sep " );
SHOULD_MATCH( 2, "From abcd  Mon Sep  1 12:33:02 1997" );
MATCH( 0, "From abcd  Mon Sep  1 12:33" );
MATCH( 1, "Sep  " );
SHOULD_NOT_MATCH("From abcd  Sep 01 12:33:02 1997");


EXPRESSION( "^12.34", PCRE_DOTALL );
SHOULD_MATCH( 1, "12\n34" );
MATCH( 0, "12\n34" );
SHOULD_MATCH( 1, "12\r34" );
MATCH( 0, "12\r34" );


EXPRESSION( "\\w+(?=\t)", 0 );
SHOULD_MATCH( 1, "the quick brown\t fox" );
MATCH( 0, "brown" );


EXPRESSION( "foo(?!bar)(.*)", 0 );
SHOULD_MATCH( 2, "foobar is foolish see?" );
MATCH( 0, "foolish see?" );
MATCH( 1, "lish see?" );


EXPRESSION( "(?:(?!foo)...|^.{0,2})bar(.*)", 0 );
SHOULD_MATCH( 2, "foobar crowbar etc" );
MATCH( 0, "rowbar etc" );
MATCH( 1, " etc" );
SHOULD_MATCH( 2, "barrel" );
MATCH( 0, "barrel" );
MATCH( 1, "rel" );
SHOULD_MATCH( 2, "2barrel" );
MATCH( 0, "2barrel" );
MATCH( 1, "rel" );
SHOULD_MATCH( 2, "A barrel" );
MATCH( 0, "A barrel" );
MATCH( 1, "rel" );


EXPRESSION( "^(\\D*)(?=\\d)(?!123)", 0 );
SHOULD_MATCH( 2, "abc456" );
MATCH( 0, "abc" );
MATCH( 1, "abc" );
SHOULD_NOT_MATCH("abc123");


EXPRESSION( "^1234(?# test newlines\n  inside)", 0 );
SHOULD_MATCH( 1, "1234" );
MATCH( 0, "1234" );


EXPRESSION( "^1234 #comment in extended re\n  ", PCRE_EXTENDED );
SHOULD_MATCH( 1, "1234" );
MATCH( 0, "1234" );


EXPRESSION( "#rhubarb\n  abcd", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abcd" );
MATCH( 0, "abcd" );


EXPRESSION( "^abcd#rhubarb", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abcd" );
MATCH( 0, "abcd" );


EXPRESSION( "^(a)\1{2,3}(.)", 0 );
SHOULD_MATCH( 3, "aaab" );
MATCH( 0, "aaab" );
MATCH( 1, "a" );
MATCH( 2, "b" );
SHOULD_MATCH( 3, "aaaab" );
MATCH( 0, "aaaab" );
MATCH( 1, "a" );
MATCH( 2, "b" );
SHOULD_MATCH( 3, "aaaaab" );
MATCH( 0, "aaaaa" );
MATCH( 1, "a" );
MATCH( 2, "a" );
SHOULD_MATCH( 3, "aaaaaab" );
MATCH( 0, "aaaaa" );
MATCH( 1, "a" );
MATCH( 2, "a" );


EXPRESSION( "(?!^)abc", 0 );
SHOULD_MATCH( 1, "the abc" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("abc");


EXPRESSION( "(?=^)abc", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("the abc");


EXPRESSION( "^[ab]{1,3}(ab*|b)", 0 );
SHOULD_MATCH( 2, "aabbbbb" );
MATCH( 0, "aabb" );
MATCH( 1, "b" );


EXPRESSION( "^[ab]{1,3}?(ab*|b)", 0 );
SHOULD_MATCH( 2, "aabbbbb" );
MATCH( 0, "aabbbbb" );
MATCH( 1, "abbbbb" );


EXPRESSION( "^[ab]{1,3}?(ab*?|b)", 0 );
SHOULD_MATCH( 2, "aabbbbb" );
MATCH( 0, "aa" );
MATCH( 1, "a" );


EXPRESSION( "^[ab]{1,3}(ab*?|b)", 0 );
SHOULD_MATCH( 2, "aabbbbb" );
MATCH( 0, "aabb" );
MATCH( 1, "b" );


// section of really long involved stuff removed from original test here

//EXPRESSION( "abc\\0def\\00pqr\\000xyz\\0000AB", 0 );
//SHOULD_MATCH( 1, "abc\\0def\\00pqr\\000xyz\\0000AB" );
//MATCH( 0, "abc\x00def\x00pqr\x00xyz\x000AB" );
//SHOULD_MATCH( 1, "abc456 abc\\0def\\00pqr\\000xyz\\0000ABCDE" );
//MATCH( 0, "abc\x00def\x00pqr\x00xyz\x000AB" );


//EXPRESSION( "abc\x0def\x00pqr\x000xyz\x0000AB", 0 );
//SHOULD_MATCH( 1, "abc\x0def\x00pqr\x000xyz\x0000AB" );
//MATCH( 0, "abc\x0def\x00pqr\x000xyz\x0000AB" );
//SHOULD_MATCH( 1, "abc456 abc\x0def\x00pqr\x000xyz\x0000ABCDE" );
//MATCH( 0, "abc\x0def\x00pqr\x000xyz\x0000AB" );


EXPRESSION( "^[\\000-\\037]", 0 );
SHOULD_MATCH( 1, "\\0A" );
MATCH( 0, "\x00" );
SHOULD_MATCH( 1, "\\01B" );
MATCH( 0, "\x01" );
SHOULD_MATCH( 1, "\\037C" );
MATCH( 0, "\x1f" );


EXPRESSION( "\\0*", 0 );
SHOULD_MATCH( 1, "\\0\\0\\0\\0" );
MATCH( 0, "\x00\x00\x00\x00" );


EXPRESSION( "A\x0{2,3}Z", 0 );
SHOULD_MATCH( 1, "The A\x0\x0Z" );
MATCH( 0, "A\x00\x00Z" );
SHOULD_MATCH( 1, "An A\\0\x0\\0Z" );
MATCH( 0, "A\x00\x00\x00Z" );
SHOULD_NOT_MATCH("A\\0Z");
SHOULD_NOT_MATCH("A\\0\x0\\0\x0Z");


EXPRESSION( "^(cow|)\1(bell)", 0 );
SHOULD_MATCH( 3, "cowcowbell" );
MATCH( 0, "cowcowbell" );
MATCH( 1, "cow" );
MATCH( 2, "bell" );
SHOULD_MATCH( 3, "bell" );
MATCH( 0, "bell" );
MATCH( 1, "" );
MATCH( 2, "bell" );
SHOULD_NOT_MATCH("cowbell");

#endif

EXPRESSION( "^\\s", 0 );
SHOULD_MATCH( 1, " abc" );
MATCH( 0, " " );
SHOULD_MATCH( 1, "\tabc" );
MATCH( 0, "\t" );
SHOULD_MATCH( 1, "\nabc" );
MATCH( 0, "\n" );
SHOULD_MATCH( 1, "\rabc" );
MATCH( 0, "\r" );
SHOULD_MATCH( 1, "\fabc" );
MATCH( 0, "\f" );
SHOULD_NOT_MATCH("abc");

#ifdef REGEX_FULL

EXPRESSION( "^a	b\n  \r  \f  c", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );


EXPRESSION( "^(a|)\1*b", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaaab" );
MATCH( 0, "aaaab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "b" );
MATCH( 1, "" );
SHOULD_NOT_MATCH("acb");


EXPRESSION( "^(a|)\1+b", 0 );
SHOULD_MATCH( 2, "aab" );
MATCH( 0, "aab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaaab" );
MATCH( 0, "aaaab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "b" );
MATCH( 1, "" );
SHOULD_NOT_MATCH("ab");


EXPRESSION( "^(a|)\1?b", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab" );
MATCH( 0, "aab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "b" );
MATCH( 1, "" );
SHOULD_NOT_MATCH("acb");


EXPRESSION( "^(a|)\1{2}b", 0 );
SHOULD_MATCH( 2, "aaab" );
MATCH( 0, "aaab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "b" );
MATCH( 1, "" );
SHOULD_NOT_MATCH("ab");
SHOULD_NOT_MATCH("aab");
SHOULD_NOT_MATCH("aaaab");


EXPRESSION( "^(a|)\1{2,3}b", 0 );
SHOULD_MATCH( 2, "aaab" );
MATCH( 0, "aaab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaaab" );
MATCH( 0, "aaaab" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "b" );
MATCH( 1, "" );
SHOULD_NOT_MATCH("ab");
SHOULD_NOT_MATCH("aab");
SHOULD_NOT_MATCH("aaaaab");

#endif

EXPRESSION( "ab{1,3}bc", 0 );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "abbbbc" );
SHOULD_MATCH( 1, "abbbc" );
MATCH( 0, "abbbc" );
SHOULD_MATCH( 1, "abbc" );
MATCH( 0, "abbc" );
SHOULD_NOT_MATCH("abc");
SHOULD_NOT_MATCH("abbbbbc");

#ifdef REGEX_FULL

EXPRESSION( "([^.]*)\\.([^:]*):[T ]+(.*)", 0 );
SHOULD_MATCH( 4, "track1.title:TBlah blah blah" );
MATCH( 0, "track1.title:TBlah blah blah" );
MATCH( 1, "track1" );
MATCH( 2, "title" );
MATCH( 3, "Blah blah blah" );


EXPRESSION( "([^.]*)\\.([^:]*):[T ]+(.*)", PCRE_CASELESS );
SHOULD_MATCH( 4, "track1.title:TBlah blah blah" );
MATCH( 0, "track1.title:TBlah blah blah" );
MATCH( 1, "track1" );
MATCH( 2, "title" );
MATCH( 3, "Blah blah blah" );


EXPRESSION( "([^.]*)\\.([^:]*):[t ]+(.*)", PCRE_CASELESS );
SHOULD_MATCH( 4, "track1.title:TBlah blah blah" );
MATCH( 0, "track1.title:TBlah blah blah" );
MATCH( 1, "track1" );
MATCH( 2, "title" );
MATCH( 3, "Blah blah blah" );


EXPRESSION( "^[W-c]+$", 0 );
SHOULD_MATCH( 1, "WXY_^abc" );
MATCH( 0, "WXY_^abc" );
SHOULD_NOT_MATCH("wxy");


EXPRESSION( "^[W-c]+$", PCRE_CASELESS );
SHOULD_MATCH( 1, "WXY_^abc" );
MATCH( 0, "WXY_^abc" );
SHOULD_MATCH( 1, "wxy_^ABC" );
MATCH( 0, "wxy_^ABC" );


EXPRESSION( "^[\x3f-\x5F]+$", PCRE_CASELESS );
SHOULD_MATCH( 1, "WXY_^abc" );
MATCH( 0, "WXY_^abc" );
SHOULD_MATCH( 1, "wxy_^ABC" );
MATCH( 0, "wxy_^ABC" );


EXPRESSION( "^abc$", PCRE_MULTILINE );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "qqq\nabc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "abc\nzzz" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "qqq\nabc\nzzz" );
MATCH( 0, "abc" );

#endif

EXPRESSION( "^abc$", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("qqq\nabc");
SHOULD_NOT_MATCH("abc\nzzz");
SHOULD_NOT_MATCH("qqq\nabc\nzzz");

#ifdef REGEX_FULL

EXPRESSION( "\\Aabc\\Z", PCRE_MULTILINE );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "abc\n " );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("qqq\nabc");
SHOULD_NOT_MATCH("abc\nzzz");
SHOULD_NOT_MATCH("qqq\nabc\nzzz");


EXPRESSION( "\\A(.)*\\Z", PCRE_DOTALL );
SHOULD_MATCH( 2, "abc\ndef" );
MATCH( 0, "abc\ndef" );
MATCH( 1, "f" );


EXPRESSION( "(?:b)|(?::+)", 0 );
SHOULD_MATCH( 1, "b::c" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "c::b" );
MATCH( 0, "::" );

#endif

EXPRESSION( "[-az]+", 0 );
SHOULD_MATCH( 1, "az-" );
MATCH( 0, "az-" );
SHOULD_NOT_MATCH("b");


EXPRESSION( "[az-]+", 0 );
SHOULD_MATCH( 1, "za-" );
MATCH( 0, "za-" );
SHOULD_NOT_MATCH("b");


EXPRESSION( "[a\\-z]+", 0 );
SHOULD_MATCH( 1, "a-z" );
MATCH( 0, "a-z" );
SHOULD_NOT_MATCH("b");


EXPRESSION( "[a-z]+", 0 );
SHOULD_MATCH( 1, "abcdxyz" );
MATCH( 0, "abcdxyz" );


EXPRESSION( "[\\d-]+", 0 );
SHOULD_MATCH( 1, "12-34" );
MATCH( 0, "12-34" );
SHOULD_NOT_MATCH("aaa");


EXPRESSION( "[\\d-z]+", 0 );
SHOULD_MATCH( 1, "12-34z" );
MATCH( 0, "12-34z" );
SHOULD_NOT_MATCH("aaa");

#ifdef REGEX_FULL

EXPRESSION( "\x5c", 0 );
SHOULD_MATCH( 1, "\\" );
MATCH( 0, "" );


EXPRESSION( "\x20Z", 0 );
SHOULD_MATCH( 1, "the Zoo" );
MATCH( 0, " Z" );
SHOULD_NOT_MATCH("Zulu");


EXPRESSION( "(abc)\1", PCRE_CASELESS );
SHOULD_MATCH( 2, "abcabc" );
MATCH( 0, "abcabc" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "ABCabc" );
MATCH( 0, "ABCabc" );
MATCH( 1, "ABC" );
SHOULD_MATCH( 2, "abcABC" );
MATCH( 0, "abcABC" );
MATCH( 1, "abc" );


EXPRESSION( "ab{3cd", 0 );
SHOULD_MATCH( 1, "ab{3cd" );
MATCH( 0, "ab{3cd" );


EXPRESSION( "ab{3,cd", 0 );
SHOULD_MATCH( 1, "ab{3,cd" );
MATCH( 0, "ab{3,cd" );


EXPRESSION( "ab{3,4a}cd", 0 );
SHOULD_MATCH( 1, "ab{3,4a}cd" );
MATCH( 0, "ab{3,4a}cd" );


EXPRESSION( "{4,5a}bc", 0 );
SHOULD_MATCH( 1, "{4,5a}bc" );
MATCH( 0, "{4,5a}bc" );


EXPRESSION( "^a.b", 0 );
SHOULD_MATCH( 1, "a\rb" );
MATCH( 0, "a\x0db" );
SHOULD_NOT_MATCH("a\nb");


EXPRESSION( "abc$", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "abc\n" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("abc\ndef");


EXPRESSION( "(abc)\123", 0 );
SHOULD_MATCH( 2, "abc\x53" );
MATCH( 0, "abcS" );
MATCH( 1, "abc" );


EXPRESSION( "(abc)\\223", 0 );
SHOULD_MATCH( 2, "abc\x93" );
MATCH( 0, "abc\x93" );
MATCH( 1, "abc" );


EXPRESSION( "(abc)\\323", 0 );
SHOULD_MATCH( 2, "abc\xd3" );
MATCH( 0, "abc\xd3" );
MATCH( 1, "abc" );


EXPRESSION( "(abc)\\@", 0 );
SHOULD_MATCH( 2, "abc\x40" );
MATCH( 0, "abc@" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abc\100" );
MATCH( 0, "abc@" );
MATCH( 1, "abc" );


EXPRESSION( "(abc)\\@0", 0 );
SHOULD_MATCH( 2, "abc@0" );
MATCH( 0, "abc@0" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abc\x40\x30" );
MATCH( 0, "abc@0" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abc\1000" );
MATCH( 0, "abc@0" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abc\100\x30" );
MATCH( 0, "abc@0" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abc\100\\060" );
MATCH( 0, "abc@0" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abc\100\60" );
MATCH( 0, "abc@0" );
MATCH( 1, "abc" );


EXPRESSION( "abc\\81", 0 );
SHOULD_MATCH( 1, "abc\\081" );
MATCH( 0, "abc\x0081" );
SHOULD_MATCH( 1, "abc\\0\x38\x31" );
MATCH( 0, "abc\x0081" );


EXPRESSION( "abc\\91", 0 );
SHOULD_MATCH( 1, "abc\\091" );
MATCH( 0, "abc\x0091" );
SHOULD_MATCH( 1, "abc\\0\x39\x31" );
MATCH( 0, "abc\x0091" );


EXPRESSION( "(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)(l)\12\123", 0 );
SHOULD_MATCH( 13, "abcdefghijkllS" );
MATCH( 0, "abcdefghijkllS" );
MATCH( 1, "a" );
MATCH( 2, "b" );
MATCH( 3, "c" );
MATCH( 4, "d" );
MATCH( 5, "e" );
MATCH( 6, "f" );
MATCH( 7, "g" );
MATCH( 8, "h" );
MATCH( 9, "i" );
MATCH( 10, "j" );
MATCH( 11, "k" );
MATCH( 12, "l" );


EXPRESSION( "(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)\12\123", 0 );
SHOULD_MATCH( 12, "abcdefghijk\12S" );
MATCH( 0, "abcdefghijk\nS" );
MATCH( 1, "a" );
MATCH( 2, "b" );
MATCH( 3, "c" );
MATCH( 4, "d" );
MATCH( 5, "e" );
MATCH( 6, "f" );
MATCH( 7, "g" );
MATCH( 8, "h" );
MATCH( 9, "i" );
MATCH( 10, "j" );
MATCH( 11, "k" );


EXPRESSION( "ab\\gdef", 0 );
SHOULD_MATCH( 1, "abgdef" );
MATCH( 0, "abgdef" );


EXPRESSION( "a{0}bc", 0 );
SHOULD_MATCH( 1, "bc" );
MATCH( 0, "bc" );

#endif

EXPRESSION( "(a|(bc)){0,0}?xyz", 0 );
SHOULD_MATCH( 1, "xyz" );
MATCH( 0, "xyz" );


//EXPRESSION( "abc[\10]de", 0 );
//SHOULD_MATCH( 1, "abc\\010de" );
//MATCH( 0, "abc\x08de" );


//EXPRESSION( "abc[\1]de", 0 );
//SHOULD_MATCH( 1, "abc\1de" );
//MATCH( 0, "abc\x01de" );


//EXPRESSION( "(abc)[\1]de", 0 );
//SHOULD_MATCH( 1, "abc\1de" );
//MATCH( 0, "abc\x01de" );
//MATCH( 1, "abc" );


#ifdef REGEX_FULL

EXPRESSION( "(?s)a.b", 0 );
SHOULD_MATCH( 1, "a\nb" );
MATCH( 0, "a\nb" );


EXPRESSION( "^([^a])([^\b])([^c]*)([^d]{3,4})", 0 );
SHOULD_MATCH( 5, "baNOTccccd" );
MATCH( 0, "baNOTcccc" );
MATCH( 1, "b" );
MATCH( 2, "a" );
MATCH( 3, "NOT" );
MATCH( 4, "cccc" );
SHOULD_MATCH( 5, "baNOTcccd" );
MATCH( 0, "baNOTccc" );
MATCH( 1, "b" );
MATCH( 2, "a" );
MATCH( 3, "NOT" );
MATCH( 4, "ccc" );
SHOULD_MATCH( 5, "baNOTccd" );
MATCH( 0, "baNOTcc" );
MATCH( 1, "b" );
MATCH( 2, "a" );
MATCH( 3, "NO" );
MATCH( 4, "Tcc" );
SHOULD_MATCH( 5, "bacccd" );
MATCH( 0, "baccc" );
MATCH( 1, "b" );
MATCH( 2, "a" );
MATCH( 3, "" );
MATCH( 4, "ccc" );
SHOULD_NOT_MATCH("anything");
SHOULD_NOT_MATCH("b\bc   ");
SHOULD_NOT_MATCH("baccd");


EXPRESSION( "[^a]", 0 );
SHOULD_MATCH( 1, "Abc" );
MATCH( 0, "A" );


EXPRESSION( "[^a]", PCRE_CASELESS );
SHOULD_MATCH( 1, "Abc " );
MATCH( 0, "b" );


EXPRESSION( "[^a]+", 0 );
SHOULD_MATCH( 1, "AAAaAbc" );
MATCH( 0, "AAA" );


EXPRESSION( "[^a]+", PCRE_CASELESS );
SHOULD_MATCH( 1, "AAAaAbc " );
MATCH( 0, "bc" );


EXPRESSION( "[^a]+", 0 );
SHOULD_MATCH( 1, "bbb\nccc" );
MATCH( 0, "bbb\nccc" );


EXPRESSION( "[^k]$", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "c" );
SHOULD_NOT_MATCH("abk   ");


EXPRESSION( "[^k]{2,3}$", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "kbc" );
MATCH( 0, "bc" );
SHOULD_MATCH( 1, "kabc " );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("abk");
SHOULD_NOT_MATCH("akb");
SHOULD_NOT_MATCH("akk ");


EXPRESSION( "^\\d{8,}\\@.+[^k]$", 0 );
SHOULD_MATCH( 1, "12345678\\@a.b.c.d" );
MATCH( 0, "12345678@a.b.c.d" );
SHOULD_MATCH( 1, "123456789\\@x.y.z" );
MATCH( 0, "123456789@x.y.z" );
SHOULD_NOT_MATCH("12345678\\@x.y.uk");
SHOULD_NOT_MATCH("1234567\\@a.b.c.d       ");


EXPRESSION( "(a)\1{8,}", 0 );
SHOULD_MATCH( 2, "aaaaaaaaa" );
MATCH( 0, "aaaaaaaaa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaaaaaaaaa" );
MATCH( 0, "aaaaaaaaaa" );
MATCH( 1, "a" );
SHOULD_NOT_MATCH("aaaaaaa   ");


EXPRESSION( "[^a]", 0 );
SHOULD_MATCH( 1, "aaaabcd" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "aaAabcd " );
MATCH( 0, "A" );


EXPRESSION( "[^a]", PCRE_CASELESS );
SHOULD_MATCH( 1, "aaaabcd" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "aaAabcd " );
MATCH( 0, "b" );


EXPRESSION( "[^az]", 0 );
SHOULD_MATCH( 1, "aaaabcd" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "aaAabcd " );
MATCH( 0, "A" );


EXPRESSION( "[^az]", PCRE_CASELESS );
SHOULD_MATCH( 1, "aaaabcd" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "aaAabcd " );
MATCH( 0, "b" );


EXPRESSION( "\\000\\001\\002\\003\\004\\005\\006\\007\\010\\011\\012\\013\\014\\015\\016\\017\\020\\021\\022\\023\\024\\025\\026\\027\\030\\031\\032\\033\\034\\035\\036\\037\\040\\041\\042\\043\\044\\045\\046\\047\\050\\051\\052\\053\\054\\055\\056\\057\\060\\061\\062\\063\\064\\065\\066\\067\\070\\071\\072\\073\\074\\075\\076\\077\100\101\102\103\104\105\106\107\110\111\112\113\114\115\116\117\120\121\122\123\124\125\126\127\130\131\132\133\134\135\136\137\140\141\142\143\144\145\146\147\150\151\152\153\154\155\156\157\160\161\162\163\164\165\166\167\170\171\172\173\174\175\176\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377", 0 );
SHOULD_MATCH( 1, "\\000\\001\\002\\003\\004\\005\\006\\007\\010\\011\\012\\013\\014\\015\\016\\017\\020\\021\\022\\023\\024\\025\\026\\027\\030\\031\\032\\033\\034\\035\\036\\037\\040\\041\\042\\043\\044\\045\\046\\047\\050\\051\\052\\053\\054\\055\\056\\057\\060\\061\\062\\063\\064\\065\\066\\067\\070\\071\\072\\073\\074\\075\\076\\077\100\101\102\103\104\105\106\107\110\111\112\113\114\115\116\117\120\121\122\123\124\125\126\127\130\131\132\133\134\135\136\137\140\141\142\143\144\145\146\147\150\151\152\153\154\155\156\157\160\161\162\163\164\165\166\167\170\171\172\173\174\175\176\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377" );
MATCH( 0, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\n\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff" );


EXPRESSION( "P[^*]TAIRE[^*]{1,6}?LL", 0 );
SHOULD_MATCH( 1, "xxxxxxxxxxxPSTAIREISLLxxxxxxxxx" );
MATCH( 0, "PSTAIREISLL" );


EXPRESSION( "P[^*]TAIRE[^*]{1,}?LL", 0 );
SHOULD_MATCH( 1, "xxxxxxxxxxxPSTAIREISLLxxxxxxxxx" );
MATCH( 0, "PSTAIREISLL" );

#endif

EXPRESSION( "(\\.\\d\\d[1-9]?)\\d+", 0 );
SHOULD_MATCH( 2, "1.230003938" );
MATCH( 0, ".230003938" );
MATCH( 1, ".23" );
SHOULD_MATCH( 2, "1.875000282   " );
MATCH( 0, ".875000282" );
MATCH( 1, ".875" );
SHOULD_MATCH( 2, "1.235  " );
MATCH( 0, ".235" );
MATCH( 1, ".23" );


EXPRESSION( "(\\.\\d\\d((?=0)|\\d(?=\\d)))", 0 );
SHOULD_MATCH( 3, "1.230003938      " );
MATCH( 0, ".23" );
MATCH( 1, ".23" );
MATCH( 2, "" );
SHOULD_MATCH( 3, "1.875000282" );
MATCH( 0, ".875" );
MATCH( 1, ".875" );
MATCH( 2, "5" );
SHOULD_NOT_MATCH("1.235 ");

#ifdef REGEX_FULL

EXPRESSION( "a(?)b", 0 );
SHOULD_MATCH( 1, "ab " );
MATCH( 0, "ab" );


EXPRESSION( "\b(foo)\\s+(\\w+)", PCRE_CASELESS );
SHOULD_MATCH( 3, "Food is on the foo table" );
MATCH( 0, "foo table" );
MATCH( 1, "foo" );
MATCH( 2, "table" );


EXPRESSION( "foo(.*)bar", 0 );
SHOULD_MATCH( 2, "The food is under the bar in the barn." );
MATCH( 0, "food is under the bar in the bar" );
MATCH( 1, "d is under the bar in the " );


EXPRESSION( "foo(.*?)bar", 0 );
SHOULD_MATCH( 2, "The food is under the bar in the barn." );
MATCH( 0, "food is under the bar" );
MATCH( 1, "d is under the " );


EXPRESSION( "(.*)(\\d*)", 0 );
SHOULD_MATCH( 3, "I have 2 numbers: 53147" );
MATCH( 0, "I have 2 numbers: 53147" );
MATCH( 1, "I have 2 numbers: 53147" );
MATCH( 2, "" );


EXPRESSION( "(.*)(\\d+)", 0 );
SHOULD_MATCH( 3, "I have 2 numbers: 53147" );
MATCH( 0, "I have 2 numbers: 53147" );
MATCH( 1, "I have 2 numbers: 5314" );
MATCH( 2, "7" );


EXPRESSION( "(.*?)(\\d*)", 0 );
SHOULD_MATCH( 3, "I have 2 numbers: 53147" );
MATCH( 0, "" );
MATCH( 1, "" );
MATCH( 2, "" );


EXPRESSION( "(.*?)(\\d+)", 0 );
SHOULD_MATCH( 3, "I have 2 numbers: 53147" );
MATCH( 0, "I have 2" );
MATCH( 1, "I have " );
MATCH( 2, "2" );


EXPRESSION( "(.*)(\\d+)$", 0 );
SHOULD_MATCH( 3, "I have 2 numbers: 53147" );
MATCH( 0, "I have 2 numbers: 53147" );
MATCH( 1, "I have 2 numbers: 5314" );
MATCH( 2, "7" );


EXPRESSION( "(.*?)(\\d+)$", 0 );
SHOULD_MATCH( 3, "I have 2 numbers: 53147" );
MATCH( 0, "I have 2 numbers: 53147" );
MATCH( 1, "I have 2 numbers: " );
MATCH( 2, "53147" );


EXPRESSION( "(.*)\b(\\d+)$", 0 );
SHOULD_MATCH( 3, "I have 2 numbers: 53147" );
MATCH( 0, "I have 2 numbers: 53147" );
MATCH( 1, "I have 2 numbers: " );
MATCH( 2, "53147" );


EXPRESSION( "(.*\\D)(\\d+)$", 0 );
SHOULD_MATCH( 3, "I have 2 numbers: 53147" );
MATCH( 0, "I have 2 numbers: 53147" );
MATCH( 1, "I have 2 numbers: " );
MATCH( 2, "53147" );


EXPRESSION( "^\\D*(?!123)", 0 );
SHOULD_MATCH( 1, "ABC123" );
MATCH( 0, "AB" );


EXPRESSION( "^(\\D*)(?=\\d)(?!123)", 0 );
SHOULD_MATCH( 2, "ABC445" );
MATCH( 0, "ABC" );
MATCH( 1, "ABC" );
SHOULD_NOT_MATCH("ABC123");


EXPRESSION( "^[W-]46]", 0 );
SHOULD_MATCH( 1, "W46]789 " );
MATCH( 0, "W46]" );
SHOULD_MATCH( 1, "-46]789" );
MATCH( 0, "-46]" );
SHOULD_NOT_MATCH("Wall");
SHOULD_NOT_MATCH("Zebra");
SHOULD_NOT_MATCH("42");
SHOULD_NOT_MATCH("[abcd] ");
SHOULD_NOT_MATCH("]abcd[");


EXPRESSION( "^[W-\\]46]", 0 );
SHOULD_MATCH( 1, "W46]789 " );
MATCH( 0, "W" );
SHOULD_MATCH( 1, "Wall" );
MATCH( 0, "W" );
SHOULD_MATCH( 1, "Zebra" );
MATCH( 0, "Z" );
SHOULD_MATCH( 1, "Xylophone  " );
MATCH( 0, "X" );
SHOULD_MATCH( 1, "42" );
MATCH( 0, "4" );
SHOULD_MATCH( 1, "[abcd] " );
MATCH( 0, "[" );
SHOULD_MATCH( 1, "]abcd[" );
MATCH( 0, "]" );
SHOULD_MATCH( 1, "\\backslash " );
MATCH( 0, "" );
SHOULD_NOT_MATCH("-46]789");
SHOULD_NOT_MATCH("well");

#endif

EXPRESSION( "\\d\\d\\/\\d\\d\\/\\d\\d\\d\\d", 0 );
SHOULD_MATCH( 1, "01/01/2000" );
MATCH( 0, "01/01/2000" );

#ifdef REGEX_FULL

EXPRESSION( "word (?:[a-zA-Z0-9]+ ){0,10}otherword", 0 );
SHOULD_MATCH( 1, "word cat dog elephant mussel cow horse canary baboon snake shark otherword" );
MATCH( 0, "word cat dog elephant mussel cow horse canary baboon snake shark otherword" );
SHOULD_NOT_MATCH( "word cat dog elephant mussel cow horse canary baboon snake shark" );


EXPRESSION( "word (?:[a-zA-Z0-9]+ ){0,300}otherword", 0 );
SHOULD_NOT_MATCH( "word cat dog elephant mussel cow horse canary baboon snake shark the quick brown fox and the lazy dog and several other words getting close to thirty by now I hope" );


EXPRESSION( "^(a){0,0}", 0 );
SHOULD_MATCH( 1, "bcd" );
MATCH( 0, "" );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "" );
SHOULD_MATCH( 1, "aab     " );
MATCH( 0, "" );


EXPRESSION( "^(a){0,1}", 0 );
SHOULD_MATCH( 1, "bcd" );
MATCH( 0, "" );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab  " );
MATCH( 0, "a" );
MATCH( 1, "a" );


EXPRESSION( "^(a){0,2}", 0 );
SHOULD_MATCH( 1, "bcd" );
MATCH( 0, "" );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab  " );
MATCH( 0, "aa" );
MATCH( 1, "a" );


EXPRESSION( "^(a){0,3}", 0 );
SHOULD_MATCH( 1, "bcd" );
MATCH( 0, "" );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab" );
MATCH( 0, "aa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaa   " );
MATCH( 0, "aaa" );
MATCH( 1, "a" );


EXPRESSION( "^(a){0,}", 0 );
SHOULD_MATCH( 1, "bcd" );
MATCH( 0, "" );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab" );
MATCH( 0, "aa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaa" );
MATCH( 0, "aaa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaaaaaaa    " );
MATCH( 0, "aaaaaaaa" );
MATCH( 1, "a" );


EXPRESSION( "^(a){1,1}", 0 );
SHOULD_NOT_MATCH("bcd");
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab  " );
MATCH( 0, "a" );
MATCH( 1, "a" );


EXPRESSION( "^(a){1,2}", 0 );
SHOULD_NOT_MATCH("bcd");
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab  " );
MATCH( 0, "aa" );
MATCH( 1, "a" );


EXPRESSION( "^(a){1,3}", 0 );
SHOULD_NOT_MATCH("bcd");
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab" );
MATCH( 0, "aa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaa   " );
MATCH( 0, "aaa" );
MATCH( 1, "a" );


EXPRESSION( "^(a){1,}", 0 );
SHOULD_NOT_MATCH("bcd");
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aab" );
MATCH( 0, "aa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaa" );
MATCH( 0, "aaa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaaaaaaa    " );
MATCH( 0, "aaaaaaaa" );
MATCH( 1, "a" );

#endif

EXPRESSION( ".*\\.gif", 0 );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "bib.gif" );

#ifdef REGEX_FULL

EXPRESSION( ".{0,}\\.gif", 0 );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "bib.gif" );


EXPRESSION( ".*\\.gif", PCRE_MULTILINE );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "bib.gif" );


EXPRESSION( ".*\\.gif", PCRE_DOTALL );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "borfle\nbib.gif" );


EXPRESSION( ".*\\.gif", PCRE_DOTALL + PCRE_MULTILINE );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "borfle\nbib.gif" );


EXPRESSION( ".*$", 0 );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "no" );


EXPRESSION( ".*$", PCRE_MULTILINE );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "borfle" );


EXPRESSION( ".*$", PCRE_DOTALL );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "borfle\nbib.gif\nno" );


EXPRESSION( ".*$", PCRE_DOTALL + PCRE_MULTILINE );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno" );
MATCH( 0, "borfle\nbib.gif\nno" );


EXPRESSION( ".*$", 0 );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno\n" );
MATCH( 0, "no" );


EXPRESSION( ".*$", PCRE_MULTILINE );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno\n" );
MATCH( 0, "borfle" );


EXPRESSION( ".*$", PCRE_DOTALL );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno\n" );
MATCH( 0, "borfle\nbib.gif\nno\n" );


EXPRESSION( ".*$", PCRE_DOTALL + PCRE_MULTILINE );
SHOULD_MATCH( 1, "borfle\nbib.gif\nno\n" );
MATCH( 0, "borfle\nbib.gif\nno\n" );


EXPRESSION( "(.*X|^B)", 0 );
SHOULD_MATCH( 2, "abcde\n1234Xyz" );
MATCH( 0, "1234X" );
MATCH( 1, "1234X" );
SHOULD_MATCH( 2, "BarFoo " );
MATCH( 0, "B" );
MATCH( 1, "B" );
SHOULD_NOT_MATCH("abcde\nBar  ");


EXPRESSION( "(.*X|^B)", PCRE_MULTILINE );
SHOULD_MATCH( 2, "abcde\n1234Xyz" );
MATCH( 0, "1234X" );
MATCH( 1, "1234X" );
SHOULD_MATCH( 2, "BarFoo " );
MATCH( 0, "B" );
MATCH( 1, "B" );
SHOULD_MATCH( 2, "abcde\nBar  " );
MATCH( 0, "B" );
MATCH( 1, "B" );


EXPRESSION( "(.*X|^B)", PCRE_DOTALL );
SHOULD_MATCH( 2, "abcde\n1234Xyz" );
MATCH( 0, "abcde\n1234X" );
MATCH( 1, "abcde\n1234X" );
SHOULD_MATCH( 2, "BarFoo " );
MATCH( 0, "B" );
MATCH( 1, "B" );
SHOULD_NOT_MATCH("abcde\nBar  ");


EXPRESSION( "(.*X|^B)", PCRE_DOTALL + PCRE_MULTILINE );
SHOULD_MATCH( 2, "abcde\n1234Xyz" );
MATCH( 0, "abcde\n1234X" );
MATCH( 1, "abcde\n1234X" );
SHOULD_MATCH( 2, "BarFoo " );
MATCH( 0, "B" );
MATCH( 1, "B" );
SHOULD_MATCH( 2, "abcde\nBar  " );
MATCH( 0, "B" );
MATCH( 1, "B" );


EXPRESSION( "(?s)(.*X|^B)", 0 );
SHOULD_MATCH( 2, "abcde\n1234Xyz" );
MATCH( 0, "abcde\n1234X" );
MATCH( 1, "abcde\n1234X" );
SHOULD_MATCH( 2, "BarFoo " );
MATCH( 0, "B" );
MATCH( 1, "B" );
SHOULD_NOT_MATCH("abcde\nBar  ");


EXPRESSION( "(?s:.*X|^B)", 0 );
SHOULD_MATCH( 1, "abcde\n1234Xyz" );
MATCH( 0, "abcde\n1234X" );
SHOULD_MATCH( 1, "BarFoo " );
MATCH( 0, "B" );
SHOULD_NOT_MATCH("abcde\nBar  ");


EXPRESSION( "^.*B", 0 );
SHOULD_NOT_MATCH("abc\nB");


EXPRESSION( "(?s)^.*B", 0 );
SHOULD_MATCH( 1, "abc\nB" );
MATCH( 0, "abc\nB" );


EXPRESSION( "(?m)^.*B", 0 );
SHOULD_MATCH( 1, "abc\nB" );
MATCH( 0, "B" );


EXPRESSION( "(?ms)^.*B", 0 );
SHOULD_MATCH( 1, "abc\nB" );
MATCH( 0, "abc\nB" );


EXPRESSION( "(?ms)^B", 0 );
SHOULD_MATCH( 1, "abc\nB" );
MATCH( 0, "B" );


EXPRESSION( "(?s)B$", 0 );
SHOULD_MATCH( 1, "B\n" );
MATCH( 0, "B" );

#endif

EXPRESSION( "^[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]", 0 );
SHOULD_MATCH( 1, "123456654321" );
MATCH( 0, "123456654321" );


EXPRESSION( "^\\d\\d\\d\\d\\d\\d\\d\\d\\d\\d\\d\\d", 0 );
SHOULD_MATCH( 1, "123456654321 " );
MATCH( 0, "123456654321" );


EXPRESSION( "^[\\d][\\d][\\d][\\d][\\d][\\d][\\d][\\d][\\d][\\d][\\d][\\d]", 0 );
SHOULD_MATCH( 1, "123456654321" );
MATCH( 0, "123456654321" );


EXPRESSION( "^[abc]{12}", 0 );
SHOULD_MATCH( 1, "abcabcabcabc" );
MATCH( 0, "abcabcabcabc" );


EXPRESSION( "^[a-c]{12}", 0 );
SHOULD_MATCH( 1, "abcabcabcabc" );
MATCH( 0, "abcabcabcabc" );


EXPRESSION( "^(a|b|c){12}", 0 );
SHOULD_MATCH( 2, "abcabcabcabc " );
MATCH( 0, "abcabcabcabc" );
MATCH( 1, "c" );


EXPRESSION( "^[abcdefghijklmnopqrstuvwxy0123456789]", 0 );
SHOULD_MATCH( 1, "n" );
MATCH( 0, "n" );
SHOULD_NOT_MATCH("z ");

#ifdef REGEX_FULL

EXPRESSION( "abcde{0,0}", 0 );
SHOULD_MATCH( 1, "abcd" );
MATCH( 0, "abcd" );
SHOULD_NOT_MATCH("abce  ");


EXPRESSION( "ab[cd]{0,0}e", 0 );
SHOULD_MATCH( 1, "abe" );
MATCH( 0, "abe" );
SHOULD_NOT_MATCH("abcde ");


EXPRESSION( "ab(c){0,0}d", 0 );
SHOULD_MATCH( 1, "abd" );
MATCH( 0, "abd" );
SHOULD_NOT_MATCH("abcd   ");


EXPRESSION( "a(b*)", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "a" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "b" );
SHOULD_MATCH( 2, "abbbb" );
MATCH( 0, "abbbb" );
MATCH( 1, "bbbb" );
SHOULD_NOT_MATCH("bbbbb    ");


EXPRESSION( "ab\\d{0}e", 0 );
SHOULD_MATCH( 1, "abe" );
MATCH( 0, "abe" );
SHOULD_NOT_MATCH("ab1e   ");


EXPRESSION( "\"([^\\\"]+|\\.)*\"", 0 );
SHOULD_MATCH( 2, "the \"quick\" brown fox" );
MATCH( 0, "\"quick\"" );
MATCH( 1, "quick" );
SHOULD_MATCH( 2, "\"the \\\"quick\\\" brown fox\" " );
MATCH( 0, "\"the \"quick\" brown fox\"" );
MATCH( 1, " brown fox" );
// another section of very involved stuff removed here

#endif

EXPRESSION( "a[^a]b", 0 );
SHOULD_MATCH( 1, "acb" );
MATCH( 0, "acb" );
SHOULD_MATCH( 1, "a\nb" );
MATCH( 0, "a\nb" );


EXPRESSION( "a.b", 0 );
SHOULD_MATCH( 1, "acb" );
MATCH( 0, "acb" );
SHOULD_NOT_MATCH("a\nb   ");


EXPRESSION( "a[^a]b", PCRE_DOTALL );
SHOULD_MATCH( 1, "acb" );
MATCH( 0, "acb" );
SHOULD_MATCH( 1, "a\nb  " );
MATCH( 0, "a\nb" );


EXPRESSION( "a.b", PCRE_DOTALL );
SHOULD_MATCH( 1, "acb" );
MATCH( 0, "acb" );
SHOULD_MATCH( 1, "a\nb  " );
MATCH( 0, "a\nb" );

#ifdef REGEX_FULL

EXPRESSION( "^(b+?|a){1,2}?c", 0 );
SHOULD_MATCH( 2, "bac" );
MATCH( 0, "bac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbac" );
MATCH( 0, "bbac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbbac" );
MATCH( 0, "bbbac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbbbac" );
MATCH( 0, "bbbbac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbbbbac " );
MATCH( 0, "bbbbbac" );
MATCH( 1, "a" );


EXPRESSION( "^(b+|a){1,2}?c", 0 );
SHOULD_MATCH( 2, "bac" );
MATCH( 0, "bac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbac" );
MATCH( 0, "bbac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbbac" );
MATCH( 0, "bbbac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbbbac" );
MATCH( 0, "bbbbac" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "bbbbbac " );
MATCH( 0, "bbbbbac" );
MATCH( 1, "a" );


EXPRESSION( "(?!\\A)x", PCRE_MULTILINE );
SHOULD_NOT_MATCH("x\nb\n");
SHOULD_MATCH( 1, "a\bx\n  " );
MATCH( 0, "x" );


EXPRESSION( "\x0{ab}", 0 );
SHOULD_MATCH( 1, "\\0{ab} " );
MATCH( 0, "\x00{ab}" );


EXPRESSION( "(A|B)*?CD", 0 );
SHOULD_MATCH( 1, "CD " );
MATCH( 0, "CD" );


EXPRESSION( "(A|B)*CD", 0 );
SHOULD_MATCH( 1, "CD " );
MATCH( 0, "CD" );


EXPRESSION( "(AB)*?\1", 0 );
SHOULD_MATCH( 2, "ABABAB" );
MATCH( 0, "ABAB" );
MATCH( 1, "AB" );


EXPRESSION( "(AB)*\1", 0 );
SHOULD_MATCH( 2, "ABABAB" );
MATCH( 0, "ABABAB" );
MATCH( 1, "AB" );


EXPRESSION( "(?<!bar)foo", 0 );
SHOULD_MATCH( 1, "foo" );
MATCH( 0, "foo" );
SHOULD_MATCH( 1, "catfood" );
MATCH( 0, "foo" );
SHOULD_MATCH( 1, "arfootle" );
MATCH( 0, "foo" );
SHOULD_MATCH( 1, "rfoosh" );
MATCH( 0, "foo" );
SHOULD_NOT_MATCH("barfoo");
SHOULD_NOT_MATCH("towbarfoo");


EXPRESSION( "\\w{3}(?<!bar)foo", 0 );
SHOULD_MATCH( 1, "catfood" );
MATCH( 0, "catfoo" );
SHOULD_NOT_MATCH("foo");
SHOULD_NOT_MATCH("barfoo");
SHOULD_NOT_MATCH("towbarfoo");


EXPRESSION( "(?<=(foo)a)bar", 0 );
SHOULD_MATCH( 2, "fooabar" );
MATCH( 0, "bar" );
MATCH( 1, "foo" );
SHOULD_NOT_MATCH("bar");
SHOULD_NOT_MATCH("foobbar");


EXPRESSION( "\\Aabc\\z", PCRE_MULTILINE );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("abc\n   ");
SHOULD_NOT_MATCH("qqq\nabc");
SHOULD_NOT_MATCH("abc\nzzz");
SHOULD_NOT_MATCH("qqq\nabc\nzzz");

EXPRESSION( "(?>.*/)foo", 0 );
SHOULD_NOT_MATCH("/this/is/a/very/long/line/in/deed/with/very/many/slashes/in/it/you/see/");

EXPRESSION( "(?>.*/)foo", 0 );
SHOULD_MATCH( 1, "/this/is/a/very/long/line/in/deed/with/very/many/slashes/in/and/foo" );
MATCH( 0, "/this/is/a/very/long/line/in/deed/with/very/many/slashes/in/and/foo" );


EXPRESSION( "(?>(\\.\\d\\d[1-9]?))\\d+", 0 );
SHOULD_MATCH( 2, "1.230003938" );
MATCH( 0, ".230003938" );
MATCH( 1, ".23" );
SHOULD_MATCH( 2, "1.875000282" );
MATCH( 0, ".875000282" );
MATCH( 1, ".875" );
SHOULD_NOT_MATCH("1.235 ");


EXPRESSION( "^((?>\\w+)|(?>\\s+))*$", 0 );
SHOULD_MATCH( 2, "now is the time for all good men to come to the aid of the party" );
MATCH( 0, "now is the time for all good men to come to the aid of the party" );
MATCH( 1, "party" );
SHOULD_NOT_MATCH("this is not a line with only words and spaces!");


EXPRESSION( "(\\d+)(\\w)", 0 );
SHOULD_MATCH( 3, "12345a" );
MATCH( 0, "12345a" );
MATCH( 1, "12345" );
MATCH( 2, "a" );
SHOULD_MATCH( 3, "12345+ " );
MATCH( 0, "12345" );
MATCH( 1, "1234" );
MATCH( 2, "5" );


EXPRESSION( "((?>\\d+))(\\w)", 0 );
SHOULD_MATCH( 3, "12345a" );
MATCH( 0, "12345a" );
MATCH( 1, "12345" );
MATCH( 2, "a" );
SHOULD_NOT_MATCH("12345+ ");


EXPRESSION( "(?>a+)b", 0 );
SHOULD_MATCH( 1, "aaab" );
MATCH( 0, "aaab" );


EXPRESSION( "((?>a+)b)", 0 );
SHOULD_MATCH( 2, "aaab" );
MATCH( 0, "aaab" );
MATCH( 1, "aaab" );


EXPRESSION( "(?>(a+))b", 0 );
SHOULD_MATCH( 2, "aaab" );
MATCH( 0, "aaab" );
MATCH( 1, "aaa" );


EXPRESSION( "(?>b)+", 0 );
SHOULD_MATCH( 1, "aaabbbccc" );
MATCH( 0, "bbb" );


EXPRESSION( "(?>a+|b+|c+)*c", 0 );
SHOULD_MATCH( 1, "aaabbbbccccd" );
MATCH( 0, "aaabbbbc" );


EXPRESSION( "((?>[^()]+)|\\([^()]*\\))+", 0 );
SHOULD_MATCH( 2, "((abc(ade)ufh()()x" );
MATCH( 0, "abc(ade)ufh()()x" );
MATCH( 1, "x" );


EXPRESSION( "\\(((?>[^()]+)|\\([^()]+\\))+\\)", 0 );
SHOULD_MATCH( 2, "(abc)" );
MATCH( 0, "(abc)" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "(abc(def)xyz)" );
MATCH( 0, "(abc(def)xyz)" );
MATCH( 1, "xyz" );
SHOULD_NOT_MATCH("((()aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa   ");


EXPRESSION( "a(?-i)b", PCRE_CASELESS );
SHOULD_MATCH( 1, "ab" );
MATCH( 0, "ab" );
SHOULD_MATCH( 1, "Ab" );
MATCH( 0, "Ab" );
SHOULD_NOT_MATCH("aB");
SHOULD_NOT_MATCH("AB");


EXPRESSION( "(a (?x)b c)d e", 0 );
SHOULD_MATCH( 2, "a bcd e" );
MATCH( 0, "a bcd e" );
MATCH( 1, "a bc" );
SHOULD_NOT_MATCH("a b cd e");
SHOULD_NOT_MATCH("abcd e   ");
SHOULD_NOT_MATCH("a bcde ");


EXPRESSION( "(a b(?x)c d (?-x)e f)", 0 );
SHOULD_MATCH( 2, "a bcde f" );
MATCH( 0, "a bcde f" );
MATCH( 1, "a bcde f" );
SHOULD_NOT_MATCH("abcdef  ");


EXPRESSION( "(a(?i)b)c", 0 );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "abc" );
MATCH( 1, "ab" );
SHOULD_MATCH( 2, "aBc" );
MATCH( 0, "aBc" );
MATCH( 1, "aB" );
SHOULD_NOT_MATCH("abC");
SHOULD_NOT_MATCH("aBC  ");
SHOULD_NOT_MATCH("Abc");
SHOULD_NOT_MATCH("ABc");
SHOULD_NOT_MATCH("ABC");
SHOULD_NOT_MATCH("AbC");


EXPRESSION( "a(?i:b)c", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "aBc" );
MATCH( 0, "aBc" );
SHOULD_NOT_MATCH("ABC");
SHOULD_NOT_MATCH("abC");
SHOULD_NOT_MATCH("aBC");


EXPRESSION( "a(?i:b)*c", 0 );
SHOULD_MATCH( 1, "aBc" );
MATCH( 0, "aBc" );
SHOULD_MATCH( 1, "aBBc" );
MATCH( 0, "aBBc" );
SHOULD_NOT_MATCH("aBC");
SHOULD_NOT_MATCH("aBBC");


EXPRESSION( "a(?=b(?i)c)\\w\\wd", 0 );
SHOULD_MATCH( 1, "abcd" );
MATCH( 0, "abcd" );
SHOULD_MATCH( 1, "abCd" );
MATCH( 0, "abCd" );
SHOULD_NOT_MATCH("aBCd");
SHOULD_NOT_MATCH("abcD     ");


EXPRESSION( "(?s-i:more.*than).*million", PCRE_CASELESS );
SHOULD_MATCH( 1, "more than million" );
MATCH( 0, "more than million" );
SHOULD_MATCH( 1, "more than MILLION" );
MATCH( 0, "more than MILLION" );
SHOULD_MATCH( 1, "more \n than Million " );
MATCH( 0, "more \n than Million" );
SHOULD_NOT_MATCH("MORE THAN MILLION    ");
SHOULD_NOT_MATCH("more \n than \n million ");


EXPRESSION( "(?:(?s-i)more.*than).*million", PCRE_CASELESS );
SHOULD_MATCH( 1, "more than million" );
MATCH( 0, "more than million" );
SHOULD_MATCH( 1, "more than MILLION" );
MATCH( 0, "more than MILLION" );
SHOULD_MATCH( 1, "more \n than Million " );
MATCH( 0, "more \n than Million" );
SHOULD_NOT_MATCH("MORE THAN MILLION    ");
SHOULD_NOT_MATCH("more \n than \n million ");


EXPRESSION( "(?>a(?i)b+)+c", 0);
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "aBbc" );
MATCH( 0, "aBbc" );
SHOULD_MATCH( 1, "aBBc " );
MATCH( 0, "aBBc" );
SHOULD_NOT_MATCH("Abc");
SHOULD_NOT_MATCH("abAb    ");
SHOULD_NOT_MATCH("abbC ");


EXPRESSION( "(?=a(?i)b)\\w\\wc", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "aBc" );
MATCH( 0, "aBc" );
SHOULD_NOT_MATCH("Ab ");
SHOULD_NOT_MATCH("abC");
SHOULD_NOT_MATCH("aBC     ");


EXPRESSION( "(?<=a(?i)b)(\\w\\w)c", 0 );
SHOULD_MATCH( 2, "abxxc" );
MATCH( 0, "xxc" );
MATCH( 1, "xx" );
SHOULD_MATCH( 2, "aBxxc" );
MATCH( 0, "xxc" );
MATCH( 1, "xx" );
SHOULD_NOT_MATCH("Abxxc");
SHOULD_NOT_MATCH("ABxxc");
SHOULD_NOT_MATCH("abxxC      ");


EXPRESSION( "(?:(a)|b)(?(1)A|B)", 0 );
SHOULD_MATCH( 2, "aA" );
MATCH( 0, "aA" );
MATCH( 1, "a" );
SHOULD_MATCH( 1, "bB" );
MATCH( 0, "bB" );
SHOULD_NOT_MATCH("aB");
SHOULD_NOT_MATCH("bA    ");


EXPRESSION( "^(a)?(?(1)a|b)+$", 0 );
SHOULD_MATCH( 2, "aa" );
MATCH( 0, "aa" );
MATCH( 1, "a" );
SHOULD_MATCH( 1, "b" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "bb  " );
MATCH( 0, "bb" );
SHOULD_NOT_MATCH("ab   ");


EXPRESSION( "^(?(?=abc)\\w{3}:|\\d\\d)$", 0 );
SHOULD_MATCH( 1, "abc:" );
MATCH( 0, "abc:" );
SHOULD_MATCH( 1, "12" );
MATCH( 0, "12" );
SHOULD_NOT_MATCH("123");
SHOULD_NOT_MATCH("xyz    ");


EXPRESSION( "^(?(?!abc)\\d\\d|\\w{3}:)$", 0 );
SHOULD_MATCH( 1, "abc:" );
MATCH( 0, "abc:" );
SHOULD_MATCH( 1, "12" );
MATCH( 0, "12" );
SHOULD_NOT_MATCH("123");
SHOULD_NOT_MATCH("xyz    ");


EXPRESSION( "(?(?<=foo)bar|cat)", 0 );
SHOULD_MATCH( 1, "foobar" );
MATCH( 0, "bar" );
SHOULD_MATCH( 1, "cat" );
MATCH( 0, "cat" );
SHOULD_MATCH( 1, "fcat" );
MATCH( 0, "cat" );
SHOULD_MATCH( 1, "focat   " );
MATCH( 0, "cat" );
SHOULD_NOT_MATCH("foocat  ");


EXPRESSION( "(?(?<!foo)cat|bar)", 0 );
SHOULD_MATCH( 1, "foobar" );
MATCH( 0, "bar" );
SHOULD_MATCH( 1, "cat" );
MATCH( 0, "cat" );
SHOULD_MATCH( 1, "fcat" );
MATCH( 0, "cat" );
SHOULD_MATCH( 1, "focat   " );
MATCH( 0, "cat" );
SHOULD_NOT_MATCH("foocat  ");


EXPRESSION( "( \\( )? [^()]+ (?(1) \\) |) ", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abcd" );
MATCH( 0, "abcd" );
SHOULD_MATCH( 2, "(abcd)" );
MATCH( 0, "(abcd)" );
MATCH( 1, "(" );
SHOULD_MATCH( 1, "the quick (abcd) fox" );
MATCH( 0, "the quick " );
SHOULD_MATCH( 1, "(abcd   " );
MATCH( 0, "abcd" );


EXPRESSION( "( \\( )? [^()]+ (?(1) \\) ) ", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abcd" );
MATCH( 0, "abcd" );
SHOULD_MATCH( 2, "(abcd)" );
MATCH( 0, "(abcd)" );
MATCH( 1, "(" );
SHOULD_MATCH( 1, "the quick (abcd) fox" );
MATCH( 0, "the quick " );
SHOULD_MATCH( 1, "(abcd   " );
MATCH( 0, "abcd" );


EXPRESSION( "^(?(2)a|(1)(2))+$", 0 );
SHOULD_MATCH( 3, "12" );
MATCH( 0, "12" );
MATCH( 1, "1" );
MATCH( 2, "2" );
SHOULD_MATCH( 3, "12a" );
MATCH( 0, "12a" );
MATCH( 1, "1" );
MATCH( 2, "2" );
SHOULD_MATCH( 3, "12aa" );
MATCH( 0, "12aa" );
MATCH( 1, "1" );
MATCH( 2, "2" );
SHOULD_NOT_MATCH("1234    ");


EXPRESSION( "((?i)blah)\\s+\1", 0 );
SHOULD_MATCH( 2, "blah blah" );
MATCH( 0, "blah blah" );
MATCH( 1, "blah" );
SHOULD_MATCH( 2, "BLAH BLAH" );
MATCH( 0, "BLAH BLAH" );
MATCH( 1, "BLAH" );
SHOULD_MATCH( 2, "Blah Blah" );
MATCH( 0, "Blah Blah" );
MATCH( 1, "Blah" );
SHOULD_MATCH( 2, "blaH blaH" );
MATCH( 0, "blaH blaH" );
MATCH( 1, "blaH" );
SHOULD_NOT_MATCH("blah BLAH");
SHOULD_NOT_MATCH("Blah blah      ");
SHOULD_NOT_MATCH("blaH blah ");


EXPRESSION( "((?i)blah)\\s+(?i:\1)", 0 );
SHOULD_MATCH( 2, "blah blah" );
MATCH( 0, "blah blah" );
MATCH( 1, "blah" );
SHOULD_MATCH( 2, "BLAH BLAH" );
MATCH( 0, "BLAH BLAH" );
MATCH( 1, "BLAH" );
SHOULD_MATCH( 2, "Blah Blah" );
MATCH( 0, "Blah Blah" );
MATCH( 1, "Blah" );
SHOULD_MATCH( 2, "blaH blaH" );
MATCH( 0, "blaH blaH" );
MATCH( 1, "blaH" );
SHOULD_MATCH( 2, "blah BLAH" );
MATCH( 0, "blah BLAH" );
MATCH( 1, "blah" );
SHOULD_MATCH( 2, "Blah blah      " );
MATCH( 0, "Blah blah" );
MATCH( 1, "Blah" );
SHOULD_MATCH( 2, "blaH blah " );
MATCH( 0, "blaH blah" );
MATCH( 1, "blaH" );


EXPRESSION( "(?>a*)*", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "aa" );
MATCH( 0, "aa" );
SHOULD_MATCH( 1, "aaaa" );
MATCH( 0, "aaaa" );


EXPRESSION( "(abc|)+", 0 );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "abc" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "abcabc" );
MATCH( 0, "abcabc" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "abcabcabc" );
MATCH( 0, "abcabcabc" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "xyz      " );
MATCH( 0, "" );
MATCH( 1, "" );

#endif

EXPRESSION( "([a]*)*", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "a" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "aaaaa " );
MATCH( 0, "aaaaa" );
MATCH( 1, "" );


EXPRESSION( "([ab]*)*", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "a" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "b" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "ababab" );
MATCH( 0, "ababab" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "aaaabcde" );
MATCH( 0, "aaaab" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "bbbb    " );
MATCH( 0, "bbbb" );
MATCH( 1, "" );


EXPRESSION( "([^a]*)*", 0 );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "b" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "bbbb" );
MATCH( 0, "bbbb" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "aaa   " );
MATCH( 0, "" );
MATCH( 1, "" );


EXPRESSION( "([^ab]*)*", 0 );
SHOULD_MATCH( 2, "cccc" );
MATCH( 0, "cccc" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "abab  " );
MATCH( 0, "" );
MATCH( 1, "" );


EXPRESSION( "([a]*?)*", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "aaaa " );
MATCH( 0, "" );
MATCH( 1, "" );


EXPRESSION( "([ab]*?)*", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "abab" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "baba   " );
MATCH( 0, "" );
MATCH( 1, "" );


EXPRESSION( "([^a]*?)*", 0 );
SHOULD_MATCH( 2, "b" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "bbbb" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "aaa   " );
MATCH( 0, "" );
MATCH( 1, "" );


EXPRESSION( "([^ab]*?)*", 0 );
SHOULD_MATCH( 2, "c" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "cccc" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "baba   " );
MATCH( 0, "" );
MATCH( 1, "" );

#ifdef REGEX_FULL

EXPRESSION( "(?>a*)*", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "aaabcde " );
MATCH( 0, "aaa" );


EXPRESSION( "((?>a*))*", 0 );
SHOULD_MATCH( 2, "aaaaa" );
MATCH( 0, "aaaaa" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "aabbaa " );
MATCH( 0, "aa" );
MATCH( 1, "" );


EXPRESSION( "((?>a*?))*", 0 );
SHOULD_MATCH( 2, "aaaaa" );
MATCH( 0, "" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "aabbaa " );
MATCH( 0, "" );
MATCH( 1, "" );


EXPRESSION( "(?(?=[^a-z]+[a-z])  \\d{2}-[a-z]{3}-\\d{2}  |  \\d{2}-\\d{2}-\\d{2} ) ", PCRE_EXTENDED );
SHOULD_MATCH( 1, "12-sep-98" );
MATCH( 0, "12-sep-98" );
SHOULD_MATCH( 1, "12-09-98" );
MATCH( 0, "12-09-98" );
SHOULD_NOT_MATCH("sep-12-98");


EXPRESSION( "(?<=(foo))bar\1", 0 );
SHOULD_MATCH( 2, "foobarfoo" );
MATCH( 0, "barfoo" );
MATCH( 1, "foo" );
SHOULD_MATCH( 2, "foobarfootling " );
MATCH( 0, "barfoo" );
MATCH( 1, "foo" );
SHOULD_NOT_MATCH("foobar");
SHOULD_NOT_MATCH("barfoo   ");


EXPRESSION( "(?i:saturday|sunday)", 0 );
SHOULD_MATCH( 1, "saturday" );
MATCH( 0, "saturday" );
SHOULD_MATCH( 1, "sunday" );
MATCH( 0, "sunday" );
SHOULD_MATCH( 1, "Saturday" );
MATCH( 0, "Saturday" );
SHOULD_MATCH( 1, "Sunday" );
MATCH( 0, "Sunday" );
SHOULD_MATCH( 1, "SATURDAY" );
MATCH( 0, "SATURDAY" );
SHOULD_MATCH( 1, "SUNDAY" );
MATCH( 0, "SUNDAY" );
SHOULD_MATCH( 1, "SunDay" );
MATCH( 0, "SunDay" );


EXPRESSION( "(a(?i)bc|BB)x", 0 );
SHOULD_MATCH( 2, "abcx" );
MATCH( 0, "abcx" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "aBCx" );
MATCH( 0, "aBCx" );
MATCH( 1, "aBC" );
SHOULD_MATCH( 2, "bbx" );
MATCH( 0, "bbx" );
MATCH( 1, "bb" );
SHOULD_MATCH( 2, "BBx" );
MATCH( 0, "BBx" );
MATCH( 1, "BB" );
SHOULD_NOT_MATCH("abcX");
SHOULD_NOT_MATCH("aBCX");
SHOULD_NOT_MATCH("bbX");
SHOULD_NOT_MATCH("BBX               ");


EXPRESSION( "^([ab](?i)[cd]|[ef])", 0 );
SHOULD_MATCH( 2, "ac" );
MATCH( 0, "ac" );
MATCH( 1, "ac" );
SHOULD_MATCH( 2, "aC" );
MATCH( 0, "aC" );
MATCH( 1, "aC" );
SHOULD_MATCH( 2, "bD" );
MATCH( 0, "bD" );
MATCH( 1, "bD" );
SHOULD_MATCH( 2, "elephant" );
MATCH( 0, "e" );
MATCH( 1, "e" );
SHOULD_MATCH( 2, "Europe " );
MATCH( 0, "E" );
MATCH( 1, "E" );
SHOULD_MATCH( 2, "frog" );
MATCH( 0, "f" );
MATCH( 1, "f" );
SHOULD_MATCH( 2, "France" );
MATCH( 0, "F" );
MATCH( 1, "F" );
SHOULD_NOT_MATCH("Africa     ");


EXPRESSION( "^(ab|a(?i)[b-c](?m-i)d|x(?i)y|z)", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "ab" );
SHOULD_MATCH( 2, "aBd" );
MATCH( 0, "aBd" );
MATCH( 1, "aBd" );
SHOULD_MATCH( 2, "xy" );
MATCH( 0, "xy" );
MATCH( 1, "xy" );
SHOULD_MATCH( 2, "xY" );
MATCH( 0, "xY" );
MATCH( 1, "xY" );
SHOULD_MATCH( 2, "zebra" );
MATCH( 0, "z" );
MATCH( 1, "z" );
SHOULD_MATCH( 2, "Zambesi" );
MATCH( 0, "Z" );
MATCH( 1, "Z" );
SHOULD_NOT_MATCH("aCD  ");
SHOULD_NOT_MATCH("XY  ");


EXPRESSION( "(?<=foo\n)^bar", PCRE_MULTILINE );
SHOULD_MATCH( 1, "foo\nbar" );
MATCH( 0, "bar" );
SHOULD_NOT_MATCH("bar");
SHOULD_NOT_MATCH("baz\nbar   ");


EXPRESSION( "(?<=(?<!foo)bar)baz", 0 );
SHOULD_MATCH( 1, "barbaz" );
MATCH( 0, "baz" );
SHOULD_MATCH( 1, "barbarbaz " );
MATCH( 0, "baz" );
SHOULD_MATCH( 1, "koobarbaz " );
MATCH( 0, "baz" );
SHOULD_NOT_MATCH("baz");
SHOULD_NOT_MATCH("foobarbaz ");


EXPRESSION( "The case of aaaaaa is missed out below because I think Perl 5.005_02 gets", 0 );
SHOULD_NOT_MATCH( "it wrong; it sets $1 to aaa rather than aa. Compare the following test," );
SHOULD_NOT_MATCH( "where it does set $1 to aa when matching aaaaaa." );


EXPRESSION( "^(a\1?){4}$", 0 );
SHOULD_NOT_MATCH("a");
SHOULD_NOT_MATCH("aa");
SHOULD_NOT_MATCH("aaa");
SHOULD_MATCH( 2, "aaaa" );
MATCH( 0, "aaaa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaaaa" );
MATCH( 0, "aaaaa" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "aaaaaaa" );
MATCH( 0, "aaaaaaa" );
MATCH( 1, "a" );
SHOULD_NOT_MATCH("aaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaa");
SHOULD_MATCH( 2, "aaaaaaaaaa" );
MATCH( 0, "aaaaaaaaaa" );
MATCH( 1, "aaaa" );
SHOULD_NOT_MATCH("aaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaaaaaa               ");


EXPRESSION( "^(a\1?)(a\1?)(a\\2?)(a\3?)$", 0 );
SHOULD_NOT_MATCH("a");
SHOULD_NOT_MATCH("aa");
SHOULD_NOT_MATCH("aaa");
SHOULD_MATCH( 5, "aaaa" );
MATCH( 0, "aaaa" );
MATCH( 1, "a" );
MATCH( 2, "a" );
MATCH( 3, "a" );
MATCH( 4, "a" );
SHOULD_MATCH( 5, "aaaaa" );
MATCH( 0, "aaaaa" );
MATCH( 1, "a" );
MATCH( 2, "aa" );
MATCH( 3, "a" );
MATCH( 4, "a" );
SHOULD_MATCH( 5, "aaaaaa" );
MATCH( 0, "aaaaaa" );
MATCH( 1, "a" );
MATCH( 2, "aa" );
MATCH( 3, "a" );
MATCH( 4, "aa" );
SHOULD_MATCH( 5, "aaaaaaa" );
MATCH( 0, "aaaaaaa" );
MATCH( 1, "a" );
MATCH( 2, "aa" );
MATCH( 3, "aaa" );
MATCH( 4, "a" );
SHOULD_NOT_MATCH("aaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaa");
SHOULD_MATCH( 5, "aaaaaaaaaa" );
MATCH( 0, "aaaaaaaaaa" );
MATCH( 1, "a" );
MATCH( 2, "aa" );
MATCH( 3, "aaa" );
MATCH( 4, "aaaa" );
SHOULD_NOT_MATCH("aaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaaaaaaa               ");


EXPRESSION( "abc", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "xabcy" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "ababc" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("xbc");
SHOULD_NOT_MATCH("axc");
SHOULD_NOT_MATCH("abx");


EXPRESSION( "ab*c", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );


EXPRESSION( "ab*bc", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "abbc" );
MATCH( 0, "abbc" );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "abbbbc" );


EXPRESSION( ".{1}", 0 );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "a" );


EXPRESSION( ".{3,4}", 0 );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "abbb" );


EXPRESSION( "ab{0,}bc", 0 );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "abbbbc" );


EXPRESSION( "ab+bc", 0 );
SHOULD_MATCH( 1, "abbc" );
MATCH( 0, "abbc" );
SHOULD_NOT_MATCH("abc");
SHOULD_NOT_MATCH("abq");


EXPRESSION( "ab{1,}bc", 0 );


EXPRESSION( "ab+bc", 0 );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "abbbbc" );


EXPRESSION( "ab{1,}bc", 0 );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "abbbbc" );


EXPRESSION( "ab{1,3}bc", 0 );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "abbbbc" );


EXPRESSION( "ab{3,4}bc", 0 );
SHOULD_MATCH( 1, "abbbbc" );
MATCH( 0, "abbbbc" );


EXPRESSION( "ab{4,5}bc", 0 );
SHOULD_NOT_MATCH("abq");
SHOULD_NOT_MATCH("abbbbc");


EXPRESSION( "ab?bc", 0 );
SHOULD_MATCH( 1, "abbc" );
MATCH( 0, "abbc" );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );


EXPRESSION( "ab{0,1}bc", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );


EXPRESSION( "ab?bc", 0 );


EXPRESSION( "ab?c", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );


EXPRESSION( "ab{0,1}c", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );


EXPRESSION( "^abc$", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("abbbbc");
SHOULD_NOT_MATCH("abcc");


EXPRESSION( "^abc", 0 );
SHOULD_MATCH( 1, "abcc" );
MATCH( 0, "abc" );


EXPRESSION( "^abc$", 0 );


EXPRESSION( "abc$", 0 );
SHOULD_MATCH( 1, "aabc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "aabc" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("aabcd");


EXPRESSION( "^", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "" );


EXPRESSION( "$", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "" );


EXPRESSION( "a.c", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_MATCH( 1, "axc" );
MATCH( 0, "axc" );


EXPRESSION( "a.*c", 0 );
SHOULD_MATCH( 1, "axyzc" );
MATCH( 0, "axyzc" );


EXPRESSION( "a[bc]d", 0 );
SHOULD_MATCH( 1, "abd" );
MATCH( 0, "abd" );
SHOULD_NOT_MATCH("axyzd");
SHOULD_NOT_MATCH("abc");


EXPRESSION( "a[b-d]e", 0 );
SHOULD_MATCH( 1, "ace" );
MATCH( 0, "ace" );


EXPRESSION( "a[b-d]", 0 );
SHOULD_MATCH( 1, "aac" );
MATCH( 0, "ac" );


EXPRESSION( "a[-b]", 0 );
SHOULD_MATCH( 1, "a-" );
MATCH( 0, "a-" );


EXPRESSION( "a[b-]", 0 );
SHOULD_MATCH( 1, "a-" );
MATCH( 0, "a-" );


EXPRESSION( "a]", 0 );
SHOULD_MATCH( 1, "a]" );
MATCH( 0, "a]" );


EXPRESSION( "a[]]b", 0 );
SHOULD_MATCH( 1, "a]b" );
MATCH( 0, "a]b" );


EXPRESSION( "a[^bc]d", 0 );
SHOULD_MATCH( 1, "aed" );
MATCH( 0, "aed" );
SHOULD_NOT_MATCH("abd");
SHOULD_NOT_MATCH("abd");


EXPRESSION( "a[^-b]c", 0 );
SHOULD_MATCH( 1, "adc" );
MATCH( 0, "adc" );


EXPRESSION( "a[^]b]c", 0 );
SHOULD_MATCH( 1, "adc" );
MATCH( 0, "adc" );
SHOULD_MATCH( 1, "a-c" );
MATCH( 0, "a-c" );
SHOULD_NOT_MATCH("a]c");


EXPRESSION( "\ba\b", 0 );
SHOULD_MATCH( 1, "a-" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "-a" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "-a-" );
MATCH( 0, "a" );


EXPRESSION( "\by\b", 0 );
SHOULD_NOT_MATCH("xy");
SHOULD_NOT_MATCH("yz");
SHOULD_NOT_MATCH("xyz");


EXPRESSION( "\\Ba\\B", 0 );
SHOULD_NOT_MATCH("a-");
SHOULD_NOT_MATCH("-a");
SHOULD_NOT_MATCH("-a-");


EXPRESSION( "\\By\b", 0 );
SHOULD_MATCH( 1, "xy" );
MATCH( 0, "y" );


EXPRESSION( "\by\\B", 0 );
SHOULD_MATCH( 1, "yz" );
MATCH( 0, "y" );


EXPRESSION( "\\By\\B", 0 );
SHOULD_MATCH( 1, "xyz" );
MATCH( 0, "y" );


EXPRESSION( "\\w", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );


EXPRESSION( "\\W", 0 );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_NOT_MATCH("a");


EXPRESSION( "a\\sb", 0 );
SHOULD_MATCH( 1, "a b" );
MATCH( 0, "a b" );


EXPRESSION( "a\\Sb", 0 );
SHOULD_MATCH( 1, "a-b" );
MATCH( 0, "a-b" );
SHOULD_MATCH( 1, "a-b" );
MATCH( 0, "a-b" );
SHOULD_NOT_MATCH("a b");


EXPRESSION( "\\d", 0 );
SHOULD_MATCH( 1, "1" );
MATCH( 0, "1" );


EXPRESSION( "\\D", 0 );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_NOT_MATCH("1");


EXPRESSION( "[\\w]", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );


EXPRESSION( "[\\W]", 0 );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_NOT_MATCH("a");


EXPRESSION( "a[\\s]b", 0 );
SHOULD_MATCH( 1, "a b" );
MATCH( 0, "a b" );


EXPRESSION( "a[\\S]b", 0 );
SHOULD_MATCH( 1, "a-b" );
MATCH( 0, "a-b" );
SHOULD_MATCH( 1, "a-b" );
MATCH( 0, "a-b" );
SHOULD_NOT_MATCH("a b");


EXPRESSION( "[\\d]", 0 );
SHOULD_MATCH( 1, "1" );
MATCH( 0, "1" );


EXPRESSION( "[\\D]", 0 );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_NOT_MATCH("1");


EXPRESSION( "ab|cd", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "ab" );
SHOULD_MATCH( 1, "abcd" );
MATCH( 0, "ab" );


EXPRESSION( "()ef", 0 );
SHOULD_MATCH( 2, "def" );
MATCH( 0, "ef" );
MATCH( 1, "" );


EXPRESSION( "$b", 0 );


EXPRESSION( "a\\(b", 0 );
SHOULD_MATCH( 1, "a(b" );
MATCH( 0, "a(b" );


EXPRESSION( "a\\(*b", 0 );
SHOULD_MATCH( 1, "ab" );
MATCH( 0, "ab" );
SHOULD_MATCH( 1, "a((b" );
MATCH( 0, "a((b" );


EXPRESSION( "a\\b", 0 );
SHOULD_NOT_MATCH("a\b");


EXPRESSION( "((a))", 0 );
SHOULD_MATCH( 3, "abc" );
MATCH( 0, "a" );
MATCH( 1, "a" );
MATCH( 2, "a" );


EXPRESSION( "(a)b(c)", 0 );
SHOULD_MATCH( 3, "abc" );
MATCH( 0, "abc" );
MATCH( 1, "a" );
MATCH( 2, "c" );


EXPRESSION( "a+b+c", 0 );
SHOULD_MATCH( 1, "aabbabc" );
MATCH( 0, "abc" );


EXPRESSION( "a{1,}b{1,}c", 0 );
SHOULD_MATCH( 1, "aabbabc" );
MATCH( 0, "abc" );


EXPRESSION( "a.+?c", 0 );
SHOULD_MATCH( 1, "abcabc" );
MATCH( 0, "abc" );


EXPRESSION( "(a+|b)*", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "b" );


EXPRESSION( "(a+|b){0,}", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "b" );


EXPRESSION( "(a+|b)+", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "b" );


EXPRESSION( "(a+|b){1,}", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "b" );


EXPRESSION( "(a+|b)?", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "a" );
MATCH( 1, "a" );


EXPRESSION( "(a+|b){0,1}", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "a" );
MATCH( 1, "a" );


EXPRESSION( "[^ab]*", 0 );
SHOULD_MATCH( 1, "cde" );
MATCH( 0, "cde" );


EXPRESSION( "abc", 0 );
SHOULD_NOT_MATCH("b");



EXPRESSION( "a*", 0 );



EXPRESSION( "([abc])*d", 0 );
SHOULD_MATCH( 2, "abbbcd" );
MATCH( 0, "abbbcd" );
MATCH( 1, "c" );


EXPRESSION( "([abc])*bcd", 0 );
SHOULD_MATCH( 2, "abcd" );
MATCH( 0, "abcd" );
MATCH( 1, "a" );


EXPRESSION( "a|b|c|d|e", 0 );
SHOULD_MATCH( 1, "e" );
MATCH( 0, "e" );


EXPRESSION( "(a|b|c|d|e)f", 0 );
SHOULD_MATCH( 2, "ef" );
MATCH( 0, "ef" );
MATCH( 1, "e" );


EXPRESSION( "abcd*efg", 0 );
SHOULD_MATCH( 1, "abcdefg" );
MATCH( 0, "abcdefg" );


EXPRESSION( "ab*", 0 );
SHOULD_MATCH( 1, "xabyabbbz" );
MATCH( 0, "ab" );
SHOULD_MATCH( 1, "xayabbbz" );
MATCH( 0, "a" );


EXPRESSION( "(ab|cd)e", 0 );
SHOULD_MATCH( 2, "abcde" );
MATCH( 0, "cde" );
MATCH( 1, "cd" );


EXPRESSION( "[abhgefdc]ij", 0 );
SHOULD_MATCH( 1, "hij" );
MATCH( 0, "hij" );


EXPRESSION( "^(ab|cd)e", 0 );


EXPRESSION( "(abc|)ef", 0 );
SHOULD_MATCH( 2, "abcdef" );
MATCH( 0, "ef" );
MATCH( 1, "" );


EXPRESSION( "(a|b)c*d", 0 );
SHOULD_MATCH( 2, "abcd" );
MATCH( 0, "bcd" );
MATCH( 1, "b" );


EXPRESSION( "(ab|ab*)bc", 0 );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "abc" );
MATCH( 1, "a" );


EXPRESSION( "a([bc]*)c*", 0 );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "abc" );
MATCH( 1, "bc" );


EXPRESSION( "a([bc]*)(c*d)", 0 );
SHOULD_MATCH( 3, "abcd" );
MATCH( 0, "abcd" );
MATCH( 1, "bc" );
MATCH( 2, "d" );


EXPRESSION( "a([bc]+)(c*d)", 0 );
SHOULD_MATCH( 3, "abcd" );
MATCH( 0, "abcd" );
MATCH( 1, "bc" );
MATCH( 2, "d" );


EXPRESSION( "a([bc]*)(c+d)", 0 );
SHOULD_MATCH( 3, "abcd" );
MATCH( 0, "abcd" );
MATCH( 1, "b" );
MATCH( 2, "cd" );


EXPRESSION( "a[bcd]*dcdcde", 0 );
SHOULD_MATCH( 1, "adcdcde" );
MATCH( 0, "adcdcde" );


EXPRESSION( "a[bcd]+dcdcde", 0 );
SHOULD_NOT_MATCH("abcde");
SHOULD_NOT_MATCH("adcdcde");


EXPRESSION( "(ab|a)b*c", 0 );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "abc" );
MATCH( 1, "ab" );


EXPRESSION( "((a)(b)c)(d)", 0 );
SHOULD_MATCH( 5, "abcd" );
MATCH( 0, "abcd" );
MATCH( 1, "abc" );
MATCH( 2, "a" );
MATCH( 3, "b" );
MATCH( 4, "d" );


EXPRESSION( "[a-zA-Z_][a-zA-Z0-9_]*", 0 );
SHOULD_MATCH( 1, "alpha" );
MATCH( 0, "alpha" );


EXPRESSION( "^a(bc+|b[eh])g|.h$", 0 );
SHOULD_MATCH( 1, "abh" );
MATCH( 0, "bh" );


EXPRESSION( "(bc+d$|ef*g.|h?i(j|k))", 0 );
SHOULD_MATCH( 2, "effgz" );
MATCH( 0, "effgz" );
MATCH( 1, "effgz" );
SHOULD_MATCH( 3, "ij" );
MATCH( 0, "ij" );
MATCH( 1, "ij" );
MATCH( 2, "j" );
SHOULD_MATCH( 2, "reffgz" );
MATCH( 0, "effgz" );
MATCH( 1, "effgz" );
SHOULD_NOT_MATCH("effg");
SHOULD_NOT_MATCH("bcdd");


EXPRESSION( "((((((((((a))))))))))", 0 );
SHOULD_MATCH( 11, "a" );
MATCH( 0, "a" );
MATCH( 1, "a" );
MATCH( 2, "a" );
MATCH( 3, "a" );
MATCH( 4, "a" );
MATCH( 5, "a" );
MATCH( 6, "a" );
MATCH( 7, "a" );
MATCH( 8, "a" );
MATCH( 9, "a" );
MATCH( 10, "a" );


EXPRESSION( "((((((((((a))))))))))\10", 0 );
SHOULD_MATCH( 11, "aa" );
MATCH( 0, "aa" );
MATCH( 1, "a" );
MATCH( 2, "a" );
MATCH( 3, "a" );
MATCH( 4, "a" );
MATCH( 5, "a" );
MATCH( 6, "a" );
MATCH( 7, "a" );
MATCH( 8, "a" );
MATCH( 9, "a" );
MATCH( 10, "a" );


EXPRESSION( "(((((((((a)))))))))", 0 );
SHOULD_MATCH( 10, "a" );
MATCH( 0, "a" );
MATCH( 1, "a" );
MATCH( 2, "a" );
MATCH( 3, "a" );
MATCH( 4, "a" );
MATCH( 5, "a" );
MATCH( 6, "a" );
MATCH( 7, "a" );
MATCH( 8, "a" );
MATCH( 9, "a" );


EXPRESSION( "multiple words of text", 0 );
SHOULD_NOT_MATCH("aa");
SHOULD_NOT_MATCH("uh-uh");


EXPRESSION( "multiple words", 0 );
SHOULD_MATCH( 1, "multiple words, yeah" );
MATCH( 0, "multiple words" );


EXPRESSION( "(.*)c(.*)", 0 );
SHOULD_MATCH( 3, "abcde" );
MATCH( 0, "abcde" );
MATCH( 1, "ab" );
MATCH( 2, "de" );


EXPRESSION( "\\((.*), (.*)\\)", 0 );
SHOULD_MATCH( 3, "(a, b)" );
MATCH( 0, "(a, b)" );
MATCH( 1, "a" );
MATCH( 2, "b" );

#endif

EXPRESSION( "abcd", 0 );
SHOULD_MATCH( 1, "abcd" );
MATCH( 0, "abcd" );


EXPRESSION( "a(bc)d", 0 );
SHOULD_MATCH( 2, "abcd" );
MATCH( 0, "abcd" );
MATCH( 1, "bc" );


EXPRESSION( "a[-]?c", 0 );
SHOULD_MATCH( 1, "ac" );
MATCH( 0, "ac" );


EXPRESSION( "(abc)\1", 0 );
SHOULD_MATCH( 2, "abcabc" );
MATCH( 0, "abcabc" );
MATCH( 1, "abc" );


EXPRESSION( "([a-c]*)\1", 0 );
SHOULD_MATCH( 2, "abcabc" );
MATCH( 0, "abcabc" );
MATCH( 1, "abc" );


EXPRESSION( "(a)|\1", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "a" );
MATCH( 1, "a" );
SHOULD_NOT_MATCH("x");

#ifdef REGEX_FULL

EXPRESSION( "(([a-c])b*?\\2)*", 0 );
SHOULD_MATCH( 3, "ababbbcbc" );
MATCH( 0, "ababb" );
MATCH( 1, "bb" );
MATCH( 2, "b" );


EXPRESSION( "(([a-c])b*?\\2){3}", 0 );
SHOULD_MATCH( 4, "ababbbcbc" );
MATCH( 0, "ababbbcbc" );
MATCH( 1, "cbc" );
MATCH( 2, "c" );


EXPRESSION( "((\3|b)\\2(a)x)+", 0 );
SHOULD_MATCH( 4, "aaaxabaxbaaxbbax" );
MATCH( 0, "bbax" );
MATCH( 1, "bbax" );
MATCH( 2, "b" );
MATCH( 3, "a" );


EXPRESSION( "((\3|b)\\2(a)){2,}", 0 );
SHOULD_MATCH( 4, "bbaababbabaaaaabbaaaabba" );
MATCH( 0, "bbaaaabba" );
MATCH( 1, "bba" );
MATCH( 2, "b" );
MATCH( 3, "a" );

#endif

EXPRESSION( "abc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );
SHOULD_MATCH( 1, "XABCY" );
MATCH( 0, "ABC" );
SHOULD_MATCH( 1, "ABABC" );
MATCH( 0, "ABC" );
SHOULD_NOT_MATCH("aaxabxbaxbbx");
SHOULD_NOT_MATCH("XBC");
SHOULD_NOT_MATCH("AXC");
SHOULD_NOT_MATCH("ABX");


EXPRESSION( "ab*c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );


EXPRESSION( "ab*bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );
SHOULD_MATCH( 1, "ABBC" );
MATCH( 0, "ABBC" );


EXPRESSION( "ab*?bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABBBBC" );
MATCH( 0, "ABBBBC" );


EXPRESSION( "ab{0,}?bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABBBBC" );
MATCH( 0, "ABBBBC" );


EXPRESSION( "ab+?bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABBC" );
MATCH( 0, "ABBC" );


EXPRESSION( "ab+bc", PCRE_CASELESS );
SHOULD_NOT_MATCH("ABC");
SHOULD_NOT_MATCH("ABQ");


EXPRESSION( "ab+bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABBBBC" );
MATCH( 0, "ABBBBC" );


EXPRESSION( "ab{1,}?bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABBBBC" );
MATCH( 0, "ABBBBC" );


EXPRESSION( "ab{1,3}?bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABBBBC" );
MATCH( 0, "ABBBBC" );


EXPRESSION( "ab{3,4}?bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABBBBC" );
MATCH( 0, "ABBBBC" );


EXPRESSION( "ab{4,5}?bc", PCRE_CASELESS );
SHOULD_NOT_MATCH("ABQ");
SHOULD_NOT_MATCH("ABBBBC");


EXPRESSION( "ab??bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABBC" );
MATCH( 0, "ABBC" );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );


EXPRESSION( "ab{0,1}?bc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );


EXPRESSION( "ab??bc", PCRE_CASELESS );


EXPRESSION( "ab??c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );


EXPRESSION( "ab{0,1}?c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );


EXPRESSION( "^abc$", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );
SHOULD_NOT_MATCH("ABBBBC");
SHOULD_NOT_MATCH("ABCC");


EXPRESSION( "^abc", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABCC" );
MATCH( 0, "ABC" );


EXPRESSION( "^abc$", PCRE_CASELESS );


EXPRESSION( "abc$", PCRE_CASELESS );
SHOULD_MATCH( 1, "AABC" );
MATCH( 0, "ABC" );


EXPRESSION( "^", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "" );


EXPRESSION( "$", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "" );


EXPRESSION( "a.c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "ABC" );
SHOULD_MATCH( 1, "AXC" );
MATCH( 0, "AXC" );


EXPRESSION( "a.*?c", PCRE_CASELESS );
SHOULD_MATCH( 1, "AXYZC" );
MATCH( 0, "AXYZC" );


EXPRESSION( "a.*c", PCRE_CASELESS );
SHOULD_MATCH( 1, "AABC" );
MATCH( 0, "AABC" );
SHOULD_NOT_MATCH("AXYZD");


EXPRESSION( "a[bc]d", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABD" );
MATCH( 0, "ABD" );


EXPRESSION( "a[b-d]e", PCRE_CASELESS );
SHOULD_MATCH( 1, "ACE" );
MATCH( 0, "ACE" );
SHOULD_NOT_MATCH("ABC");
SHOULD_NOT_MATCH("ABD");


EXPRESSION( "a[b-d]", PCRE_CASELESS );
SHOULD_MATCH( 1, "AAC" );
MATCH( 0, "AC" );


EXPRESSION( "a[-b]", PCRE_CASELESS );
SHOULD_MATCH( 1, "A-" );
MATCH( 0, "A-" );


EXPRESSION( "a[b-]", PCRE_CASELESS );
SHOULD_MATCH( 1, "A-" );
MATCH( 0, "A-" );


EXPRESSION( "a]", PCRE_CASELESS );
SHOULD_MATCH( 1, "A]" );
MATCH( 0, "A]" );


EXPRESSION( "a[]]b", PCRE_CASELESS );
SHOULD_MATCH( 1, "A]B" );
MATCH( 0, "A]B" );


EXPRESSION( "a[^bc]d", PCRE_CASELESS );
SHOULD_MATCH( 1, "AED" );
MATCH( 0, "AED" );


EXPRESSION( "a[^-b]c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ADC" );
MATCH( 0, "ADC" );
SHOULD_NOT_MATCH("ABD");
SHOULD_NOT_MATCH("A-C");


EXPRESSION( "a[^]b]c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ADC" );
MATCH( 0, "ADC" );


EXPRESSION( "ab|cd", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABC" );
MATCH( 0, "AB" );
SHOULD_MATCH( 1, "ABCD" );
MATCH( 0, "AB" );


EXPRESSION( "()ef", PCRE_CASELESS );
SHOULD_MATCH( 2, "DEF" );
MATCH( 0, "EF" );
MATCH( 1, "" );


EXPRESSION( "$b", PCRE_CASELESS );
SHOULD_NOT_MATCH("A]C");
SHOULD_NOT_MATCH("B");


EXPRESSION( "a\\(b", PCRE_CASELESS );
SHOULD_MATCH( 1, "A(B" );
MATCH( 0, "A(B" );


EXPRESSION( "a\\(*b", PCRE_CASELESS );
SHOULD_MATCH( 1, "AB" );
MATCH( 0, "AB" );
SHOULD_MATCH( 1, "A((B" );
MATCH( 0, "A((B" );


EXPRESSION( "a\\b", PCRE_CASELESS );
SHOULD_NOT_MATCH("A\\B");


EXPRESSION( "((a))", PCRE_CASELESS );
SHOULD_MATCH( 3, "ABC" );
MATCH( 0, "A" );
MATCH( 1, "A" );
MATCH( 2, "A" );


EXPRESSION( "(a)b(c)", PCRE_CASELESS );
SHOULD_MATCH( 3, "ABC" );
MATCH( 0, "ABC" );
MATCH( 1, "A" );
MATCH( 2, "C" );


EXPRESSION( "a+b+c", PCRE_CASELESS );
SHOULD_MATCH( 1, "AABBABC" );
MATCH( 0, "ABC" );


EXPRESSION( "a{1,}b{1,}c", PCRE_CASELESS );
SHOULD_MATCH( 1, "AABBABC" );
MATCH( 0, "ABC" );


EXPRESSION( "a.+?c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABCABC" );
MATCH( 0, "ABC" );


EXPRESSION( "a.*?c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABCABC" );
MATCH( 0, "ABC" );


EXPRESSION( "a.{0,5}?c", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABCABC" );
MATCH( 0, "ABC" );


EXPRESSION( "(a+|b)*", PCRE_CASELESS );
SHOULD_MATCH( 2, "AB" );
MATCH( 0, "AB" );
MATCH( 1, "B" );


EXPRESSION( "(a+|b){0,}", PCRE_CASELESS );
SHOULD_MATCH( 2, "AB" );
MATCH( 0, "AB" );
MATCH( 1, "B" );


EXPRESSION( "(a+|b)+", PCRE_CASELESS );
SHOULD_MATCH( 2, "AB" );
MATCH( 0, "AB" );
MATCH( 1, "B" );


EXPRESSION( "(a+|b){1,}", PCRE_CASELESS );
SHOULD_MATCH( 2, "AB" );
MATCH( 0, "AB" );
MATCH( 1, "B" );


EXPRESSION( "(a+|b)?", PCRE_CASELESS );
SHOULD_MATCH( 2, "AB" );
MATCH( 0, "A" );
MATCH( 1, "A" );


EXPRESSION( "(a+|b){0,1}", PCRE_CASELESS );
SHOULD_MATCH( 2, "AB" );
MATCH( 0, "A" );
MATCH( 1, "A" );


EXPRESSION( "(a+|b){0,1}?", PCRE_CASELESS );
SHOULD_MATCH( 1, "AB" );
MATCH( 0, "" );


EXPRESSION( "[^ab]*", PCRE_CASELESS );
SHOULD_MATCH( 1, "CDE" );
MATCH( 0, "CDE" );


EXPRESSION( "abc", PCRE_CASELESS );


EXPRESSION( "a*", PCRE_CASELESS );



EXPRESSION( "([abc])*d", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABBBCD" );
MATCH( 0, "ABBBCD" );
MATCH( 1, "C" );


EXPRESSION( "([abc])*bcd", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABCD" );
MATCH( 0, "ABCD" );
MATCH( 1, "A" );


EXPRESSION( "a|b|c|d|e", PCRE_CASELESS );
SHOULD_MATCH( 1, "E" );
MATCH( 0, "E" );


EXPRESSION( "(a|b|c|d|e)f", PCRE_CASELESS );
SHOULD_MATCH( 2, "EF" );
MATCH( 0, "EF" );
MATCH( 1, "E" );


EXPRESSION( "abcd*efg", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABCDEFG" );
MATCH( 0, "ABCDEFG" );


EXPRESSION( "ab*", PCRE_CASELESS );
SHOULD_MATCH( 1, "XABYABBBZ" );
MATCH( 0, "AB" );
SHOULD_MATCH( 1, "XAYABBBZ" );
MATCH( 0, "A" );


EXPRESSION( "(ab|cd)e", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABCDE" );
MATCH( 0, "CDE" );
MATCH( 1, "CD" );


EXPRESSION( "[abhgefdc]ij", PCRE_CASELESS );
SHOULD_MATCH( 1, "HIJ" );
MATCH( 0, "HIJ" );


EXPRESSION( "^(ab|cd)e", PCRE_CASELESS );
SHOULD_NOT_MATCH("ABCDE");


EXPRESSION( "(abc|)ef", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABCDEF" );
MATCH( 0, "EF" );
MATCH( 1, "" );


EXPRESSION( "(a|b)c*d", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABCD" );
MATCH( 0, "BCD" );
MATCH( 1, "B" );


EXPRESSION( "(ab|ab*)bc", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABC" );
MATCH( 0, "ABC" );
MATCH( 1, "A" );


EXPRESSION( "a([bc]*)c*", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABC" );
MATCH( 0, "ABC" );
MATCH( 1, "BC" );


EXPRESSION( "a([bc]*)(c*d)", PCRE_CASELESS );
SHOULD_MATCH( 3, "ABCD" );
MATCH( 0, "ABCD" );
MATCH( 1, "BC" );
MATCH( 2, "D" );


EXPRESSION( "a([bc]+)(c*d)", PCRE_CASELESS );
SHOULD_MATCH( 3, "ABCD" );
MATCH( 0, "ABCD" );
MATCH( 1, "BC" );
MATCH( 2, "D" );


EXPRESSION( "a([bc]*)(c+d)", PCRE_CASELESS );
SHOULD_MATCH( 3, "ABCD" );
MATCH( 0, "ABCD" );
MATCH( 1, "B" );
MATCH( 2, "CD" );


EXPRESSION( "a[bcd]*dcdcde", PCRE_CASELESS );
SHOULD_MATCH( 1, "ADCDCDE" );
MATCH( 0, "ADCDCDE" );


EXPRESSION( "a[bcd]+dcdcde", PCRE_CASELESS );


EXPRESSION( "(ab|a)b*c", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABC" );
MATCH( 0, "ABC" );
MATCH( 1, "AB" );


EXPRESSION( "((a)(b)c)(d)", PCRE_CASELESS );
SHOULD_MATCH( 5, "ABCD" );
MATCH( 0, "ABCD" );
MATCH( 1, "ABC" );
MATCH( 2, "A" );
MATCH( 3, "B" );
MATCH( 4, "D" );


EXPRESSION( "[a-zA-Z_][a-zA-Z0-9_]*", PCRE_CASELESS );
SHOULD_MATCH( 1, "ALPHA" );
MATCH( 0, "ALPHA" );


EXPRESSION( "^a(bc+|b[eh])g|.h$", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABH" );
MATCH( 0, "BH" );


EXPRESSION( "(bc+d$|ef*g.|h?i(j|k))", PCRE_CASELESS );
SHOULD_MATCH( 2, "EFFGZ" );
MATCH( 0, "EFFGZ" );
MATCH( 1, "EFFGZ" );
SHOULD_MATCH( 3, "IJ" );
MATCH( 0, "IJ" );
MATCH( 1, "IJ" );
MATCH( 2, "J" );
SHOULD_MATCH( 2, "REFFGZ" );
MATCH( 0, "EFFGZ" );
MATCH( 1, "EFFGZ" );
SHOULD_NOT_MATCH("ADCDCDE");
SHOULD_NOT_MATCH("EFFG");
SHOULD_NOT_MATCH("BCDD");

#ifdef REGEX_FULL

EXPRESSION( "((((((((((a))))))))))", PCRE_CASELESS );
SHOULD_MATCH( 11, "A" );
MATCH( 0, "A" );
MATCH( 1, "A" );
MATCH( 2, "A" );
MATCH( 3, "A" );
MATCH( 4, "A" );
MATCH( 5, "A" );
MATCH( 6, "A" );
MATCH( 7, "A" );
MATCH( 8, "A" );
MATCH( 9, "A" );
MATCH( 10, "A" );


EXPRESSION( "((((((((((a))))))))))\10", PCRE_CASELESS );
SHOULD_MATCH( 11, "AA" );
MATCH( 0, "AA" );
MATCH( 1, "A" );
MATCH( 2, "A" );
MATCH( 3, "A" );
MATCH( 4, "A" );
MATCH( 5, "A" );
MATCH( 6, "A" );
MATCH( 7, "A" );
MATCH( 8, "A" );
MATCH( 9, "A" );
MATCH( 10, "A" );


EXPRESSION( "(((((((((a)))))))))", PCRE_CASELESS );
SHOULD_MATCH( 10, "A" );
MATCH( 0, "A" );
MATCH( 1, "A" );
MATCH( 2, "A" );
MATCH( 3, "A" );
MATCH( 4, "A" );
MATCH( 5, "A" );
MATCH( 6, "A" );
MATCH( 7, "A" );
MATCH( 8, "A" );
MATCH( 9, "A" );


EXPRESSION( "(?:(?:(?:(?:(?:(?:(?:(?:(?:(a))))))))))", PCRE_CASELESS );
SHOULD_MATCH( 2, "A" );
MATCH( 0, "A" );
MATCH( 1, "A" );


EXPRESSION( "(?:(?:(?:(?:(?:(?:(?:(?:(?:(a|b|c))))))))))", PCRE_CASELESS );
SHOULD_MATCH( 2, "C" );
MATCH( 0, "C" );
MATCH( 1, "C" );


EXPRESSION( "multiple words of text", PCRE_CASELESS );
SHOULD_NOT_MATCH("AA");
SHOULD_NOT_MATCH("UH-UH");


EXPRESSION( "multiple words", PCRE_CASELESS );
SHOULD_MATCH( 1, "MULTIPLE WORDS, YEAH" );
MATCH( 0, "MULTIPLE WORDS" );


EXPRESSION( "(.*)c(.*)", PCRE_CASELESS );
SHOULD_MATCH( 3, "ABCDE" );
MATCH( 0, "ABCDE" );
MATCH( 1, "AB" );
MATCH( 2, "DE" );


EXPRESSION( "\\((.*), (.*)\\)", PCRE_CASELESS );
SHOULD_MATCH( 3, "(A, B)" );
MATCH( 0, "(A, B)" );
MATCH( 1, "A" );
MATCH( 2, "B" );


EXPRESSION( "[k]", PCRE_CASELESS );


EXPRESSION( "abcd", PCRE_CASELESS );
SHOULD_MATCH( 1, "ABCD" );
MATCH( 0, "ABCD" );


EXPRESSION( "a(bc)d", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABCD" );
MATCH( 0, "ABCD" );
MATCH( 1, "BC" );


EXPRESSION( "a[-]?c", PCRE_CASELESS );
SHOULD_MATCH( 1, "AC" );
MATCH( 0, "AC" );


EXPRESSION( "(abc)\1", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABCABC" );
MATCH( 0, "ABCABC" );
MATCH( 1, "ABC" );


EXPRESSION( "([a-c]*)\1", PCRE_CASELESS );
SHOULD_MATCH( 2, "ABCABC" );
MATCH( 0, "ABCABC" );
MATCH( 1, "ABC" );


EXPRESSION( "a(?!b).", 0 );
SHOULD_MATCH( 1, "abad" );
MATCH( 0, "ad" );


EXPRESSION( "a(?=d).", 0 );
SHOULD_MATCH( 1, "abad" );
MATCH( 0, "ad" );


EXPRESSION( "a(?=c|d).", 0 );
SHOULD_MATCH( 1, "abad" );
MATCH( 0, "ad" );


EXPRESSION( "a(?:b|c|d)(.)", 0 );
SHOULD_MATCH( 2, "ace" );
MATCH( 0, "ace" );
MATCH( 1, "e" );


EXPRESSION( "a(?:b|c|d)*(.)", 0 );
SHOULD_MATCH( 2, "ace" );
MATCH( 0, "ace" );
MATCH( 1, "e" );


EXPRESSION( "a(?:b|c|d)+?(.)", 0 );
SHOULD_MATCH( 2, "ace" );
MATCH( 0, "ace" );
MATCH( 1, "e" );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acd" );
MATCH( 1, "d" );


EXPRESSION( "a(?:b|c|d)+(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcdbe" );
MATCH( 1, "e" );


EXPRESSION( "a(?:b|c|d){2}(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdb" );
MATCH( 1, "b" );


EXPRESSION( "a(?:b|c|d){4,5}(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcdb" );
MATCH( 1, "b" );


EXPRESSION( "a(?:b|c|d){4,5}?(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcd" );
MATCH( 1, "d" );


EXPRESSION( "((foo)|(bar))*", 0 );
SHOULD_MATCH( 4, "foobar" );
MATCH( 0, "foobar" );
MATCH( 1, "bar" );
MATCH( 2, "foo" );
MATCH( 3, "bar" );


EXPRESSION( "a(?:b|c|d){6,7}(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcdbe" );
MATCH( 1, "e" );


EXPRESSION( "a(?:b|c|d){6,7}?(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcdbe" );
MATCH( 1, "e" );


EXPRESSION( "a(?:b|c|d){5,6}(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcdbe" );
MATCH( 1, "e" );


EXPRESSION( "a(?:b|c|d){5,6}?(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcdb" );
MATCH( 1, "b" );


EXPRESSION( "a(?:b|c|d){5,7}(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcdbe" );
MATCH( 1, "e" );


EXPRESSION( "a(?:b|c|d){5,7}?(.)", 0 );
SHOULD_MATCH( 2, "acdbcdbe" );
MATCH( 0, "acdbcdb" );
MATCH( 1, "b" );


EXPRESSION( "a(?:b|(c|e){1,2}?|d)+?(.)", 0 );
SHOULD_MATCH( 3, "ace" );
MATCH( 0, "ace" );
MATCH( 1, "c" );
MATCH( 2, "e" );


EXPRESSION( "^(.+)?B", 0 );
SHOULD_MATCH( 2, "AB" );
MATCH( 0, "AB" );
MATCH( 1, "A" );


EXPRESSION( "^([^a-z])|(\\^)$", 0 );
SHOULD_MATCH( 2, "." );
MATCH( 0, "." );
MATCH( 1, "." );


EXPRESSION( "^[<>]&", 0 );
SHOULD_MATCH( 1, "<&OUT" );
MATCH( 0, "<&" );


EXPRESSION( "^(a\1?){4}$", 0 );
SHOULD_MATCH( 2, "aaaaaaaaaa" );
MATCH( 0, "aaaaaaaaaa" );
MATCH( 1, "aaaa" );
SHOULD_NOT_MATCH("AB");
SHOULD_NOT_MATCH("aaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaa");


EXPRESSION( "^(a(?(1)\1)){4}$", 0 );
SHOULD_MATCH( 2, "aaaaaaaaaa" );
MATCH( 0, "aaaaaaaaaa" );
MATCH( 1, "aaaa" );
SHOULD_NOT_MATCH("aaaaaaaaa");
SHOULD_NOT_MATCH("aaaaaaaaaaa");


EXPRESSION( "(?:(f)(o)(o)|(b)(a)(r))*", 0 );
SHOULD_MATCH( 7, "foobar" );
MATCH( 0, "foobar" );
MATCH( 1, "f" );
MATCH( 2, "o" );
MATCH( 3, "o" );
MATCH( 4, "b" );
MATCH( 5, "a" );
MATCH( 6, "r" );


EXPRESSION( "(?<=a)b", 0 );
SHOULD_MATCH( 1, "ab" );
MATCH( 0, "b" );
SHOULD_NOT_MATCH("cb");
SHOULD_NOT_MATCH("b");


EXPRESSION( "(?<!c)b", 0 );
SHOULD_MATCH( 1, "ab" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "b" );
MATCH( 0, "b" );
SHOULD_MATCH( 1, "b" );
MATCH( 0, "b" );


EXPRESSION( "(?:..)*a", 0 );
SHOULD_MATCH( 1, "aba" );
MATCH( 0, "aba" );


EXPRESSION( "(?:..)*?a", 0 );
SHOULD_MATCH( 1, "aba" );
MATCH( 0, "a" );


EXPRESSION( "^(?:b|a(?=(.)))*\1", 0 );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "ab" );
MATCH( 1, "b" );


EXPRESSION( "^(){3,5}", 0 );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "" );
MATCH( 1, "" );


EXPRESSION( "^(a+)*ax", 0 );
SHOULD_MATCH( 2, "aax" );
MATCH( 0, "aax" );
MATCH( 1, "a" );


EXPRESSION( "^((a|b)+)*ax", 0 );
SHOULD_MATCH( 3, "aax" );
MATCH( 0, "aax" );
MATCH( 1, "a" );
MATCH( 2, "a" );


EXPRESSION( "^((a|bc)+)*ax", 0 );
SHOULD_MATCH( 3, "aax" );
MATCH( 0, "aax" );
MATCH( 1, "a" );
MATCH( 2, "a" );


EXPRESSION( "(a|x)*ab", 0 );
SHOULD_MATCH( 1, "cab" );
MATCH( 0, "ab" );


EXPRESSION( "(a)*ab", 0 );
SHOULD_MATCH( 1, "cab" );
MATCH( 0, "ab" );


EXPRESSION( "(?:(?i)a)b", 0 );
SHOULD_MATCH( 1, "ab" );
MATCH( 0, "ab" );


EXPRESSION( "((?i)a)b", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "a" );


EXPRESSION( "(?:(?i)a)b", 0 );
SHOULD_MATCH( 1, "Ab" );
MATCH( 0, "Ab" );


EXPRESSION( "((?i)a)b", 0 );
SHOULD_MATCH( 2, "Ab" );
MATCH( 0, "Ab" );
MATCH( 1, "A" );


EXPRESSION( "(?:(?i)a)b", 0 );
SHOULD_NOT_MATCH("cb");
SHOULD_NOT_MATCH("aB");


EXPRESSION( "((?i)a)b", 0 );


EXPRESSION( "(?i:a)b", 0 );
SHOULD_MATCH( 1, "ab" );
MATCH( 0, "ab" );


EXPRESSION( "((?i:a))b", 0 );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "a" );


EXPRESSION( "(?i:a)b", 0 );
SHOULD_MATCH( 1, "Ab" );
MATCH( 0, "Ab" );


EXPRESSION( "((?i:a))b", 0 );
SHOULD_MATCH( 2, "Ab" );
MATCH( 0, "Ab" );
MATCH( 1, "A" );


EXPRESSION( "(?i:a)b", 0 );
SHOULD_NOT_MATCH("aB");
SHOULD_NOT_MATCH("aB");


EXPRESSION( "((?i:a))b", 0 );


EXPRESSION( "(?:(?-i)a)b", PCRE_CASELESS );
SHOULD_MATCH( 1, "ab" );
MATCH( 0, "ab" );


EXPRESSION( "((?-i)a)b", PCRE_CASELESS );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "a" );


EXPRESSION( "(?:(?-i)a)b", PCRE_CASELESS );
SHOULD_MATCH( 1, "aB" );
MATCH( 0, "aB" );


EXPRESSION( "((?-i)a)b", PCRE_CASELESS );
SHOULD_MATCH( 2, "aB" );
MATCH( 0, "aB" );
MATCH( 1, "a" );


EXPRESSION( "(?:(?-i)a)b", PCRE_CASELESS );
SHOULD_MATCH( 1, "aB" );
MATCH( 0, "aB" );
SHOULD_NOT_MATCH("Ab");


EXPRESSION( "((?-i)a)b", PCRE_CASELESS );


EXPRESSION( "(?:(?-i)a)b", PCRE_CASELESS );
SHOULD_MATCH( 1, "aB" );
MATCH( 0, "aB" );


EXPRESSION( "((?-i)a)b", PCRE_CASELESS );
SHOULD_MATCH( 2, "aB" );
MATCH( 0, "aB" );
MATCH( 1, "a" );


EXPRESSION( "(?:(?-i)a)b", PCRE_CASELESS );
SHOULD_NOT_MATCH("Ab");
SHOULD_NOT_MATCH("AB");


EXPRESSION( "((?-i)a)b", PCRE_CASELESS );


EXPRESSION( "(?-i:a)b", PCRE_CASELESS );
SHOULD_MATCH( 1, "ab" );
MATCH( 0, "ab" );


EXPRESSION( "((?-i:a))b", PCRE_CASELESS );
SHOULD_MATCH( 2, "ab" );
MATCH( 0, "ab" );
MATCH( 1, "a" );


EXPRESSION( "(?-i:a)b", PCRE_CASELESS );
SHOULD_MATCH( 1, "aB" );
MATCH( 0, "aB" );


EXPRESSION( "((?-i:a))b", PCRE_CASELESS );
SHOULD_MATCH( 2, "aB" );
MATCH( 0, "aB" );
MATCH( 1, "a" );


EXPRESSION( "(?-i:a)b", PCRE_CASELESS );
SHOULD_NOT_MATCH("AB");
SHOULD_NOT_MATCH("Ab");


EXPRESSION( "((?-i:a))b", PCRE_CASELESS );


EXPRESSION( "(?-i:a)b", PCRE_CASELESS );
SHOULD_MATCH( 1, "aB" );
MATCH( 0, "aB" );


EXPRESSION( "((?-i:a))b", PCRE_CASELESS );
SHOULD_MATCH( 2, "aB" );
MATCH( 0, "aB" );
MATCH( 1, "a" );


EXPRESSION( "(?-i:a)b", PCRE_CASELESS );
SHOULD_NOT_MATCH("Ab");
SHOULD_NOT_MATCH("AB");


EXPRESSION( "((?-i:a))b", PCRE_CASELESS );


EXPRESSION( "((?-i:a.))b", PCRE_CASELESS );
SHOULD_NOT_MATCH("AB");
SHOULD_NOT_MATCH("a\nB");


EXPRESSION( "((?s-i:a.))b", PCRE_CASELESS );
SHOULD_MATCH( 2, "a\nB" );
MATCH( 0, "a\nB" );
MATCH( 1, "a\n" );


EXPRESSION( "(?:c|d)(?:)(?:a(?:)(?:b)(?:b(?:))(?:b(?:)(?:b)))", 0 );
SHOULD_MATCH( 1, "cabbbb" );
MATCH( 0, "cabbbb" );


EXPRESSION( "(?:c|d)(?:)(?:aaaaaaaa(?:)(?:bbbbbbbb)(?:bbbbbbbb(?:))(?:bbbbbbbb(?:)(?:bbbbbbbb)))", 0 );
SHOULD_MATCH( 1, "caaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" );
MATCH( 0, "caaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" );


EXPRESSION( "(ab)\\d\1", PCRE_CASELESS );
SHOULD_MATCH( 2, "Ab4ab" );
MATCH( 0, "Ab4ab" );
MATCH( 1, "Ab" );
SHOULD_MATCH( 2, "ab4Ab" );
MATCH( 0, "ab4Ab" );
MATCH( 1, "ab" );


EXPRESSION( "foo\\w*\\d{4}baz", 0 );
SHOULD_MATCH( 1, "foobar1234baz" );
MATCH( 0, "foobar1234baz" );


EXPRESSION( "x(~~)*(?:(?:F)?)?", 0 );
SHOULD_MATCH( 2, "x~~" );
MATCH( 0, "x~~" );
MATCH( 1, "~~" );


EXPRESSION( "^a(?#xxx){3}c", 0 );
SHOULD_MATCH( 1, "aaac" );
MATCH( 0, "aaac" );


EXPRESSION( "^a (?#xxx) (?#yyy) {3}c", PCRE_EXTENDED );
SHOULD_MATCH( 1, "aaac" );
MATCH( 0, "aaac" );


EXPRESSION( "(?<![cd])b", 0 );
SHOULD_NOT_MATCH("B\nB");
SHOULD_NOT_MATCH("dbcb");


EXPRESSION( "(?<![cd])[ab]", 0 );
SHOULD_MATCH( 1, "dbaacb" );
MATCH( 0, "a" );


EXPRESSION( "(?<!(c|d))b", 0 );


EXPRESSION( "(?<!(c|d))[ab]", 0 );
SHOULD_MATCH( 1, "dbaacb" );
MATCH( 0, "a" );


EXPRESSION( "(?<!cd)[ab]", 0 );
SHOULD_MATCH( 1, "cdaccb" );
MATCH( 0, "b" );


EXPRESSION( "^(?:a?b?)*$", 0 );
SHOULD_NOT_MATCH("dbcb");
SHOULD_NOT_MATCH("a--");


EXPRESSION( "((?s)^a(.))((?m)^b$)", 0 );
SHOULD_MATCH( 4, "a\nb\nc\n" );
MATCH( 0, "a\nb" );
MATCH( 1, "a\n" );
MATCH( 2, "\n" );
MATCH( 3, "b" );


EXPRESSION( "((?m)^b$)", 0 );
SHOULD_MATCH( 2, "a\nb\nc\n" );
MATCH( 0, "b" );
MATCH( 1, "b" );


EXPRESSION( "(?m)^b", 0 );
SHOULD_MATCH( 1, "a\nb\n" );
MATCH( 0, "b" );


EXPRESSION( "(?m)^(b)", 0 );
SHOULD_MATCH( 2, "a\nb\n" );
MATCH( 0, "b" );
MATCH( 1, "b" );


EXPRESSION( "((?m)^b)", 0 );
SHOULD_MATCH( 2, "a\nb\n" );
MATCH( 0, "b" );
MATCH( 1, "b" );


EXPRESSION( "\n((?m)^b)", 0 );
SHOULD_MATCH( 2, "a\nb\n" );
MATCH( 0, "\nb" );
MATCH( 1, "b" );


EXPRESSION( "((?s).)c(?!.)", 0 );
SHOULD_MATCH( 2, "a\nb\nc\n" );
MATCH( 0, "\nc" );
MATCH( 1, "\n" );
SHOULD_MATCH( 2, "a\nb\nc\n" );
MATCH( 0, "\nc" );
MATCH( 1, "\n" );


EXPRESSION( "((?s)b.)c(?!.)", 0 );
SHOULD_MATCH( 2, "a\nb\nc\n" );
MATCH( 0, "b\nc" );
MATCH( 1, "b\n" );
SHOULD_MATCH( 2, "a\nb\nc\n" );
MATCH( 0, "b\nc" );
MATCH( 1, "b\n" );


EXPRESSION( "^b", 0 );


EXPRESSION( "()^b", 0 );
SHOULD_NOT_MATCH("a\nb\nc\n");
SHOULD_NOT_MATCH("a\nb\nc\n");


EXPRESSION( "((?m)^b)", 0 );
SHOULD_MATCH( 2, "a\nb\nc\n" );
MATCH( 0, "b" );
MATCH( 1, "b" );


EXPRESSION( "(?(1)a|b)", 0 );


EXPRESSION( "(?(1)b|a)", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );


EXPRESSION( "(x)?(?(1)a|b)", 0 );
SHOULD_NOT_MATCH("a");
SHOULD_NOT_MATCH("a");


EXPRESSION( "(x)?(?(1)b|a)", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );


EXPRESSION( "()?(?(1)b|a)", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );


EXPRESSION( "()(?(1)b|a)", 0 );


EXPRESSION( "()?(?(1)a|b)", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "a" );
MATCH( 1, "" );


EXPRESSION( "^(\\()?blah(?(1)(\\)))$", 0 );
SHOULD_MATCH( 3, "(blah)" );
MATCH( 0, "(blah)" );
MATCH( 1, "(" );
MATCH( 2, ")" );
SHOULD_MATCH( 1, "blah" );
MATCH( 0, "blah" );
SHOULD_NOT_MATCH("a");
SHOULD_NOT_MATCH("blah)");
SHOULD_NOT_MATCH("(blah");


EXPRESSION( "^(\\(+)?blah(?(1)(\\)))$", 0 );
SHOULD_MATCH( 3, "(blah)" );
MATCH( 0, "(blah)" );
MATCH( 1, "(" );
MATCH( 2, ")" );
SHOULD_MATCH( 1, "blah" );
MATCH( 0, "blah" );
SHOULD_NOT_MATCH("blah)");
SHOULD_NOT_MATCH("(blah");


EXPRESSION( "(?(?!a)a|b)", 0 );


EXPRESSION( "(?(?!a)b|a)", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );


EXPRESSION( "(?(?=a)b|a)", 0 );
SHOULD_NOT_MATCH("a");
SHOULD_NOT_MATCH("a");


EXPRESSION( "(?(?=a)a|b)", 0 );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );


EXPRESSION( "(?=(a+?))(\1ab)", 0 );
SHOULD_MATCH( 3, "aaab" );
MATCH( 0, "aab" );
MATCH( 1, "a" );
MATCH( 2, "aab" );


EXPRESSION( "^(?=(a+?))\1ab", 0 );


EXPRESSION( "(\\w+:)+", 0 );
SHOULD_MATCH( 2, "one:" );
MATCH( 0, "one:" );
MATCH( 1, "one:" );


EXPRESSION( "$(?<=^(a))", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "" );
MATCH( 1, "a" );


EXPRESSION( "(?=(a+?))(\1ab)", 0 );
SHOULD_MATCH( 3, "aaab" );
MATCH( 0, "aab" );
MATCH( 1, "a" );
MATCH( 2, "aab" );


EXPRESSION( "^(?=(a+?))\1ab", 0 );
SHOULD_NOT_MATCH("aaab");
SHOULD_NOT_MATCH("aaab");

#endif

EXPRESSION( "([\\w:]+::)?(\\w+)$", 0 );
SHOULD_MATCH( 3, "abcd" );
MATCH( 0, "abcd" );
MATCH( 1, "<unset>" );
MATCH( 2, "abcd" );
SHOULD_MATCH( 3, "xy:z:::abcd" );
MATCH( 0, "xy:z:::abcd" );
MATCH( 1, "xy:z:::" );
MATCH( 2, "abcd" );


EXPRESSION( "^[^bcd]*(c+)", 0 );
SHOULD_MATCH( 2, "aexycd" );
MATCH( 0, "aexyc" );
MATCH( 1, "c" );


EXPRESSION( "(a*)b+", 0 );
SHOULD_MATCH( 2, "caab" );
MATCH( 0, "aab" );
MATCH( 1, "aa" );


EXPRESSION( "([\\w:]+::)?(\\w+)$", 0 );
SHOULD_MATCH( 3, "abcd" );
MATCH( 0, "abcd" );
MATCH( 1, "<unset>" );
MATCH( 2, "abcd" );
SHOULD_MATCH( 3, "xy:z:::abcd" );
MATCH( 0, "xy:z:::abcd" );
MATCH( 1, "xy:z:::" );
MATCH( 2, "abcd" );
SHOULD_NOT_MATCH("abcd:");
SHOULD_NOT_MATCH("abcd:");


EXPRESSION( "^[^bcd]*(c+)", 0 );
SHOULD_MATCH( 2, "aexycd" );
MATCH( 0, "aexyc" );
MATCH( 1, "c" );


EXPRESSION( "(>a+)ab", 0 );


EXPRESSION( "(?>a+)b", 0 );
SHOULD_MATCH( 1, "aaab" );
MATCH( 0, "aaab" );


EXPRESSION( "([[:]+)", 0 );
SHOULD_MATCH( 2, "a:[b]:" );
MATCH( 0, ":[" );
MATCH( 1, ":[" );


EXPRESSION( "([[=]+)", 0 );
SHOULD_MATCH( 2, "a=[b]=" );
MATCH( 0, "=[" );
MATCH( 1, "=[" );


EXPRESSION( "([[.]+)", 0 );
SHOULD_MATCH( 2, "a.[b]." );
MATCH( 0, ".[" );
MATCH( 1, ".[" );


EXPRESSION( "((?>a+)b)", 0 );
SHOULD_MATCH( 2, "aaab" );
MATCH( 0, "aaab" );
MATCH( 1, "aaab" );


EXPRESSION( "(?>(a+))b", 0 );
SHOULD_MATCH( 2, "aaab" );
MATCH( 0, "aaab" );
MATCH( 1, "aaa" );


EXPRESSION( "((?>[^()]+)|\\([^()]*\\))+", 0 );
SHOULD_MATCH( 2, "((abc(ade)ufh()()x" );
MATCH( 0, "abc(ade)ufh()()x" );
MATCH( 1, "x" );


EXPRESSION( "a\\Z", 0 );
SHOULD_NOT_MATCH("aaab");
SHOULD_NOT_MATCH("a\nb\n");


EXPRESSION( "b\\Z", 0 );
SHOULD_MATCH( 1, "a\nb\n" );
MATCH( 0, "b" );


EXPRESSION( "b\\z", 0 );


EXPRESSION( "b\\Z", 0 );
SHOULD_MATCH( 1, "a\nb" );
MATCH( 0, "b" );


EXPRESSION( "b\\z", 0 );
SHOULD_MATCH( 1, "a\nb" );
MATCH( 0, "b" );

#ifdef REGEX_FULL

EXPRESSION( "^(?>(?(1)\\.|())[^\\W_](?>[a-z0-9-]*[^\\W_])?)+$", 0 );
SHOULD_MATCH( 2, "a" );
MATCH( 0, "a" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "abc" );
MATCH( 0, "abc" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "a-b" );
MATCH( 0, "a-b" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "0-9 " );
MATCH( 0, "0-9" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "a.b" );
MATCH( 0, "a.b" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "5.6.7  " );
MATCH( 0, "5.6.7" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "the.quick.brown.fox" );
MATCH( 0, "the.quick.brown.fox" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "a100.b200.300c  " );
MATCH( 0, "a100.b200.300c" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "12-ab.1245 " );
MATCH( 0, "12-ab.1245" );
MATCH( 1, "" );
SHOULD_NOT_MATCH("");
SHOULD_NOT_MATCH(".a");
SHOULD_NOT_MATCH("-a");
SHOULD_NOT_MATCH("a-");
SHOULD_NOT_MATCH("a.  ");
SHOULD_NOT_MATCH("a_b ");
SHOULD_NOT_MATCH("a.-");
SHOULD_NOT_MATCH("a..  ");
SHOULD_NOT_MATCH("ab..bc ");
SHOULD_NOT_MATCH("the.quick.brown.fox-");
SHOULD_NOT_MATCH("the.quick.brown.fox.");
SHOULD_NOT_MATCH("the.quick.brown.fox_");
SHOULD_NOT_MATCH("the.quick.brown.fox+       ");


EXPRESSION( "(?>.*)(?<=(abcd|wxyz))", 0 );
SHOULD_MATCH( 2, "alphabetabcd" );
MATCH( 0, "alphabetabcd" );
MATCH( 1, "abcd" );
SHOULD_MATCH( 2, "endingwxyz" );
MATCH( 0, "endingwxyz" );
MATCH( 1, "wxyz" );
SHOULD_NOT_MATCH("a rather long string that doesn't end with one of them");


EXPRESSION( "word (?>(?:(?!otherword)[a-zA-Z0-9]+ ){0,30})otherword", 0 );
SHOULD_MATCH( 1, "word cat dog elephant mussel cow horse canary baboon snake shark otherword" );
MATCH( 0, "word cat dog elephant mussel cow horse canary baboon snake shark otherword" );
SHOULD_NOT_MATCH("word cat dog elephant mussel cow horse canary baboon snake shark");


EXPRESSION( "word (?>[a-zA-Z0-9]+ ){0,30}otherword", 0 );
SHOULD_NOT_MATCH("word cat dog elephant mussel cow horse canary baboon snake shark the quick brown fox and the lazy dog and several other words getting close to thirty by now I hope");


EXPRESSION( "(?<=\\d{3}(?!999))foo", 0 );
SHOULD_MATCH( 1, "999foo" );
MATCH( 0, "foo" );
SHOULD_MATCH( 1, "123999foo " );
MATCH( 0, "foo" );
SHOULD_NOT_MATCH("123abcfoo");


EXPRESSION( "(?<=(?!...999)\\d{3})foo", 0 );
SHOULD_MATCH( 1, "999foo" );
MATCH( 0, "foo" );
SHOULD_MATCH( 1, "123999foo " );
MATCH( 0, "foo" );
SHOULD_NOT_MATCH("123abcfoo");


EXPRESSION( "(?<=\\d{3}(?!999)...)foo", 0 );
SHOULD_MATCH( 1, "123abcfoo" );
MATCH( 0, "foo" );
SHOULD_MATCH( 1, "123456foo " );
MATCH( 0, "foo" );
SHOULD_NOT_MATCH("123999foo  ");


EXPRESSION( "(?<=\\d{3}...)(?<!999)foo", 0 );
SHOULD_MATCH( 1, "123abcfoo   " );
MATCH( 0, "foo" );
SHOULD_MATCH( 1, "123456foo " );
MATCH( 0, "foo" );
SHOULD_NOT_MATCH("123999foo  ");


EXPRESSION( "<a[\\s]+href[\\s]*=[\\s]*          # find <a href=\n ([\"\'])?                       # find single or double quote\n (?(1) (.*?)\1 | ([^\\s]+))       # if quote found, match up to next matching\n                                 # quote, otherwise match up to next space\n", PCRE_EXTENDED+PCRE_CASELESS+PCRE_DOTALL );
SHOULD_MATCH( 4, "<a href=abcd xyz" );
MATCH( 0, "<a href=abcd" );
MATCH( 1, "<unset>" );
MATCH( 2, "<unset>" );
MATCH( 3, "abcd" );
SHOULD_MATCH( 3, "<a href=\"abcd xyz pqr\" cats" );
MATCH( 0, "<a href=\"abcd xyz pqr\"" );
MATCH( 1, "\"" );
MATCH( 2, "abcd xyz pqr" );
SHOULD_MATCH( 3, "<a href=\'abcd xyz pqr\' cats" );
MATCH( 0, "<a href='abcd xyz pqr'" );
MATCH( 1, "'" );
MATCH( 2, "abcd xyz pqr" );


EXPRESSION( "<a\\s+href\\s*=\\s*                # find <a href=\n ([\"'])?                         # find single or double quote\n (?(1) (.*?)\1 | (\\S+))          # if quote found, match up to next matching\n                                 # quote, otherwise match up to next space\n", PCRE_EXTENDED+PCRE_CASELESS+PCRE_DOTALL );
SHOULD_MATCH( 4, "<a href=abcd xyz" );
MATCH( 0, "<a href=abcd" );
MATCH( 1, "<unset>" );
MATCH( 2, "<unset>" );
MATCH( 3, "abcd" );
SHOULD_MATCH( 3, "<a href=\"abcd xyz pqr\" cats" );
MATCH( 0, "<a href=\"abcd xyz pqr\"" );
MATCH( 1, "\"" );
MATCH( 2, "abcd xyz pqr" );
SHOULD_MATCH( 3, "<a href       =       \'abcd xyz pqr\' cats" );
MATCH( 0, "<a href       =       'abcd xyz pqr'" );
MATCH( 1, "'" );
MATCH( 2, "abcd xyz pqr" );


EXPRESSION( "<a\\s+href(?>\\s*)=(?>\\s*)        # find <a href=\n ([\"'])?                         # find single or double quote\n (?(1) (.*?)\1 | (\\S+))          # if quote found, match up to next matching\n                                 # quote, otherwise match up to next space\n", PCRE_EXTENDED+PCRE_CASELESS+PCRE_DOTALL );
SHOULD_MATCH( 4, "<a href=abcd xyz" );
MATCH( 0, "<a href=abcd" );
MATCH( 1, "<unset>" );
MATCH( 2, "<unset>" );
MATCH( 3, "abcd" );
SHOULD_MATCH( 3, "<a href=\"abcd xyz pqr\" cats" );
MATCH( 0, "<a href=\"abcd xyz pqr\"" );
MATCH( 1, "\"" );
MATCH( 2, "abcd xyz pqr" );
SHOULD_MATCH( 3, "<a href       =       \'abcd xyz pqr\' cats" );
MATCH( 0, "<a href       =       'abcd xyz pqr'" );
MATCH( 1, "'" );
MATCH( 2, "abcd xyz pqr" );


EXPRESSION( "((Z)+|A)*", 0 );
SHOULD_MATCH( 3, "ZABCDEFG" );
MATCH( 0, "ZA" );
MATCH( 1, "A" );
MATCH( 2, "Z" );


EXPRESSION( "(Z()|A)*", 0 );
SHOULD_MATCH( 3, "ZABCDEFG" );
MATCH( 0, "ZA" );
MATCH( 1, "A" );
MATCH( 2, "" );


EXPRESSION( "(Z(())|A)*", 0 );
SHOULD_MATCH( 4, "ZABCDEFG" );
MATCH( 0, "ZA" );
MATCH( 1, "A" );
MATCH( 2, "" );
MATCH( 3, "" );


EXPRESSION( "((?>Z)+|A)*", 0 );
SHOULD_MATCH( 2, "ZABCDEFG" );
MATCH( 0, "ZA" );
MATCH( 1, "A" );


EXPRESSION( "((?>)+|A)*", 0 );
SHOULD_MATCH( 2, "ZABCDEFG" );
MATCH( 0, "" );
MATCH( 1, "" );


EXPRESSION( "^[a-\\d]", 0 );
SHOULD_MATCH( 1, "abcde" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "-things" );
MATCH( 0, "-" );
SHOULD_MATCH( 1, "0digit" );
MATCH( 0, "0" );
SHOULD_NOT_MATCH("bcdef    ");


EXPRESSION( "^[\\d-a]", 0 );
SHOULD_MATCH( 1, "abcde" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "-things" );
MATCH( 0, "-" );
SHOULD_MATCH( 1, "0digit" );
MATCH( 0, "0" );
SHOULD_NOT_MATCH("bcdef    ");


EXPRESSION( "[[:space:]]+", 0 );
SHOULD_MATCH( 1, "> \x09\n\x0c\x0d\x0b<" );
MATCH( 0, " \x09\n\x0c\x0d\x0b" );


EXPRESSION( "[[:blank:]]+", 0 );
SHOULD_MATCH( 1, "> \x09\n\x0c\x0d\x0b<" );
MATCH( 0, " \x09" );


EXPRESSION( "[\\s]+", 0 );
SHOULD_MATCH( 1, "> \x09\n\x0c\x0d\x0b<" );
MATCH( 0, " \x09\n\x0c\x0d" );


EXPRESSION( "\\s+", 0 );
SHOULD_MATCH( 1, "> \x09\n\x0c\x0d\x0b<" );
MATCH( 0, " \x09\n\x0c\x0d" );


EXPRESSION( "ab", PCRE_EXTENDED );
SHOULD_NOT_MATCH("ab");


EXPRESSION( "(?!\\A)x", PCRE_MULTILINE );
SHOULD_MATCH( 1, "a\nxb\n" );
MATCH( 0, "x" );


EXPRESSION( "(?!^)x", PCRE_MULTILINE );
SHOULD_NOT_MATCH( "a\nxb\n" );


EXPRESSION( "abc\\Qabc\\Eabc", 0 );
SHOULD_MATCH( 1, "abcabcabc" );
MATCH( 0, "abcabcabc" );


EXPRESSION( "abc\\Q(*+|\\Eabc", 0 );
SHOULD_MATCH( 1, "abc(*+|abc " );
MATCH( 0, "abc(*+|abc" );


EXPRESSION( "   abc\\Q abc\\Eabc", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abc abcabc" );
MATCH( 0, "abc abcabc" );
SHOULD_NOT_MATCH("abcabcabc  ");


EXPRESSION( "abc#comment\n    \\Q#not comment\n    literal\\E", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abc#not comment\n    literal     " );
MATCH( 0, "abc#not comment\n    literal" );


EXPRESSION( "abc#comment\n    \\Q#not comment\n    literal", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abc#not comment\n    literal     " );
MATCH( 0, "abc#not comment\n    literal" );


EXPRESSION( "abc#comment\n    \\Q#not commet\n    literal\\E #more comment\n    ", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abc#not comment\n    literal     " );
MATCH( 0, "abc#not comment\n    literal" );


EXPRESSION( "abc#comment\n    \\Q#not comment\n    literal\\E #more comment", PCRE_EXTENDED );
SHOULD_MATCH( 1, "abc#not comment\n    literal     " );
MATCH( 0, "abc#not comment\n    literal" );


EXPRESSION( "\\Qabc\\$xyz\\E", 0 );
SHOULD_MATCH( 1, "abc\\\\$xyz" );
MATCH( 0, "abc\\$xyz" );


EXPRESSION( "\\Qabc\\E\\$\\Qxyz\\E", 0 );
SHOULD_MATCH( 1, "abc\\$xyz" );
MATCH( 0, "abc$xyz" );


EXPRESSION( "\\Gabc", 0 );
SHOULD_MATCH( 1, "abc" );
MATCH( 0, "abc" );
SHOULD_NOT_MATCH("xyzabc  ");


EXPRESSION( "a(?x: b c )d", 0 );
SHOULD_MATCH( 1, "XabcdY" );
MATCH( 0, "abcd" );
SHOULD_NOT_MATCH("Xa b c d Y ");


EXPRESSION( "((?x)x y z | a b c)", 0 );
SHOULD_MATCH( 2, "XabcY" );
MATCH( 0, "abc" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "AxyzB " );
MATCH( 0, "xyz" );
MATCH( 1, "xyz" );


EXPRESSION( "(?i)AB(?-i)C", 0 );
SHOULD_MATCH( 1, "XabCY" );
MATCH( 0, "abC" );
SHOULD_NOT_MATCH("XabcY  ");


EXPRESSION( "((?i)AB(?-i)C|D)E", 0 );
SHOULD_MATCH( 2, "abCE" );
MATCH( 0, "abCE" );
MATCH( 1, "abC" );
SHOULD_MATCH( 2, "DE" );
MATCH( 0, "DE" );
MATCH( 1, "D" );
SHOULD_NOT_MATCH("abcE");
SHOULD_NOT_MATCH("abCe  ");
SHOULD_NOT_MATCH("dE");
SHOULD_NOT_MATCH("De    ");


EXPRESSION( "(.*)\\d+\1", 0 );
SHOULD_MATCH( 2, "abc123abc" );
MATCH( 0, "abc123abc" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abc123bc " );
MATCH( 0, "bc123bc" );
MATCH( 1, "bc" );


EXPRESSION( "(.*)\\d+\1", PCRE_DOTALL );
SHOULD_MATCH( 2, "abc123abc" );
MATCH( 0, "abc123abc" );
MATCH( 1, "abc" );
SHOULD_MATCH( 2, "abc123bc " );
MATCH( 0, "bc123bc" );
MATCH( 1, "bc" );


EXPRESSION( "((.*))\\d+\1", 0 );
SHOULD_MATCH( 3, "abc123abc" );
MATCH( 0, "abc123abc" );
MATCH( 1, "abc" );
MATCH( 2, "abc" );
SHOULD_MATCH( 3, "abc123bc  " );
MATCH( 0, "bc123bc" );
MATCH( 1, "bc" );
MATCH( 2, "bc" );


EXPRESSION( "^(?!:)                       # colon disallowed at start\n  (?:                         # start of item\n    (?: [0-9a-f]{1,4} |       # 1-4 hex digits or\n    (?(1)0 | () ) )           # if null previously matched, fail; else null\n    :                         # followed by colon\n  ){1,7}                      # end item; 1-7 of them required               \n  [0-9a-f]{1,4} $             # final hex number at end of string\n  (?(1)|.)                    # check that there was an empty component\n  ", PCRE_EXTENDED+PCRE_CASELESS );
SHOULD_MATCH( 2, "a123::a123" );
MATCH( 0, "a123::a123" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "a123:b342::abcd" );
MATCH( 0, "a123:b342::abcd" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "a123:b342::324e:abcd" );
MATCH( 0, "a123:b342::324e:abcd" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "a123:ddde:b342::324e:abcd" );
MATCH( 0, "a123:ddde:b342::324e:abcd" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "a123:ddde:b342::324e:dcba:abcd" );
MATCH( 0, "a123:ddde:b342::324e:dcba:abcd" );
MATCH( 1, "" );
SHOULD_MATCH( 2, "a123:ddde:9999:b342::324e:dcba:abcd" );
MATCH( 0, "a123:ddde:9999:b342::324e:dcba:abcd" );
MATCH( 1, "" );
SHOULD_NOT_MATCH("1:2:3:4:5:6:7:8");
SHOULD_NOT_MATCH("a123:bce:ddde:9999:b342::324e:dcba:abcd");
SHOULD_NOT_MATCH("a123::9999:b342::324e:dcba:abcd");
SHOULD_NOT_MATCH("abcde:2:3:4:5:6:7:8");
SHOULD_NOT_MATCH("::1");
SHOULD_NOT_MATCH("abcd:fee0:123::   ");
SHOULD_NOT_MATCH(":1");
SHOULD_NOT_MATCH(" ");


EXPRESSION( "[z\\Qa-d]\\E]", 0 );
SHOULD_MATCH( 1, "z" );
MATCH( 0, "z" );
SHOULD_MATCH( 1, "a" );
MATCH( 0, "a" );
SHOULD_MATCH( 1, "-" );
MATCH( 0, "-" );
SHOULD_MATCH( 1, "d" );
MATCH( 0, "d" );
SHOULD_MATCH( 1, "] " );
MATCH( 0, "]" );
SHOULD_NOT_MATCH("b     ");


EXPRESSION( "[\\z\\C]", 0 );
SHOULD_MATCH( 1, "z" );
MATCH( 0, "z" );
SHOULD_MATCH( 1, "C " );
MATCH( 0, "C" );


EXPRESSION( "\\M", 0 );
SHOULD_MATCH( 1, "M " );
MATCH( 0, "M" );


EXPRESSION( "(a+)*b", 0 );
SHOULD_NOT_MATCH("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa ");


EXPRESSION( "(?i)reg(?:ul(?:[aÅ‰]|ae)r|ex)", 0 );
SHOULD_MATCH( 1, "REGular" );
MATCH( 0, "REGular" );
SHOULD_MATCH( 1, "regulaer" );
MATCH( 0, "regulaer" );
SHOULD_MATCH( 1, "Regex  " );
MATCH( 0, "Regex" );
SHOULD_MATCH( 1, "regulÅ‰r " );
MATCH( 0, "regul\xe4r" );


EXPRESSION( "Å≈ÅÊÅÂÅ‰[Å‡-ÅˇÅ¿-Åﬂ]+", 0 );
SHOULD_MATCH( 1, "Å≈ÅÊÅÂÅ‰Å‡" );
MATCH( 0, "\xc5\xe6\xe5\xe4\xe0" );
SHOULD_MATCH( 1, "Å≈ÅÊÅÂÅ‰Åˇ" );
MATCH( 0, "\xc5\xe6\xe5\xe4\xff" );
SHOULD_MATCH( 1, "Å≈ÅÊÅÂÅ‰Å¿" );
MATCH( 0, "\xc5\xe6\xe5\xe4\xc0" );
SHOULD_MATCH( 1, "Å≈ÅÊÅÂÅ‰Åﬂ" );
MATCH( 0, "\xc5\xe6\xe5\xe4\xdf" );


EXPRESSION( "(?<=Z)X.", 0 );
SHOULD_MATCH( 1, "\x84XAZXB" );
MATCH( 0, "XB" );

#endif
#endif

delete TheRegEx;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlRegExTest);

