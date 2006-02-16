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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;

public class BinaryFormatConverterTest extends TestCase {

    public void testConvert() throws IOException {
        byte[] expectedBytes = IOUtils.toByteArray(getClass().getResourceAsStream("config.bin"));
        ByteArrayOutputStream actualStream = new ByteArrayOutputStream();
        InputStream in = getClass().getResourceAsStream("config.txt");
        BinaryFormatConverter.convert("00000000", in, actualStream);
        byte[] actualBytes = actualStream.toByteArray();
        String actual = BinaryFormatConverter.byteArrayToHexString(actualBytes);
        String expected = BinaryFormatConverter.byteArrayToHexString(expectedBytes);
        assertEquals(expected, actual);
    }

    public void testHexStringToByteArray() {
        byte[] actual;

        actual = BinaryFormatConverter.hexStringToByteArray("00");
        assertTrue(Arrays.equals(new byte[] {
            (byte) 0x00
        }, actual));

        actual = BinaryFormatConverter.hexStringToByteArray("fe");
        assertTrue(Arrays.equals(new byte[] {
            (byte) 0xfe
        }, actual));

        actual = BinaryFormatConverter.hexStringToByteArray("0123ff");
        assertTrue(Arrays.equals(new byte[] {
            (byte) 0x01, (byte) 0x23, (byte) 0xff
        }, actual));
    }

    public void testHexStringToByteArrayIllegalFormat() {
        try {
            BinaryFormatConverter.hexStringToByteArray("g0");
            fail();
        } catch (IllegalArgumentException expected) {
        }
    }

    public void testByteArrayToHexString() {
        String actual = BinaryFormatConverter.byteArrayToHexString(new byte[] {
            (byte) 0x01, (byte) 0x23, (byte) 0xff
        });
        assertEquals("0123ff", actual);
    }

    public void testHexStringToByteArrayOddLength() {
        try {
            BinaryFormatConverter.hexStringToByteArray("0");
            fail();
        } catch (IllegalArgumentException expected) {
        }
        try {
            BinaryFormatConverter.hexStringToByteArray("000");
            fail();
        } catch (IllegalArgumentException expected) {
        }
    }
}
