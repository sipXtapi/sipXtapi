//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpDtlsIdentity_h_
#define _MpDtlsIdentity_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <os/OsStatus.h>
#include <os/OsMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// Forward declarations to keep OpenSSL headers out of this header.
#ifdef __cplusplus
extern "C" {
#endif
   struct x509_st;
   typedef struct x509_st X509;
   struct evp_pkey_st;
   typedef struct evp_pkey_st EVP_PKEY;
#ifdef __cplusplus
}
#endif

// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
 *  @brief Process-wide owner of the long-lived DTLS-SRTP identity
 *         (X.509 certificate + private key).
 *
 *  Singleton. Holds a single cert + key pair used as our local DTLS
 *  identity for every DTLS-SRTP connection. The cert's fingerprint is
 *  what the SIP layer puts in outgoing SDP a=fingerprint attributes,
 *  and what the peer verifies our cert against during DTLS handshake.
 *
 *  The identity is either:
 *    - Loaded from PEM files via setIdentity(), or
 *    - Auto-generated as a self-signed ECDSA P-256 certificate on
 *      first use (when getFingerprint(), getCertificate(), or
 *      getPrivateKey() is first called and no identity has been set).
 *
 *  Auto-generated certs use:
 *    - ECDSA P-256 (NID_X9_62_prime256v1) keys
 *    - CN=sipXtapi
 *    - 10 year validity
 *    - SHA-256 signature
 *
 *  Thread-safe.
 *
 *  @nosubgrouping
 */
class MpDtlsIdentity
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Get the singleton instance. Lazy-instantiates on first call.
   static MpDtlsIdentity* getInstance();

     /// Release the singleton. Called from
     /// CpTopologyGraphFactoryImpl shutdown. Safe to call multiple times.
   static void release();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Load identity from PEM-encoded files.
     ///
     /// Replaces any previously loaded or auto-generated identity.
     /// On failure, the existing identity (if any) is preserved.
   OsStatus setIdentity(const UtlString& certPemPath,
                        const UtlString& privateKeyPemPath);
     /**
     *  @param[in] certPemPath - path to a PEM-encoded X.509 certificate.
     *  @param[in] privateKeyPemPath - path to a PEM-encoded private key
     *             matching the certificate. May be the same file as
     *             certPemPath if the key is bundled.
     *
     *  @retval OS_SUCCESS    identity loaded.
     *  @retval OS_NOT_FOUND  a file could not be opened.
     *  @retval OS_FAILED     a file could not be parsed, or the key did
     *                        not match the certificate.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get the fingerprint of the current identity.
     ///
     /// Triggers auto-generation of a self-signed cert if no identity
     /// has been set. The returned fingerprint is upper-case hex pairs
     /// separated by colons, e.g. "AB:CD:EF:01:23:..." (RFC 4572 §5
     /// format, without the algorithm-name prefix).
   OsStatus getFingerprint(UtlString& fingerprint,
                           const UtlString& hashAlgorithm);
     /**
     *  @param[out] fingerprint - hex string with colon separators.
     *              Empty on failure.
     *  @param[in]  hashAlgorithm - "SHA-256" (default), "SHA-1",
     *              "SHA-384", or "SHA-512". Case-insensitive.
     *
     *  @retval OS_SUCCESS           fingerprint computed.
     *  @retval OS_INVALID_ARGUMENT  unsupported hash algorithm.
     *  @retval OS_FAILED            cert auto-generation failed, or
     *                               digest computation failed.
     */

     /// Get the X509 certificate handle. Used by MpDtls when configuring
     /// per-connection SSL_CTX. Triggers auto-generation if needed.
     ///
     /// Pointer is owned by this class -- do not free.
     /// May be NULL if auto-generation failed.
   X509* getCertificate();

     /// Get the private key handle. Used by MpDtls when configuring
     /// per-connection SSL_CTX. Triggers auto-generation if needed.
     ///
     /// Pointer is owned by this class -- do not free.
     /// May be NULL if auto-generation failed.
   EVP_PKEY* getPrivateKey();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Constructor. Private; use getInstance().
   MpDtlsIdentity();

     /// Destructor. Frees mpCert and mpKey.
   ~MpDtlsIdentity();

     /// If no identity is loaded, auto-generate one. Caller MUST hold mLock.
   OsStatus ensureIdentityLoaded();

     /// Generate self-signed ECDSA P-256 cert. Caller MUST hold mLock.
   OsStatus generateSelfSigned();

     /// Free mpCert and mpKey (if non-NULL). Caller MUST hold mLock.
   void freeIdentity();

   X509*     mpCert;       ///< Owned. NULL until loaded or generated.
   EVP_PKEY* mpKey;        ///< Owned. NULL until loaded or generated.
   OsMutex   mLock;        ///< Protects mpCert and mpKey.

   static MpDtlsIdentity* spInstance;
   static OsMutex         sInstanceLock;

     /// Disabled
   MpDtlsIdentity(const MpDtlsIdentity& rhs);
     /// Disabled
   MpDtlsIdentity& operator=(const MpDtlsIdentity& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDtlsIdentity_h_