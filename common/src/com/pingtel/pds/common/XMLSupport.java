/*
 * $Id: //depot/OPENDEV/sipXconfig/common/src/com/pingtel/pds/common/XMLSupport.java#4 $
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

import java.net.URLEncoder;
import java.net.URLDecoder;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * XML/URL convienence methods 
 * @author DLH
 */
public class XMLSupport 
{
    /**
     * JDOM beta 7 iterators allowed detaching elements in iterators, beta 10 does not
     * you can wrap beta 10 iterators with this iterator to avoid rewriting logic
     */
    public static Iterator detachableIterator(Iterator i)
    {
        ArrayList l = new ArrayList();
        for (;i.hasNext(); l.add(i.next()));
        return l.iterator();
    }

    /** 
     * Encode a string for UTF8 only. 
     *
     * @throws RuntimeException if UTF 8 is not available
     */
    public static String encodeUtf8(String s)
    {
        try 
        {
            return URLEncoder.encode(s ,"UTF-8");
        } 
        catch (UnsupportedEncodingException impossible) 
        {
            throw new RuntimeException("Unsupported UTF-8", impossible);
        }
    }

    /** 
     * Encode a string for UTF8 only. 
     *
     * @throws RuntimeException if UTF 8 is not available
     */
    public static String decodeUtf8(String s)
    {
        try 
        {
            return URLDecoder.decode(s ,"UTF-8");
        } 
        catch (UnsupportedEncodingException impossible) 
        {
            throw new RuntimeException("Unsupported UTF-8", impossible);
        }
    }
}
