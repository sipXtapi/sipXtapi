/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/calllog/CallLogManager.java#2 $
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
import org.sipfoundry.util.PropertyManager;
import org.sipfoundry.stapi.*;
import org.sipfoundry.util.*;
import org.sipfoundry.sipxphone.sys.Shell;
import javax.telephony.*;

/**
 * CallLogManager.java
 *
 *<p> CallLogManager handles adding/updating/removing CallLegSessions
 * and storing them permanently as well. It is a singleton class
 * and the point of access to external apps which need call log information.
 *
 *<p> This is how call leg entries are added to CallLog repository.
 * Whenever a call is created, an instance of CallLegEntry is created and when
 * the call fails/disconnects/gets destroyed, the entry's lifecycle is over.
 * Those call leg entries are added to an instance of a CallLegSession. A call
 * leg session contains all the call leg entries for that session. For a
 * one-to-one call, a call leg session will only contain one call but for
 * sessions like conference or transfer, a call leg session *may* contain  more
 * than one call leg entries. These call leg sessions are in turn added to
 * a Call Log repository  which gets serialized and is written to a
 * permanent location( flash, hard-drive, etc ).
 *
 * <p> Currently, CallLogForm just shows all the CallLegEntries. It does not
 * categorize them in terms of sessions when displaying.
 *
 *@see org.sipfoundry.sipxphone.sys.calllog.CallLegEntry
 *@see org.sipfoundry.sipxphone.sys.calllog.CallLegSession
 *@see org.sipfoundry.sipxphone.sys.calllog.CallLogConnectionListener
 *@see org.sipfoundry.sipxphone.app.calllog.CallLogForm
 *
 * Created: Tue Sep 25 10:53:24 2001
 * @author Bob Andearsan
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class CallLogManager{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** The default max number of entries the call log can/well hold */
    public static final int DEFAULT_MAX_ENTRIES = 25 ;

    /** The CallLog remember this max number of entries */
    public int MAX_ENTRIES = DEFAULT_MAX_ENTRIES ;

    /** Sorting order: Chronological */
    public static final int SORT_CHRONOLOGICAL = 0 ;
    /** Sorting order: Reverse Chronological */
    public static final int SORT_REVERSE_CHRONOLOGICAL = 1 ;
    /** Sorting order: Alphanumeric */
    public static final int SORT_ALPHANUMERIC = 2 ;
    /** Sorting order: Reverse Alphanumeric */
    public static final int SORT_REVERSE_ALPHANUMERIC = 3 ;


    /** denotes INBOUND CALL */
    public static  final int INBOUND = 1;

    /** denotes OUTBOUND CALL */
    public static  final int OUTBOUND = 2;

    /////////////////STATES////////////////////////
    /** outbound or inbound call was made successfully */
    public static final int COMPLETE = 4 ;

    /** outbound call failed  or inbound call missed
        for one of many reasons */
    public static final int INCOMPLETE = 8 ;

    /** state to denote that this call is in progress */
    public static final int ACTIVE = 16;


    /** constant to denote conference participant */
    public static final int CONFERENCE_PARTICIPANT = 1;

    /** constant to denote transferred call */
    public static final int TRANSFERRED = 2;

    /** constant to denote forwarded call */
    public static final int FORWARDED   = 3;

    /** vector of callLegSessions */
    private Vector m_vSessionEntries ;

    /** has the call log data changed ? */
    private boolean m_bHasChanged = false;

    /** vector of callLegEntries of all sessions */
    private Vector m_vCallLegEntries ;

    /** vector of calllog listeners */
    private Vector m_vCallLogListeners = new Vector() ;


    /** static singelton instance of this CallLogManager */
    private static CallLogManager m_sCallLogManager = new CallLogManager();

   /**
    * private constructor of CallLogManager.
    * It tries to find the number of max callLegEntries
    * and loads the callLogEntries from where it is permanently stored.
    */
    private CallLogManager(){
        String strMaxCallLogEntries =
            PingerConfig.getInstance().getValue("PHONESET_MAX_CALL_LOG_ENTRIES") ;
        if (strMaxCallLogEntries != null) {
            try {
                MAX_ENTRIES = Integer.parseInt(strMaxCallLogEntries) ;
            } catch (NumberFormatException e) {
                System.out.println("Invalid value set for max call log entries:") ;
                SysLog.log(e) ;
            }
        }
       loadCallLegSessions();
       updateCallLegEntries();
   }

   /**
    * commit the changes. It saves the callLegSessions and writes them to
    * permanent storage.
    */
    public void commit(){
      if( m_bHasChanged ){
        saveCallLegSessions();
        m_bHasChanged = false;
      }
    }

    /**
     * gets an instanceof CallLogManager. Anyone trying to
     * get an instance of CallLogManager should use this method
     * as the default constructor is made private and only one instance
     * of this class is available.
     */
    public static CallLogManager getInstance(){
      return m_sCallLogManager;
    }

     /**
     * updates the call leg session in the call manager. If the call leg session
     * does not exist, it adds the entry to a collection of sesssions.
     * @param session new CallLegSession to replace the old one.
     * @see CallLegSession
     */
    public synchronized void updateCallLegSession( CallLegSession session ){
       int iIndex = m_vSessionEntries.indexOf(session);
       if( iIndex != -1 ){
         m_vSessionEntries.removeElementAt(iIndex);
         m_vSessionEntries.insertElementAt(session, iIndex);
         updateCallLegEntries();
       }else{
          addCallLegSession( session );
       }
       m_bHasChanged = true;
       commit();
    }

    /**
     * adds a callLogListener to this CallManager to listen to
     * event like contents changed.
     */
    public void addCallLogListener( CallLogListener listener)
    {
        m_vCallLogListeners.addElement( listener );
    }

    /**
     * removes the CallLogListener listener from this CallManager.
     */
    public void removeCallLogListener( CallLogListener listener)
    {
        m_vCallLogListeners.removeElement( listener );
    }


    /**
     * adds a CallLegSession to a collection of sessions managed by this manager.
     */
     private synchronized void addCallLegSession( CallLegSession session ){
       m_vSessionEntries.insertElementAt(session, 0);
       while (m_vSessionEntries.size() > MAX_ENTRIES) {
            m_vSessionEntries.removeElement(m_vSessionEntries.lastElement()) ;
      }
       updateCallLegEntries();
       m_bHasChanged = true;
     }


     /**
      * removes a CallLegEntry from the CallLegEntries stored.
      * First it finds the CallLegSession associated with this entry
      * and then removes the CallLegEntry from that session.
      * @param entry CallLegEntry that is to be removed.
      */
     public void removeEntry(CallLegEntry entry){
       m_vSessionEntries.removeElement(entry.getCallLegSession());
       updateCallLegEntries();
       m_bHasChanged = true;
       commit();
     }

     /**
      * removes all CallLogSessions from the storage.
      */
     public void removeAll(){
       m_vSessionEntries.removeAllElements();
       updateCallLegEntries();
       m_bHasChanged = true;
       commit();
     }

     /**
      * gets all the Call Leg Sessions as an enumeration.
      * @return an enumeration of all CallLegSessions
      */
     public Enumeration getCallSessions(){
        return m_vSessionEntries.elements();
     }

     /**
      * gets all CallLegEntries as an array .
      * @return an array of all CallLegEntries
      */
     public CallLegEntry[] getCallLegEntries(){
       CallLegEntry[] array = new CallLegEntry[m_vCallLegEntries.size()];
       m_vCallLegEntries.copyInto(array);
       return array;
    }


    /**
     * Gets all the callLegEntries that match the inbound filter,
     *outbound filter and sorted in the sorting order.
     * Multple call status may be ORed to build a more complex query.  For
     * example (CallLogManager.COMPLETE | CallLogManager.INCOMPLETE) will
     * return both missed and answered for incoming filter.
     *  CallLogManager.COMPLETE | CallLogManager.INCOMPLETE
     *@return an array of CallLegEntries that match the inbound filter,
     *  outbound filter and sorted in the sorting order.
     *@param iCallInboundFilter inbound filter to denote whether it is
     *                inbound complete, inbound missed or both.
     *@param iCallOutboundFilter inbound filter to denote whether it is
     *                outbound complete, outbound failed or both.
     *@param iSortOrder sorting order of the entries. Sorting order can be one
     *   of the following:
     * SORT_CHRONOLOGICAL
     * SORT_REVERSE_CHRONOLOGICAL
     * SORT_ALPHANUMERIC
     * SORT_REVERSE_ALPHANUMERIC
     *
     */
     public synchronized CallLegEntry[] getEntriesByCallState
            (int iCallInboundFilter, int iCallOutboundFilter, int iSortOrder){
        CallLegEntry rc[] = null ;
        CallLegEntry allEntries[] = getCallLegEntries();
        Vector vMatches = new Vector() ;

        /*
         * Step 1: find all of our matches
         */

        synchronized (m_vSessionEntries) {
            for (int i=0; i<allEntries.length; i++) {
                CallLegEntry entry = allEntries[i];
                if( entry != null ){
                    if (entry.getDirection() == this.INBOUND ){
                        if(( iCallInboundFilter & entry.getState()) != 0 ){
                            vMatches.addElement(entry) ;
                        }
                    }else if (entry.getDirection() == this.OUTBOUND ){
                        if(( iCallOutboundFilter & entry.getState()) != 0 ){
                            vMatches.addElement(entry) ;
                        }
                    }
                }
            }
        }

        /*
         * Step 2: Create an array out of the matches
         */
        rc = new CallLegEntry[vMatches.size()] ;
        vMatches.copyInto(rc) ;


        /*
         * Step 3: Apply sorting
         */
        switch (iSortOrder) {
            case SORT_CHRONOLOGICAL:
                // We simply need to reverse the list
                reverseCallLogEntries(rc) ;
                break ;
            case SORT_REVERSE_CHRONOLOGICAL:
                // Data is naturally sorted this way; nothing to do.
                break ;
            case SORT_ALPHANUMERIC:
                // We need to sort them
                sortAlphanumeric(rc, true) ;
                break ;
            case SORT_REVERSE_ALPHANUMERIC:
                // We need to sort them
                sortAlphanumeric(rc, false) ;
                break ;
        }
        return rc ;
     }

    public synchronized CallLegEntry getLastDialedCallLeg()
    {
        CallLegEntry calllegEntry = null;
        PAddress rc = null ;
        String strDialedAddress = null ;

        CallLegEntry[] entries = getCallLegEntries();
        for (int i=0;i<entries.length; i++)
        {
            calllegEntry = entries[i];
            if ((calllegEntry.getDirection() == this.OUTBOUND) &&
                (calllegEntry.getType() != TRANSFERRED))
            {
                break;
            }
        }

        return calllegEntry;
    }

     /**
     * What was the last outbound or missed outbound call made?
     *
     * @return last outbound or missed outbound call made
     */
    public synchronized PAddress getRemoteAddressOfLastDialed()
    {
        PAddress rc = null ;
        String strDialedAddress = null ;
        CallLegEntry legEntry = getLastDialedCallLeg();
        if( legEntry != null )
        {
            strDialedAddress = (String) legEntry.getOtherField("dialed_address") ;
            if (strDialedAddress == null)
            {
                PAddress remoteAddress = legEntry.getRemoteAddress();
                if (remoteAddress != null)
                    strDialedAddress = remoteAddress.getAddress() ;
            }
        }

        if (strDialedAddress != null)
        {
            strDialedAddress = SipParser.stripFieldParameters(strDialedAddress) ;
            try
            {
                rc = PAddressFactory.getInstance().createAddress(strDialedAddress) ;

            } catch (Exception e) { }
         }
        return rc ;
    }

    public synchronized PAddress getLastDialed()
    {
        return getRemoteAddressOfLastDialed();
    }

    public synchronized PAddress getLocalAddressOfLastDialed()
    {
        PAddress rc = null ;
        String strLocalAddress = null ;
        CallLegEntry legEntry = getLastDialedCallLeg();
        if( legEntry != null )
        {
            PAddress localAddress = legEntry.getLocalAddress();
            if (localAddress != null)
                strLocalAddress = localAddress.getAddress() ;
        }

        if (strLocalAddress != null)
        {
            strLocalAddress = SipParser.stripFieldParameters(strLocalAddress) ;
            try
            {
                rc = PAddressFactory.getInstance().createAddress(strLocalAddress) ;

            } catch (Exception e) { }
         }
        return rc ;
    }


     /**
      * reverses the CallLegEntries.
      */
     private void reverseCallLogEntries(CallLegEntry entries[])
    {
        int iLength = entries.length ;
        int iMidPoint = iLength / 2 ;

        for (int i=0; i<iMidPoint; i++) {
            swapCallLogEntries(entries, i, (iLength-1)-i) ;
        }
    }

    /**
     * swaps the cal log entries. The one at index1 goes to index2
     * and the one at index2 goes to index1.
     */
    private void swapCallLogEntries(CallLegEntry entries[], int iIndex1, int iIndex2)
    {
        CallLegEntry temp = entries[iIndex1] ;
        entries[iIndex1] = entries[iIndex2] ;
        entries[iIndex2] = temp ;
    }

    /**
     *  Saves the call leg sessions permanently by writing to
     *  the permament location like flash in VXWorks.
     */
    private void saveCallLegSessions(){
       PropertyManager manager = PropertyManager.getInstance() ;
        try {
           manager.putObject("calllog", m_vSessionEntries) ;
        } catch (Exception e) {
            System.out.println("------------------Unable to save call log: " + e) ;
            SysLog.log(e) ;
        }
    }

    /**
     * reads the serialized call leg sessions from the permanent location
     * they were stored in.
     */
    private  void loadCallLegSessions(){
        PropertyManager manager = PropertyManager.getInstance() ;
        try {
            m_vSessionEntries = (Vector) manager.getObject("calllog") ;
        } catch (Exception e) {
            m_vSessionEntries = new Vector() ;
        }
        //to be removed later
        migrateOldCallLog();

    }

    /**
     * reloads the internal cache of CalllegEntries from the
     * CallLegSessions.
     */
    private void updateCallLegEntries(){
        m_vCallLegEntries = new Vector();
        int iSessionVectorSize = m_vSessionEntries.size();
        for( int i = 0; i<iSessionVectorSize; i++ ){
            CallLegSession session = (CallLegSession)(m_vSessionEntries.elementAt(i));
            Vector callLegsVector = session.getCallLegs();
            Enumeration enum = callLegsVector.elements();
            while( enum.hasMoreElements() ){
                  m_vCallLegEntries.addElement( (CallLegEntry)enum.nextElement() );
            }
        }

       //fireCallLogListeners
       for( int i = 0; i<m_vCallLogListeners.size(); i++ )
       {
          CallLogListener listener =
            (CallLogListener)( m_vCallLogListeners.elementAt(i) );
          listener.contentsChanged(new CallLogEvent(new Object()) );
       }
    }



    /**
     *prints out the list of callLegSessions.
     */
    public String toString(){
      StringBuffer buffer = new StringBuffer();
      int size = m_vSessionEntries.size();
      for( int i = 0; i<size; i++ ){
          buffer.append(((CallLegSession)(m_vSessionEntries.elementAt(i)))+"\n");
      }
      return buffer.toString();
    }



    /**
     * sorts alpha-numerically the CallLegEntries.
     * It depends on how CallLegEntry implements the compareTo method
     * from Comparable interface. Current implementation does the alpha-numeric
     * comparison of remote address of CallLegEntry.
     * @see org.sipfoundry.sipxphone.sys.calllog.CallLegEntry
     * @see org.sipfoundry.util.QuickSort
     * @see org.sipfoundry.util.Comparable
     */
    private void sortAlphanumeric(CallLegEntry entries[], boolean bAscending)
    {
        QuickSort qsort = new QuickSort();
        qsort.sort(entries);
        if( !bAscending )
            reverseCallLogEntries(entries);
    }


     private static final boolean m_sDebug = false;
     private void debug( String str )
      {
         if( m_sDebug )
          System.out.println("---------------CallLogManager: DEBUG -->"+str );
      }

      /**
       * migrates the old call log to new one. In the old one, they were stored
       * as CallLogEntries, whereas in the new one they are stored as
       * CallLegEntries which then are stored in CallLegSessions.
       */
      public void migrateOldCallLog(){
         PropertyManager propertyManager = PropertyManager.getInstance();
         CallLog calllog = Shell.getCallLog();
         Object oldCallLog = null;
         try{
            oldCallLog = propertyManager.getObject("xpressa_calllog") ;
         }catch(Exception e){
            //SysLog.log(e);
         }
             if( oldCallLog != null ){
                 System.out.println(".............Migrating old calllog");
                 CallLogEntry[] entries = calllog.getAllEntries();
                 int length = entries.length;
                 for(int i = 0;i<length; i++ ){
                    CallLegSession callLegSession = new CallLegSession();
                    CallLogEntry oldEntry = entries[i];
                    CallLegEntry newEntry = new CallLegEntry();
                    newEntry.setCallID("unknown");
                    newEntry.setRemoteAddress(oldEntry.getAddress());
                    newEntry.setStartTime(oldEntry.getTimeStamp().getTime());
                    long newTime = oldEntry.getTimeStamp().getTime()+
                        ((oldEntry.getDuration())*1000);
                    newEntry.setEndTime(newTime);
                    if( oldEntry.getCallStatus() == CallLogEntry.INBOUND  ){
                        newEntry.setDirection(this.INBOUND);
                        newEntry.setType(this.COMPLETE);
                    }else if( oldEntry.getCallStatus() == CallLogEntry.INBOUND_MISSED  ){
                        newEntry.setDirection(this.INBOUND);
                        newEntry.setType(this.INCOMPLETE);
                    }else if(oldEntry.getCallStatus() == CallLogEntry.OUTBOUND  ){
                        newEntry.setDirection(this.OUTBOUND);
                        newEntry.setType(this.COMPLETE);
                    }else if(oldEntry.getCallStatus() == CallLogEntry.OUTBOUND_FAILED  ){
                        newEntry.setDirection(this.OUTBOUND);
                        newEntry.setType(this.INCOMPLETE);
                    }
                    callLegSession.addCallLegEntry(newEntry);
                    m_vSessionEntries.addElement(callLegSession);
                 }
                 try{
                    propertyManager.removeObject("xpressa_calllog");
                }catch( Exception e ){
                    SysLog.log(e);
                }
             }

      }

}// CallLogManager
