/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
 
package com.pingtel.pds.pgs.profile.propertysort;

import java.util.Collection;

import org.jdom.*;
import org.jdom.input.*;
import org.jdom.output.*;
import java.io.*;
import java.util.*;

public class AlphanumInsensitiveDescending extends PropertySorter {

    private static String value1 = "<USER_LINE ref_property_id=\"45\" id=\"68756768206a616e75733c7369703a34303440666f6f2e636f6d3e\">"+
      "<USER_LINE>"+
         "<ALLOW_FORWARDING>"+
            "<![CDATA[ENABLE]]>"+
         "</ALLOW_FORWARDING>"+

         "<REGISTRATION>"+
            "<![CDATA[REGISTER]]>"+
         "</REGISTRATION>"+

         "<CREDENTIAL>"+
            "<PASSTOKEN>"+
               "<![CDATA[c11dfc903ac7ea65feaf993fe609db71]]>"+
            "</PASSTOKEN>"+

            "<REALM>"+
               "<![CDATA[worldcom.com]]>"+
            "</REALM>"+

            "<USERID>"+
               "<![CDATA[ian]]>"+
            "</USERID>"+
         "</CREDENTIAL>"+

         "<URL>"+
            "<![CDATA[hugh janus<sip:404@foo.com>]]>"+
         "</URL>"+

         "<CREDENTIAL>"+
            "<PASSTOKEN>"+
               "<![CDATA[44a3e3fd9cf5f0b9014a4b9b6198758b]]>"+
            "</PASSTOKEN>"+

            "<REALM>"+
               "<![CDATA[foo.com]]>"+
            "</REALM>"+

            "<USERID>"+
               "<![CDATA[404]]>"+
            "</USERID>"+
         "</CREDENTIAL>"+
      "</USER_LINE>"+
   "</USER_LINE>";

    private static String id1 = "hugh janus<sip:404@foo.com>";

    private static String value2 = "<USER_LINE ref_property_id=\"45\" id=\"7369703a6c696e65322e70696e672e636f6d\">"+
        "<USER_LINE>"+
         "<ALLOW_FORWARDING>"+
            "<![CDATA[DISABLE]]>"+
         "</ALLOW_FORWARDING>"+

         "<URL>"+
            "<![CDATA[sip:line2.ping.com]]>"+
         "</URL>"+

         "<REGISTRATION>"+
            "<![CDATA[PROVISION]]>"+
         "</REGISTRATION>"+

         "<CREDENTIAL>"+
            "<USERID>"+
               "<![CDATA[user1]]>"+
            "</USERID>"+

            "<PASSTOKEN>"+
               "<![CDATA[7e151971a9953c126464a783eb48a9af]]>"+
            "</PASSTOKEN>"+

            "<REALM>"+
               "<![CDATA[realm]]>"+
            "</REALM>"+
         "</CREDENTIAL>"+
        "</USER_LINE>"+
        "</USER_LINE>";

    public static String id2 = "sip:line2.ping.com";

    public static void main ( String args [] ) {

        AlphanumInsensitiveAscending sorter = new AlphanumInsensitiveAscending();

        try {
            XMLOutputter x = new XMLOutputter();

            sorter.addProperty( new SAXBuilder().build( new ByteArrayInputStream ( value1.getBytes()) ).getRootElement(), id1 );

            sorter.addProperty( new SAXBuilder().build( new ByteArrayInputStream ( value2.getBytes()) ).getRootElement(), id2 );

            Collection c = sorter.getValues();
            for ( Iterator i = c.iterator(); i.hasNext(); ) {
                Element e = (Element) i.next();
                System.out.println ( x.outputString( e ) );
            }
        }
        catch ( Exception ex ) {
            ex.printStackTrace();
        }
    }

    public Collection getValues() {
        Collection values = this.m_propertyMap.values();
        ArrayList al = new ArrayList();
        al.addAll( values );
        Collections.reverse( al );
        return al;
    }
}