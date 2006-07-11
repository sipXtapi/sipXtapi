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


package org.sipfoundry.sipxphone.service.logger ;

import org.sipfoundry.sipxphone.service.* ;
import java.io.* ;
import java.net.* ;
import java.util.* ;

import java.text.DateFormat ;


/**
 *
 */
public class LogMessage implements Serializable
{
    public String strCategory ;
    public int    iType ;
    public String strDescription ;
    public Object objParam1 ;
    public Object objParam2 ;
    public Object objParam3 ;
    public Object objParam4 ;
    public long   lTime ;


    public LogMessage()
    {
        strCategory = null ;
        iType = -1 ;
        strDescription = null ;
        objParam1 = null ;
        objParam2 = null ;
        lTime = 0 ;
    }


    public LogMessage(String strCategory, int iType, String strDescription, Object objParam1, Object objParam2, Object objParam3, Object objParam4)
    {
        this.strCategory = strCategory ;
        this.iType = iType ;
        this.strDescription = strDescription ;
        this.objParam1 = objParam1 ;
        this.objParam2 = objParam2 ;
        this.objParam3 = objParam3 ;
        this.objParam4 = objParam4 ;

        lTime = Calendar.getInstance().getTime().getTime() ;
    }

    public String getTypeString()
    {
        String strType = "Unknown" ;

        switch (iType)
        {
            case Logger.LOG_INFORMATION:
                strType = "Information" ;
                break ;
            case Logger.LOG_WARNING:
                strType = "Warning" ;
                break ;
            case Logger.LOG_ERROR:
                strType = "Error" ;
                break ;
            case Logger.TRAIL_COMMAND:
                strType = "Command" ;
                break ;
            case Logger.TRAIL_NOTIFICATION:
                strType = "Notification" ;
                break ;
        }
        return strType ;
    }


    public String getDateString()
    {
        DateFormat df = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT) ;
        Date d = new Date(lTime) ;
        return df.format(d) ;
    }


    public String shortString()
    {
        StringBuffer results = new StringBuffer() ;

        results.append(strCategory) ;
        results.append(" ") ;
        results.append(getTypeString()) ;
        results.append(": ") ;
        results.append(strDescription) ;
        results.append(" ") ;
        results.append(objParam1) ;
        results.append(" ") ;
        results.append(objParam2) ;
        results.append(" ") ;
        results.append(objParam3) ;
        results.append(" ") ;
        results.append(objParam4) ;

        return results.toString() ;
    }


    public String toString()
    {
        StringBuffer results = new StringBuffer() ;

        results.append(getDateString()) ;
        results.append(" ") ;
        results.append(strCategory) ;
        results.append(" ") ;
        results.append(getTypeString()) ;
        results.append(": ") ;
        results.append(strDescription) ;
        results.append(" ") ;
        results.append(objParam1) ;
        results.append(" ") ;
        results.append(objParam2) ;
        results.append(" ") ;
        results.append(objParam3) ;
        results.append(" ") ;
        results.append(objParam4) ;

        return results.toString() ;
    }
}