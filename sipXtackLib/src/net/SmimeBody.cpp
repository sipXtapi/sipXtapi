#ifdef OPENSSL
// 
// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SmimeBody.h>
#include <net/HttpMessage.h>
#include <os/OsSysLog.h>
#include <os/OsDefs.h>

//#define ENABLE_OPENSSL_SMIME
#ifdef ENABLE_OPENSSL_SMIME
#   include <openssl/bio.h>
#   include <openssl/pem.h>
#   include <openssl/crypto.h>
#   include <openssl/err.h>
#   include <openssl/pkcs12.h>
#endif

//#define ENABLE_NSS_SMIME
#ifdef ENABLE_NSS_SMIME
#   include <nspr.h>
#   include <nss.h>
#   include <ssl.h>
#   include <cms.h>
#   include <pk11func.h>
#   include <p12.h>
#   include <base64.h>
    extern "C" CERTCertificate *
    __CERT_DecodeDERCertificate(SECItem *derSignedCert, PRBool copyDER, char *nickname);
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
#ifdef ENABLE_OPENSSL_SMIME
static void dumpOpensslError()
{
    BIO* bioErrorBuf = BIO_new(BIO_s_mem());
    ERR_print_errors(bioErrorBuf);
    BUF_MEM *bioErrorMem;
    BIO_get_mem_ptr(bioErrorBuf, &bioErrorMem);
    if(bioErrorMem)
    {
        UtlString errorString;
        errorString.append(bioErrorMem->data, bioErrorMem->length);
        printf("OPENSSL Error:\n%s\n", errorString.data());
    }
}
#endif
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default Constructor
SmimeBody::SmimeBody()
{
    mContentEncoding = SMIME_ENODING_BINARY;
    append(CONTENT_SMIME_PKCS7);
    mClassType = SMIME_BODY_CLASS;
    mpDecryptedBody = NULL;
}
// Constructor
SmimeBody::SmimeBody(const char* bytes, 
                     int length,
                     const char* smimeEncodingType)
{
    bodyLength = length;
    mBody.append(bytes, length);

    // Remove to ake sure base class did not already set it
    remove(0);
    append(CONTENT_SMIME_PKCS7);

    mClassType = SMIME_BODY_CLASS;
    mContentEncoding = SMIME_ENODING_UNKNOWN;
    if(smimeEncodingType)
    {
        UtlString sMimeEncodingString(smimeEncodingType);
        sMimeEncodingString.toUpper();

        if(sMimeEncodingString.compareTo(HTTP_CONTENT_TRANSFER_ENCODING_BINARY, UtlString::ignoreCase) == 0)
        {
            mContentEncoding = SMIME_ENODING_BINARY;
        }
        else if(sMimeEncodingString.compareTo(HTTP_CONTENT_TRANSFER_ENCODING_BASE64, UtlString::ignoreCase) == 0)
        {
            mContentEncoding = SMIME_ENODING_BASE64;
        }
        else
        {
            // TODO: We could probably put a hack in here to heuristically
            // determine if the encoding is base64 or not based upon the
            // byte values.
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "Invalid transport encoding for S/MIME content");
        }
    }

    mpDecryptedBody = NULL;
}

// Copy constructor
SmimeBody::SmimeBody(const SmimeBody& rSmimeBody)
{
    // Copy the base class stuff
    this->HttpBody::operator=((const HttpBody&)rSmimeBody);

    mpDecryptedBody = NULL;
    if(rSmimeBody.mpDecryptedBody)
    {
        mpDecryptedBody = HttpBody::copyBody(*(rSmimeBody.mpDecryptedBody));
    }

    mClassType = SMIME_BODY_CLASS;

    // Remove to ake sure base class did not already set it
    remove(0);
    append(CONTENT_SMIME_PKCS7);
    mContentEncoding = rSmimeBody.mContentEncoding;
}

// Destructor
SmimeBody::~SmimeBody()
{
   if(mpDecryptedBody)
   {
       delete mpDecryptedBody;
       mpDecryptedBody = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SmimeBody&
SmimeBody::operator=(const SmimeBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Copy the parent
   *((HttpBody*)this) = rhs;

   // Set the class type just to play it safe
   mClassType = SMIME_BODY_CLASS;

   // Remove the decrypted body if one is attached,
   // in case we copy over it
   if(mpDecryptedBody)
   {
       delete mpDecryptedBody;
       mpDecryptedBody = NULL;
   }

   // If the source has a body copy it
   if(rhs.mpDecryptedBody)
   {
       mpDecryptedBody = HttpBody::copyBody(*(rhs.mpDecryptedBody));
   }

   mContentEncoding = rhs.mContentEncoding;
   return *this;
}

UtlBoolean SmimeBody::decrypt(const char* derPkcs12,
                              int derPkcs12Length,
                              const char* pkcs12Password)
{
    UtlBoolean decryptionSucceeded = FALSE;

    UtlString decryptedData;

#ifdef ENABLE_OPENSSL_SMIME
    decryptionSucceeded = 
        opensslSmimeDecrypt(derPkcs12,
                            derPkcs12Length,
                            pkcs12Password,
                            (mContentEncoding == SMIME_ENODING_BASE64),
                            mBody.data(),
                            mBody.length(),
                            decryptedData);
#elif ENABLE_NSS_SMIME
    OsSysLog::add(FAC_SIP, PRI_ERR, "NSS S/MIME decrypt not implemented");
#endif

    // Decryption succeeded, so create a HttpBody for the result
    if(decryptionSucceeded && 
        decryptedData.length() > 0)
    {
        HttpBody* newDecryptedBody = NULL;
        // Need to read the headers before the real body to see
        // what the content type of the decrypted body is
        UtlDList bodyHeaders;
        int parsedBytes = 
            HttpMessage::parseHeaders(decryptedData.data(), 
                                      decryptedData.length(),
                                      bodyHeaders);

        UtlString contentTypeName(HTTP_CONTENT_TYPE_FIELD);
        NameValuePair* contentType = 
            (NameValuePair*) bodyHeaders.find(&contentTypeName);
        UtlString contentEncodingName(HTTP_CONTENT_TRANSFER_ENCODING_FIELD);
        NameValuePair* contentEncoding = 
            (NameValuePair*) bodyHeaders.find(&contentEncodingName);
        
        const char* realBodyStart = decryptedData.data() + parsedBytes;
        int realBodyLength = decryptedData.length() - parsedBytes;

        newDecryptedBody = 
            HttpBody::createBody(realBodyStart,
                                 realBodyLength,
                                 contentType ? contentType->getValue() : NULL,
                                 contentEncoding ? contentEncoding->getValue() : NULL);

        bodyHeaders.destroyAll();

        // If one already exists, delete it.  This should not typically
        // be the case.  Infact it might make sense to make this method
        // a no-op if a decrypted body already exists
        if(mpDecryptedBody)
        {
            delete mpDecryptedBody;
            mpDecryptedBody = NULL;
        }

        mpDecryptedBody = newDecryptedBody;
    }

    return(decryptionSucceeded);
}

UtlBoolean SmimeBody::encrypt(HttpBody* bodyToEncrypt,
                              int numRecipients,
                              const char* derPublicKeyCerts[],
                              int derPubliceKeyCertLengths[])
{
    UtlBoolean encryptionSucceeded = FALSE;

    // Clean up an residual decrypted body or encrypted body content.
    // Should typically not be any.
    if(mpDecryptedBody)
    {
        delete mpDecryptedBody;
        mpDecryptedBody = NULL;
    }
    mBody.remove(0);


    if(bodyToEncrypt)
    {
        UtlString dataToEncrypt;
        UtlString contentType = bodyToEncrypt->getContentType();

        // Add the content-type and content-encoding headers to
        // the body to be decrypted so that when the otherside
        // decrypts this body, it can tell what the content is
        dataToEncrypt ="Content-Type: ";
        dataToEncrypt.append(contentType);
        dataToEncrypt.append(END_OF_LINE_DELIMITOR);
        dataToEncrypt.append("Content-Transfer-Encoding: binary");
        dataToEncrypt.append(END_OF_LINE_DELIMITOR);
        dataToEncrypt.append(END_OF_LINE_DELIMITOR);

        // Append the real body content
        const char* dataPtr;
        int dataLength;
        bodyToEncrypt->getBytes(&dataPtr, &dataLength);
        dataToEncrypt.append(dataPtr, dataLength);

        // Attach the decrypted version of the body for
        // future reference.
        mpDecryptedBody = bodyToEncrypt;

        // We almost always want to use binary for SIP as it is 
        // much more efficient than base64.
        UtlBoolean encryptedDataInBase64Format = FALSE;

#ifdef ENABLE_OPENSSL_SMIME
        encryptionSucceeded = 
            opensslSmimeEncrypt(numRecipients,
                                derPublicKeyCerts,
                                derPubliceKeyCertLengths,
                                dataToEncrypt.data(),
                                dataToEncrypt.length(),
                                encryptedDataInBase64Format,
                                mBody);
#elif ENABLE_NSS_SMIME

        encryptionSucceeded = 
            nssSmimeEncrypt(numRecipients,
                            derPublicKeyCerts,
                            derPubliceKeyCertLengths,
                            dataToEncrypt.data(),
                            dataToEncrypt.length(),
                            encryptedDataInBase64Format,
                            mBody);
#endif

        // There should always be content if encryption succeeds
        if(encryptionSucceeded &&
           mBody.length() <= 0)
        {
            encryptionSucceeded = FALSE;
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SmimeBody::encrypt no encrypted content");
        }
    }

    bodyLength = mBody.length(); 
    return(encryptionSucceeded);
}

#ifdef ENABLE_NSS_SMIME
static void nssOutToUtlString(void *sink, const char *data, unsigned long dataLength)
{
    printf("nssOutToUtlString recieved %d bytes\n", dataLength);
    UtlString* outputSink = (UtlString*) sink;
    outputSink->append(data, dataLength);
}

#endif

UtlBoolean SmimeBody::nssSmimeEncrypt(int numResipientCerts,
                           const char* derPublicKeyCerts[], 
                           int derPublicKeyCertLengths[],
                           const char* dataToEncrypt,
                           int dataToEncryptLength,
                           UtlBoolean encryptedDataInBase64Format,
                           UtlString& encryptedData)
{
    UtlBoolean encryptionSucceeded = FALSE;
    encryptedData.remove(0);

#ifdef ENABLE_NSS_SMIME

    // nickname can be NULL as we are not putting this in a database
    char *nickname = NULL;
    // copyDER = true so we copy the DER format cert passed in so memory
    // is internally alloc'd and freed
    PRBool copyDER = true;

    // Create an envelope or container for the encrypted data
    SECOidTag algorithm = SEC_OID_DES_EDE3_CBC; // or  SEC_OID_AES_128_CBC
    // Should be able to get the key size from the cert somehow
    int keysize = 1024;
    NSSCMSMessage* cmsMessage = NSS_CMSMessage_Create(NULL);
    NSSCMSEnvelopedData* myEnvelope = 
        NSS_CMSEnvelopedData_Create(cmsMessage, algorithm, keysize);

    // Do the following for each recipient if there is more than one.
    // For each recipient:
    for(int certIndex = 0; certIndex < numResipientCerts; certIndex++)
    {
        // Convert the DER to a NSS CERT
        SECItem derFormatCertItem;
        SECItem* derFormatCertItemPtr = &derFormatCertItem;
        derFormatCertItem.data = (unsigned char*) derPublicKeyCerts[certIndex];
        derFormatCertItem.len = derPublicKeyCertLengths[certIndex];
        CERTCertificate* myCertFromDer = NULL;
        myCertFromDer = __CERT_DecodeDERCertificate(&derFormatCertItem, 
                                                   copyDER, 
                                                   nickname);

        // Add just the recipient Subject key Id, if it exists to the envelope
        // This is the minimal information needed to identify which recipient
        // the the symetric/session key is encrypted for
        NSSCMSRecipientInfo* recipientInfo = NULL; 

        // Add the full set of recipient information including 
        // the Cert. issuer location and org. info.
        recipientInfo = 
            NSS_CMSRecipientInfo_Create(cmsMessage, myCertFromDer);

        if(recipientInfo)
        {
            if(NSS_CMSEnvelopedData_AddRecipient(myEnvelope , recipientInfo) != 
                SECSuccess)
            {
                NSS_CMSEnvelopedData_Destroy(myEnvelope);
                myEnvelope = NULL;
                NSS_CMSRecipientInfo_Destroy(recipientInfo);
            }
        }
        // No recipientInfo
        else
        {
            NSS_CMSEnvelopedData_Destroy(myEnvelope);
            myEnvelope = NULL;
        }

    } //end for each recipient

    // Get the content out of the envelop
    NSSCMSContentInfo* envelopContentInfo =
       NSS_CMSEnvelopedData_GetContentInfo(myEnvelope);    

    //TODO: why are we copying or setting the content pointer from the envelope into the msg????????
    if (NSS_CMSContentInfo_SetContent_Data(cmsMessage, 
                                           envelopContentInfo, 
                                           NULL, 
                                           PR_FALSE) != 
        SECSuccess)
    {
        // release cmsg and other stuff
        NSS_CMSEnvelopedData_Destroy(myEnvelope);
        myEnvelope = NULL;
    }

    //TODO: why are we copying or setting the content pointer from the message and 
    // putting it back into the msg????????
    NSSCMSContentInfo* messageContentInfo = 
        NSS_CMSMessage_GetContentInfo(cmsMessage);
    if(NSS_CMSContentInfo_SetContent_EnvelopedData(cmsMessage, 
                                                   messageContentInfo, 
                                                   myEnvelope) != 
       SECSuccess)
    {
        // release cmsg and other stuff
        NSS_CMSEnvelopedData_Destroy(myEnvelope);
        myEnvelope = NULL;
    }

    if(cmsMessage)
    {
        // Create an encoder  and context to do the encryption.
        // The encodedItem will stort the encoded result
        //SECItem encodedItem;
        //encodedItem.data = NULL;
        //encodedItem.len = 0;
        //SECITEM_AllocItem(NULL, &encodedItem, 0);
        printf("start encoder\n");
        NSSCMSEncoderContext* encoderContext = 
            NSS_CMSEncoder_Start(cmsMessage, nssOutToUtlString, &encryptedData, NULL, 
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        // Add encrypted content
        printf("update encoder\n");
        NSS_CMSEncoder_Update(encoderContext, dataToEncrypt, dataToEncryptLength);

        // Finished encrypting
        printf("finish encoder\n");
        NSS_CMSEncoder_Finish(encoderContext);

        myEnvelope = NULL;

        if(encryptedData.length() > 0)
        {
            encryptionSucceeded = TRUE;
        }

        // Clean up the message memory, the envelop gets cleaned up
        // with the message
        NSS_CMSMessage_Destroy(cmsMessage);
        cmsMessage = NULL;
    }

#else
    OsSysLog::add(FAC_SIP, PRI_ERR, "SmimeBody::nssSmimeEncrypt invoked with ENABLE_NSS_SMIME not defined");
#endif

    return(encryptionSucceeded);
}

UtlBoolean SmimeBody::nssSmimeDecrypt(const char* derPkcs12,
                               int derPkcs12Length,
                               const char* pkcs12Password,
                               UtlBoolean dataIsInBase64Format,
                               const char* dataToDecrypt,
                               int dataToDecryptLength,
                               UtlString& decryptedData)
{
    UtlBoolean decryptSucceeded = FALSE;
    decryptedData.remove(0);

#ifdef ENABLE_NSS_SMIME
    OsSysLog::add(FAC_SIP, PRI_ERR, "SmimeBody::nssSmimeDecrypt not implemented");

    ////// BEGIN WARNING: THIS CODE HAS NOT BEEN TESTED AT ALL ///////

    // allocate a temporaty slot in the database
    PK11SlotInfo *slot = PK11_GetInternalKeySlot();
    PRBool swapUnicode = PR_FALSE;
    SEC_PKCS12DecoderContext *p12Decoder = NULL;

    // Need to put the pkcs12 password into a SECItem
    SECItem passwordItem;
    passwordItem.data = (unsigned char*) pkcs12Password;
    passwordItem.len = strlen(pkcs12Password);
    SECItem uniPasswordItem;
    uniPasswordItem.data = NULL;
    uniPasswordItem.len = 0;

#ifdef IS_LITTLE_ENDIAN
    swapUnicode = PR_TRUE;
#endif

    // Allocate a temporary internal slot
    slot = PK11_GetInternalSlot();
    if(slot == NULL)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "unable to use slot in NSS dataqbase for S/MIME decryption");
    }
    else
    {
        // Do UNICODE conversion of password based upon the platform
        // (not sure this is even neccessary in our application).  I do not
        // know how we would get unicode passwords
        if(0) //P12U_UnicodeConversion(NULL, &uniPasswordItem, passwordItem, PR_TRUE,
			  //    swapUnicode) != SECSuccess)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR, 
                "NSS Unicode conversion failed for PKCS12 object for S/MIME decryption");
        }
        else
        {
            // Initialze the decoder for the PKCS12 container for the private key
            p12Decoder = SEC_PKCS12DecoderStart(&passwordItem, slot, NULL,
				    NULL, NULL, NULL, NULL, NULL);
            if(!p12Decoder) 
            {
                OsSysLog::add(FAC_SIP, PRI_ERR,
                    "failed to initialize PKCS12 decoder to extract private key for S/MIME decryption");
            }
            else
            {
                // Add the PKCS12 data to the decoder
                if(SEC_PKCS12DecoderUpdate(p12Decoder, 
                                           (unsigned char *) derPkcs12, 
                                           derPkcs12Length) != SECSuccess ||
                   // Validate the decoded PKCS12
                   SEC_PKCS12DecoderVerify(p12Decoder) != SECSuccess)

                {
                    OsSysLog::add(FAC_SIP, PRI_ERR,
                        "unable to decrypt PKCS12 for S/MIME decryption. Perhaps invalid PKCS12 or PKCS12 password");
                }
                else
                {
                    // Import the private key and certificate from the 
                    // decoded PKCS12 into the database
                    if(SEC_PKCS12DecoderImportBags(p12Decoder) != SECSuccess)
                    {
                        OsSysLog::add(FAC_SIP, PRI_ERR,
                            "failed to import private key and certificate into NSS database");
                    }
                    else
                    {
                        // Put the S/MIME data in a SECItem
                        SECItem dataToDecodeItem;
                        dataToDecodeItem.data = (unsigned char *) dataToDecrypt;
                        dataToDecodeItem.len = dataToDecryptLength;

                        if(dataIsInBase64Format)
                        {
                            // TODO:
                            // Use some NSS util. to convert base64 to binary
                            OsSysLog::add(FAC_SIP, PRI_ERR,
                                "NSS decrypt of base64 S/MIME message not implemented");
                        }
                        else
                        {
                            // Decode the S/MIME blob
                            NSSCMSMessage *cmsMessage = 
                                NSS_CMSMessage_CreateFromDER(&dataToDecodeItem,
                                                             nssOutToUtlString, 
                                                             &decryptedData,
                                                             NULL, NULL,
                                                             NULL, NULL);


                            if(cmsMessage &&
                               decryptedData.length() > 0)
                            {
                                decryptSucceeded = TRUE;
                            }

                            // TODO:
                            // Remove the temporary private key from the 
                            // database using the slot handle
                        }
                    }

                }
            }
        }
    }

    // Clean up
    if(p12Decoder) 
    {
	    SEC_PKCS12DecoderFinish(p12Decoder);
    }
    if(uniPasswordItem.data) 
    {
	    SECITEM_ZfreeItem(&uniPasswordItem, PR_FALSE);
    }

    ////// END WARNING   /////
#else
    OsSysLog::add(FAC_SIP, PRI_ERR, "SmimeBody::nssSmimeDecrypt invoked with ENABLE_NSS_SMIME not defined");
#endif

    return(decryptSucceeded);
}

UtlBoolean SmimeBody::opensslSmimeEncrypt(int numRecipients,
                                          const char* derPublicKeyCerts[],
                                          int derPublicKeyCertLengths[],
                                          const char* dataToEncrypt,
                                          int dataToEncryptLength,
                                          UtlBoolean encryptedDataInBase64Format,
                                          UtlString& encryptedData)
{
    UtlBoolean encryptSucceeded = FALSE;
    encryptedData.remove(0);

#ifdef ENABLE_OPENSSL_SMIME

    // Convert the DER format Certs. to a stack of X509
    STACK_OF(X509)* recipientX509Certs = sk_X509_new_null();
    for(int recipIndex = 0; recipIndex < numRecipients; recipIndex++)
    {
        X509* oneX509Cert = d2i_X509(NULL, (unsigned char**)&derPublicKeyCerts[recipIndex], 
                                   derPublicKeyCertLengths[recipIndex]);
        if(oneX509Cert)
        {
            sk_X509_push(recipientX509Certs, oneX509Cert);
        }
        else
        {
            printf("Invalid DER format cert. Cannot convert to X509 OpenSSL (DER length: %d)\n",
                derPublicKeyCertLengths[recipIndex]);
        }
    }

    // Put the data to be encryped into a BIO that can be used as input to
    // the PKCS7 encoder
    BIO* dataToEncryptBio = BIO_new_mem_buf((void*)dataToEncrypt,
                                            dataToEncryptLength);

    //  Use the X509 certs to create a PKCS7 encoder
    int encodeOpts = 0;
    if(encryptedDataInBase64Format)
    {
        PKCS7_BINARY;  // binary format, not base64 
    }

    const EVP_CIPHER* cipher = EVP_des_ede3_cbc();
    PKCS7* pkcs7Encoder = PKCS7_encrypt(recipientX509Certs, dataToEncryptBio, cipher, encodeOpts);

    BIO* encryptedDataBio = BIO_new(BIO_s_mem());
    if(pkcs7Encoder && encryptedDataBio)
    {
        // Get the raw binary encrypted data out of the encoder
        i2d_PKCS7_bio(encryptedDataBio, pkcs7Encoder);

        BUF_MEM *encryptedDataBioMem = NULL;
        BIO_get_mem_ptr(encryptedDataBio, &encryptedDataBioMem);

        if(encryptedDataBioMem)
        {
            encryptSucceeded = TRUE;
            encryptedData.append(encryptedDataBioMem->data,
                                 encryptedDataBioMem->length);

            BIO_free(encryptedDataBio);
            encryptedDataBio = NULL;
        }
    }
    else
    {
        if(pkcs7Encoder == NULL)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                "failed to create openssl PKCS12 encoder");
        }
        if(encryptedDataBio)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "Failed to allocate openssl BIO");
        }
    }

    if(pkcs7Encoder)
    {
        // TODO: free up PKCS7
        pkcs7Encoder = NULL;
    }
    if(dataToEncryptBio)
    {
        BIO_free(dataToEncryptBio);
        dataToEncryptBio = NULL;
    }
    // Free up the cert stack
    sk_X509_free(recipientX509Certs);
#endif

    return(encryptSucceeded);
}

UtlBoolean SmimeBody::opensslSmimeDecrypt(const char* derPkcs12,
                               int derPkcs12Length,
                               const char* pkcs12Password,
                               UtlBoolean dataIsInBase64Format,
                               const char* dataToDecrypt,
                               int dataToDecryptLength,
                               UtlString& decryptedData)
{
    UtlBoolean decryptSucceeded = FALSE;
    decryptedData.remove(0);

#ifdef ENABLE_OPENSSL_SMIME

    EVP_PKEY* privateKey = NULL;
    X509* publicKeyCert = NULL;

    //  Create the PKCS12 which contains both cert. and private key
    // from the DER format
    BIO* pkcs12Bio = BIO_new_mem_buf((void*)derPkcs12,
                                            derPkcs12Length);
    PKCS12 *pkcs12 = d2i_PKCS12_bio(pkcs12Bio, NULL);

    // The PKCS12 contains both the private key and the cert. which
    // are protected by symmetric encryption using the given password.
	PKCS12_parse(pkcs12, pkcs12Password, &privateKey, &publicKeyCert, NULL);
	PKCS12_free(pkcs12);
	pkcs12 = NULL;

    if(privateKey == NULL)
    {
        dumpOpensslError();
        OsSysLog::add(FAC_SIP, PRI_ERR, "PKCS12 or PKCS12 password invalid or does not contain private key for S/MIME decrypt operation");
    }
    if(publicKeyCert)
    {
        dumpOpensslError();
        OsSysLog::add(FAC_SIP, PRI_ERR, "PKCS12 or PKCS12 password invalid or does not contain certificate and public key for S/MIME decrypt operation");
    }

    // Create a memory BIO to put the body into
    BIO* encryptedBodyBioBuf = BIO_new_mem_buf((void*)dataToDecrypt,
                                                dataToDecryptLength);

    // Create the pkcs7 structure
    // The clearTextSignatureBio only gets set if it
    // is provided.
    BIO* decryptedBodyBioBuf = BIO_new(BIO_s_mem());
    PKCS7* pkcs7 = NULL;

    if(dataIsInBase64Format)  // base64
    {
        BIO* clearTextSignatureBio = NULL;
        pkcs7 = SMIME_read_PKCS7(encryptedBodyBioBuf,
                                 &clearTextSignatureBio);
        if(clearTextSignatureBio)
        {
            BIO_free(clearTextSignatureBio);
        }
    }
    else // binary
    {
        pkcs7 = d2i_PKCS7_bio(encryptedBodyBioBuf, 0);
    }

    if(pkcs7 == NULL)
    {
        // Unable to initialize PKCS7
        OsSysLog::add(FAC_SIP, PRI_ERR, 
            "Unable to create OpenSSL PKCS7 context for S/MIME decrypt operation\n");
    }

    if(pkcs7 && privateKey && publicKeyCert && decryptedBodyBioBuf)
    {
        // Decrypt the pkcs7 structure into a memory BIO
        int decryptOk = PKCS7_decrypt(pkcs7, 
                                      privateKey,
                                      publicKeyCert, 
                                      decryptedBodyBioBuf, 
                                      0);

        // Unable to decrypt
        if(!decryptOk)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "Unable to decrypt S/MIME message using OpenSSL\n");
        }

        else
        {
            // Get the data from the decrypted BIO
            BUF_MEM *bioMemoryStructure;
            BIO_get_mem_ptr(decryptedBodyBioBuf, &bioMemoryStructure);
            if (bioMemoryStructure)
            {
                decryptedData.append(bioMemoryStructure->data,
                                     bioMemoryStructure->length);
                decryptSucceeded = TRUE;
            }
        }
    }

    // Free up the BIOs
    if(encryptedBodyBioBuf) 
    {
        BIO_free(encryptedBodyBioBuf);
        encryptedBodyBioBuf = NULL;
    }
    if(decryptedBodyBioBuf) 
    {
        BIO_free(decryptedBodyBioBuf);
        decryptedBodyBioBuf;
    }

#else
    OsSysLog::add(FAC_SIP, PRI_ERR, "SmimeBody::opensslSmimeDecrypt invoked with ENABLE_OPENSSL_SMIME not defined");
#endif

    return(decryptSucceeded);
}

UtlBoolean SmimeBody::convertPemToDer(UtlString& pemData,
                                      UtlString& derData)
{
    UtlBoolean conversionSucceeded = FALSE;
    derData.remove(0);

#ifdef ENABLE_OPENSSL_SMIME
    // Convert PEM to openssl X509
    BIO* certPemBioBuf = BIO_new_mem_buf((void*)pemData.data(),
                                      pemData.length());
    if(certPemBioBuf)
    {
        X509* x509 = PEM_read_bio_X509(certPemBioBuf, NULL, NULL, NULL);
        BIO* certDerBio = BIO_new(BIO_s_mem());

        if(x509 && certDerBio)
        {
            // Convert the X509 to a DER buffer
            i2d_X509_bio(certDerBio, x509);

            BUF_MEM* certDerBioMem = NULL;
            BIO_get_mem_ptr(certDerBio, &certDerBioMem);
            if(certDerBioMem && certDerBioMem->data && certDerBioMem->length > 0)
            {
                derData.append(certDerBioMem->data, certDerBioMem->length);
            }
            BIO_free(certDerBio);
            certDerBio = NULL;
            X509_free(x509);
            x509 = NULL;
        }
        BIO_free(certPemBioBuf);
        certPemBioBuf = NULL;
    }

#elif ENABLE_NSS_SMIME
    // Code from NSS secutil.c

    char* body = NULL;
    char* pemDataPtr = (char*) pemData.data();

	/* check for headers and trailers and remove them */
	if ((body = strstr(pemDataPtr, "-----BEGIN")) != NULL) {
	    char *trailer = NULL;
	    pemData = body;
	    body = PORT_Strchr(body, '\n');
	    if (!body)
		body = PORT_Strchr(pemDataPtr, '\r'); /* maybe this is a MAC file */
	    if (body)
		trailer = strstr(++body, "-----END");
	    if (trailer != NULL) {
		*trailer = '\0';
	    } else {
		OsSysLog::add(FAC_SIP, PRI_ERR, 
            "input has header but no trailer\n");
	    }
	} else {
	    body = pemDataPtr;
	}
     
	/* Convert to binary */
    SECItem derItem;
    derItem.data = NULL;
    derItem.len = 0;
	if(ATOB_ConvertAsciiToItem(&derItem, body))
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, 
            "error converting PEM base64 data to binary");
    }
    else
    {
        derData.append(((char*)derItem.data), derItem.len);
        conversionSucceeded = TRUE;
    }
#else
    OsSysLog::add(FAC_SIP, PRI_ERR, 
        "SmimeBody::convertPemToDer implemented with NSS and OpenSSL disabled");
#endif

    return(conversionSucceeded);
}

/* ============================ ACCESSORS ================================= */

const HttpBody* SmimeBody::getDecyptedBody() const
{
    return(mpDecryptedBody);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SmimeBody::isDecrypted() const
{
    return(mpDecryptedBody != NULL);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

#else // NSS
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)


// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SmimeBody.h>
#include <net/HttpMessage.h>
#include <os/OsSysLog.h>
#ifdef SIP_TLS
#include <os/OsTLS.h>
#endif

#ifdef HAVE_NSS



extern "C" CERTCertificate *
    __CERT_DecodeDERCertificate(SECItem *derSignedCert, PRBool copyDER, char *nickname);

static char * PR_CALLBACK pk11Passwordcallback(PK11SlotInfo *slot, PRBool retry, void *arg);

#endif

    

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MAX_SMIME_BODY_SIZE 8192
#define MAX_RECIPIENTS 64
// STATIC VARIABLE INITIALIZATIONS
/*SECStatus
P12U_UnicodeConversion(PRArenaPool *arena, SECItem *dest, SECItem *src,
		       PRBool toUnicode, PRBool swapBytes);
*/

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SmimeBody::SmimeBody(const char* bytes, 
                     int length,
                     const char* smimeEncodingType) :
    mpSmimeSink(NULL)
{
    bodyLength = length;
    mBody.append(bytes, length);

    mContentEncoding = SMIME_ENODING_UNKNOWN;
    if(smimeEncodingType)
    {
        UtlString sMimeEncodingString(smimeEncodingType);
        sMimeEncodingString.toUpper();

        if(sMimeEncodingString.compareTo(HTTP_CONTENT_TRANSFER_ENCODING_BINARY, UtlString::ignoreCase) == 0)
        {
            mContentEncoding = SMIME_ENODING_BINARY;
        }
        else if(sMimeEncodingString.compareTo(HTTP_CONTENT_TRANSFER_ENCODING_BASE64, UtlString::ignoreCase) == 0)
        {
            mContentEncoding = SMIME_ENODING_BASE64;
        }
        else
        {
            // TODO: We could probably put a hack in here to heuristically
            // determine if the encoding is base64 or not based upon the
            // byte values.
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "Invalid transport encoding for S/MIME content");
        }
    }

    mpDecryptedBody = NULL;
}

// Copy constructor
SmimeBody::SmimeBody(const SmimeBody& rSmimeBody)
    : HttpBody(rSmimeBody)
{
    mpDecryptedBody = NULL;
    if(rSmimeBody.mpDecryptedBody)
    {
        mpDecryptedBody = HttpBody::copyBody(*(rSmimeBody.mpDecryptedBody));
    }

    mContentEncoding = rSmimeBody.mContentEncoding;
}

// Destructor
SmimeBody::~SmimeBody()
{
   if(mpDecryptedBody)
   {
       delete mpDecryptedBody;
       mpDecryptedBody = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SmimeBody&
SmimeBody::operator=(const SmimeBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Copy the parent
   *((HttpBody*)this) = rhs;

   // Remove the decrypted body if one is attached,
   // in case we copy over it
   if(mpDecryptedBody)
   {
       delete mpDecryptedBody;
       mpDecryptedBody = NULL;
   }

   // If the source has a body copy it
   if(rhs.mpDecryptedBody)
   {
       mpDecryptedBody = HttpBody::copyBody(*(rhs.mpDecryptedBody));
   }

   mContentEncoding = rhs.mContentEncoding;
   return *this;
}

UtlBoolean SmimeBody::decrypt(const char* derPkcs12,
                              int derPkcs12Length,
                              const char* pkcs12Password,
                              const char* certDbPassword,
                              const char* signerCertDER,
                              int signerCertDERLength,
                              ISmimeNotifySink* pSink)
{
    UtlBoolean decryptionSucceeded = FALSE;

    mpSmimeSink = pSink;
    UtlString decryptedData;

    decryptionSucceeded =    
               nssSmimeDecrypt(derPkcs12,
                               derPkcs12Length,
                               pkcs12Password,
                               certDbPassword,
                               signerCertDER,
                               signerCertDERLength,
                              (mContentEncoding == SMIME_ENODING_BASE64),
                               mBody.data(),
                               mBody.length(),
                               decryptedData,
                               pSink);
    

    // Decryption succeeded, so create a HttpBody for the result
    if(decryptionSucceeded && 
        decryptedData.length() > 0)
    {
        HttpBody* newDecryptedBody = NULL;
        // Need to read the headers before the real body to see
        // what the content type of the decrypted body is
        UtlDList bodyHeaders;
        int parsedBytes = 
            HttpMessage::parseHeaders(decryptedData.data(), 
                                      decryptedData.length(),
                                      bodyHeaders);

        UtlString contentTypeName(HTTP_CONTENT_TYPE_FIELD);
        NameValuePair* contentType = 
            (NameValuePair*) bodyHeaders.find(&contentTypeName);
        UtlString contentEncodingName(HTTP_CONTENT_TRANSFER_ENCODING_FIELD);
        NameValuePair* contentEncoding = 
            (NameValuePair*) bodyHeaders.find(&contentEncodingName);
        
        const char* realBodyStart = decryptedData.data() + parsedBytes;
        int realBodyLength = decryptedData.length() - parsedBytes;

        newDecryptedBody = 
            HttpBody::createBody(realBodyStart,
                                 realBodyLength,
                                 contentType ? contentType->getValue() : NULL,
                                 contentEncoding ? contentEncoding->getValue() : NULL);

        bodyHeaders.destroyAll();

        // If one already exists, delete it.  This should not typically
        // be the case.  Infact it might make sense to make this method
        // a no-op if a decrypted body already exists
        if(mpDecryptedBody)
        {
            delete mpDecryptedBody;
            mpDecryptedBody = NULL;
        }

        mpDecryptedBody = newDecryptedBody;
    }

    return(decryptionSucceeded);
}

UtlBoolean SmimeBody::encrypt(HttpBody* bodyToEncrypt,
                              int numRecipients,
                              const char* derPublicKeyCerts[],
                              int derPubliceKeyCertLengths[],
                              const char* szMyCertNickname,
                              const char* szCertDbPassword,
                              ISmimeNotifySink* pSink)
{
    UtlBoolean encryptionSucceeded = FALSE;

    mpSmimeSink = pSink;
    // Clean up an residual decrypted body or encrypted body content.
    // Should typically not be any.
    if(mpDecryptedBody)
    {
        delete mpDecryptedBody;
        mpDecryptedBody = NULL;
    }
    mBody.remove(0);


    if(bodyToEncrypt)
    {
        UtlString dataToEncrypt;
        UtlString contentType = bodyToEncrypt->getContentType();
        // We almost always want to use binary for SIP as it is 
        // much more efficient than base64.
        UtlBoolean encryptedDataInBase64Format = FALSE;

        // Add the content-type and content-encoding headers to
        // the body to be decrypted so that when the otherside
        // decrypts this body, it can tell what the content is
        dataToEncrypt ="Content-Type: ";
        dataToEncrypt.append(contentType);
        dataToEncrypt.append(END_OF_LINE_DELIMITOR);
        dataToEncrypt.append("Content-Transfer-Encoding: binary");
        dataToEncrypt.append(END_OF_LINE_DELIMITOR);
        dataToEncrypt.append(END_OF_LINE_DELIMITOR);

        // Append the real body content
        const char* dataPtr;
        int dataLength;
        bodyToEncrypt->getBytes(&dataPtr, &dataLength);
        dataToEncrypt.append(dataPtr, dataLength);

        // Attach the decrypted version of the body for
        // future reference.
        mpDecryptedBody = bodyToEncrypt;

        UtlString encryptedData;

        encryptionSucceeded = 
            nssSmimeEncrypt(numRecipients,
                            derPublicKeyCerts,
                            derPubliceKeyCertLengths,
                            szMyCertNickname,
                            szCertDbPassword,
                            dataToEncrypt.data(),
                            dataToEncrypt.length(),
                            encryptedDataInBase64Format,
                            mBody,
                            pSink);
        encryptedData = mBody;
        bodyLength = mBody.length();
        // There should always be content if encryption succeeds
        if(encryptionSucceeded &&
           encryptedData.length() <= 0)
        {
            encryptionSucceeded = FALSE;
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SmimeBody::encrypt no encrypted content");
        }
    }

    return(encryptionSucceeded);
}

#ifdef HAVE_NSS
void SmimeBody::getSubjAltName(char* szSubjAltName,
                               const CERTCertificate* pCert,
                               const size_t length)
{
    CERTCertExtension** extensions = pCert->extensions;
    if (extensions)
    {
        while (*extensions)
        {
            SECItem *ext_oid = &(*extensions)->id;
            SECItem *ext_critical = &(*extensions)->critical;
            SECItem *ext_value = &(*extensions)->value;
            
            /* id attribute of the extension */
            SECOidData *oiddata = SECOID_FindOID(ext_oid);

            /* value attribute of the extension */
            /* SECItem ext_value has type (SECItemType), data (unsigned char *) and len (unsigned int) fields 
                - the application interprets these */
            SECOidTag oidtag = SECOID_FindOIDTag(ext_oid);
            if (oidtag == SEC_OID_X509_SUBJECT_ALT_NAME) // this is the subject alt name
            {
                strncpy(szSubjAltName, (const char*)ext_value->data, sipx_min(ext_value->len, length));
                break;
            }

            extensions++;
        }
    }
}
#endif 

static void nssOutToUtlString(void *sink, const char *data, unsigned long dataLength)
{

//    printf("nssOutToUtlString recieved %d bytes\n", dataLength);
    UtlString* outputSink = (UtlString*) sink;
    outputSink->append(data, dataLength);
}

static void nssSignedDataToUtlString(void *sink, const char *data, unsigned long dataLength)
{

//    printf("nssOutToUtlString recieved %d bytes\n", dataLength);
    UtlString* outputSink = (UtlString*) sink;
    outputSink->append(data, dataLength);
}


UtlBoolean SmimeBody::nssSmimeEncrypt(int numResipientCerts,
                           const char* derPublicKeyCerts[], 
                           int derPublicKeyCertLengths[],
                           const char* szMyCertNickname,
                           const char* szCertDbPassword,
                           const char* dataToEncrypt,
                           int dataToEncryptLength,
                           UtlBoolean encryptedDataInBase64Format,
                           UtlString& encryptedData,
                           ISmimeNotifySink* pSmimeSink)
{
    UtlBoolean encryptionSucceeded = FALSE;
#ifdef HAVE_NSS
    bool bFoundRecipient = false;
    encryptedData.remove(0);
    //errorCode = SMIME_ENCRYPT_FAILURE_INVALID_PARAMETER;

    // nickname can be NULL as we are not putting this in a database
    char *nickname = NULL;
    // copyDER = true so we copy the DER format cert passed in so memory
    // is internally alloc'd and freed
    PRBool copyDER = PR_TRUE;

    // Create an envelope or container for the encrypted data
    SECOidTag algorithm = SEC_OID_DES_EDE3_CBC; // or  SEC_OID_AES_128_CBC
    // Should be able to get the key size from the cert somehow
    int keysize = 192;
    NSSCMSMessage* cmsMessage = NSS_CMSMessage_Create(NULL);
    
    CERTCertificate* myCert = PK11_FindCertFromNickname((char*)szMyCertNickname, NULL);//szMyCertNickname, NULL);
    NSSCMSSignedData* sigd = NULL;
    
    if (CERT_CheckCertUsage (myCert, KU_DIGITAL_SIGNATURE) != SECSuccess)
    {
        // should fail out here
    }

    UtlString signedData = createSignedData(myCert, dataToEncrypt, dataToEncryptLength, sigd, (char*)szCertDbPassword);
    
    NSSCMSEnvelopedData* myEnvelope = 
        NSS_CMSEnvelopedData_Create(cmsMessage, algorithm, keysize);

    // Do the following for each recipient if there is more than one.
    // For each recipient:
    NSSCMSRecipientInfo* recipientInfo[MAX_RECIPIENTS]; 
    memset(recipientInfo, 0, sizeof(recipientInfo));
    for(int certIndex = 0; certIndex < numResipientCerts; certIndex++)
    {
        // Convert the DER to a NSS CERT
        SECItem derFormatCertItem;
        derFormatCertItem.data = (unsigned char*)derPublicKeyCerts[certIndex];
        derFormatCertItem.len = derPublicKeyCertLengths[certIndex];
        //SECITEM_AllocItem(NULL, &derFormatCertItem, 0);
//        memcpy(&derFormatCertItem.data,  derPublicKeyCerts[certIndex], derPublicKeyCertLengths[certIndex]) ;
        CERTCertificate* recipCertFromDer = NULL;
        recipCertFromDer = __CERT_DecodeDERCertificate(&derFormatCertItem, 
                                                   copyDER, 
                                                   nickname);
                                                   
        int x = PR_GetError();                                                   
                                                   
        if (!recipCertFromDer)
        {
            if (pSmimeSink)
                pSmimeSink->OnError(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_FAILURE_BAD_PUBLIC_KEY);
            //errorCode = SMIME_ENCRYPT_FAILURE_BAD_PUBLIC_KEY;
            continue;
        }                                                   

        // Add just the recipient Subject key Id, if it exists to the envelope
        // This is the minimal information needed to identify which recipient
        // the the symetric/session key is encrypted for

        // Add the full set of recipient information including 
        // the Cert. issuer location and org. info.
        recipientInfo[certIndex] = 
            NSS_CMSRecipientInfo_Create(cmsMessage, recipCertFromDer);

        if(recipientInfo[certIndex])
        {
            if(NSS_CMSEnvelopedData_AddRecipient(myEnvelope , recipientInfo[certIndex]) != 
                SECSuccess)
            {
                NSS_CMSEnvelopedData_Destroy(myEnvelope);
                myEnvelope = NULL;
                NSS_CMSRecipientInfo_Destroy(recipientInfo[certIndex]);
            }
            else
            {
                bFoundRecipient = true;
            }
        }
        else
        {
            // No recipientInfo
            NSS_CMSEnvelopedData_Destroy(myEnvelope);
            myEnvelope = NULL;
        }
        if (recipCertFromDer)
        {
            CERT_DestroyCertificate(recipCertFromDer);
        }
    } //end for each recipient

    if (bFoundRecipient)
    {
        // Get the content out of the envelop
        NSSCMSContentInfo* envelopContentInfo =
        NSS_CMSEnvelopedData_GetContentInfo(myEnvelope);    


        //TODO: why are we copying or setting the content pointer from the envelope into the msg????????
        if (NSS_CMSContentInfo_SetContent_Data(cmsMessage, 
                                            envelopContentInfo, 
                                            NULL, 
                                            PR_FALSE) != 
            SECSuccess)
        {
            // release cmsg and other stuff
            NSS_CMSEnvelopedData_Destroy(myEnvelope);
            myEnvelope = NULL;
        }

        //TODO: why are we copying or setting the content pointer from the message and 
        // putting it back into the msg????????

        NSSCMSContentInfo* messageContentInfo = 
            NSS_CMSMessage_GetContentInfo(cmsMessage);
        if(NSS_CMSContentInfo_SetContent_EnvelopedData(cmsMessage, 
                                                    messageContentInfo, 
                                                    myEnvelope) != SECSuccess)
        {
            // release cmsg and other stuff
            NSS_CMSEnvelopedData_Destroy(myEnvelope);
            myEnvelope = NULL;
        }
        
        if(cmsMessage)
        {
            // Create an encoder  and context to do the encryption.
            // The encodedItem will store the encoded result
            NSSCMSEncoderContext* encoderContext = 
                NSS_CMSEncoder_Start(cmsMessage, nssOutToUtlString, &encryptedData, NULL, 
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL);

            // Add encrypted content
            NSS_CMSEncoder_Update(encoderContext, signedData.data(), signedData.length());

            // Finished encrypting
            NSS_CMSEncoder_Finish(encoderContext);

            if(encryptedData.length() > 0)
            {
                encryptionSucceeded = TRUE;
                if (pSmimeSink)
                    pSmimeSink->OnError(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS);
            }


            // Clean up the message memory, the envelop gets cleaned up
            // with the message
            NSS_CMSMessage_Destroy(cmsMessage);

            cmsMessage = NULL;

        }

    } // if bFoundRecipient
#endif
    return(encryptionSucceeded);
}

#ifdef HAVE_NSS
UtlString SmimeBody::createSignedData(CERTCertificate *cert,
                            const char* dataToSign,
                            const int dataToSignLength,
                             NSSCMSSignedData*& sigd,
                             char* szCertDbPassword)
{
    NSSCMSMessage* cmsg = NULL;
    NSSCMSContentInfo* cinfo = NULL;    
    NSSCMSSignerInfo *signerinfo = NULL;
    CERTCertificate *ekpcert = NULL;
    UtlString signedData("");
    
    cmsg = NSS_CMSMessage_Create(NULL);
    /*
     * build chain of objects: message->signedData->data
     */
    if ((sigd = NSS_CMSSignedData_Create(cmsg)) == NULL)
    {
	    fprintf(stderr, "ERROR: cannot create CMS signedData object.\n");
	    goto loser;
    }


    cinfo = NSS_CMSSignedData_GetContentInfo(sigd);
    if (NSS_CMSContentInfo_SetContent_Data(cmsg, cinfo, NULL, false) !=
        SECSuccess)
    {
        goto loser;     
    }
    
    /* 
     * create & attach signer information
     */
    signerinfo = NSS_CMSSignerInfo_Create(cmsg, cert, SEC_OID_SHA1);
    if (signerinfo == NULL)
    {
	    fprintf(stderr, "ERROR: cannot create CMS signerInfo object.\n");
	    goto loser;
    }

	if (NSS_CMSSignerInfo_AddSigningTime(signerinfo, PR_Now()) 
	    != SECSuccess)
    {
	    fprintf(stderr, "ERROR: cannot add signingTime attribute.\n");
	    goto loser;
	}
	/*
	if (NSS_CMSSignerInfo_AddSMIMECaps(signerinfo) != SECSuccess)
	{
	    fprintf(stderr, "ERROR: cannot add SMIMECaps attribute.\n");
	    goto loser;
	}
*/
    /* add signing cert as EncryptionKeyPreference */
    /*
    if (NSS_CMSSignerInfo_AddSMIMEEncKeyPrefs(signerinfo, cert, 
                                        signOptions->options->certHandle)
            != SECSuccess)
    {
        fprintf(stderr, 
            "ERROR: cannot add default SMIMEEncKeyPrefs attribute.\n");
        goto loser;
    }
    */  
          
    if (NSS_CMSSignedData_AddSignerInfo(sigd, signerinfo) != SECSuccess)
    {
	    fprintf(stderr, "ERROR: cannot add CMS signerInfo object.\n");
	    goto loser;
    }
    
    
    SECItem* digItem;    
    digItem = SECITEM_AllocItem(NULL, NULL, SHA1_LENGTH);
    
    PK11_HashBuf(SEC_OID_SHA1, digItem->data, (unsigned char*)dataToSign, dataToSignLength);
    NSS_CMSSignedData_SetDigestValue(sigd, SEC_OID_SHA1, digItem);   

    cinfo = NSS_CMSMessage_GetContentInfo(cmsg);
    NSSCMSContentInfo* signedInfo = NSS_CMSSignedData_GetContentInfo(sigd);
    
    NSS_CMSContentInfo_SetContent_SignedData(cmsg, cinfo, sigd);

    SECItem secData;
    secData.data = (unsigned char*)dataToSign;
    secData.len = dataToSignLength;
    
    if (NSS_CMSContentInfo_SetContent_Data(cmsg, signedInfo, NULL, PR_FALSE) 
          != SECSuccess)
    {
    	goto loser;
    }


    SECStatus status;
    
    char* prPass = (char*)PR_Malloc(1024);
  
    strncpy(prPass, szCertDbPassword, 256);
    NSSCMSEncoderContext* encoderContext = 
        NSS_CMSEncoder_Start(cmsg, nssSignedDataToUtlString, &signedData, NULL, NULL, pk11Passwordcallback, 
                            (void*)prPass, NULL, NULL, NULL, NULL);

    // Add encrypted content
    status = NSS_CMSEncoder_Update(encoderContext, dataToSign, dataToSignLength);

    // Finished encrypting
    status = NSS_CMSEncoder_Finish(encoderContext);   
    int x = PR_GetError(); 

/*
    SECItem secDerIn;
    secDerIn.data = (unsigned char*)signedData.data();
    secDerIn.len = signedData.length();

    SECItem secDerOut;
    secDerOut.data = NULL;
    secDerOut.len = 0;

	PLArenaPool *tmparena = PORT_NewArena(secDerIn.len *2);
    status = NSS_CMSDEREncode(cmsg, &secDerIn, &secDerOut, tmparena);
    
    PORT_Free(prPass);
    return UtlString((const char*)secDerOut.data, secDerOut.len);
*/    

    return signedData;
loser:
    if (cert) {
	CERT_DestroyCertificate(cert);
    }
    return "";
}
#endif

UtlBoolean SmimeBody::nssSmimeDecrypt(const char* derPkcs12,
                                      int derPkcs12Length,
                                      const char* pkcs12Password,
                                      const char* certDbPassword,
                                      const char* signerCertDER,
                                      int signerCertDERLength,                      
                                      UtlBoolean dataIsInBase64Format,
                                      const char* dataToDecrypt,
                                      int dataToDecryptLength,
                                      UtlString& decryptedData,
                                      ISmimeNotifySink* pSmimeSink)
{
    UtlBoolean decryptSucceeded = FALSE;
#ifdef HAVE_NSS
    decryptedData.remove(0);
    char* szDbPassword;
    
    //errorCode = SMIME_DECRYPT_FAILURE_INVALID_PARAMETER;

    szDbPassword = (char*)PR_Malloc(256);
    strncpy(szDbPassword, certDbPassword, 255);
    
    SECItem decryptedEnvelope;
    decryptedEnvelope.data = NULL;
    decryptedEnvelope.len = 0;
    SECITEM_AllocItem(NULL, &decryptedEnvelope, 0);

    decryptedEnvelope.data = new unsigned char [MAX_SMIME_BODY_SIZE];
    memset(decryptedEnvelope.data, 0, MAX_SMIME_BODY_SIZE);
    
    SECItem decryptedContent;
    decryptedContent.data = NULL;
    decryptedContent.len = 0;
    SECITEM_AllocItem(NULL, &decryptedContent, 0);

    decryptedContent.data = new unsigned char [MAX_SMIME_BODY_SIZE];
    memset(decryptedContent.data, 0, MAX_SMIME_BODY_SIZE);
    

    PRBool swapUnicode = PR_FALSE;
    SEC_PKCS12DecoderContext *p12Decoder = NULL;

    SECItem slotPwd;
    memset(&slotPwd, 0, sizeof(SECItem));
    slotPwd.data = (unsigned char*)PL_strdup(certDbPassword);
    slotPwd.len = strlen(certDbPassword);
    
    // Allocate a temporary internal slot
    PK11SlotInfo *slot = PK11_GetInternalKeySlot();

    if(slot == NULL)
    {
        if (pSmimeSink)
        {
            pSmimeSink->OnError(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_FAILURE_DB_INIT);
        }
        OsSysLog::add(FAC_SIP, PRI_ERR, "unable to use slot in NSS dataqbase for S/MIME decryption\n");
    }
    else
    {
    
        // first, decrypt the enveloped data
        SECItem dataToDecodeItem;
        dataToDecodeItem.data = (unsigned char *) dataToDecrypt;
        dataToDecodeItem.len = dataToDecryptLength;

        // Decode the S/MIME blob
//            mnCallbackDataLength = 0;
        NSSCMSMessage *cmsMessageEnveloped = 
            NSS_CMSMessage_CreateFromDER(&dataToDecodeItem,
                                            nssOutToUtlString, 
                                            &decryptedEnvelope,
                                            pk11Passwordcallback, (void*)szDbPassword,
                                            NULL, NULL);

                                        
                                        
        // now decode to get the signed data                                        
        // Prepare to decode the message
        NSSCMSDecoderContext* decoder = NSS_CMSDecoder_Start(NULL,
             nssSignedDataToUtlString, &decryptedData,
             pk11Passwordcallback, (void*)szDbPassword,
            NULL, NULL);
            
        if (!decoder)
        {
            return FALSE;
        }
        NSS_CMSDecoder_Update(decoder, (const char*)decryptedEnvelope.data, decryptedEnvelope.len);                                            

        NSSCMSMessage* cmsMsg = NSS_CMSDecoder_Finish(decoder);
        int x = PR_GetError();                                                   
        
        if (!cmsMsg || !NSS_CMSMessage_IsSigned(cmsMsg)) 
        {
            if (pSmimeSink)
            {
                pSmimeSink->OnError(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE);
            }
            // we only accept signed messages
            return FALSE;            
        }
        
        if (!cmsMessageEnveloped)
        {
            // TODO - check for a bad password, then set the error
            // code, if it is a bad password
            //errorCode = SMIME_DECRYPT_FAILURE_BAD_DB_PASSWORD;
        }
        
        
        int nlevels = NSS_CMSMessage_ContentLevelCount(cmsMsg);
        for (int i = 0; i < nlevels; i++)
        {
	        NSSCMSContentInfo *cinfo;
	        SECOidTag typetag;

	        cinfo = NSS_CMSMessage_ContentLevel(cmsMsg, i);
	        typetag = NSS_CMSContentInfo_GetContentTypeTag(cinfo);        
        
            if (SEC_OID_PKCS7_SIGNED_DATA == typetag)
            {
	            NSSCMSSignedData *sigd = NULL;
	            sigd = (NSSCMSSignedData *)NSS_CMSContentInfo_GetContent(cinfo);
	            if (sigd == NULL)
	            {
                    pSmimeSink->OnError(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE);
	            }
                else
                {

		            NSSCMSSignerInfo *si;
		            si = NSS_CMSSignedData_GetSignerInfo(sigd, 0);
    		        
                    // send the signature up to the user
                    CERTCertificate* pCert = 
                        NSS_CMSSignerInfo_GetSigningCertificate(si,
                                                                CERT_GetDefaultCertDB());
                    char szSubjAltName[256];
                    memset(szSubjAltName, 0, sizeof(szSubjAltName));

                    getSubjAltName(szSubjAltName, pCert, sizeof(szSubjAltName));

                    if (!pCert || !pSmimeSink->OnSignature((void*)pCert, szSubjAltName))
                    {
                        if (pCert)
                        {
                            pSmimeSink->OnError(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SIGNATURE_REJECTED);
                        }
                        else
                        {
                            pSmimeSink->OnError(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE);
                        }
                        decryptSucceeded = false;
                        decryptedData = "";
                    }
                    else
                    {

                        // Verify the signature
                        HRESULT hr = S_OK;
                        if (NSS_CMSSignedData_VerifySignerInfo(sigd, 0, CERT_GetDefaultCertDB(), 
                            certUsageEmailSigner) != SECSuccess)    
                        {        
                            switch (PORT_GetError())
                            {           
                            case SEC_ERROR_PKCS7_BAD_SIGNATURE:
                            case SEC_ERROR_BAD_SIGNATURE:
                                decryptSucceeded = false;
                                if (pSmimeSink)
                                {
                                    pSmimeSink->OnError(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_BAD_SIGNATURE);
                                }
                                break;
                            case SEC_ERROR_UNTRUSTED_ISSUER:
                            case SEC_ERROR_UNTRUSTED_CERT:
#ifndef SIP_TLS_TEST
                                decryptSucceeded = false;
                                if (pSmimeSink)
                                {
                                    pSmimeSink->OnError(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_BAD_SIGNATURE);
                                }
#endif
                                break;
                            default:
                                // Perhaps there was en error with the certificate itself,
                                // let's verify it and see what if it's suspect.
                                int x = PR_GetError();
                            }
                        }
                    }
                }
            }	                // end for
        }
                                            
                                            
        if(decryptedData.length() > 0)
        {
            decryptSucceeded = true;
            if (pSmimeSink)
            {
                pSmimeSink->OnError(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS);
            }
        }
        if (cmsMessageEnveloped)
        {
            NSS_CMSMessage_Destroy(cmsMessageEnveloped);
        }
    }
    
    if (slot) PK11_FreeSlot(slot);
    
    if (decryptedEnvelope.data)
    {
        delete[] decryptedEnvelope.data;
        decryptedEnvelope.data = NULL;
    }

    if (decryptedContent.data)
    {
        delete[] decryptedContent.data;
        decryptedContent.data = NULL;
    }

    SECITEM_FreeItem(&decryptedEnvelope, PR_FALSE);
    SECITEM_FreeItem(&decryptedContent, PR_FALSE);
    
//    if (secuPwd.data)
//        PR_Free(secuPwd.data);
    if (slotPwd.data)
        PR_Free(slotPwd.data);
#endif
    return(decryptSucceeded);
}

bool SmimeBody::importPKCS12Object(const char* derPkcs12,
                                       int derPkcs12Length,
                                       const char* pkcs12Password,
                                       const char* certDbLocation,
                                       const char* certDbPassword)
{
    bool bRet = false;
#ifdef HAVE_NSS
    if (P12Wrapper_Init((char*)certDbLocation, ""))
    {
        PRBool swapUnicode = PR_FALSE;
        SEC_PKCS12DecoderContext *p12Decoder = NULL;

        SECItem secuPwd;
        memset(&secuPwd, 0, sizeof(SECItem));
        secuPwd.data = (unsigned char*)PL_strdup(pkcs12Password);
        secuPwd.len = strlen(pkcs12Password);

        SECItem slotPwd;
        memset(&slotPwd, 0, sizeof(SECItem));
        slotPwd.data = (unsigned char*)PL_strdup(certDbPassword);
        slotPwd.len = strlen(certDbPassword);
        
        // Allocate a temporary internal slot
        PK11SlotInfo *slot = PK11_GetInternalKeySlot();

        if(slot == NULL)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR, "unable to use slot in NSS dataqbase for S/MIME decryption\n");
        }
        else
        {
            UtlString derPkcs12(derPkcs12, derPkcs12Length);

            // import the pkcs12 into the db
            int pkcs12_rc = P12Wrapper_ImportPKCS12ObjectFromBuffer((char*)derPkcs12.data(), derPkcs12Length, slot, &slotPwd, &secuPwd);
           
            if (pkcs12_rc == 0)
            {
                bRet = true;
            }
        }
        
        if (slot) PK11_FreeSlot(slot);
        
        if (slotPwd.data)
            PR_Free(slotPwd.data);
    }
#endif
    return bRet;
}                                       


UtlBoolean SmimeBody::opensslSmimeDecrypt(const char* derPkcs12,
                               int derPkcs12Length,
                               const char* pkcs12Password,
                               UtlBoolean dataIsInBase64Format,
                               const char* dataToDecrypt,
                               int dataToDecryptLength,
                               UtlString& decryptedData)
{
    UtlBoolean decryptSucceeded = FALSE;
    decryptedData.remove(0);

#ifdef ENABLE_OPENSSL_SMIME

    EVP_PKEY* privateKey = NULL;
    X509* publicKeyCert = NULL;

    //  Create the PKCS12 which contains both cert. and private key
    // from the DER format
    BIO* pkcs12Bio = BIO_new_mem_buf((void*)derPkcs12,
                                            derPkcs12Length);
    PKCS12 *pkcs12 = d2i_PKCS12_bio(pkcs12Bio, NULL);

    // The PKCS12 contains both the private key and the cert. which
    // are protected by symmetric encryption using the given password.
	PKCS12_parse(pkcs12, pkcs12Password, &privateKey, &publicKeyCert, NULL);
	PKCS12_free(pkcs12);
	pkcs12 = NULL;

    if(privateKey == NULL)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "PKCS12 or PKCS12 password invalid or does not contain private key for S/MIME decrypt operation");
    }
    if(publicKeyCert)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "PKCS12 or PKCS12 password invalid or does not contain certificate and public key for S/MIME decrypt operation");
    }

    // Create a memory BIO to put the body into
    BIO* encryptedBodyBioBuf = BIO_new_mem_buf((void*)dataToDecrypt,
                                                dataToDecryptLength);

    // Create the pkcs7 structure
    // The clearTextSignatureBio only gets set if it
    // is provided.
    BIO* decryptedBodyBioBuf = BIO_new(BIO_s_mem());
    PKCS7* pkcs7 = NULL;

    if(dataIsInBase64Format)  // base64
    {
        BIO* clearTextSignatureBio = NULL;
        pkcs7 = SMIME_read_PKCS7(encryptedBodyBioBuf,
                                 &clearTextSignatureBio);
        if(clearTextSignatureBio)
        {
            BIO_free(clearTextSignatureBio);
        }
    }
    else // binary
    {
        pkcs7 = d2i_PKCS7_bio(encryptedBodyBioBuf, 0);
    }

    if(pkcs7 == NULL)
    {
        // Unable to initialize PKCS7
        OsSysLog::add(FAC_SIP, PRI_ERR, 
            "Unable to create OpenSSL PKCS7 context for S/MIME decrypt operation\n");
    }

    if(pkcs7 && privateKey && publicKeyCert && decryptedBodyBioBuf)
    {
        // Decrypt the pkcs7 structure into a memory BIO
        int decryptOk = PKCS7_decrypt(pkcs7, 
                                      privateKey,
                                      publicKeyCert, 
                                      decryptedBodyBioBuf, 
                                      0);

        // Unable to decrypt
        if(!decryptOk)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "Unable to decrypt S/MIME message using OpenSSL\n");
        }

        else
        {
            // Get the data from the decrypted BIO
            BUF_MEM *bioMemoryStructure;
            BIO_get_mem_ptr(decryptedBodyBioBuf, &bioMemoryStructure);
            if (bioMemoryStructure)
            {
                decryptedData.append(bioMemoryStructure->data,
                                     bioMemoryStructure->length);
                decryptSucceeded = TRUE;
            }
        }
    }

    // Free up the BIOs
    if(encryptedBodyBioBuf) 
    {
        BIO_free(encryptedBodyBioBuf);
        encryptedBodyBioBuf = NULL;
    }
    if(decryptedBodyBioBuf) 
    {
        BIO_free(decryptedBodyBioBuf);
        decryptedBodyBioBuf;
    }

#else
    OsSysLog::add(FAC_SIP, PRI_ERR, "SmimeBody::opensslSmimeDecrypt invoked with ENABLE_OPENSSL_SMIME not defined");
#endif

    return(decryptSucceeded);
}

UtlBoolean SmimeBody::convertPemToDer(UtlString& pemData,
                                      UtlString& derData)
{
    UtlBoolean conversionSucceeded = FALSE;
#ifdef HAVE_NSS
    derData.remove(0);

    // Code from NSS secutil.c
    char* body = NULL;
    char* pemDataPtr = (char*) pemData.data();
    char* pemDataCopy = NULL;

	/* check for headers and trailers and remove them */
	if ((body = strstr(pemDataPtr, "-----BEGIN")) != NULL) {
	    char *trailer = NULL;
        pemDataCopy = strdup(body);
	    pemData = pemDataCopy;
	    body = PORT_Strchr(body, '\n');
	    if (!body)
		body = PORT_Strchr(pemDataPtr, '\r'); /* maybe this is a MAC file */
	    if (body)
		trailer = strstr(++body, "-----END");
	    if (trailer != NULL) {
		*trailer = '\0';
	    } else {
		OsSysLog::add(FAC_SIP, PRI_ERR, 
            "input has header but no trailer\n");
	    }
	} else {
	    body = pemDataPtr;
	}
     
	/* Convert to binary */
    SECItem derItem;
    derItem.data = NULL;
    derItem.len = 0;
	if(ATOB_ConvertAsciiToItem(&derItem, body))
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, 
            "error converting PEM base64 data to binary");
    }
    else
    {
        derData.append(((char*)derItem.data), derItem.len);
        SECITEM_FreeItem(&derItem, false);
        if (pemDataCopy)
            free(pemDataCopy);
        conversionSucceeded = TRUE;
    }
#endif
    return(conversionSucceeded);
}

/* ============================ ACCESSORS ================================= */

const HttpBody* SmimeBody::getDecryptedBody() const
{
    return(mpDecryptedBody);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SmimeBody::isDecrypted() const
{
    return(mpDecryptedBody != NULL);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
#ifdef HAVE_NSS
char * PR_CALLBACK pk11Passwordcallback(PK11SlotInfo *slot, PRBool retry, void *arg)
{
    return (char*)arg;
}
#endif

#endif