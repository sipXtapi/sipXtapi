/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package com.pingtel.pds.pgs.jsptags.util;

import java.io.StringReader;

import javax.servlet.jsp.JspException;

import junit.framework.TestCase;

public class XtagXslTagReplacementTest extends TestCase {
    
    private static String XML = "<root/>";
    
    private static String XSL = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
        "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">" +
        "    <xsl:output method=\"html\"/>" +
        "    <xsl:template match=\"/\">" +
        "        hi " +
        "    </xsl:template>" +
        "</xsl:stylesheet>";
    
    public void testStyle() throws JspException {
        StringReader xml = new StringReader(XML);
        StringReader xsl = new StringReader(XSL);
        String output = XtagXslTagReplacement.style(xml, xsl, null).trim();
        assertEquals("hi", output);
    }

}
