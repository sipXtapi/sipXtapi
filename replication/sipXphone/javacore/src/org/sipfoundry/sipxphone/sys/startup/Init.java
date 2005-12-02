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

 
package org.sipfoundry.sipxphone.sys.startup ;

import java.util.Properties;

import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.shell.* ;
import org.sipfoundry.util.*;


import org.sipfoundry.sipxphone.sys.util.* ;
import java.io.* ;
import java.net.* ;

public class Init 
{
    public static final String HTTP_PROXY_HOST = "PHONESET_HTTP_PROXY_HOST";
    public static final String HTTP_PROXY_PORT = "PHONESET_HTTP_PROXY_PORT";
    
    private static void initHttpProxySettings()
    {
	    Properties props = System.getProperties();	    
        String proxyHost = (String) PingerConfig.getInstance().getValue(HTTP_PROXY_HOST);
        String proxyPort = (String) PingerConfig.getInstance().getValue(HTTP_PROXY_PORT) ;

	    // Check to see if BOTH of the keys are set
	    if ((proxyHost != null) && (proxyHost.length() > 0) && (proxyPort != null) && (proxyPort.length() > 0)) {
            System.out.println("Init::initHttpProxySettings proxy settings are: " + proxyHost + ":" + proxyPort);
	        props.put("http.proxyHost", proxyHost);
	        props.put("http.proxyPort", proxyPort);
	    }
    }

    public static void main(String argv[])
    {
        initHttpProxySettings();
        
        // Start Logger
        Logger.startup() ;        
        
        // Start Timer
        Timer.startup() ;        
        
        // Start Pingtel Event Queue
        PingtelEventDispatcher.startup() ;                
        
        // Force Application Registry Initialization
        ApplicationRegistry.getInstance() ;        
        
        // Force Hook Initialization
        Shell.getHookManager() ;        
    }    
}