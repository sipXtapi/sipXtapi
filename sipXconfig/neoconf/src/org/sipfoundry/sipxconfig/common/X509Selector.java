/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.PrintStream;
import java.security.NoSuchAlgorithmException;

import javax.net.ssl.KeyManagerFactory;

/**
 * Jetty defaults to SunX509 algorithm which is not available on IBM's VM.  This finds the 
 * available X509 algorithms and returns one.
 */
public class X509Selector {

    public static void main(String[] args) {
        PrintStream out = System.out;
        out.print(new X509Selector().getAvailableAlgorithm());
    }
    
    public String getAvailableAlgorithm() {
        String[] algs = new String[] { 
            "SunX509", "IbmX509" 
        };
        return getAvailableAlgorithm(algs);
    }
    
    String getAvailableAlgorithm(String[] algs) {
        for (int i = 0; i < algs.length; i++) {
            try {
                KeyManagerFactory.getInstance(algs[i]);
                return algs[i];
            } catch (NoSuchAlgorithmException e) {
                PrintStream err = System.err;
                err.println("Checking for algorithm " + algs[i] + "...not found");
            }
        }

        return null;        
    }

}
