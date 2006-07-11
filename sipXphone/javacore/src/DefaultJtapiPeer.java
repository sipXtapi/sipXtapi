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

import org.sipfoundry.telephony.PtJtapiPeer ;


/**
 * The JtapiPeer class looks for a DefaultJtapiPeer.class within the classpath
 * when searching for a provider.  This allows users to call getJtapiPeer(null)
 * instead of something like getJtapiPeer("org.sipfoundry.telephony.PtJtapiPeer").
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DefaultJtapiPeer extends PtJtapiPeer
{    
    /*
     * No code should be added here- this is simply a wrapper, please make 
     * changes to org.sipfoundry.telephony.PtJavaPeer
     */
}
