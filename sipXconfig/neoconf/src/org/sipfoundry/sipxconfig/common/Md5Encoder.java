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

import org.apache.commons.codec.digest.DigestUtils;

/**
 * Helper method for creating passtokens
 */
public final class Md5Encoder {
    /** MD5 message digest lenght */
    public static final int LEN = 32;

    private Md5Encoder() {
        // do not instantiate
    }

    /**
     * Computes the digest without DNS domain name
     */
    public static final String digestPassword(String user, String realm, String password) {
        String full = user + ':' + realm + ':' + password;
        return DigestUtils.md5Hex(full);
    }

    /**
     * Computes the digest with DNS domain name - "old way" left for compatibility In future we
     * may allow user to choose this method or "no DNS" method
     * 
     * @deprecated use version that does not require credentials
     */
    public static final String digestPassword(String user, String domain, String realm,
            String password) {
        String full = user + '@' + domain + ':' + realm + ':' + password;
        return DigestUtils.md5Hex(full);
    }
}
