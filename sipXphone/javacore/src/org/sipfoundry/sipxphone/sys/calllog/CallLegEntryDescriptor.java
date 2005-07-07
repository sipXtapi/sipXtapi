/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/calllog/CallLegEntryDescriptor.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.calllog;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import org.sipfoundry.stapi.*;
import org.sipfoundry.stapi.event.PConnectionEvent;
import org.sipfoundry.util.DateUtils;

/**
 * CallLegEntryDescriptor.java
 *
 * Contains methods which describes CallLegEntry's properties.
 *
 * Created: Tue Oct 16 16:45:17 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */


public class CallLegEntryDescriptor {

    /** CallLegEntry this descriptor is describing */
    private CallLegEntry m_callLegEntry;



    /** constructor
     *  takes a CallLegEntry to be described as a parameter.
     */
    public CallLegEntryDescriptor( CallLegEntry entry){
        m_callLegEntry = entry;
    }

    /**
     * @return the display string for the call leg entry
     * It has the following fields:
     * From(or To):
     * Start( time):
     * duration:
     * type
     * other( like conference participant )
     */
    public String getDisplayString(){
        StringBuffer buffer = new StringBuffer();
        buffer.append("Type: "+ getStateAsString()+"\n");

        String remoteFromOrTo;
        String localFromOrTo;
        if( m_callLegEntry.getDirection() == CallLogManager.INBOUND ){
            remoteFromOrTo = "From: ";
            localFromOrTo = "To: ";
        }else{
            remoteFromOrTo = "To: ";
            localFromOrTo = "From: ";
        }
        PAddress remoteAddress = m_callLegEntry.getRemoteAddress();
        if( remoteAddress != null)
             buffer.append( remoteFromOrTo+ stripTag(remoteAddress.getAddress())+"\n");
        PAddress localAddress = m_callLegEntry.getLocalAddress();
        if( localAddress != null)
             buffer.append( localFromOrTo+ stripTag(localAddress.getAddress())+"\n");

        if( m_callLegEntry.getStartTime() != null)
            buffer.append
                ("Start: "+ (new SimpleDateFormat("dd-MMM-yy hh:mm a"))
                                .format(m_callLegEntry.getStartTime())+"\n");
        if( m_callLegEntry.getDuration() != -1 ){
            buffer.append
                ("Duration: "+ DateUtils.getDurationAsString(m_callLegEntry.getDuration())+"\n");
        }
        if( m_callLegEntry.getType() == CallLogManager.CONFERENCE_PARTICIPANT )
            buffer.append("Other: Conference Participant \n");
        else if( m_callLegEntry.getType() == CallLogManager.FORWARDED )
            buffer.append("Other: Forwarded \n");
        else if( m_callLegEntry.getType() == CallLogManager.TRANSFERRED )
            buffer.append("Other: transferee  \n");

        if( m_callLegEntry.getErrorCode() != PConnectionEvent.DEFAULT_RESPONSE_CODE ){
            buffer.append("SIP Code: "+ m_callLegEntry.getErrorText());
            buffer.append(" ("+ m_callLegEntry.getErrorCode()+")\n");
        }
        if( m_callLegEntry.getOtherField("jtapiCause") != null ){
            int jtapiCause =
                ((Integer)(m_callLegEntry.getOtherField("jtapiCause"))).intValue();
            buffer.append("JTAPI Code: "+
                        PConnectionEvent.getCauseAsString(jtapiCause));
        }

        return buffer.toString();
    }





    /**
     * gets the state as String.
     * The values it returns are:
     * <br>unknown
     * <br>inbound complete
     * <br>inbound missed
     * <br>inbound active
     * <br>outbound complete
     * <br>outbound failed
     * <br>outbound active
     * <br>unknown
     */
    public String getStateAsString(){
      String strState = "unknown";
      if( m_callLegEntry.getDirection() == CallLogManager.INBOUND ){

         if( m_callLegEntry.getState() == CallLogManager.COMPLETE)
            strState = "inbound complete";
         else if( m_callLegEntry.getState() == CallLogManager.INCOMPLETE)
            strState = "inbound missed";
         else if( m_callLegEntry.getState() == CallLogManager.ACTIVE)
           strState = "inbound active";

      }else if( m_callLegEntry.getDirection() == CallLogManager.OUTBOUND ){

         if( m_callLegEntry.getState() == CallLogManager.COMPLETE)
            strState = "outbound complete";
         else if( m_callLegEntry.getState() == CallLogManager.INCOMPLETE)
            strState = "outbound failed";
         else if( m_callLegEntry.getState() == CallLogManager.ACTIVE)
           strState = "outbound active";
      }
      return strState;
    }

     /**
     * gets the state as short String.
     * The values it returns are:
     * <br>unknown
     * <br>inbound
     * <br>inbound missed
     * <br>outbound
     * <br>outbound failed
     * <br>active
     * <br>unknown
     */
    public String getStateAsShortString(){
      String strState = "unknown";
      if( m_callLegEntry.getDirection() == CallLogManager.INBOUND ){

          if( m_callLegEntry.getState() == CallLogManager.COMPLETE)
            strState = "inbound";
         else if( m_callLegEntry.getState() == CallLogManager.INCOMPLETE)
            strState = "missed";
         else if( m_callLegEntry.getState() == CallLogManager.ACTIVE)
           strState = "active";

      }else if( m_callLegEntry.getDirection() == CallLogManager.OUTBOUND ){

         if( m_callLegEntry.getState() == CallLogManager.COMPLETE)
            strState = "outbound";
         else if( m_callLegEntry.getState() == CallLogManager.INCOMPLETE)
            strState = "failed";
         else if( m_callLegEntry.getState() == CallLogManager.ACTIVE)
           strState = "active";
      }
      return strState;
    }


     /**
     * get direction as string.
     * if its inbound, returns "inbound"
     * if its outbound, returns "outbound"
     * else returns "unknown"
     */
    public String getDirectionAsString(){
      String str = "unknown";
      if( m_callLegEntry.getDirection() == CallLogManager.INBOUND ){
        str = "inbound";
      }else if( m_callLegEntry.getDirection() == CallLogManager.OUTBOUND ){
        str = "outbound";
      }
      return str;
    }

    /**
     * get start time as String
     * in the short date format and short time format.
     */
     public String getStartTimeAsString()
    {
       if( m_callLegEntry.getStartTime() != null ){
           return DateFormat.getDateInstance(DateFormat.SHORT)
                            .format(m_callLegEntry.getStartTime())
                    + " " + DateFormat.getTimeInstance(DateFormat.SHORT)
                            .format(m_callLegEntry.getStartTime()) ;
        }
        else return ("");
    }

    /**
     * strips ";tag" from an address.
     */
    private String stripTag( String strAddress ){
        String strRet = strAddress;
        int index = -1;
        if( strAddress != null ){
            index = strAddress.indexOf(";tag");
            if( index > 1 )
                strRet = strAddress.substring(0, index);
        }
        return strRet;
    }

}// CallLegDescriptor
