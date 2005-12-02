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
import org.sipfoundry.sipxphone.app.slideshow.* ;


/**
 * A quick hack of an app to perform a basic slide show
 * 
 * @deprecated For internal use only
 * @author Robert J. Andreasen, Jr.
 */
public class SlideshowApp extends Application
{
    public void main(String argv[])
    {
        SlideshowForm form = new SlideshowForm(this) ;
        
        form.showModal() ;
    }    
}
