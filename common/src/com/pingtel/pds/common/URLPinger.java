/*
 * $Id: //depot/OPENDEV/sipXconfig/common/src/com/pingtel/pds/common/URLPinger.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.common;

import java.io.IOException;
import java.io.InputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;

/**
 * Title:        Boundary Interface Objects
 * Description:  This is used to build the inter cmponet library
 * Copyright:    Copyright (c) 2001
 * Company:      Pingtel Corporation
 * @author
 * @version 1.0
 */

public class URLPinger {

    public URLPinger() {}

    /** Utility class to ping a URL for validity */
    public static boolean isValid ( String urlString ) {
        try {
            URL url = new URL( urlString );
            HttpURLConnection urlConnection = (HttpURLConnection)url.openConnection();
            InputStream inputStream = urlConnection.getInputStream();
            if (inputStream != null) {
                // Ping the Connection by getResponseCode, this send the request
                try {
                    int httpStatusCode = urlConnection.getResponseCode();
                    if (httpStatusCode != urlConnection.HTTP_OK) {
                        return false;
                    }
                    return true;
                } catch (Exception ex) {
                    return false;
                } finally {
                    // Close input stream as these streams buffer
                    inputStream.close();
                }
            }
        } catch (MalformedURLException ex) {
            return false;
        } catch (IOException ex) {
            return false;
        }
        return false;
    }
}