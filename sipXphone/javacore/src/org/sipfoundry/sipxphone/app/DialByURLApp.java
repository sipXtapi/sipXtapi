/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.app ;

import javax.telephony.* ;

import org.sipfoundry.stapi.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;

import org.sipfoundry.sipxphone.app.dialbyurl.* ;

/**
 * Allows users to dial the phone by URL as opposed to a number/userid.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DialByURLApp extends Application
{
    public void main(String argv[])
    {
        Shell       shell = Shell.getInstance() ;

        /*
         * Go ahead and show the DialByURLForm
         */
        DialByURLForm form = new DialByURLForm(this, Shell.getCallManager().getInFocusCall()) ;
        if (form.showModal() == DialByURLForm.DIAL) {
            String strDial = form.getDialString() ;
            try
            {
                shell.getDialingStrategy().dial(PAddressFactory.getInstance().createAddress(strDial)) ;
            }
            catch (Exception e)
            {
                Shell.getInstance().showUnhandledException(e, false) ;
            }
        } else
            shell.getDialingStrategy().abort() ;
    }
}
