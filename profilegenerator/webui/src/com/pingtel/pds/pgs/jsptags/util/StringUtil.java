/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/util/StringUtil.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags.util;

import java.util.StringTokenizer;
import java.util.Vector;

/**
 */
public class StringUtil {

    /*
     This causes many problems with the class loader in JBOSS @JC

    static LocalStrings ls =
        LocalStrings.getLocalStrings( StringUtil.class );
     */

    /**
     *
     * @param s
     *
     * @return
     */
    public static String trimit(String s) {
        if (null != s) {
            s = s.trim();
            if (s.length() == 0) {
                s = null;
            }
        }

        return s;
    }

    /**
     *
     * @param s
     * @param sep
     *
     * @return
     */
    public static String []splitArray(String s,
                                      String sep) {
        StringTokenizer st = new StringTokenizer(s, sep);
        Vector v = new Vector(st.countTokens());

        while (st.hasMoreTokens()) {
            v.addElement(st.nextToken());
        }

        String []rc = new String[v.size()];
        if (rc.length > 0) {
            v.copyInto(rc);
        }

        return rc;
    }

    /**
     * Translates raw string into escaped html with <, > and apersands translated into
     * their HTML equivalents
     *
     * @param html
     *
     * @return
     */
    public static String escapHTML(String html) {
        if ((html.indexOf('<') == -1) &&
            (html.indexOf('>') == -1) &&
            (html.indexOf('&') == -1)) {
            return html;
        }

        int len = html.length();
        StringBuffer sb = new StringBuffer(html.length());
        for (int i = 0 ; i < len ; i++) {
            char c = html.charAt(i);
            if ('<' == c) {
                sb.append("&lt;");
            } else if ('>' == c) {
                sb.append("&gt;");
            } else if ('&' == c) {
                sb.append("&amp;");
            } else {
                sb.append(c);
            }
        }
        return sb.toString();
    }


    public static String unescapHTML(String html) {
        if ((html.indexOf("&lt;") == -1) && (html.indexOf("&gt;") == -1)) {
            return html;
        }

        int len = html.length();
        StringBuffer sb = new StringBuffer();
        for (int i = 0 ; i < len ; ) {
            char c = html.charAt(i);
            if ('&' == c && html.charAt( i+1) == 'l' && html.charAt( i+2) == 't' && html.charAt( i+3) == ';' ) {
                sb.append("<");
                i = i + 4;
            } else if ('&' == c && html.charAt( i+1) == 'g' && html.charAt( i+2) == 't' && html.charAt( i+3) == ';' ) {
                sb.append(">");
                i = i + 4;
            } else if ('&' == c && html.charAt( i+1) == 'a' &&
                    html.charAt( i+2) == 'm' &&
                    html.charAt( i+3) == 'p' &&
                    html.charAt( i+4) == ';' ) {
                sb.append("&");
                i = i + 5;
            } else {
                sb.append(c);
                ++i;
            }
        }
        return sb.toString();
    }

}