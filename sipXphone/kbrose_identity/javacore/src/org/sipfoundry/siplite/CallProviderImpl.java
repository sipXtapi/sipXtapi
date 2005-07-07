/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/CallProviderImpl.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.siplite ;

import java.util.* ;

import javax.siplite.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sip.*;
import org.sipfoundry.sip.event.*;
import org.sipfoundry.sipxphone.sys.app.ShellApp;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.* ;

/**
 * The role of the CallProvider object is that of creating Call objects.
 * The Call Object is responsible for creating and managing Dialogs.
 * Any number of Calls can be created. For an incoming request or response,
 * the Call object associated with this message can be obtained.
 *
 * Pingtel has added the following extensions:<br>
 *    getCallByCallID<br>
 *    removeCall<br>
 *
 **/
public class CallProviderImpl implements CallProvider, NewSipMessageListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Default SIP version string */
    private static String SIP_VERSION_STRING="SIP/2.0";
    /** garbage collection timeout (in ms)  */
    private static final long CALL_EXPIRE_TIMEOUT= 40000;

    private static final long TRANSACTION_TIMEOUT_MS = 40000*2;
    /** 40 secs x2 for safety */
    /* it should probably get the base time (currently 500ms) from the lower layer */
    /* in the absence of time, because we need to get this out */
    /* Im hard coding these x2 the lower layer values DWW */

    private static final long INVITE_TIMEOUT_MS = (60*3*1000)*2;
    /** 3 mins x2 for safety */
    /* it should probably get the base time (currently 500ms) from the lower layer */
    /* in the absence of time, because we need to get this out */
    /* Im hard coding these x2 the lower layer values DWW */

    //To DO: Need to come back to this later on to make
    //it equal or comparable to the limit imposed in the lower layer.
    public static final int MAX_CALLS_ALLOWED = 9;

    private UserImpl m_localUser;

    private int numberOfMessages = 0;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Vector m_vProviderListenerList;
    private Vector m_vCallListenerList;
    private Vector m_vCalls = new Vector();
    private static CallProviderImpl  m_callProviderImpl;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * private constructor of CallProviderImpl. Those who
     * want an instance of  CallProviderImpl should get it
     * using getInstance() method.
     */
    private CallProviderImpl()
    {
        m_vProviderListenerList = new Vector();
        m_vCallListenerList = new Vector();
        SipMessageFilterCriteria criteria = new SipMessageFilterCriteria( SipMessageFilterCriteria.NONE ) ;
        SipUserAgent.getInstance().addIncomingMessageListener(criteria, this) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Add a CallProviderListener implementation for receiving new incoming
     * Calls. The CallProviderListener interface is implemented by the
     * application.
     * @param callProviderListener	the CallProviderListener interface
	 *                                                                                                                                                                                                                     			implementation
     */
    public void addCallProviderListener( CallProviderListener callProviderListener )
    {
        synchronized (m_vProviderListenerList) {
            if (!m_vProviderListenerList.contains(callProviderListener)) {
                m_vProviderListenerList.addElement(callProviderListener) ;
            }
        }
     }


    /**
     * Remove a CallProviderListener implementation that is registered with
     * the CallProvider.
     * @param callProviderListener	the CallProviderListener interface
	 *                                                                                                                                                                                                                     			implementation
     */
    public void removeListener( CallProviderListener callProviderListener )
    {
         if (m_vProviderListenerList != null) {
            synchronized (m_vProviderListenerList) {
                m_vProviderListenerList.removeElement(callProviderListener) ;
            }
        }
    }


    /**
     * This method is used to create a Call. On creation, a Call will be
     * associated with call id which will remain constant over the lifetime of
     * the Call.
     * @param user	The User associated with the Call.
     * @return 	The created Call
     **/
    public Call createCall(User user)
    {
        return createCall( user, getNewCallID(), true );
    }


    /**
     * This method returns an array of the current calls known to the CallProvider.
     * @return an array of the current Calls
     **/
    public Call[] getCalls()
    {
        removeExpiredCalls();
        Call[] calls;
        synchronized( m_vCalls){
            int size = m_vCalls.size();
            calls = new Call[size];
            for( int i = 0; i<size; i++ ){
               calls[i] = (Call)(m_vCalls.elementAt(i));
            }
        }
        return  calls;
    }


    /**
     * Removes the call from this call provider. This should be called by the
     * programmer( or the system) when the call created is no longer needed.
     *
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param strCallID callid of the call that you want to be removed.
     */
    public void removeCall(Call callRemove)
    {
        /*
         * Implementation Note: The call is not immediately removed. Its
         * marked as expired and gets garbage collected whenever getCalls()
         * is called.
         */

        String strCallID = callRemove.getCallId() ;
        if( strCallID != null )
        {
            synchronized( m_vCalls)
            {
                for (Enumeration e = m_vCalls.elements() ; e.hasMoreElements() ;)
                {
                    CallImpl call = (CallImpl)e.nextElement() ;
                    if( call.getCallId().equals(strCallID) )
                    {
                        call.setExpireTime(new java.util.Date().getTime()+CALL_EXPIRE_TIMEOUT);
                        break;
                    }
                }
            }
        }
    }


    /**
     * Retrieves a call by the call's designated CallID.
     * We do not create a new call if the number of calls in
     * the system exceeds the max allowed or if the method
     * is not SUBSCRIBE OR INVITE.
     * <BR><BR>
     * PINGTEL EXTENSION
     *
     * @param strCallID callid of the call that you are trying to find.
     * @return the call with the callid specified by strCallID.
     */
    public Call getCallByCallID( String strCallID)
    {
        Call retCall = null;
        Call[] calls = getCalls();
        if( calls != null ){
            for( int i = 0; i<calls.length; i++ ){
                if( calls[i].getCallId().equals(strCallID) ){
                    retCall = calls[i];
                    break;
                }
            }
        }
        return retCall;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////


    /**
     * Exposed as a side effect of implementing the NewSipMessageListener
     * interface.
     * @deprecated so that this method doesn't show up in the javadocs.
     *
     * TODO: This should be moved to an inner class
     */
    public void newMessage(NewSipMessageEvent event)
    {
        SipMessage incomingMessage =  event.getMessage() ;
        Util.debug((numberOfMessages++)+" incoming message is \n" + incomingMessage);
        boolean bFireOutgoingListeners = false;
        try{
            SIPMessageImpl jainIncomingMessage = new SIPMessageImpl(incomingMessage.toString());
            String  strCallID      = jainIncomingMessage.getHeaderValue("Call-Id");
            String  strFromAddress = jainIncomingMessage.getHeaderValue("From");
            String  strToAddress   = jainIncomingMessage.getHeaderValue("To");
            String  strMethod      = jainIncomingMessage.getMethod();

            Address fromAddress    = new AddressImpl( strFromAddress );
            Address toAddress      = new AddressImpl( strToAddress );
            boolean bIsRequest =  !(isResponse(jainIncomingMessage));
            Address localAddress ;
            Address remoteAddress ;
            if( bIsRequest ){
                localAddress = toAddress;
                remoteAddress = fromAddress;
            }else{
                localAddress = fromAddress;
                remoteAddress = toAddress;
            }
            if(  strCallID != null )
            {
              CallImpl existingCall = (CallImpl) (getCall(strCallID));
              if (existingCall == null){
                if(  ( m_vCalls.size() < MAX_CALLS_ALLOWED)       &&
                     ( strMethod.equals(Constants.SUBSCRIBE) ||
                       strMethod.equals(Constants.INVITE)        )      )
                {

                    UserImpl localUser = getLocalUser();
                    CallImpl call = (CallImpl) (createCall(localUser,
                                     strCallID,
                                     bFireOutgoingListeners));
                    Util.debug(" new call created for callid " + strCallID +
                               "\n" +
                               "calls size in vector " + m_vCalls.size());

                    DialogImpl dialog = null;
                    dialog = (DialogImpl)call.createDialog(localAddress, remoteAddress,
                        bFireOutgoingListeners);
                    dialog.setExpireTime(new java.util.Date().getTime()+INVITE_TIMEOUT_MS);

                    //if the incoming message is a request
                    //then service the call provider listners and call listeners
                    if (bIsRequest) {
                        String method = ( (SipRequest) (incomingMessage)).getMethod();
                        Util.debug( "servicing call provider listeners for method " + method);
                        fireIncomingCallListeners(call, dialog, jainIncomingMessage, method);
                        fireIncomingDialogListeners(dialog, jainIncomingMessage);
                    }
                    else {
                        fireOutgoingCallListeners(call);
                        fireOutgoingDialogListeners(dialog);
                    }
                }else{
                  Util.debug(" NOT ACCEPTING THIS NEW CALL "+strCallID);
                }

              }
              else {
                //if no call leg exists, then service the callListeners.
                DialogImpl dialog =
                    (DialogImpl) (getDialog(existingCall, remoteAddress));
                if (dialog == null) {

                  dialog = (DialogImpl) (existingCall.createDialog
                                         (localAddress, remoteAddress,
                                          bFireOutgoingListeners));

                  dialog.setExpireTime(new java.util.Date().getTime()+TRANSACTION_TIMEOUT_MS);

                  if (bIsRequest) {
                    fireIncomingDialogListeners(dialog, jainIncomingMessage);
                  }
                  else {
                    fireOutgoingDialogListeners(dialog);
                  }
                }
                else {
                  //update the local and remote address if they have "tag" fields
                  dialog.setLocalAddress(localAddress);
                  dialog.setRemoteAddress(remoteAddress);
                }
            }
          }
        }catch( Exception e ){
            SysLog.log(e);
        }

    }

    /**
     *  returns the singleton instance of CallProviderImpl.
     * @return returns the instance of CallProviderImpl
     * @deprecated DO NOT EXPOSE
     *
     * TODO: This method should not exist (get from sipstack) or should
     *       minimally be protected.
     */
    public static CallProviderImpl getInstance(){
        if( m_callProviderImpl == null ){
            m_callProviderImpl = new CallProviderImpl();
        }
        return m_callProviderImpl;
    }


    /**
     *  adds call listener. call listeners are serviced from here
     *  for efficiency purpose as callListeners and callProviderListeners
     *  both listen for
     */
    protected void addCallListener( CallListener callListener )
    {
        synchronized (m_vCallListenerList) {
            if (!m_vCallListenerList.contains(callListener)) {
                m_vCallListenerList.addElement(callListener) ;
            }
        }
    }

    /**
     * removes call listener
     */
    protected void removeCallListener( CallListener callListener )
    {
         if (m_vCallListenerList != null) {
            synchronized (m_vCallListenerList) {
                m_vCallListenerList.removeElement(callListener) ;
            }
        }
    }

    /**
     * fire call provider listeners when an incoming call comes.
     */
    private void fireIncomingCallListeners
        (Call call, Dialog dialog, SIPMessage jainIncomingMessage, String method){

        for (Enumeration e = m_vProviderListenerList.elements() ; e.hasMoreElements() ;) {
            CallProviderListener listener = (CallProviderListener) e.nextElement() ;
            if (listener != null)
            {
                // Bob: Changed to compile against latest sources
                // listener.incomingCall( call, dialog, jainIncomingMessage, method);
                listener.incomingCall( call, dialog, jainIncomingMessage);
            }
        }
    }

    /**
     *returns a cached copy of localUser or creates a new one and keeps
     *it in cache. The cached copy is refreshed when the default line changes.
     */
    private UserImpl getLocalUser(){
        if( m_localUser == null )
        {
            String strLocalID =
                ShellApp.getInstance().getCoreApp().getDefaultUser();
            Address localAddress    = new AddressImpl( strLocalID );
            m_localUser =  new UserImpl(localAddress);
        }
        return m_localUser;
    }


    /**
     * fire call provider listeners when an outgoing call is made.
     */
    private void fireOutgoingCallListeners(Call call){

        for (Enumeration e = m_vProviderListenerList.elements() ; e.hasMoreElements() ;) {
            CallProviderListener listener = (CallProviderListener) e.nextElement() ;
            if (listener != null) {
                if( listener instanceof CallProviderListenerExt )
                    ((CallProviderListenerExt)(listener)).outgoingCall( call);
            }
        }
    }

    /**
     * fire call listeners when an incoming callleg comes.
     */
    private void fireIncomingDialogListeners
                (Dialog dialog, SIPMessage jainIncomingMessage){
        //I think the api is  not right here
        //if its a new incoming request, how does it have a statusCode
        //and statusString.
        int statusCode   = -1;
        String statusString = null;
        for (Enumeration e = m_vCallListenerList.elements() ; e.hasMoreElements() ;) {
            CallListener listener = (CallListener) e.nextElement() ;
            if (listener != null) {
                // Bob: Changed to compile against latest API
                // listener.incomingDialog( dialog, jainIncomingMessage, statusCode, statusString);
                listener.incomingDialog( dialog, jainIncomingMessage);
            }
        }
    }

    /**
     * fire call listeners when a outgoing callleg is made
     */
    protected void fireOutgoingDialogListeners(Dialog dialog){
        for (Enumeration e = m_vCallListenerList.elements() ; e.hasMoreElements() ;) {
            CallListener listener = (CallListener) e.nextElement() ;
            if (listener != null) {
                if( listener instanceof CallListenerExt )
                    ((CallListenerExt)(listener)).outgoingDialog( dialog);
            }
        }
    }

    /**
     * create a call with the given user and the callid.
     */
    protected Call createCall(User user, String callid)
    {
       return createCall(user, callid, true );
    }


    /**
     * create a call with the given user and the callid.
     * Fire the outgoing call listeners if bFireListeners is true.
     */
     private Call createCall(User user, String callid , boolean bFireListeners)
    {
        Call newCall = new CallImpl(user, callid);
        m_vCalls.addElement( newCall );
        if( bFireListeners ){
            fireOutgoingCallListeners(newCall);
        }
        return newCall;
    }

    /**
     * get the call with this callID
     */
    private Call getCall(String strCallID){
        removeExpiredCalls();
        Call retCall = null;
        if( strCallID != null ){
            synchronized( m_vCalls){
                for (Enumeration e = m_vCalls.elements() ; e.hasMoreElements() ;) {
                    CallImpl call = (CallImpl)e.nextElement() ;
                    if( call.getCallId().equals(strCallID) ){
                        retCall = call;
                        break;
                    }

                }
            }
        }
        return retCall;
    }


    /**
     * get the callleg with this callID, and remoteaddresss
     */
    private Dialog getDialog(Call call, Address remoteAddress){
        Dialog retDialog = null;
        if ( call != null ) {
            Dialog[] dialogs = call.getDialogs();
            for( int i = 0; i<dialogs.length; i++ ){
                DialogImpl dialog = (DialogImpl)(dialogs[i]) ;
                if( dialog.getRemoteAddress().equals(remoteAddress )){
                        retDialog = dialog;
                        break;
                }
            }
        }
        return retDialog;
    }

    /**
     * removes the expired calls from the vector of calls.
     * Also removes the calllegs associated with the call.
     */
    private void removeExpiredCalls(){
        long currentTime = (new java.util.Date()).getTime();
        synchronized( m_vCalls){
            for (Enumeration e = m_vCalls.elements() ; e.hasMoreElements() ;) {
                CallImpl call = (CallImpl)e.nextElement() ;
                if( ( call.getExpireTime() != -1 ) &&
                    (  call.getExpireTime() <= currentTime ) ){
                    Util.debug("garbage collecting call and calllegs of callid " + call.getCallId() );
                    call.removeDialogs();
                    m_vCalls.removeElement(call);
                }
                else
                    call.removeExpiredDialogs();
            }
        }
    }

    /**
     * generates a new callid and returns it.
     */
    private String getNewCallID(){
        return ( Calendar.getInstance().getTime().getTime()+"");
    }



    /**
     * Is this message a response?
     */
    private boolean isResponse(SIPMessage message){
         boolean bRet = false;
         String startLine = ((SIPMessageImpl)(message)).getStartLine();
         if(( startLine != null ) && ( startLine.startsWith(SIP_VERSION_STRING) ))
            bRet = true;

        return bRet;
    }

    /**
     * implementation of SipLineListener to listen to changes
     * to default line.
     */
    private class  icSipLineListner implements SipLineListener
    {
        public void lineEnabled(SipLineEvent event){}
        public void lineFailed(SipLineEvent event) {}
        public void lineTimeout(SipLineEvent event){}
        public void lineAdded(SipLineEvent event) {}
        public void lineDeleted(SipLineEvent event) {}
        public void lineChanged(SipLineEvent event) {}
        public void lineDefault(SipLineEvent event)
        {
            //refresh the cached copy of localuser when default line changes.
            m_localUser = null;
            getLocalUser();
        }
    }

}
