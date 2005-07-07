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
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.app.calllog.* ;

import org.sipfoundry.stapi.* ;


/**
 * Allows users to display a log of calls (in, out, missed)
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CallLogApp extends Application
{
    public void main(String argv[])
    {
        CallLogForm form = new CallLogForm(this) ;
        form.showModal();
    }

}
