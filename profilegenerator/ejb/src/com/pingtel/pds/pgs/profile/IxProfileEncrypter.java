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

package com.pingtel.pds.pgs.profile;

import javax.crypto.spec.PBEParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.Cipher;
import java.security.NoSuchAlgorithmException;
import java.security.GeneralSecurityException;
import java.security.spec.InvalidKeySpecException;

/**
 * IxProfileEncrypter, as the name suggest encyrpts 'plain text'
 * profiles using the PKCS #5 method.  To be more excact it uses
 * MD5 digesting of the plain-text password to produce a key
 * and DES as its cipher.
 *
 * @author ibutcher
 *
 */
public final class IxProfileEncrypter {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static byte[] SALT = {
            (byte) 0xc9, (byte) 0x36, (byte) 0x78, (byte) 0x99,
            (byte) 0x52, (byte) 0x3e, (byte) 0xea, (byte) 0xf2 };

    private static final int INTERATION_COUNT = 2048;

    private static final String CIPHER_ALGORITHM = "PBEWithMD5AndDES";

    private static final PBEParameterSpec mParameterSpec =
            new PBEParameterSpec(SALT, INTERATION_COUNT);


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private SecretKey mSecretKey;
    private Cipher mCipher;

//////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * this constructor takes a 'plain-text' password which is then
     * digests to create a SecretKey for DES.   The password that
     * we use is the owner's ProfileEncryptionKey.
     *
     * @param password value to use for the encryption key.
     * @throws GeneralSecurityException is thrown for a variety of
     * exceptions, none of which should be expected.
     */
    public IxProfileEncrypter (String password)
            throws GeneralSecurityException{

        mSecretKey = getKeyFromPassword(password);
        mCipher = Cipher.getInstance(CIPHER_ALGORITHM);
        mCipher.init(Cipher.ENCRYPT_MODE, mSecretKey, mParameterSpec);
    }

//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * encryptProfile produces a cipher text version of the given
     * byte stream profile.
     *
     * @param plainText clear version of the profile to encrypt.
     * @return ciphered version of the profile.
     * @throws GeneralSecurityException is thrown for a variety of
     * exceptions, none of which should be expected.
     */
    public byte [] encryptProfile(byte [] plainText)
            throws GeneralSecurityException {

        byte [] cipherText = null;
        cipherText = new byte[mCipher.getOutputSize(plainText.length)];
        cipherText = mCipher.doFinal(plainText);

        return cipherText;
    }

    /**
     * getCipherTextLength returns the length of the ciphered
     * profile.   You should use this method before you call
     * encryptProfile so you know how big a byte array you will
     * need for the result.   Note block ciphers output length
     * will mostly likely not be the same as the lenght of the
     * plain text they are encrypting.
     *
     * @param plainTextLength length of the profile that you
     * are planning to encrypt.
     * @return length of the ciphered profile once you have
     * run encryptProfile.
     */
    public int getCipherTextLength(int plainTextLength){
        return mCipher.getOutputSize(plainTextLength);
    }

//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    private SecretKey getKeyFromPassword(String password)
            throws NoSuchAlgorithmException, InvalidKeySpecException {

        SecretKey secretKey = null;
        PBEKeySpec keySpec = new PBEKeySpec(password.toCharArray());

        SecretKeyFactory keyFact =
                SecretKeyFactory.getInstance("PBEWithMD5AndDES");

        secretKey = keyFact.generateSecret(keySpec);

        return secretKey;
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
