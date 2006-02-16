/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public final class BinaryFormatConverter {
    private static final byte[] HEADER = new byte[] {
        0x00, 0x00, 0x00, 0x44, 0x58, 0x1B
    };
    private static final byte[] CRLF = new byte[] {
        0x0d, 0x0a
    };
    private static final int LOBYTE = 0x0f;
    private static final int NIBBLE_SIZE = 4;
    
    private static final char[] HEX_DIGITS = new char[] {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    private BinaryFormatConverter() {        
    }

    public static void convert(String macAddress, InputStream in_, OutputStream out)
        throws IOException {
        BufferedOutputStream bout = new BufferedOutputStream(out);
        bout.write(HEADER);
        bout.write(BinaryFormatConverter.hexStringToByteArray(macAddress));
        bout.write(CRLF);
        bout.write(CRLF);
        bout.flush();
        bout.close();
    }

    static String byteArrayToHexString(byte[] bytes) {
        StringBuffer s = new StringBuffer(bytes.length);
        for (int i = 0; i < bytes.length; i++) {
            char hi = HEX_DIGITS[bytes[i] >> NIBBLE_SIZE & LOBYTE];
            char lo = HEX_DIGITS[bytes[i] & LOBYTE];
            s.append(hi);
            s.append(lo);
        }

        return s.toString();
    }

    static byte[] hexStringToByteArray(String macAddress) {
        int len = macAddress.length();
        if ((len % 2) != 0) {
            throw new IllegalArgumentException("hex string must be multiples of 2");
        }
        byte[] asBytes = new byte[len / 2];
        for (int i = 0; i < macAddress.length(); i += 2) {
            String hex = "0x" + macAddress.substring(i, i + 2);
            int b = Integer.decode(hex).intValue();
            asBytes[i / 2] = (byte) b;
        }
        return asBytes;
    }
}
