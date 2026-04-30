//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <mp/MpDtlsIdentity.h>

#ifdef HAVE_SSL
#  include <openssl/ssl.h>
#  include <openssl/x509.h>
#  include <openssl/pem.h>
#  include <openssl/evp.h>
#  include <openssl/ec.h>
#  include <openssl/bn.h>
#  include <openssl/rand.h>
#  include <openssl/err.h>
#endif

#include <stdio.h>
#include <string.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

MpDtlsIdentity* MpDtlsIdentity::spInstance = NULL;
OsMutex         MpDtlsIdentity::sInstanceLock(OsMutex::Q_FIFO);

// LOCAL HELPERS

#ifdef HAVE_SSL
namespace
{
   /// Map a hash algorithm name to an OpenSSL EVP_MD*.
   /// Returns NULL for unsupported names.
   const EVP_MD* evpMdFromName(const UtlString& hashAlgorithm)
   {
      // Case-insensitive comparison via UtlString::compareTo.
      if (hashAlgorithm.compareTo("SHA-256", UtlString::ignoreCase) == 0 ||
          hashAlgorithm.compareTo("SHA256",  UtlString::ignoreCase) == 0)
      {
         return EVP_sha256();
      }
      if (hashAlgorithm.compareTo("SHA-1", UtlString::ignoreCase) == 0 ||
          hashAlgorithm.compareTo("SHA1",  UtlString::ignoreCase) == 0)
      {
         return EVP_sha1();
      }
      if (hashAlgorithm.compareTo("SHA-384", UtlString::ignoreCase) == 0 ||
          hashAlgorithm.compareTo("SHA384",  UtlString::ignoreCase) == 0)
      {
         return EVP_sha384();
      }
      if (hashAlgorithm.compareTo("SHA-512", UtlString::ignoreCase) == 0 ||
          hashAlgorithm.compareTo("SHA512",  UtlString::ignoreCase) == 0)
      {
         return EVP_sha512();
      }
      return NULL;
   }

   /// Format a raw byte array as upper-case hex with colon separators.
   /// e.g. {0xAB, 0xCD, 0xEF} -> "AB:CD:EF"
   void formatFingerprint(const unsigned char* raw,
                          unsigned int rawLen,
                          UtlString& outString)
   {
      outString.remove(0);
      char hex[4]; // "XX\0" plus a colon when prepended
      for (unsigned int i = 0; i < rawLen; i++)
      {
         if (i > 0)
         {
            outString.append(":");
         }
         sprintf(hex, "%02X", raw[i]);
         outString.append(hex);
      }
   }

   /// Log the topmost OpenSSL error, if any.
   void logOpenSslError(const char* where)
   {
      unsigned long err = ERR_peek_last_error();
      if (err != 0)
      {
         char buf[256];
         ERR_error_string_n(err, buf, sizeof(buf));
         OsSysLog::add(FAC_MP, PRI_ERR, "%s: OpenSSL error: %s", where, buf);
         ERR_clear_error();
      }
   }
}
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpDtlsIdentity* MpDtlsIdentity::getInstance()
{
   OsLock lock(sInstanceLock);
   if (spInstance == NULL)
   {
      spInstance = new MpDtlsIdentity();
   }
   return spInstance;
}

void MpDtlsIdentity::release()
{
   OsLock lock(sInstanceLock);
   if (spInstance != NULL)
   {
      delete spInstance;
      spInstance = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpDtlsIdentity::setIdentity(const UtlString& certPemPath,
                                     const UtlString& privateKeyPemPath)
{
#if defined(ENABLE_SRTP) && defined(HAVE_SSL)
   if (certPemPath.isNull() || privateKeyPemPath.isNull())
   {
      return OS_INVALID_ARGUMENT;
   }

   FILE* certFp = fopen(certPemPath.data(), "r");
   if (certFp == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtlsIdentity::setIdentity: cannot open cert file '%s'",
         certPemPath.data());
      return OS_NOT_FOUND;
   }

   X509* cert = PEM_read_X509(certFp, NULL, NULL, NULL);
   fclose(certFp);
   if (cert == NULL)
   {
      logOpenSslError("MpDtlsIdentity::setIdentity (PEM_read_X509)");
      return OS_FAILED;
   }

   FILE* keyFp = fopen(privateKeyPemPath.data(), "r");
   if (keyFp == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtlsIdentity::setIdentity: cannot open key file '%s'",
         privateKeyPemPath.data());
      X509_free(cert);
      return OS_NOT_FOUND;
   }

   EVP_PKEY* key = PEM_read_PrivateKey(keyFp, NULL, NULL, NULL);
   fclose(keyFp);
   if (key == NULL)
   {
      logOpenSslError("MpDtlsIdentity::setIdentity (PEM_read_PrivateKey)");
      X509_free(cert);
      return OS_FAILED;
   }

   // Verify the key matches the cert. X509_check_private_key returns 1
   // on match, 0 on mismatch.
   if (X509_check_private_key(cert, key) != 1)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtlsIdentity::setIdentity: private key does not match certificate");
      ERR_clear_error();
      EVP_PKEY_free(key);
      X509_free(cert);
      return OS_FAILED;
   }

   // Install. Replace any previously loaded or auto-generated identity.
   {
      OsLock lock(mLock);
      freeIdentity();
      mpCert = cert;
      mpKey  = key;
   }

   OsSysLog::add(FAC_MP, PRI_INFO,
      "MpDtlsIdentity::setIdentity: loaded cert='%s', key='%s'",
      certPemPath.data(), privateKeyPemPath.data());
   return OS_SUCCESS;
#else
   OsSysLog::add(FAC_CP, PRI_WARNING,
      "MpDtlsIdentity::setIdentity: ENABLE_SRTP AND HAVE_SSL not defined");
   return OS_FAILED;
#endif
}

/* ============================ ACCESSORS ================================= */

OsStatus MpDtlsIdentity::getFingerprint(UtlString& fingerprint,
                                        const UtlString& hashAlgorithm)
{
   fingerprint.remove(0);

#if defined(ENABLE_SRTP) && defined(HAVE_SSL)
   const EVP_MD* md = evpMdFromName(hashAlgorithm);
   if (md == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpDtlsIdentity::getFingerprint: unsupported hash '%s'",
         hashAlgorithm.data());
      return OS_INVALID_ARGUMENT;
   }

   OsLock lock(mLock);
   OsStatus status = ensureIdentityLoaded();
   if (status != OS_SUCCESS)
   {
      return status;
   }

   unsigned char raw[EVP_MAX_MD_SIZE];
   unsigned int  rawLen = 0;
   if (X509_digest(mpCert, md, raw, &rawLen) != 1)
   {
      logOpenSslError("MpDtlsIdentity::getFingerprint (X509_digest)");
      return OS_FAILED;
   }

   formatFingerprint(raw, rawLen, fingerprint);
   return OS_SUCCESS;
#else
   OsSysLog::add(FAC_CP, PRI_WARNING,
      "MpDtlsIdentity::getFingerprint: ENABLE_SRTP and HAVE_SSL not defined");
   return OS_FAILED;
#endif
}

X509* MpDtlsIdentity::getCertificate()
{
#ifdef HAVE_SSL
   OsLock lock(mLock);
   if (ensureIdentityLoaded() != OS_SUCCESS)
   {
      return NULL;
   }
   return mpCert;
#else
   OsSysLog::add(FAC_CP, PRI_WARNING,
      "MpDtlsIdentity::getCertificate: HAVE_SSL not defined");
   return NULL;
#endif
}

EVP_PKEY* MpDtlsIdentity::getPrivateKey()
{
#ifdef HAVE_SSL
   OsLock lock(mLock);
   if (ensureIdentityLoaded() != OS_SUCCESS)
   {
      return NULL;
   }
   return mpKey;
#else
   OsSysLog::add(FAC_CP, PRI_WARNING,
      "MpDtlsIdentity::getPrivateKey: HAVE_SSL not defined");
   return NULL;
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

MpDtlsIdentity::MpDtlsIdentity()
   : mpCert(NULL)
   , mpKey(NULL)
   , mLock(OsMutex::Q_FIFO)
{
}

MpDtlsIdentity::~MpDtlsIdentity()
{
   OsLock lock(mLock);
   freeIdentity();
}

OsStatus MpDtlsIdentity::ensureIdentityLoaded()
{
   // Caller holds mLock.
   if (mpCert != NULL && mpKey != NULL)
   {
      return OS_SUCCESS;
   }
   return generateSelfSigned();
}

OsStatus MpDtlsIdentity::generateSelfSigned()
{
#ifdef HAVE_SSL
   // Caller holds mLock. On entry, mpCert and mpKey may be partially
   // set (e.g. if a prior setIdentity failed mid-way). Free anything
   // lingering before we generate.
   freeIdentity();

   EVP_PKEY* key  = NULL;
   X509*     cert = NULL;

   //
   // 1. Generate ECDSA P-256 key.
   //
   // OpenSSL 1.1.x and 3.x both support the EVP-based "automatic"
   // path via EVP_PKEY_CTX_new_id(EVP_PKEY_EC) + parameter init. This
   // works on both versions without deprecation warnings on 3.x.
   //
   {
      EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
      if (pctx == NULL)
      {
         logOpenSslError("MpDtlsIdentity::generateSelfSigned (EVP_PKEY_CTX_new_id)");
         return OS_FAILED;
      }
      if (EVP_PKEY_keygen_init(pctx) <= 0)
      {
         logOpenSslError("MpDtlsIdentity::generateSelfSigned (EVP_PKEY_keygen_init)");
         EVP_PKEY_CTX_free(pctx);
         return OS_FAILED;
      }
      if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1) <= 0)
      {
         logOpenSslError("MpDtlsIdentity::generateSelfSigned (set curve)");
         EVP_PKEY_CTX_free(pctx);
         return OS_FAILED;
      }
      if (EVP_PKEY_keygen(pctx, &key) <= 0 || key == NULL)
      {
         logOpenSslError("MpDtlsIdentity::generateSelfSigned (EVP_PKEY_keygen)");
         EVP_PKEY_CTX_free(pctx);
         return OS_FAILED;
      }
      EVP_PKEY_CTX_free(pctx);
   }

   //
   // 2. Build the X.509 certificate.
   //
   cert = X509_new();
   if (cert == NULL)
   {
      logOpenSslError("MpDtlsIdentity::generateSelfSigned (X509_new)");
      EVP_PKEY_free(key);
      return OS_FAILED;
   }

   // X.509 v3 (numeric value 2; see RFC 5280 §4.1.2.1).
   X509_set_version(cert, 2);

   // Random 64-bit positive serial number.
   {
      ASN1_INTEGER* serial = X509_get_serialNumber(cert);
      BIGNUM* bn = BN_new();
      if (bn == NULL)
      {
         X509_free(cert);
         EVP_PKEY_free(key);
         return OS_FAILED;
      }
      if (BN_rand(bn, 63, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY) != 1 ||
          BN_to_ASN1_INTEGER(bn, serial) == NULL)
      {
         logOpenSslError("MpDtlsIdentity::generateSelfSigned (serial)");
         BN_free(bn);
         X509_free(cert);
         EVP_PKEY_free(key);
         return OS_FAILED;
      }
      BN_free(bn);
   }

   // Validity: now to now + 10 years.
   const long kTenYearsSeconds = 10L * 365L * 24L * 60L * 60L;
   if (X509_gmtime_adj(X509_getm_notBefore(cert), 0) == NULL ||
       X509_gmtime_adj(X509_getm_notAfter(cert),  kTenYearsSeconds) == NULL)
   {
      logOpenSslError("MpDtlsIdentity::generateSelfSigned (validity)");
      X509_free(cert);
      EVP_PKEY_free(key);
      return OS_FAILED;
   }

   // Public key.
   if (X509_set_pubkey(cert, key) != 1)
   {
      logOpenSslError("MpDtlsIdentity::generateSelfSigned (X509_set_pubkey)");
      X509_free(cert);
      EVP_PKEY_free(key);
      return OS_FAILED;
   }

   // Subject = Issuer = "CN=sipXtapi".
   X509_NAME* name = X509_get_subject_name(cert);
   if (name == NULL ||
       X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                                  (const unsigned char*)"sipXtapi", -1, -1, 0) != 1 ||
       X509_set_issuer_name(cert, name) != 1)
   {
      logOpenSslError("MpDtlsIdentity::generateSelfSigned (subject/issuer)");
      X509_free(cert);
      EVP_PKEY_free(key);
      return OS_FAILED;
   }

   // Self-sign with SHA-256.
   if (X509_sign(cert, key, EVP_sha256()) == 0)
   {
      logOpenSslError("MpDtlsIdentity::generateSelfSigned (X509_sign)");
      X509_free(cert);
      EVP_PKEY_free(key);
      return OS_FAILED;
   }

   // Install.
   mpCert = cert;
   mpKey  = key;

   OsSysLog::add(FAC_MP, PRI_INFO,
      "MpDtlsIdentity::generateSelfSigned: generated self-signed "
      "ECDSA P-256 cert (CN=sipXtapi, 10 year validity)");
   return OS_SUCCESS;
#else
   return OS_FAILED;
#endif
}

void MpDtlsIdentity::freeIdentity()
{
#ifdef HAVE_SSL
   // Caller holds mLock.
   if (mpCert != NULL)
   {
      X509_free(mpCert);
      mpCert = NULL;
   }
   if (mpKey != NULL)
   {
      EVP_PKEY_free(mpKey);
      mpKey = NULL;
   }
#endif
}
