/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/CallImpl.java#2 $
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

import java.util.Vector ;
import java.util.Enumeration;
import javax.siplite.Call ;
import javax.siplite.Dialog ;
import javax.siplite.User;
import javax.siplite.Address;
import javax.siplite.CallListener;
import javax.siplite.InvalidArgumentException;
import javax.siplite.AttributeSetException;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.* ;

/**
 * This interface is responsible for creating and managing Dialogs. When a
 * call is created, this call is identified by a call id. This call id will
 * not change over the lifetime of the call and therefore each Dialog created
 * by the call will inherit the call id.
 *
 * Pingtel has added the following extensions:
 *     removeDialogs<br>
 **/
public class CallImpl implements Call
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** user who created this call */
    private User m_user;

    /** callid of this call  */
    private String m_strCallID;

    /** when the request is supposed to expire  */
    private long m_iTimeToExpire = -1;

    /** vector to store call legs created by this call */
    private Vector m_vDialogs = new Vector();


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor accepting user and callid
     */
    public CallImpl(User user, String strCallID)
    {
        m_user = user;
        m_strCallID = strCallID;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * This method creates a Dialog with the from and to address and these
     * will remain constant over the length of the Dialog. The Dialog will
     * have the same call-id as the Call that created it.
     * @param	from	 The address of whom the message is from
     * @param	to       The address of whom the message is to
     * @throws  InvalidArgumentException	If an argument value is
     *                                      incorrect and thus the Dialog
     *                                      cannot be created
     **/
    public Dialog createDialog(Address from, Address to)
        throws InvalidArgumentException
    {
       return createDialog( from, to, true);
    }


    /**
     * Removes the calllegs from this call. This should be called by the
     * programmer (or the system) when the call created is no longer needed.
     * <BR><BR>
     * PINGTEL EXTENION
     */
    public void removeDialogs()
    {
        synchronized(m_vDialogs)
        {
            m_vDialogs.removeAllElements();
        }
    }

    public void removeExpiredDialogs()
    {
        synchronized(m_vDialogs)
        {
            long currentTime = (new java.util.Date()).getTime();
            for (Enumeration e = m_vDialogs.elements() ; e.hasMoreElements() ;) {
                DialogImpl dialog = (DialogImpl)e.nextElement() ;
                if( ( dialog.getExpireTime() != -1 ) &&
                    (  dialog.getExpireTime() <= currentTime ) ){
                    Util.debug("garbage collecting dialog ");
                    m_vDialogs.removeElement(dialog);
                }
            }
        }
    }

    /**
     * This method returns an array of the current Dialogs being used in this call.
     * @return an array of the current Dialogs
     **/
    public Dialog[] getDialogs()
    {
        Dialog[] dialogArray ;

        synchronized(m_vDialogs)
        {
            dialogArray = new Dialog[m_vDialogs.size()];
            for( int i = 0; i<m_vDialogs.size(); i++){
                dialogArray[i] = (DialogImpl)(m_vDialogs.elementAt(i));
            }
        }
        return dialogArray;
    }


    /**
     * Add a CallListener implementation to be notified of new Dialogs. The
     * CallListener interface is implemented by the application.
     * @param callListener	the CallListener interface implementation
     */
    public void addCallListener( CallListener callListener )
    {
       CallProviderImpl.getInstance().addCallListener( callListener );
    }


    /**
     * Remove a CallListener implementation that is registered with the Call.
     * @param 	callListener	the CallListener interface implementation
     */
    public void removeCallListener( CallListener callListener )
    {
       CallProviderImpl.getInstance().removeCallListener( callListener );
    }


    /**
     * Returns the User object that is currently being used in this call.
     * The User must be set in the Call before Dialogs can be created.
     * @return the current User object being used in this call
     */
    public User getUser()
    {
        return m_user ;
    }

    /**
     * Set the current User details. The User must be set when a new incoming
     * or outgoing Call is created.
     * @param user  The User to associate this Call with.
     * @throws AttributeSetException if the user has already been set
     */
    public void setUser(User user) throws AttributeSetException
    {
        throw new AttributeSetException("Not allowed to set user after a Call is created ");
    }


    /**
     * Returns the Call's id. This id will not change over the lifetime
     * of the Call.
     * @return A string representation of the id
     */
    public String getCallId(){
        return m_strCallID;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     *
     */
    protected Dialog createDialog(Address from,
                                  Address to,
                                  boolean bFireOutgoingListeners)
                    throws InvalidArgumentException
    {

        DialogImpl dialog = new DialogImpl(from, to, this);
        m_vDialogs.addElement(dialog);
        if( bFireOutgoingListeners ){
            CallProviderImpl.getInstance().fireOutgoingDialogListeners(dialog);
        }
        return dialog;
    }


    /**
     * sets the expire time of this call.
     * Calls can be garbage collected any time after their
     * expire time exceeds the current time.
     */
    protected void setExpireTime( long iExpireTime)
    {
        m_iTimeToExpire = iExpireTime;
    }

    /**
     * get the expire time of this call.
     */
    protected long getExpireTime()
    {
        return m_iTimeToExpire;
    }


}
