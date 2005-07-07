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

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.security.acl.Group;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import javax.security.auth.Subject;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.login.LoginException;

import org.jboss.security.SimpleGroup;
import org.jboss.security.SimplePrincipal;
import org.jboss.security.auth.spi.BaseCertLoginModule;

public class CertLoginModule extends BaseCertLoginModule {

    private static List s_trustedClients;
    
    public final static String TRUSTED_CERTS_PROPERTY = CertLoginModule.class.getName() + ".trustedCerts";

    public void initialize(Subject subject, CallbackHandler callbackHandler, Map sharedState,
            Map options) {
        super.initialize(subject, callbackHandler, sharedState, options);

        String trustedClientsFile = (String) options.get("trustedClients");
        if (trustedClientsFile == null) {
            throw new IllegalArgumentException("trusted clients file missing");
        }

        FileReader rdr = null;
        try {
            rdr = new FileReader(trustedClientsFile);
            // unmodifiable as extra measure so that entries cannot be inserted, thereby
            // gaining access
            s_trustedClients = Collections.unmodifiableList(readTrustedClients(rdr));
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            closeQuietly(rdr);
        }
    }
    
    /**
     * Make available via static method because X509CertVerifier has no access LoginModule
     * package protected to no other access it
     */
    static List getTrustedClients() {
        return s_trustedClients;
    }

    static void closeQuietly(Reader rdr) {
        if (rdr != null) {
            try {
                rdr.close();
            } catch (IOException ignore) {
            }
        }
    }

    static List readTrustedClients(Reader rdr) throws IOException {
        List trustedClients = new ArrayList();
        BufferedReader buffRdr = new BufferedReader(rdr);
        String line = null;
        do {
            line = buffRdr.readLine();
            if (line != null) {
                String cleanLine = line.trim();
                if (cleanLine.length() > 0) {
                    trustedClients.add(line);
                }
            }
        } while (line != null);

        return trustedClients;
    }

    protected Group[] getRoleSets() throws LoginException {
        // we've already verified user's cert is valid, log them 
        // in as SUPER
        Group[] roleSets = new Group[] { new SimpleGroup("Roles") };
        roleSets[0].addMember(new SimplePrincipal("SUPER"));

        return roleSets;
    }
}
