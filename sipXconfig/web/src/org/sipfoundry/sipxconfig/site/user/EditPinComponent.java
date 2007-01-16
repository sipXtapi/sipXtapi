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
package org.sipfoundry.sipxconfig.site.user;


/**
 * Components that edit pin
 * 
 * @see UserForm.initializePin and UserForm.updatePin
 */
public interface EditPinComponent {
    
    // Display this dummy PIN value (masked) to indicate that a PIN exists.
    // We can't use a real PIN.  We don't know the real PIN and if we did,
    // we shouldn't show it.
    // Pick an obscure PIN to avoid colliding with real user PINs.  (I tried using a
    // non-printable PIN "\1\1\1\1\1\1\1\1" but Tapestry silently discards the string!)
    public static final String DUMMY_PIN = "`p1n6P0\361g";
    
    public void setPin(String pin);
    
    public String getPin();
}
