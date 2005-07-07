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

package com.pingtel.pds.common;

import java.security.MessageDigest;

public class MD5Encoder {
    /**
     * Using java.security.MessageDigest, encode the given string (e.g.
     * password) using MD5.
     */
    public static String encode(String src) {
        String des = "";

        try {
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            byte[] digest = md5.digest(src.getBytes());
            for (int i = 0; i < digest.length; i++) {
                String s = Integer.toHexString((digest[i] & 0xFF) + 256);
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
    public static String digestPassword(String user, String realm, String password) {
        String full = user + ":" + realm + ":" + password;
        return encode(full);
    }

    /**
     * Computes the digest with DNS domain name - "old way" lefy for compatibility 
     * In future we may allow user to choose this method or "no DNS" method
     */
    public static String digestPassword(String user, String domain, String realm, String password) {
        String full = user + "@" + domain + ":" + realm + ":" + password;
        return encode(full);
    }
}
