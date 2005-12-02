/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/calllog/CallLegSession.java#2 $
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
import java.util.Vector;
import java.util.Enumeration;
import org.sipfoundry.stapi.PAddress;

/**
 * CallLegSession.java
 *
 * CallLegSession consists of call leg entries within the
 * same session( like transfer, conference, forwarding, etc).
 * If its a one-to-one call, a callLegSession will only consist of
 * one callLegEntry.
 *
 * Created: Fri Sep 21 12:00:53 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class CallLegSession implements java.io.Serializable{

    /**
     * vector to store call leg entries
     */
    private Vector m_vecCallLegs;

    /**
     * creates a new vector when a new session is created.
     */
    public CallLegSession(){
        m_vecCallLegs = new Vector();
    }

    /**
     * add a new call leg entry to this session.
     * @param entry CallLegEntry to add
     */
    public void addCallLegEntry(CallLegEntry entry ){
        entry.setCallLegSession( this );
        m_vecCallLegs.insertElementAt( entry, 0 );
    }

    /**
     *remove a call leg entry from this session.
     */
    public void removeCallLegEntry(CallLegEntry entry){
        m_vecCallLegs.removeElement( entry );
    }

    /**
     * gets the number of call leg entries in this session.
     */
    public int getCallLegsCount(){
        return m_vecCallLegs.size();
    }

    /**
     * gets the Vector of call leg entries in this session.
     */
    public Vector getCallLegs(){
      return m_vecCallLegs;
    }

    /**
     * gets the callID of the first element
     */
    public String getTopLegCallID(){
      String strCallID = null;
       CallLegEntry entry = null;
      if(m_vecCallLegs.size() > 0 )
        entry = (CallLegEntry)(m_vecCallLegs.firstElement());
      if( entry != null )
        strCallID = entry.getCallID();
      return strCallID;
    }

    /**
     * gets the call leg entry with the callid of strCallID and
     * remote address of addrRemoteAddress. If not found, returns null.
     * @param  strCallID callID of the callLegEntry you are trying to get
     * @param  addrRemoteAddress remote address of CallLegEntry you are
     *          trying to get.
     * @return CallLegEntry with the given callID and remoteAddress. null if
     *          none is found.
     * @see org.sipfoundry.sipxphone.sys.calllog.CallLegEntry
     */
    public  CallLegEntry getCallLegEntry
              ( String strCallID, PAddress addrRemoteAddress ){
       //makes a new instance of CallLegEntry just to check if and
       //where the entry with the same callid and remote address exists.
       //equal method in CallLegEntry is overridden to check for equality
       //of callid and remote address for a CallLegEntry instance to be equal
       //with another one.
       CallLegEntry ret = null;
       CallLegEntry entry = new CallLegEntry();
       entry.setCallID(strCallID);
       entry.setRemoteAddress(addrRemoteAddress);
       int iIndex = m_vecCallLegs.indexOf(entry);
       if( iIndex != -1 )
         ret = (CallLegEntry)(m_vecCallLegs.elementAt(iIndex));
       return ret;
    }

    /**
     * gets the call leg entry with the callid of strCallID.
     * If not found, returns null.
     * @param  strCallID callID of the callLegEntry you are trying to get
     * @return CallLegEntry with the given callID . null if
     *          none is found.
     * @see org.sipfoundry.sipxphone.sys.calllog.CallLegEntry
     */
     public  CallLegEntry getCallLegEntry( String strCallID ){
       //probably due to a bug or something,
       //At the receiver's end, the event in CallDestroyed gives a event with
       //a same callid
       //but different address if the receiver hung up the call. If that is not
       //a bug and and it is expected, we need to get a callleg with the same call
       //legid.
       CallLegEntry ret = null;
       CallLegEntry entry = new CallLegEntry();
       entry.setCallID(strCallID);
       int iIndex = m_vecCallLegs.indexOf(entry);
       if( iIndex != -1 )
         ret = (CallLegEntry)(m_vecCallLegs.elementAt(iIndex));
       return ret;
    }



    /**
     * updates the call leg entry in the session. If the call leg entry
     * does not exist, it adds the entry to the call leg session.
     * The way it finds the old one depends on how CallLegEntry's equals
     * method is implemented. Currently, if the callID matches ( if address is
     * null on either or both entries  ) or if the callID and address both match
     * ( if the address is not null in both entries), then two callLegEntries
     * are considered equal.
     *
     * @param entry CallLegEntry to be placed instead of old one.
     */
    public void updateCallLegEntry( CallLegEntry entry ){
       int iIndex = m_vecCallLegs.indexOf(entry);
       if( iIndex != -1 ){
         m_vecCallLegs.removeElementAt(iIndex);
         m_vecCallLegs.insertElementAt(entry, iIndex);
       }else{
          addCallLegEntry( entry );
       }
    }

    /**
     * Two sessions are equal if the number of entries
     * in one callSession and the other are equal AND
     * each of the callLegEntries is equal to each of the
     * callLegEntries in the other session.
     */
    //need to find an efficient method to compare sessions.
    public boolean equals(Object obj ){
       boolean bRet = true;
       if( obj instanceof CallLegSession ) {
          CallLegSession paramSession = (CallLegSession)(obj);
          if( paramSession.getCallLegsCount() != this.getCallLegsCount() )
            bRet = false;
          else{
            Enumeration paramEnum = paramSession.getCallLegs().elements();
            Enumeration localEnum = this.getCallLegs().elements();
             while( paramEnum.hasMoreElements() && localEnum.hasMoreElements() ){
                 CallLegEntry paramEntry = (CallLegEntry)(paramEnum.nextElement());
                 CallLegEntry localEntry = (CallLegEntry)(localEnum.nextElement());
                 if( ! paramEntry.equals(localEntry) ){
                    bRet = false;
                    break;
                 }
              }
          }
       }
      return bRet;
    }


    private static final boolean m_sDebug = false;
      private void debug( String str )
      {
         if( m_sDebug ){
            System.out.println("----------------------------------------");
            System.out.println("---------------CallLegSeesion : DEBUG -->"+str );
            System.out.println("----------------------------------------");
         }
      }



}// CallLegSession
