/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/calllog/CallLogConnectionListener.java#2 $
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
import org.sipfoundry.stapi.*;
import org.sipfoundry.stapi.event.*;
import org.sipfoundry.sipxphone.awt.event.*;
import org.sipfoundry.sipxphone.sys.app.CoreApp;
import org.sipfoundry.sipxphone.sys.app.ShellApp;
import org.sipfoundry.sipxphone.sys.Shell;
import org.sipfoundry.sipxphone.service.* ;

/**
 * CallLogConnectionListener.java
 *
 * This is the listener for CallLog to listen to call state changes.
 * It is added to PCallmanager in ShellApp.
 * It creates a new call leg entry when a call is created and
 * adds to a new/existing call leg session. It then stores a variety of info
 * in the call leg entry depending on the events that
 * it can listen to. For example, if it listens to connectionDisconnected event,
 * then it changes the state of the calllegEntry to complete/incomplete
 * depeneding on whether it was ever answered. It also stores the "endTime"
 * as well as a variety of other informations.
 *
 *@see org.sipfoundry.sipxphone.sys.calllog.CallLegEntry
 *@see org.sipfoundry.sipxphone.sys.calllog.CallLegSession
 *@see org.sipfoundry.sipxphone.sys.calllog.CallLogManager
 *@see org.sipfoundry.sipxphone.app.calllog.CallLogForm
 *
 * Created: Fri Sep 21 17:20:13 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class CallLogConnectionListener implements PConnectionListener
{

      /** action listener to listen to Timer event */
      protected  icEventDispatcher      m_dispatcher = new icEventDispatcher();

      /** was a call leg session recently added? */
      protected boolean m_bSessionRecentlyAdded = false;

      /** when creating an instance of CallLogConnectionListener,
       *  it adds a timer whose purpose is to clean up the session holder
       *  every now and then, in case, there are some lingering sessions
       *  that are not needed anymore, which are leftout if
       *  "connectionDisconnected" or "callDestroyed" event get never bubbled.
       */
      public CallLogConnectionListener(){
            //5 minutes
            try{
                Timer.getInstance().addTimer(  300000,  m_dispatcher, null, true ) ;
            }catch( Exception e ){
                e.printStackTrace();
            }
      }

      /**
       * vector to hold call leg sessions. The sessions are removed as they are
       * added to the central repository managed by CallLogManager. They are
       * also checked every 2 minutes to empty if there are some lingering
       * sessions that are not needed anymore, which are leftout if
       * "connectionDisconnected" or "callDestroyed" event get never bubbled.
       */
       Vector m_sessionHolder = new Vector();


      /**
       *  when a call is created, makes a new call leg entry.
       *  If there is already a session associated with that call leg entry
       *  ( found out by comparing call IDs ), then that session is used,
       *  otherwise a new Call Leg Session is created, added to the session
       *  holder. The call leg entry is then added to that session.
       */
      public  void callCreated(PConnectionEvent event)
      {

          makeNewCallLegEntry( event );

          debug("") ;
          debug("callCreated") ;
          debug("\n listener: "+event.toString());
          debug("") ;

      }




     /**
      * connection trying event.
      * This tells us that this call was outbound.
      * If this call was part of a conference, it also
      * sets the type to CallLogManager.CONFERENCE_PARTICIPANT.
      *
      */
      public  void connectionTrying(PConnectionEvent event)
      {
          CallLegEntry callLegEntry = getAssociatedCallLegEntry( event );
          debug("" + callLegEntry );
          //is its a conference call,
          //callCreated event doesn't get called,
          //it only starts from connectionTrying.
          debug(" in connection trying");

          if( callLegEntry == null ){
              if( event.getCall().isConferenceCall() ){
                    callLegEntry = makeNewCallLegEntry( event );
                    callLegEntry.setType(CallLogManager.CONFERENCE_PARTICIPANT);
                    debug("call  leg entry after confernce is " + callLegEntry );
              }
          }

          if( callLegEntry != null ){
                callLegEntry.setCallCreatedEventOnly( false );
                callLegEntry.setDirection(CallLogManager.OUTBOUND);
                if( event.getCause() == event.CAUSE_TRANSFER ){
                    callLegEntry.setType(CallLogManager.TRANSFERRED);
                }
                if( event.getCall() != null){
                  if( event.getCall().isConferenceCall() ){
                  }
                }

                callLegEntry.getCallLegSession().updateCallLegEntry( callLegEntry );
          }
          debug("") ;
          debug("connectionTrying") ;
          debug("\n listener: "+event.toString());
          debug("") ;

      }

      /**
       * connection outbound alerting event. This tells us that the call was
       * outbound.
       */
      public void  connectionOutboundAlerting(PConnectionEvent event)
      {
          CallLegEntry callLegEntry = getAssociatedCallLegEntry( event );
          debug("" + callLegEntry );
          if( callLegEntry != null ){
                callLegEntry.setCallCreatedEventOnly( false );
                callLegEntry.setDirection(CallLogManager.OUTBOUND);


                if( (event.getAddress() != null) &&
                    (callLegEntry.getRemoteAddress() == null)){
                    callLegEntry.setRemoteAddress(event.getAddress());
                    callLegEntry.setLocalAddress(event.getCall().getLocalAddress()) ;
                    String strDialedAddress = (String)
                            event.getCall().getCallData("last_called") ;
                    if( callLegEntry.getType() != CallLogManager.TRANSFERRED){
                        if (strDialedAddress != null)
                            callLegEntry.setOtherField("dialed_address", strDialedAddress);
                    }
                }
                callLegEntry.setState(CallLogManager.INCOMPLETE);
                callLegEntry.getCallLegSession().updateCallLegEntry( callLegEntry );
          }
          debug("") ;
          debug("connectionOutboundAlerting") ;
          debug("\n listener: "+event.toString());
          debug("") ;

        }

      /**
       * connection inbound alerting event. This tells us that the call was
       * inbound. Since this is just the start of a call, it sets the STATE
       * to INACTIVE which will later be AVTIVE if the call got connected or
       * INACTIVE again, if the call failed  or disconnected without connecting.
       */
      public void  connectionInboundAlerting(PConnectionEvent event)
      {
            CallLegEntry callLegEntry = getAssociatedCallLegEntry( event );
            debug("" + callLegEntry );

            if( callLegEntry != null ){
                callLegEntry.setCallCreatedEventOnly( false );
                callLegEntry.setDirection(CallLogManager.INBOUND);
                try{
                    callLegEntry.setLocalAddress(event.getCall().getCalledAddress()) ;
                }catch (Exception e){
                    e.printStackTrace() ;
                }

                if( (event.getAddress() != null) &&
                        (callLegEntry.getRemoteAddress() == null)){
                    callLegEntry.setRemoteAddress(event.getAddress());
                }
                //if its just alerting, its state is incomplete
                //which will be overridden if other methods in connection listener
                //is called.
                callLegEntry.setState(CallLogManager.INCOMPLETE);
                  callLegEntry.getCallLegSession().updateCallLegEntry( callLegEntry );
          }
            debug("") ;
            debug("connectionInboundAlerting") ;
            debug("\n listener: "+event.toString());
            debug("") ;

      }

      /**
       * connection connected. Sets the state to ACTIVE. Also resets the
       * start time as the call duration for a connected call is counted
       * from the time it was connected.
       *
       * Some callleg may just start from connectionConnected, like in the case
       * of a transfer target.
       */
      public void  connectionConnected(PConnectionEvent event)
      {
          CallLegEntry callLegEntry = getAssociatedCallLegEntry( event );
          debug("" + callLegEntry );

          if( callLegEntry == null ){
              callLegEntry = makeNewCallLegEntry( event );
          }
          if( callLegEntry != null ){
              callLegEntry.setState(CallLogManager.ACTIVE);
              callLegEntry.setCallCreatedEventOnly( false );
              callLegEntry.setStartTime( (new java.util.Date()).getTime() );
              if( event.getCause() == event.CAUSE_TRANSFER ){
                callLegEntry.setType(CallLogManager.TRANSFERRED);
                //this is the inbound call from transferee
                callLegEntry.setDirection(CallLogManager.INBOUND);
              }
              callLegEntry.getCallLegSession().updateCallLegEntry( callLegEntry );
          }

          /*
          //TEST CODE
          event.getCall().addDTMFListener(new iCDTMFListener());
          System.out.println("====================JAVA LAYER DTMF LISTENER ADDED.....................");
          */

          debug("") ;
          debug("connectionConnected") ;
          debug("\n listener: "+event.toString());
          debug("") ;


      }

      /*
      class iCDTMFListener implements PDTMFListener
      {
            public void buttonDown( PDTMFEvent event )
            {
                System.out.println("====================JAVA LAYER DTMF EVENT DOWN.....................");
            }
            public void buttonUp( PDTMFEvent event )
            {
                    System.out.println("===================JAVA LAYER DTMF EVENT UP.....................");

            }
      }
    */

      /**
       * connection failed. sets "jtapiCause" field with the value of
       * cause from PConnectionEvent. It also sets the sip response code
       * and sip response text of the CallLegEntry to the ones it gets from
       * the event. Sets the STATE to INCOMPLETE.
       */
      public void  connectionFailed(PConnectionEvent event)
      {
          CallLegEntry callLegEntry = getAssociatedCallLegEntry( event );
          debug("" + callLegEntry );

          if( callLegEntry != null ){
               callLegEntry.setCallCreatedEventOnly( false );
               callLegEntry.setState(CallLogManager.INCOMPLETE);
               try{
                 callLegEntry.setOtherField("jtapiCause", new Integer(event.getCause()));
               }catch( Exception e ){
                e.printStackTrace();
               }
               callLegEntry.setErrorCode(event.getResponseCode());
               callLegEntry.setErrorText(event.getResponseText());
                callLegEntry.getCallLegSession().updateCallLegEntry( callLegEntry );
          }
          debug("") ;
          debug("connectionFailed") ;
          debug("\n listener: "+event.toString());
          debug("") ;

      }

      /**
       * Connection unknown.
       */
      public void  connectionUnknown(PConnectionEvent event)
      {
          CallLegEntry callLegEntry = getAssociatedCallLegEntry( event );
          if( callLegEntry != null ){
            callLegEntry.setCallCreatedEventOnly( false );
            callLegEntry.getCallLegSession().updateCallLegEntry( callLegEntry );
          }
          debug("") ;
          debug("connectionUnknown") ;
          debug("\n listener: "+event.toString());
          debug("") ;

       }

      /**
       * As of now, in conferences, connectionDisconnected may be the
       * last listener method to be called when calls are complete.
       * So most of the setter values aplied to CallLegEntry object here are
       * also done in callDestroyed event.
       *
       * If the current STATE is ACTIVE, it sets the STATE to COMPLETE.
       * otherwise, it sets the STATE to INCOMPLETE.
       * It also sets the endTime of the callLegEntry to the current time.
       *
       * If the event cause is PConnectionEvent.CAUSE_REDIRECTED, it sets the
       * type to CallLogManager.FORWARDED.
       * @see org.sipfoundry.sipxphone.sys.calllog.CallLogManager
       */
      public void  connectionDisconnected(PConnectionEvent event)
      {
          CallLegEntry callLegEntry = getAssociatedCallLegEntry( event );
          debug("" + callLegEntry );
          if( callLegEntry != null ){
                callLegEntry.setCallCreatedEventOnly( false );
                if( callLegEntry.getStartTime() != null)
                    callLegEntry.setEndTime((new java.util.Date()).getTime());
                if( ( callLegEntry.getState() == CallLogManager.ACTIVE) ||
                    ( callLegEntry.getState() == CallLogManager.COMPLETE) ){
                    callLegEntry.setState(CallLogManager.COMPLETE);
                }else{
                    callLegEntry.setState(CallLogManager.INCOMPLETE);
                }

                //for calls that were missed or failed
                //the only place you can grab the address is at
                //connectionDestroyed
                if( (event.getAddress() != null) &&
                ( callLegEntry.getRemoteAddress() == null)){
                     callLegEntry.setRemoteAddress(event.getAddress());
                }
                if( event.getCause() == PConnectionEvent.CAUSE_REDIRECTED){
                    callLegEntry.setType(CallLogManager.FORWARDED);
                    callLegEntry.setDirection(CallLogManager.INBOUND);
                }
                callLegEntry.getCallLegSession().updateCallLegEntry( callLegEntry );
                //adding seesion at connectionDisocnnected.
                CallLogManager.getInstance().updateCallLegSession( callLegEntry.getCallLegSession() );
          }
          debug("") ;
          debug("connectionDisconnected") ;
          debug("\n listener: "+event.toString());
          debug("") ;

      }

      /**
       * call destroyed gets called at the end.
       * In most cases,  callDestroyed gets called after connectionDisconnected.
       * But if callDisocnnected doesn't get called, then callDestroyed needs to
       * doalmost similar things that callDisocnnected does in terms of logging
       * the information for calllog. So most of the setter values aplied to
       * CallLegEntry object here are also done in connectionDisconnected event.
       *
       * If the current STATE is ACTIVE, it sets the STATE to COMPLETE.
       * otherwise, it sets the STATE to INCOMPLETE.
       * It also sets the endTime of the callLegEntry to the current time.
       *
       * @see org.sipfoundry.sipxphone.sys.calllog.CallLogManager
       */

      public void  callDestroyed(PConnectionEvent event)
      {
            CallLegEntry callLegEntry = getAssociatedCallLegEntry( event );
            debug("" + callLegEntry );

            if( (callLegEntry == null) && (! event.getCall().isConferenceCall())){
                callLegEntry = getCallLegEntryWithSameCallID( event );
                debug("call leg entry with same callid ( but not in conference )is " + callLegEntry );
            }
            if( callLegEntry != null ){

                if(( callLegEntry.getStartTime() != null) &&( callLegEntry.getEndTime() == null)){
                    callLegEntry.setEndTime((new java.util.Date()).getTime());

                    if( ( callLegEntry.getState() == CallLogManager.ACTIVE) ||
                        ( callLegEntry.getState() == CallLogManager.COMPLETE) ){
                        callLegEntry.setState(CallLogManager.COMPLETE);
                    }else{
                        callLegEntry.setState(CallLogManager.INCOMPLETE);
                    }

                    //for calls that were missed or failed
                    //the only place you can grab the address is at
                    //connectionDestroyed
                    if( (event.getAddress() != null) &&
                      ( callLegEntry.getRemoteAddress() == null)){
                        callLegEntry.setRemoteAddress(event.getAddress());
                    }
                    //if only callCreated and callDestroyed event got called,
                    //remove the already added  call log entry
                    if( callLegEntry.getCallCreatedEventOnly() ){
                        callLegEntry.getCallLegSession().removeCallLegEntry(callLegEntry);
                    }else{
                        callLegEntry.getCallLegSession().updateCallLegEntry( callLegEntry );
                    }
                    CallLogManager.getInstance().updateCallLegSession( callLegEntry.getCallLegSession() );

                }
                m_sessionHolder.removeElement(callLegEntry.getCallLegSession());
          }

          debug("") ;
          debug("callDestroyed") ;
          debug("\n listener: "+event.toString());
          debug("") ;

      }


      /**
       * call held.
       */
      public void  callHeld(PConnectionEvent event)
      {
          debug("") ;
          debug("callHeld") ;
          debug("\n listener: "+event.toString());
          debug("") ;

      }

      /**
       * call released.
       */
      public void  callReleased(PConnectionEvent event)
      {
          debug("") ;
          debug("callReleased") ;
          debug("\n listener: "+event.toString());
          debug("") ;

      }

      /**
       * gets a call leg entry in the current session with the same callid and
       * same remote address as the ones from the event.
       */
       private  CallLegEntry getAssociatedCallLegEntry( PConnectionEvent event ){
          //both the callid and the remote address make a call leg unique
          //in most cases, just a callid is enough but in case of a conference
          //call, callid could be same for the calls made , so remote address is
          //necessary to make it unique.
          String strCallID = null;
          PAddress addrRemoteAddress = null;
          CallLegEntry callLegEntry = null;

          CallLegSession session = null;
          strCallID = event.getCall().getCallID();
          addrRemoteAddress = event.getAddress();

          if( m_sessionHolder.size() == 1 ){
                session = (CallLegSession)(m_sessionHolder.firstElement());
          }else  if( m_sessionHolder.size() > 1 ){
              if( event.getCall() != null ){
                   session = getSessionWithSameCallID( strCallID );
              }
          }
          if( (strCallID != null) && (addrRemoteAddress != null) && (session != null) ){
            callLegEntry = session.getCallLegEntry(strCallID, addrRemoteAddress );
          }
          return callLegEntry;
      }


      /**
       * Gets the seesion with the call leg entry/entries that has/have the
       * same callid as the paramater.
       */
      private CallLegSession getSessionWithSameCallID( String strCallID ){
        CallLegSession retSession = null;
        Enumeration sessionEnum = m_sessionHolder.elements();
        while( sessionEnum.hasMoreElements() ){
            CallLegSession session = (CallLegSession)(sessionEnum.nextElement());
            Enumeration  callLegEnum =  session.getCallLegs().elements();
            while( callLegEnum.hasMoreElements() ){
                CallLegEntry legEntry = (CallLegEntry)( callLegEnum.nextElement());
                if( legEntry.getCallID().equals( strCallID )){
                    retSession = session;
                    break;
                }
            }
            if( retSession != null )
                break;
        }
        return retSession;
      }


      /**
       * gets a call leg entry with the same callid of the PConnectionEvent
       * event from session holder.
       */
      private  CallLegEntry getCallLegEntryWithSameCallID( PConnectionEvent event ){
          //both the callid and the remote address make a call leg unique
          //in most cases, just a callid is enough but in case of a conference
          //call, callid could be same for the calls made , so remote address is
          //necessary to make it unique.

          String strCallID = null;
          CallLegEntry callLegEntry = null;
          if( event.getCall() != null ){
            strCallID = event.getCall().getCallID();
          }
          if(strCallID != null) {
            CallLegSession session = getSessionWithSameCallID( strCallID );
            if( session != null )
                callLegEntry = session.getCallLegEntry(strCallID);
          }
          return callLegEntry;
      }



       /**
        * makes a new call leg entry.
        */
       private CallLegEntry makeNewCallLegEntry( PConnectionEvent event){
           CallLegEntry newEntry = new CallLegEntry();
           newEntry.setCallID(event.getCall().getCallID());
           newEntry.setStartTime( (new java.util.Date()).getTime() );
           newEntry.setLocalAddress(event.getCall().getLocalAddress()) ;

              //if a call is outbound, when the call gets created
              //the address associated with it is null. However,
              //if a call is inbound, the address associated with it is not null.
              if(  event.getAddress() != null )
                 newEntry.setRemoteAddress(event.getAddress());

             //hack
             newEntry.setCallCreatedEventOnly( true );
             newEntry.setErrorCode(event.getResponseCode());
             newEntry.setErrorText(event.getResponseText());
             m_bSessionRecentlyAdded = true;

             CallLegSession associatedSession = getSessionWithSameCallID( newEntry.getCallID());
             if( associatedSession != null ){
                newEntry.setCallLegSession( associatedSession );
                associatedSession.addCallLegEntry( newEntry );
             }else{
                associatedSession = new CallLegSession();
                associatedSession.addCallLegEntry( newEntry );
                m_sessionHolder.addElement( associatedSession );
             }
             return newEntry;
       }



       /**
        * Action Listener to clean session holder.
        */
       public class icEventDispatcher implements PActionListener
       {
            public void actionEvent(PActionEvent event)
            {
                if( (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
                    && (m_bSessionRecentlyAdded = true ) )
                {
                    cleanSessionHolder() ;
                    m_bSessionRecentlyAdded = false;
                 }
            }
        }

        /**
         * cleans the session holder if the number of entries in the session
         * holder is more than zero. If it is, then checks if the number of
         * calls using CallManager.If the number of calls is zero but the number
         * sessions is more than zero, then go ahead and clean it up as they must be
         * the lingering unwanted sessions.
         */
        private synchronized void cleanSessionHolder(){
           if(  m_sessionHolder.size() > 0 ){
               //System.out.println("...checking to see if I can remove " + m_sessionHolder.size()
               //                     + " sessions from callleg session holder");
               PCall[] calls = Shell.getCallManager().getCalls();
               if( calls.length == 0 ) {
                 m_sessionHolder.removeAllElements();
               }
           }
        }

        /**
         * boolean to turn on/off debug state.
         */
        private static final boolean m_sDebug = false;

        /**
         * prints a debugging message if the debug state is turned on.
         */
        private void debug( String str )
        {
           if( m_sDebug )
              System.out.println("---------------Listener: DEBUG -->"+str );
        }


}// CallLogConnectionListener
