/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/authentication/src/com/pingtel/pds/jbossauth/MD5Encoder.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.jbossauth;

import java.security.MessageDigest;

public class MD5Encoder {

    /**
     * Using java.security.MessageDigest, encode the given string
     * (e.g. password) using MD5.
     */
    public String encode(String src) {
	    String des = "";

        try {
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            byte[] digest = md5.digest(src.getBytes());
            for (int i = 0; i < digest.length; i++) {
                String s = Integer.toHexString((digest[i]&0xFF) + 256 );
                des += s.substring(1);
            }
        }
        catch (Exception e) {
            System.out.println(e);
        }

        return des;
    }

}
