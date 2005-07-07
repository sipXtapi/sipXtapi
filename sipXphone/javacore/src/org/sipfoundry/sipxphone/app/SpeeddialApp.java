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

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.sys.* ;

import org.sipfoundry.sipxphone.app.speeddial.* ;

/**
 * Allows users to display speed dial numbers.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SpeeddialApp extends Application
{
    public void main(String argv[])
    {
        //show a speeddial form based on property filename passed as argument.
        SpeeddialForm form = new SpeeddialForm(this, argv) ;
        Shell shell = Shell.getInstance() ;

        if (form.showModal() == SpeeddialForm.OK)
        {
            try
            {
                shell.getDialingStrategy().dial(PAddressFactory.getInstance().createAddress(form.getAddress())) ;
            }
            catch (Exception e)
            {
                shell.showUnhandledException(e, true) ;
            }
        } else {
            shell.getDialingStrategy().abort(Shell.getCallManager().getInFocusCall()) ;
        }
    }
}
