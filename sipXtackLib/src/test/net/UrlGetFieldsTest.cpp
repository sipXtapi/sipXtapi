//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <string.h>
#include <net/Url.h>
#include <net/HttpMessage.h>
#include <net/NetMd5Codec.h>
#include <utl/UtlTokenizer.h>

#define MISSING_PARAM  "---missing---"

#define ASSERT_ARRAY_MESSAGE(message, expected, actual) \
  UrlGetFieldsTest::assertArrayMessage((expected),(actual), \
      CPPUNIT_SOURCELINE(), (message))

class UrlGetFieldsTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UrlGetFieldsTest);
    CPPUNIT_TEST(testGetFields);
    CPPUNIT_TEST_SUITE_END();

private:

    UtlString *assertValue;

public:

    void setUp()
    {
        assertValue = new UtlString();
    }

    void tearDown()
    {
        delete assertValue;
    }

    // Service functions that extract values (after removal
    // of quoting) from components of a Url for use by
    // testParameterQuoting.
    static void testParameterQuotingGetDisplayName(Url& uri, UtlString& value)
    {
       uri.getDisplayName(value);
    }
    static void testParameterQuotingGetUser(Url& uri, UtlString& value)
    {
       uri.getUserId(value);
    }
    static void testParameterQuotingGetPassword(Url& uri, UtlString& value)
    {
       uri.getPassword(value);
    }
    static void testParameterQuotingGetHost(Url& uri, UtlString& value)
    {
       uri.getHostAddress(value);
    }
    static void testParameterQuotingGetUrlParameterName(Url& uri,
                                                        UtlString& name)
    {
       UtlString value;
       uri.getUrlParameter(0, name, value);
    }
    static void testParameterQuotingGetUrlParameterValue(Url& uri,
                                                         UtlString& value)
    {
       uri.getUrlParameter("pname", value, 0);
    }
    static void testParameterQuotingGetHeaderName(Url& uri,
                                                  UtlString& name)
    {
       UtlString value;
       uri.getHeaderParameter(0, name, value);
    }
    static void testParameterQuotingGetHeaderValue(Url& uri,
                                                   UtlString& value)
    {
       uri.getHeaderParameter("hname", value, 0);
    }
    static void testParameterQuotingGetFieldParameterName(Url& uri,
                                                          UtlString& name)
    {
       UtlString value;
       uri.getFieldParameter(0, name, value);
    }
    static void testParameterQuotingGetFieldParameterValue(Url& uri,
                                                           UtlString& value)
    {
       uri.getFieldParameter("pname", value, 0);
    }

    void testGetFields()
    {
       // Test to check that characters are quoted and un-quoted when
       // components of URIs are saved and retrieved.

       int total_tests_executed = 0;

       // Information to describe each way in which a character can be used
       // in an URI.
       struct test_group {
          // Description of this test group to be used in output labels.
          const char* description;
          // The characters that can appear in this test group.
          const char* characters_to_test;
          // sprintf format to generate a URI incorporating a character
          // in this test group.
          const char* uri_string_format;
          // The URI format version, either URI or contact-param.
          UtlBoolean uri_version;
          // sprintf format to generate the expected value string.
          const char* expected_value_format;
          // Function to extract the actual (as opposed to expected)
          // value from the Url.
          // See the testParameterQuotingGet* helper functions
          // defined above.
          void (*get_actual_value)(Url& uri, UtlString& value);
       };

       // Character classes.
       // The classes are named after the productions in the grammar in
       // section 25.1 of RFC 3261.
       #define CHAR_CLASS_ALPHANUM \
                   "ABCDEFGHIJKLMNOPQRSTUVEXYZabcdefghijklmnopqrstuvexyz" \
                   "0123456789"
       #define CHAR_CLASS_RESERVED \
                   ";/?:@&=+$,"
       #define CHAR_CLASS_MARK \
                   "-_.!~*'()"
       // characters that can be represented by %xx
       // characters that can be escaped by backslash in quoted-string
       // %xx can represent all chars from %00 to %ff, but
       // quoted-string cannot contain CR or LF.
       // I include only the graphic characters (including TAB).
       #define CHAR_CLASS_ESCAPED \
                "\t" \
                " !\"#$%&'()*+,-./0123456789:;<=>?" \
                "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_" \
                "`abcdefghijklmnopqrstuvwxyz{|}~"
       #define CHAR_CLASS_UNRESERVED \
                CHAR_CLASS_ALPHANUM \
                CHAR_CLASS_MARK
       #define CHAR_CLASS_USER_UNRESERVED \
                "&=+$,;?/"
       #define CHAR_CLASS_TOKEN \
                CHAR_CLASS_ALPHANUM \
                "-.!%*_+`'~"
       #define CHAR_CLASS_TOKEN_HOST \
                CHAR_CLASS_TOKEN \
                "[]:"
       #define CHAR_CLASS_PARAM_UNRESERVED \
                "[]/:&+$"
       // paramchar, without escaped
       #define CHAR_CLASS_PARAMCHAR \
                CHAR_CLASS_PARAM_UNRESERVED \
                CHAR_CLASS_UNRESERVED
       #define CHAR_CLASS_HNV_UNRESERVED \
                "[]/?:+$"
       // Characters that can appear in a quoted string without a backslash.
       // Backslash and double-quote are excluded.
       #define CHAR_CLASS_QSTRING_UNQUOTED \
                "\t" \
                " !#$%&'()*+,-./0123456789:;<=>?" \
                "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_" \
                "`abcdefghijklmnopqrstuvwxyz{|}~"
       // Characters that can appear in a quoted string with a backslash.
       #define CHAR_CLASS_QSTRING_QUOTED \
                CHAR_CLASS_QSTRING_UNQUOTED \
                "\"\\"

       // Code names for URI formats.
       // These values are the second argument to the Url constructor.
       // The SIP URI format (productions SIP-URI and addrspec in RFC 3261).
       #define STRING_URI TRUE
       // The header field format (production contact-param in RFC
       // 3261, as modified by the last paragraph of section 20.)
       #define STRING_HEADER FALSE

       // Ways in which characters need to be tested.
       struct test_group test_groups[] =
          {
             // URI format

             // user
             // Characters that need not be quoted
             {
                "unquoted in URI user",
                CHAR_CLASS_UNRESERVED CHAR_CLASS_USER_UNRESERVED,
                "sip:%c@host",
                STRING_URI,
                "%c",
                testParameterQuotingGetUser,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI user",
                CHAR_CLASS_ESCAPED,
                "sip:%%%02X@host",
                STRING_URI,
                "%c",
                testParameterQuotingGetUser,
             },
             // Note: According to the RFC, %-escapes must be upper-case.
             // But we test lower-case as well, for robustness.
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI user",
                CHAR_CLASS_ESCAPED,
                "sip:%%%02x@host",
                STRING_URI,
                "%c",
                testParameterQuotingGetUser,
             },

             // password
             // Characters that need not be quoted
             {
                "unquoted in URI password",
                CHAR_CLASS_UNRESERVED
                "&=+$,",
                "sip:user:%c@host",
                STRING_URI,
                "%c",
                testParameterQuotingGetPassword,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI password",
                CHAR_CLASS_ESCAPED,
                "sip:user:%%%02X@host",
                STRING_URI,
                "%c",
                testParameterQuotingGetPassword,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI password",
                CHAR_CLASS_ESCAPED,
                "sip:user:%%%02x@host",
                STRING_URI,
                "%c",
                testParameterQuotingGetPassword,
             },

             // hosts
             {
                "in host",
                CHAR_CLASS_ALPHANUM "-",
                "sip:user@a%cb",
                STRING_URI,
                "a%cb",
                testParameterQuotingGetHost,
             },

             // URL parameter names
             {
                "unquoted in URI parameter name",
                CHAR_CLASS_PARAMCHAR,
                "sip:user@host;%c=1",
                STRING_URI,
                "%c",
                testParameterQuotingGetUrlParameterName,
             },

             // URL parameter value
             // Characters that need not be quoted
             {
                "unquoted in URI parameter value",
                CHAR_CLASS_PARAMCHAR,
                "sip:user@host;pname=%c",
                STRING_URI,
                "%c",
                testParameterQuotingGetUrlParameterValue,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI parameter value",
                CHAR_CLASS_ESCAPED,
                "sip:user@host;pname=%%%02X",
                STRING_URI,
                "%c",
                testParameterQuotingGetUrlParameterValue,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI parameter value",
                CHAR_CLASS_ESCAPED,
                "sip:user@host;pname=%%%02x",
                STRING_URI,
                "%c",
                testParameterQuotingGetUrlParameterValue,
             },

             // URL header names
             // Characters that need not be quoted
             {
                "unquoted in URI header name",
                CHAR_CLASS_HNV_UNRESERVED CHAR_CLASS_UNRESERVED,
                "sip:user@host?%c=1",
                STRING_URI,
                "%c",
                testParameterQuotingGetHeaderName,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI header name",
                CHAR_CLASS_ESCAPED,
                "sip:user@host?%%%02X=1",
                STRING_URI,
                "%c",
                testParameterQuotingGetHeaderName,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI header name",
                CHAR_CLASS_ESCAPED,
                "sip:user@host?%%%02x=1",
                STRING_URI,
                "%c",
                testParameterQuotingGetHeaderName,
             },

             // URL header value
             // Characters that need not be quoted
             {
                "unquoted in URI header value",
                CHAR_CLASS_HNV_UNRESERVED CHAR_CLASS_UNRESERVED,
                "sip:user@host?hname=%c",
                STRING_URI,
                "%c",
                testParameterQuotingGetHeaderValue,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI header value",
                CHAR_CLASS_ESCAPED,
                "sip:user@host?hname=%%%02X",
                STRING_URI,
                "%c",
                testParameterQuotingGetHeaderValue,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI header value",
                CHAR_CLASS_ESCAPED,
                "sip:user@host?hname=%%%02x",
                STRING_URI,
                "%c",
                testParameterQuotingGetHeaderValue,
             },

             // header field format, without <...>

             // user
             // Characters that need not be quoted
             {
                "unquoted in URI user",
                CHAR_CLASS_UNRESERVED CHAR_CLASS_USER_UNRESERVED,
                "sip:%c@host",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUser,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI user",
                CHAR_CLASS_ESCAPED,
                "sip:%%%02X@host",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUser,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI user",
                CHAR_CLASS_ESCAPED,
                "sip:%%%02x@host",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUser,
             },

             // password
             // Characters that need not be quoted
             {
                "unquoted in URI password",
                CHAR_CLASS_UNRESERVED "&=+$,",
                "sip:user:%c@host",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetPassword,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI password",
                CHAR_CLASS_ESCAPED,
                "sip:user:%%%02X@host",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetPassword,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI password",
                CHAR_CLASS_ESCAPED,
                "sip:user:%%%02x@host",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetPassword,
             },

             // hosts
             {
                "in host",
                CHAR_CLASS_ALPHANUM "-",
                "sip:user@a%cb",
                STRING_HEADER,
                "a%cb",
                testParameterQuotingGetHost,
             },

             // URL header names
             // Characters that need not be quoted
             {
                "unquoted in URI header name",
                CHAR_CLASS_HNV_UNRESERVED CHAR_CLASS_UNRESERVED,
                "sip:user@host?%c=1",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderName,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI header name",
                CHAR_CLASS_ESCAPED,
                "sip:user@host?%%%02X=1",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderName,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI header name",
                CHAR_CLASS_ESCAPED,
                "sip:user@host?%%%02x=1",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderName,
             },

             // URL header value
             // Characters that need not be quoted
             {
                "unquoted in URI header value",
                CHAR_CLASS_HNV_UNRESERVED CHAR_CLASS_UNRESERVED,
                "sip:user@host?hname=%c",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderValue,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI header value",
                CHAR_CLASS_ESCAPED,
                "sip:user@host?hname=%%%02X",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderValue,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI header value",
                CHAR_CLASS_ESCAPED,
                "sip:user@host?hname=%%%02x",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderValue,
             },

             // header parameter names
             // Characters that need not be quoted
             {
                "unquoted in header parameter name",
                CHAR_CLASS_TOKEN,
                "sip:user@host;%c=1",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetFieldParameterName,
             },

             // header parameter value
             // The syntax is under "generic-param" and "gen-value"
             // Characters that need not be quoted
             {
                "unquoted in header parameter value",
                CHAR_CLASS_TOKEN_HOST,
                "sip:user@host;pname=%c",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetFieldParameterValue,
             },
#if 0 /* semicolon fails */
// Also, these tests don't account for the feature tag syntax.
             // Quoted characters, not backslashed.
             {
                "quoted but not backslashed in header parameter value, initial position",
                CHAR_CLASS_QSTRING_UNQUOTED,
                "sip:user@host;pname=\"%cxx\"",
                STRING_HEADER,
                "%cxx",
                testParameterQuotingGetFieldParameterValue,
             },
             {
                "quoted but not backslashed in header parameter value, medial position",
                CHAR_CLASS_QSTRING_UNQUOTED,
                "sip:user@host;pname=\"x%cx\"",
                STRING_HEADER,
                "x%cx",
                testParameterQuotingGetFieldParameterValue,
             },
             {
                "quoted but not backslashed in header parameter value, final position",
                CHAR_CLASS_QSTRING_UNQUOTED,
                "sip:user@host;pname=\"xx%c\"",
                STRING_HEADER,
                "xx%c",
                testParameterQuotingGetFieldParameterValue,
             },
             // Quoted characters, backslashed
             {
                "quoted and backslashed in header parameter value, initial position",
                CHAR_CLASS_ESCAPED,
                "sip:user@host;pname=\"\\%cxx\"",
                STRING_HEADER,
                "%cxx",
                testParameterQuotingGetFieldParameterValue,
             },
             {
                "quoted and backslashed in header parameter value",
                CHAR_CLASS_ESCAPED,
                "sip:user@host;pname=\"x\\%cx\"",
                STRING_HEADER,
                "x%cx",
                testParameterQuotingGetFieldParameterValue,
             },
             {
                "quoted and backslashed in header parameter value",
                CHAR_CLASS_ESCAPED,
                "sip:user@host;pname=\"xx\\%c\"",
                STRING_HEADER,
                "xx%c",
                testParameterQuotingGetFieldParameterValue,
             },
#endif

             // header field format, with <...>

             // display name
             // Characters that need not be quoted
             {
                "unquoted in display name",
                CHAR_CLASS_TOKEN,
                "%c <sip:user@host>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetDisplayName,
             },
             // Characters in quoted string
             {
                "quoted string in display name",
                CHAR_CLASS_QSTRING_UNQUOTED,
                "\"%c\" <sip:user@host>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetDisplayName,
             },
             // Backslash-escaped characters in quoted string
             {
                "backslashed in quoted string in display name",
                CHAR_CLASS_QSTRING_QUOTED,
                "\"\\%c\" <sip:user@host>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetDisplayName,
             },

             // user
             // Characters that need not be quoted
             {
                "unquoted in URI user",
                CHAR_CLASS_UNRESERVED CHAR_CLASS_USER_UNRESERVED,
                "<sip:%c@host>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUser,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI user",
                CHAR_CLASS_ESCAPED,
                "<sip:%%%02X@host>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUser,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI user",
                CHAR_CLASS_ESCAPED,
                "<sip:%%%02x@host>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUser,
             },

             // password
             // Characters that need not be quoted
             {
                "unquoted in URI password",
                CHAR_CLASS_UNRESERVED
                "&=+$,",
                "<sip:user:%c@host>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetPassword,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI password",
                CHAR_CLASS_ESCAPED,
                "<sip:user:%%%02X@host>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetPassword,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI password",
                CHAR_CLASS_ESCAPED,
                "sip:user:%%%02x@host",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetPassword,
             },

             // hosts
             {
                "in host",
                CHAR_CLASS_ALPHANUM "-",
                "<sip:user@a%cb>",
                STRING_HEADER,
                "a%cb",
                testParameterQuotingGetHost,
             },

             // URL parameter names
             {
                "unquoted in URI parameter name",
                CHAR_CLASS_PARAMCHAR,
                "<sip:user@host;%c=1>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUrlParameterName,
             },

             // URL parameter value
             // Characters that need not be quoted
             {
                "unquoted in URI parameter value",
                CHAR_CLASS_PARAMCHAR,
                "<sip:user@host;pname=%c>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUrlParameterValue,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI parameter value",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host;pname=%%%02X>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUrlParameterValue,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI parameter value",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host;pname=%%%02x>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetUrlParameterValue,
             },

             // URL header names
             // Characters that need not be quoted
             {
                "unquoted in URI header name",
                CHAR_CLASS_HNV_UNRESERVED CHAR_CLASS_UNRESERVED,
                "<sip:user@host?%c=1>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderName,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI header name",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host?%%%02X=1>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderName,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI header name",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host?%%%02x=1>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderName,
             },

             // URL header value
             // Characters that need not be quoted
             {
                "unquoted in URI header value",
                CHAR_CLASS_HNV_UNRESERVED CHAR_CLASS_UNRESERVED,
                "<sip:user@host?hname=%c>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderValue,
             },
             // Escaped characters, upper-case
             {
                "escaped (upper-case) in URI header value",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host?hname=%%%02X>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderValue,
             },
             // Escaped characters, lower-case
             {
                "escaped (lower-case) in URI header value",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host?hname=%%%02x>",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetHeaderValue,
             },

             // header parameter names
             // Characters that need not be quoted
             {
                "unquoted in header parameter name",
                CHAR_CLASS_TOKEN,
                "<sip:user@host>;%c=1",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetFieldParameterName,
             },

             // header parameter value
             // Characters that need not be quoted
             {
                "unquoted in header parameter value",
                CHAR_CLASS_TOKEN_HOST,
                "<sip:user@host>;pname=%c",
                STRING_HEADER,
                "%c",
                testParameterQuotingGetFieldParameterValue,
             },
#if 0
             // Quoted characters, not backslashed.
             {
                "quoted but not backslashed in header parameter value, initial position",
                CHAR_CLASS_QSTRING_UNQUOTED,
                "<sip:user@host>;pname=\"%cxx\"",
                STRING_HEADER,
                "%cxx",
                testParameterQuotingGetFieldParameterValue,
             },
             {
                "quoted but not backslashed in header parameter value, medial position",
                CHAR_CLASS_QSTRING_UNQUOTED,
                "<sip:user@host>;pname=\"x%cx\"",
                STRING_HEADER,
                "x%cx",
                testParameterQuotingGetFieldParameterValue,
             },
             {
                "quoted but not backslashed in header parameter value, final position",
                CHAR_CLASS_QSTRING_UNQUOTED,
                "<sip:user@host>;pname=\"xx%c\"",
                STRING_HEADER,
                "xx%c",
                testParameterQuotingGetFieldParameterValue,
             },
             // Quoted characters, backslashed
             {
                "quoted and backslashed in header parameter value, initial position",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host>;pname=\"\\%cxx\"",
                STRING_HEADER,
                "%cxx",
                testParameterQuotingGetFieldParameterValue,
             },
             {
                "quoted and backslashed in header parameter value",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host>;pname=\"x\\%cx\"",
                STRING_HEADER,
                "x%cx",
                testParameterQuotingGetFieldParameterValue,
             },
             {
                "quoted and backslashed in header parameter value",
                CHAR_CLASS_ESCAPED,
                "<sip:user@host>;pname=\"xx\\%c\"",
                STRING_HEADER,
                "xx%c",
                testParameterQuotingGetFieldParameterValue,
             },
#endif
          };

       // For each test group, test each character.
       for (unsigned int test_group = 0;
            test_group < sizeof (test_groups) / sizeof (struct test_group);
            test_group++)
       {
          // For each character, construct a URI containing it as a
          // parameter value.
          unsigned int number_of_chars =
             strlen(test_groups[test_group].characters_to_test);
          for (unsigned int i = 0; i < number_of_chars; i++)
          {
             // Get the character to test.
             char character_to_test = test_groups[test_group].characters_to_test[i];
             // Construct the URI string containing it.
             char uri_string[100];
             sprintf(uri_string, test_groups[test_group].uri_string_format,
                     character_to_test);
             // Construct the URI object containing it.
             Url uri(uri_string, test_groups[test_group].uri_version);
             // Construct the expected value string.
             char expected_value[100];
             sprintf(expected_value, test_groups[test_group].expected_value_format,
                     character_to_test);
             // Extract the actual value.
             UtlString actual_value;
             test_groups[test_group].get_actual_value(uri, actual_value);
             // Format the description of the test.
             char msg[1000];    // Some of these messages are quite long.
             sprintf(msg, "Testing char '%c' %s in %s format: '%s'",
                     character_to_test, test_groups[test_group].description,
                     (test_groups[test_group].uri_version ? "URI" : "header"),
                     uri_string);
//printf("%s\n", msg);
             // Increment the count of tests executed.
             total_tests_executed++;
             // Check the results.
#if 1
             ASSERT_STR_EQUAL_MESSAGE(msg, expected_value, (char*) actual_value.data());
#else
             if (strcmp(expected_value, actual_value.data()) != 0)
             {
                fprintf(stderr,
                        "test: UrlTest::testParameterQuoting line: %d "
                        __FILE__ "\n"
                        "equality assertion failed\n"
                        "- Expected: \"%s\"\n"
                        "- Actual  : \"%s\"\n"
                        "- %s\n",
                        __LINE__,
                        expected_value, actual_value.data(), msg);
             }
#endif
          }
       }

       // Print the total number of tests.
       fprintf(stderr, "Total tests executed: %d\n",
               total_tests_executed);
    }

    /////////////////////////
    // Helper Methods

    const char *getParam(const char *szName, Url &url)
    {
        UtlString name(szName);        
        if (!url.getUrlParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getHeaderParam(const char *szName, Url &url)
    {
        UtlString name(szName);        
        if (!url.getHeaderParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getFieldParam(const char *szName, Url &url, int ndx)
    {
        UtlString name(szName);        
        if (!url.getFieldParameter(name, *assertValue, ndx))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getFieldParam(const char *szName, Url &url)
    {
        UtlString name(szName);        
        if (!url.getFieldParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *toString(const Url& url)
    {
        assertValue->remove(0);
        url.toString(*assertValue);

        return assertValue->data();
    }

    const char *getHostAddress(const Url& url)
    {
        assertValue->remove(0);
        url.getHostAddress(*assertValue);

        return assertValue->data();
    }

    const char *getUrlType(const Url& url)
    {
        assertValue->remove(0);
        url.getUrlType(*assertValue);

        return assertValue->data();
    }

    /** API not declared as const **/
    const char *getUri(Url& url)
    {
        assertValue->remove(0);
        url.getUri(*assertValue);

        return assertValue->data();
    }

    /** API not declared as const **/
    const char *getPath(Url& url, UtlBoolean withQuery = FALSE)
    {
        assertValue->remove(0);
        url.getPath(*assertValue, withQuery);

        return assertValue->data();
    }

    /** API not declared as const **/
    const char *getUserId(Url& url)
    {
        assertValue->remove(0);
        url.getUserId(*assertValue);

        return assertValue->data();
    }

    /** API not declared as const **/
    const char *getIdentity(Url& url)
    {
        assertValue->remove(0);
        url.getIdentity(*assertValue);

        return assertValue->data();
    }

    const char *getDisplayName(const Url &url)
    {
        assertValue->remove(0);
        url.getDisplayName(*assertValue);

        return assertValue->data();
    }

    void assertArrayMessage(const char *expectedTokens, UtlString *actual, 
        CppUnit::SourceLine sourceLine, std::string msg)
    {
        UtlString expected;
        UtlTokenizer toks(expectedTokens);
        for (int i = 0; toks.next(expected, " "); i++)
        {
            TestUtilities::assertEquals(expected.data(), actual[i].data(), 
                sourceLine, msg);
            expected.remove(0);
        }
    }
}; 

CPPUNIT_TEST_SUITE_REGISTRATION(UrlGetFieldsTest);
