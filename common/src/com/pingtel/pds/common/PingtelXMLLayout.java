/*
 * $Id: //depot/OPENDEV/sipXconfig/common/src/com/pingtel/pds/common/PingtelXMLLayout.java#6 $
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

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.TimeZone;

import org.apache.log4j.Category;
import org.apache.log4j.Priority;
import org.apache.log4j.spi.LoggingEvent;
import org.apache.log4j.xml.XMLLayout;
import org.jdom.CDATA;
import org.jdom.Element;
import org.jdom.Content;
import org.jdom.output.XMLOutputter;

/**
 * This class extends the standard log4j xml layout.   It allows us to
 * specialize the output string (see the format method) into a Pingtel
 * standard format.
 *
 *
 * @see org.apache.log4j.xml.XMLLayout
 */
public class PingtelXMLLayout extends XMLLayout {

    private static final String DOW_SEPARATOR = ",";
    private static final String FIELD_SEPARATOR = " ";

    private String m_facility = null;

    public PingtelXMLLayout() { }

    public PingtelXMLLayout( String facility ) {
        m_facility = facility;
    }

    public String format ( LoggingEvent event ) {
        Element eventElement = new Element ( "event" );

        // datetime
        eventElement.setAttribute(  "datetime",
                                    //dateToRFC822 ( new Date (event.getStartTime() ) ) );
                                    dateToRFC822 ( new Date ( ) ) );

        // facility - event
        //eventElement.setAttribute ( "facility", event.categoryName );
        eventElement.setAttribute ( "facility", m_facility );

        // priority
        eventElement.setAttribute( "priority", convertPriority ( event.priority.toString() ) );


        // hostname
        try {
            eventElement.setAttribute(   "hostname",
                                    InetAddress.getLocalHost().getHostName());
        }
        catch ( UnknownHostException ex ) {
            // do nothing
        }

        // processid
        eventElement.setAttribute( "processid", "" );

        // task - create user?

        eventElement.setAttribute( "task", "" );

        // message
        String message = (String)event.getMessage();

        if ( event.priority.equals( Priority.DEBUG ) ) {
            // if we are in debug we probably want at least the filename where
            // it happened.
            String location = event.getLocationInformation().getClassName() +
                " " + event.getLocationInformation().getLineNumber();

            message = message + " : " + location;
        }
        CDATA msg = new CDATA ( message );
        Element text = new Element ( "message" );
        text.addContent( msg  );

        eventElement.addContent( text );

        XMLOutputter xmlOut = new XMLOutputter();

        return xmlOut.outputString( eventElement ) +
            System.getProperty("line.separator");
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // The priorites used by log4j are different that the ones used on the
    // phone.  This method maps the two.
    //
    ///////////////////////////////////////////////////////////////////////
    private String convertPriority ( String priority ) {
        String newPriority = null;

        if ( priority.equalsIgnoreCase( "FATAL") ) {
            newPriority = "PRI_CRIT";
        }
        else if ( priority.equalsIgnoreCase( "ERROR") ) {
            newPriority = "PRI_ERR";
        }
        else if ( priority.equalsIgnoreCase( "TRANSACTION") ) {
            newPriority = "PRI_NOTICE";
        }
        else if ( priority.equalsIgnoreCase( "WARNING") ) {
            newPriority = "PRI_WARNING";
        }
        else if ( priority.equalsIgnoreCase( "INFO") ) {
            newPriority = "PRI_INFO";
        }
        else if ( priority.equalsIgnoreCase( "DEBUG") ) {
            newPriority = "PRI_DEBUG";
        }

        return newPriority;
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // For reasons best know to ourselves we are using IETF RFC 822 date
    // formats.  Part of this means that we are using GMT (UTC) for the dates.
    // This method converts the standard java.util.Date into a 822 string.
    //
    ///////////////////////////////////////////////////////////////////////
    private String dateToRFC822 ( Date date ) {
        StringBuffer dateString = new StringBuffer();

        GregorianCalendar gc = new GregorianCalendar();
        gc.setTime( date );
        gc.setTimeZone( TimeZone.getTimeZone ( "GMT" ) );

        dateString.append( printDayOfWeek ( gc ) );
        dateString.append( DOW_SEPARATOR );
        dateString.append( FIELD_SEPARATOR );
        dateString.append( printTwoDigits ( gc.get( Calendar.DAY_OF_MONTH) ) );

        dateString.append( FIELD_SEPARATOR );
        dateString.append( printMonth ( gc ) );
        dateString.append( FIELD_SEPARATOR );
        dateString.append ( gc.get( Calendar.YEAR ) );
        dateString.append( FIELD_SEPARATOR );
        dateString.append( printTime ( gc ) );

        return dateString.toString();
    }

    private static String printTwoDigits ( int val ) {
        StringBuffer sb = new StringBuffer();
        if ( val < 10 )
            sb.append( "0" );

        sb.append( val );
        return sb.toString();
    }

    private String printTime ( Calendar cal ) {
        StringBuffer sb = new StringBuffer();
        int hour = 0;

        if ( cal.get(Calendar.AM_PM ) == Calendar.PM )
            hour += 12;

        sb.append( printTwoDigits ( cal.get ( Calendar.HOUR ) + hour ) );
        sb.append( ":" );
        sb.append( printTwoDigits ( cal.get ( Calendar.MINUTE ) ) );
        sb.append( ":" );
        sb.append( printTwoDigits ( cal.get ( Calendar.SECOND ) ) );
        sb.append( FIELD_SEPARATOR );
        sb.append( "GMT");
        return sb.toString();
    }

    private String printDayOfWeek ( Calendar cal ) {
        String rfcDay = null;

        int day = cal.get( Calendar.DAY_OF_WEEK );

        switch ( day ) {
            case Calendar.SUNDAY : {
                rfcDay = "Sun";
                break;
            }
            case Calendar.MONDAY : {
                rfcDay = "Mon";
                break;
            }
            case Calendar.TUESDAY : {
                rfcDay = "Tue";
                break;
            }
            case Calendar.WEDNESDAY : {
                rfcDay = "Wed";
                break;
            }
            case Calendar.THURSDAY : {
                rfcDay = "Thur";
                break;
            }
            case Calendar.FRIDAY : {
                rfcDay = "Fri";
                break;
            }
            case Calendar.SATURDAY : {
                rfcDay = "Sat";
                break;
            }

        }
        return rfcDay;
    }


    private String printMonth ( Calendar cal ) {

        int month = cal.get( Calendar.MONTH );
        String rfcMonth = null;

        switch ( month ) {
            case Calendar.JANUARY : {
                rfcMonth = "Jan";
                break;
            }
            case Calendar.FEBRUARY : {
                rfcMonth = "Feb" ;
                break;
            }
            case Calendar.MARCH : {
                rfcMonth = "Mar" ;
                break;
            }
            case Calendar.APRIL : {
                rfcMonth = "Apr" ;
                break;
            }
            case Calendar.MAY : {
                rfcMonth = "May" ;
                break;
            }
            case Calendar.JUNE : {
                rfcMonth = "Jun" ;
                break;
            }
            case Calendar.JULY : {
                rfcMonth = "Jul" ;
                break;
            }
            case Calendar.AUGUST : {
                rfcMonth = "Aug" ;
                break;
            }
            case Calendar.SEPTEMBER : {
                rfcMonth = "Sep" ;
                break;
            }
            case Calendar.OCTOBER : {
                rfcMonth = "Oct" ;
                break;
            }
            case Calendar.NOVEMBER : {
                rfcMonth = "Nov" ;
                break;
            }
            case Calendar.DECEMBER : {
                rfcMonth = "Dec" ;
                break;
            }
        } // switch

        return rfcMonth;

    }

}
