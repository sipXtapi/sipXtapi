/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package com.pingtel.pds.jbossauth;

import java.security.KeyStore;
import java.security.Principal;
import java.security.cert.X509Certificate;

import org.jboss.security.auth.certs.X509CertificateVerifier;

/**
 * Compare incoming SubjectDN with each SubjectDN in trust store
 */
public class X509CertVerifier implements X509CertificateVerifier {

   public boolean verify(X509Certificate cert, String alias, KeyStore keyStore, KeyStore trustStore) {
        Principal certDn = cert.getSubjectDN();
        boolean valid = CertLoginModule.getTrustedClients().contains(certDn.getName());
        
        return valid;
    }
}
