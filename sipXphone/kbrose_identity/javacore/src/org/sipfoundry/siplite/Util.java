/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/Util.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
package org.sipfoundry.siplite;

/**
 * Util.java
 *
 *
 * Created: Mon Mar 04 14:47:59 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 * @deprecated DO NOT EXPOSE THIS CLASS
 */

public class Util {
    private static boolean s_debug = false;

    public static void debug(String msg){
        debug( msg, null );
    }


    public static void debug(String msg, Object obj){
        if( s_debug ){
            StringBuffer buffer = new StringBuffer();
            buffer.append("\n");
            //if( obj != null )
            //    System.out.println("============ "+ obj.getClass().getName()+" ==================");
            buffer.append("============BEGIN SIPLITE DEBUG============");
            buffer.append("\n");
            buffer.append( msg );
            buffer.append("\n");
            buffer.append("============END SIPLITE   DEBUG============");
            buffer.append("\n");
            System.out.println(buffer.toString());
        }
    }

     public static void print(String msg){
            System.out.println("");
            System.out.println("==========================================");
            System.out.println( msg );
            System.out.println("==========================================");
            System.out.println("");
    }






}// Util
