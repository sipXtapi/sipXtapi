//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/SmimeBody.h>
#include <net/SdpBody.h>
#include <net/HttpMessage.h>
#include <os/OsFS.h>



#include <nspr.h>
#include <nss.h>
#include <secutil.h>
#include <secport.h>
#include <certdb.h>
#include <ssl.h>
#include <cms.h>
#include <cert.h>
#include <pk11func.h>
#include <pkcs12.h>
#include <pkcs11.h>
#include <p12plcy.h>
#include <p12.h>
#include <ciferfam.h>
#include <base64.h>
#include <prtypes.h>
#include "net/pk12wrapper.h"

/**
 * Unittest for SmimeBody
 */
class SmimeBodyTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SmimeBodyTest);
      CPPUNIT_TEST(testEncryptDecryptSuccess);
//      CPPUNIT_TEST(testEncryptFailureBadCert);
//      CPPUNIT_TEST(testEncryptFailureNoCert);
//      CPPUNIT_TEST(testDecryptFailureBadDbPassword);
//      CPPUNIT_TEST(testDecryptFailureNoDbPassword);
//      CPPUNIT_TEST(testDecryptFailureBadPkcs12Password);
//      CPPUNIT_TEST(testDecryptFailureNoPkcs12Password);
//      CPPUNIT_TEST(testDecryptFailureBadData);
//      CPPUNIT_TEST(testDecryptFailureNoData);
      CPPUNIT_TEST_SUITE_END();

public:


    void testEncryptFailureBadCert()
    {
        SMIME_ERRORS errCode;               

        // create an HttpBody
        UtlString testString("quote=So I was sitting in my cubicle today, and I realized, ever since I started working, every single day of my life has been worse than the day before it. So that means that every single day that you see me, that's on the worst day of my life.\r\n");
        SdpBody* pOriginalSdpBody = new SdpBody(testString.data(), (int)testString.length());
        
        // build a key
        const char* derPublicKeyCert[1];
        char szPublicKey[4096];
        unsigned long actualRead = 0;
        
        strcpy(szPublicKey, "this is not a valid public key");        
        actualRead = (unsigned long)strlen(szPublicKey);
        
        int certLength[1];
        UtlString der(szPublicKey, actualRead);
        
        derPublicKeyCert[0] = der.data();
        certLength[0] = (int)der.length();
        
        SmimeBody newlyEncryptedBody(NULL, 0, NULL);
               
        CPPUNIT_ASSERT(UtlBoolean(FALSE) == newlyEncryptedBody.encrypt(pOriginalSdpBody, 1, derPublicKeyCert, certLength, "mcohen@pingtel.com", "password", errCode));
    };

    void testEncryptFailureNoCert()
    {
        SMIME_ERRORS errCode;
                
        // create an HttpBody
        UtlString testString("quote=So I was sitting in my cubicle today, and I realized, ever since I started working, every single day of my life has been worse than the day before it. So that means that every single day that you see me, that's on the worst day of my life.\r\n");
        SdpBody* pOriginalSdpBody = new SdpBody(testString.data(), (int)testString.length());
        
        // build a key
        const char* derPublicKeyCert[1];
        char szPublicKey[4096];
        unsigned long actualRead = 0;
        
        strcpy(szPublicKey, "");        
        actualRead = 0;
        
        int certLength[1];
        UtlString der("");
        
        derPublicKeyCert[0] = der.data();
        certLength[0] = (int)der.length();
        
        SmimeBody newlyEncryptedBody(NULL, 0, NULL);
               
        CPPUNIT_ASSERT(UtlBoolean(FALSE) == newlyEncryptedBody.encrypt(pOriginalSdpBody, 1, derPublicKeyCert, certLength, "mcohen@pingtel.com", "password", errCode));
    }
    
    void testDecryptFailureBadDbPassword()
    {
        SMIME_ERRORS errCode;
    
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");
        
        // create an HttpBody
        UtlString testString("quote=So I was sitting in my cubicle today, and I realized, ever since I started working, every single day of my life has been worse than the day before it. So that means that every single day that you see me, that's on the worst day of my life.\r\n");
        SdpBody* pOriginalSdpBody = new SdpBody(testString.data(), (int)testString.length());
        
        // build a key
        const char* derPublicKeyCert[1];
        char szPublicKey[4096];
        unsigned long actualRead = 0;
        
        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, sizeof(szPublicKey), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        publicKeyFile.close();
        
        int certLength[1];
        UtlString der(szPublicKey, actualRead);
        
        derPublicKeyCert[0] = der.data();
        certLength[0] = (int)der.length();
        
        SmimeBody newlyEncryptedBody(NULL, 0, NULL);
               
        CPPUNIT_ASSERT(UtlBoolean(TRUE) == newlyEncryptedBody.encrypt(pOriginalSdpBody, 1, derPublicKeyCert, certLength, "mcohen@pingtel.com", "password", errCode));
        
        
        // now decrypt
        OsFile privateKeyFile("mcohen@pingtel.com.p12");
        // build a key
        char szPkcs12[4096];
        actualRead = 0;
        
        privateKeyFile.open();
        privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        privateKeyFile.close();

        UtlString pkcs12(szPkcs12, actualRead);
        
        UtlString bytes;
        int bodyLength = 0;
        
        newlyEncryptedBody.getBytes(&bytes, &bodyLength);
        SmimeBody decryptSmimeBody(bytes, bodyLength, NULL);

        CPPUNIT_ASSERT(UtlBoolean(FALSE) == decryptSmimeBody.decrypt(pkcs12, (int)pkcs12.length(), "password", "badpassword", der.data(), der.length(), errCode)); 
        
    };

    void testDecryptFailureNoDbPassword()
    {
        SMIME_ERRORS errCode;

        //NSS_SetDomesticPolicy();
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");
        
        // create an HttpBody
        UtlString testString("quote=So I was sitting in my cubicle today, and I realized, ever since I started working, every single day of my life has been worse than the day before it. So that means that every single day that you see me, that's on the worst day of my life.\r\n");
        SdpBody* pOriginalSdpBody = new SdpBody(testString.data(), (int)testString.length());
        
        // build a key
        const char* derPublicKeyCert[1];
        char szPublicKey[4096];
        unsigned long actualRead = 0;
        
        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, sizeof(szPublicKey), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        publicKeyFile.close();
        
        int certLength[1];
        UtlString der(szPublicKey, actualRead);
        
        derPublicKeyCert[0] = der.data();
        certLength[0] = (int)der.length();
        
        SmimeBody newlyEncryptedBody(NULL, 0, NULL);
               
        CPPUNIT_ASSERT(UtlBoolean(TRUE) == newlyEncryptedBody.encrypt(pOriginalSdpBody, 1, derPublicKeyCert, certLength, "mcohen@pingtel.com", "password", errCode));
        
        
        // now decrypt
        OsFile privateKeyFile("mcohen@pingtel.com.p12");
        // build a key
        char szPkcs12[4096];
        actualRead = 0;
        
        privateKeyFile.open();
        privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        privateKeyFile.close();

        UtlString pkcs12(szPkcs12, actualRead);
        
        UtlString bytes;
        int bodyLength = 0;
        
        newlyEncryptedBody.getBytes(&bytes, &bodyLength);
        SmimeBody decryptSmimeBody(bytes, bodyLength, NULL);
        
        CPPUNIT_ASSERT(UtlBoolean(FALSE) == decryptSmimeBody.decrypt(pkcs12, (int)pkcs12.length(), "password", "", der.data(), der.length(), errCode)); 
    };
    
    void testDecryptFailureBadPkcs12Password()
    {
        SMIME_ERRORS errCode;        
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");
        
        // create an HttpBody
        UtlString testString("quote=So I was sitting in my cubicle today, and I realized, ever since I started working, every single day of my life has been worse than the day before it. So that means that every single day that you see me, that's on the worst day of my life.\r\n");
        SdpBody* pOriginalSdpBody = new SdpBody(testString.data(), (int)testString.length());
        
        // build a key
        const char* derPublicKeyCert[1];
        char szPublicKey[4096];
        unsigned long actualRead = 0;
        
        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, sizeof(szPublicKey), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        publicKeyFile.close();
        
        int certLength[1];
        UtlString der(szPublicKey, actualRead);

        derPublicKeyCert[0] = der.data();
        certLength[0] = (int)der.length();
        
        SmimeBody newlyEncryptedBody(NULL, 0, NULL);
               
        CPPUNIT_ASSERT(UtlBoolean(TRUE) == newlyEncryptedBody.encrypt(pOriginalSdpBody, 1, derPublicKeyCert, certLength, "mcohen@pingtel.com", "password", errCode));
        
        
        // now decrypt
        OsFile privateKeyFile("mcohen@pingtel.com.p12");
        // build a key
        char szPkcs12[4096];
        actualRead = 0;
        
        privateKeyFile.open();
        privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        privateKeyFile.close();

        UtlString pkcs12(szPkcs12, actualRead);
        
        UtlString bytes;
        int bodyLength = 0;
        
        newlyEncryptedBody.getBytes(&bytes, &bodyLength);
        SmimeBody decryptSmimeBody(bytes, bodyLength, NULL);

        CPPUNIT_ASSERT(UtlBoolean(FALSE) == decryptSmimeBody.decrypt(pkcs12, (int)pkcs12.length(), "badpassword", "password", der.data(), der.length(), errCode)); 
    };


    void testDecryptFailureNoPkcs12Password()
    {
        SMIME_ERRORS errCode;
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");
        
        // create an HttpBody
        UtlString testString("quote=So I was sitting in my cubicle today, and I realized, ever since I started working, every single day of my life has been worse than the day before it. So that means that every single day that you see me, that's on the worst day of my life.\r\n");
        SdpBody* pOriginalSdpBody = new SdpBody(testString.data(), (int)testString.length());
        
        // build a key
        const char* derPublicKeyCert[1];
        char szPublicKey[4096];
        unsigned long actualRead = 0;
        
        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, sizeof(szPublicKey), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        publicKeyFile.close();
        
        int certLength[1];
        UtlString der(szPublicKey, actualRead);
        
        derPublicKeyCert[0] = der.data();
        certLength[0] = (int)der.length();
        
        SmimeBody newlyEncryptedBody(NULL, 0, NULL);
               
        CPPUNIT_ASSERT(UtlBoolean(TRUE) == newlyEncryptedBody.encrypt(pOriginalSdpBody, 1, derPublicKeyCert, certLength, "mcohen@pingtel.com", "password", errCode));
        
        
        // now decrypt
        OsFile privateKeyFile("mcohen@pingtel.com.p12");
        // build a key
        char szPkcs12[4096];
        actualRead = 0;
        
        privateKeyFile.open();
        privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        privateKeyFile.close();

        UtlString pkcs12(szPkcs12, actualRead);
        
        UtlString bytes;
        int bodyLength = 0;
        
        newlyEncryptedBody.getBytes(&bytes, &bodyLength);
        SmimeBody decryptSmimeBody(bytes, bodyLength, NULL);
        
        CPPUNIT_ASSERT(UtlBoolean(FALSE) == decryptSmimeBody.decrypt(pkcs12, (int)pkcs12.length(), "", "password", der.data(), der.length(), errCode)); 
    };

    void testDecryptFailureBadData()
    {
        SMIME_ERRORS errCode;        
        
        // now decrypt
        OsFile privateKeyFile("mcohen@pingtel.com.p12");
        // build a key
        char szPkcs12[4096];
        unsigned long actualRead = 0;
        
        privateKeyFile.open();
        privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        privateKeyFile.close();

        UtlString pkcs12(szPkcs12, actualRead);
        
        UtlString bytes;
        int bodyLength = 0;
        
        UtlString nonDecryptable("this will not decrypt");
        SmimeBody decryptSmimeBody(nonDecryptable, nonDecryptable.length(), NULL);

        CPPUNIT_ASSERT(UtlBoolean(FALSE) == decryptSmimeBody.decrypt(pkcs12, (int)pkcs12.length(), "password", "password", NULL, 0, errCode)); 
    };

    void testDecryptFailureNoData()
    {
        SMIME_ERRORS errCode;        
        
        // now decrypt
        OsFile privateKeyFile("mcohen@pingtel.com.p12");
        // build a key
        char szPkcs12[4096];
        unsigned long actualRead = 0;
        
        privateKeyFile.open();
        privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        privateKeyFile.close();

        UtlString pkcs12(szPkcs12, actualRead);
        
        UtlString bytes;
        int bodyLength = 0;
        
        SmimeBody decryptSmimeBody(NULL, 0, NULL);

        CPPUNIT_ASSERT(UtlBoolean(FALSE) == decryptSmimeBody.decrypt(pkcs12, (int)pkcs12.length(), "password", "password", "", 0, errCode)); 
    };

    void testEncryptDecryptSuccess()
    {
        SMIME_ERRORS errCode;
        
        int initRc = P12Wrapper_Init(".", "");

        //NSS_SetDomesticPolicy();
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");


        for (int i = 0; i < 10; i++)
        {
        
            // create an HttpBody
            UtlString testString("quote=foo foo foo\r\n");
            SdpBody* pOriginalSdpBody = new SdpBody(testString.data(), (int)testString.length());
        
            // build a key
            const char* derPublicKeyCert[1];
            char szPublicKey[4096];
            unsigned long actualRead = 0;
        
            publicKeyFile.open();
            publicKeyFile.read((void*)szPublicKey, sizeof(szPublicKey), actualRead);
            CPPUNIT_ASSERT(actualRead > 0);
            publicKeyFile.close();
        
            int certLength[1];
            UtlString der(szPublicKey, actualRead);
        
            derPublicKeyCert[0] = der.data();
            certLength[0] = (int)der.length();
        
            SmimeBody newlyEncryptedBody(NULL, 0, NULL);
               
            char szPassword[256];
            sprintf(szPassword, "password");
            CPPUNIT_ASSERT(UtlBoolean(TRUE) == newlyEncryptedBody.encrypt(pOriginalSdpBody, 1, derPublicKeyCert, certLength, "mcohen@pingtel.com", szPassword, errCode));
        
            /*
            // now decrypt
            OsFile privateKeyFile("mcohen@pingtel.com.p12");
            // build a key
            char szPkcs12[4096];
            actualRead = 0;
        
            privateKeyFile.open();
            privateKeyFile.read((void*)szPkcs12, sizeof(szPkcs12), actualRead);
            CPPUNIT_ASSERT(actualRead > 0);
            privateKeyFile.close();

            UtlString pkcs12(szPkcs12, actualRead);
            */
            
            initRc = P12Wrapper_Init(".", "");
            UtlString bytes;
            int bodyLength = 0;
        
            newlyEncryptedBody.getBytes(&bytes, &bodyLength);
            SmimeBody decryptSmimeBody(bytes, bodyLength, NULL);
        
            SMIME_ERRORS errCode;
            decryptSmimeBody.decrypt(NULL, 0, NULL, "password", der.data(), der.length(), errCode, true); 
        
            UtlString decryptedSdpBodyBytes;
            int       decryptedSdpBodyLength;
            decryptSmimeBody.getDecryptedBody()->getBytes(&decryptedSdpBodyBytes, &decryptedSdpBodyLength);
            UtlString decryptedString(decryptedSdpBodyBytes);
            CPPUNIT_ASSERT(decryptedString == testString);
        
            // the original SdpBody gets deleted when the SmimeBody 
            // (newlyEncryptedBody) goes out of scope
            //delete pOriginalSdpBody;

        }
        //NSS_Shutdown();
    };

};

CPPUNIT_TEST_SUITE_REGISTRATION(SmimeBodyTest);
