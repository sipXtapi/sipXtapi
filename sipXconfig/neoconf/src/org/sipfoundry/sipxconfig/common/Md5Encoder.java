/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package org.sipfoundry.sipxconfig.common;

import java.security.MessageDigest;

/**
 * Helper method for creating passtokens
 */
public final class Md5Encoder {
    
    private static final int LOWEST_BYTE = 0xFF;
    
    private static final int SHIFT_LEFT_LOWEST_BYTE = 256;
    
    private static final String ENCODER = "MD5";

    private Md5Encoder() {
    }

    /**
     * Using java.security.MessageDigest, encode the given string (e.g.
     * password) using MD5.
     */
    public static final String encode(String src) {
        String des = "";

        try {
            MessageDigest md5 = MessageDigest.getInstance(ENCODER);
            byte[] digest = md5.digest(src.getBytes());
            for (int i = 0; i < digest.length; i++) {
                String s = Integer.toHexString((digest[i] & LOWEST_BYTE) + SHIFT_LEFT_LOWEST_BYTE);
                des += s.substring(1);
            }
        } catch (Exception e) {
            System.out.println(e);
        }

        return des;
    }

    /**
     * Computes the digest without DNS domain name
     */
    public static final String digestPassword(String user, String realm, String password) {
        String full = user + ':' + realm + ':' + password;
        return encode(full);
    }

    /**
     * Computes the digest with DNS domain name - "old way" left for compatibility 
     * In future we may allow user to choose this method or "no DNS" method
     */
    public static final String digestPassword(String user, String domain, String realm,
            String password) {
        String full = user + '@' + domain + ':' + realm + ':' + password;
        return encode(full);
    }
}
