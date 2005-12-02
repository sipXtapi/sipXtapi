/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/StatusFrame.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.awt;

import java.util.Enumeration;
import java.util.Vector;
import java.awt.Insets;

import org.sipfoundry.sipxphone.sys.*;
import org.sipfoundry.sipxphone.awt.event.*;
/**
 * StatusFrame.java
 *
 *
 * Created: Fri Jun 15 15:30:20 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class StatusFrame extends PContainer{

    /** vector of status listenerers */
    protected  Vector m_vStatusListeners = new Vector();

    /** the label where the status text gets displayed */
    protected PLabel m_lblStatus ;

    /**
     *@param bCanceleable if it is false, "cancel" label is not shown.
     */
    public StatusFrame( String strText, boolean bCanceleable ){
        super();
        setBackgroundImage(getImage("imgTooltipFrame"));
        m_lblStatus = new PLabel(strText) ;
        m_lblStatus.setFont
            (SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        //m_lblStatus.setInsets(new Insets(6, 10, 6, 10)) ;
        if( bCanceleable )
            m_lblStatus.setBounds(8, 8, 145, 36) ;
        else
            m_lblStatus.setBounds(8, 10, 145, 54) ;

        add(m_lblStatus) ;

        if( bCanceleable ){
            PLabel buttomLeftLabel = new PLabel("") ;
            buttomLeftLabel.setBounds(0, 40, 50, 24) ;
            add(buttomLeftLabel) ;

            PLabel buttomCenterLabel = new PLabel("Cancel") ;
            buttomCenterLabel.setBounds(50, 40, 60, 24) ;
            add(buttomCenterLabel) ;

            PLabel buttomRightLabel = new PLabel("") ;
            buttomRightLabel.setBounds(110, 40, 50, 24) ;
            add(buttomRightLabel) ;
        }
    }

    public StatusFrame(String strText){
        this( strText, true );
    }

    /*
    public void doLayout()
    {
        Dimension dimSize = getSize() ;

        m_buttonBar.setBounds(lower porition of component)
        m_lblStatus.setBounds(bounds of component)


    }
    */


    /** sets the status text for this status frame */
    public void setText( String strText ){
        m_lblStatus.setText( strText );
    }


    /** adds a PStatusListener for listening to status events */
    public  void addStatusListener( PStatusListener listener ){
        m_vStatusListeners.addElement( listener );
    }

    /** removes a PStautsListener */
    public void removeStatusListener( PStatusListener listener ){
        m_vStatusListeners.removeElement( listener );
    }

    /** This gets called when status frame closes. It calls
     * <code>statusClosed</ */
    public void fireStatusClosedEvent(Object objSource, Object objParam){
        for (Enumeration e = m_vStatusListeners.elements();
             e.hasMoreElements() ;) {

            PStatusListener listener =
                (PStatusListener) e.nextElement() ;
            if (listener != null) {
                try {
                    PStatusEvent event = new PStatusEvent
                        (objSource, objParam) ;
                    listener.statusClosed(event) ;

                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }

    public void fireStatusAbortedEvent(Object objSource, Object objParam){

        for (Enumeration e = m_vStatusListeners.elements();
             e.hasMoreElements() ;) {

            PStatusListener listener =
                (PStatusListener) e.nextElement() ;
            if (listener != null) {
                try {
                    PStatusEvent event = new PStatusEvent
                        (objSource, objParam) ;
                    listener.statusAborted(event) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }

    public void fireStatusOpenedEvent(Object objSource, Object objParam){
        for (Enumeration e = m_vStatusListeners.elements();
             e.hasMoreElements() ;) {

            PStatusListener listener =
                (PStatusListener) e.nextElement() ;
            if (listener != null) {
                try {
                    PStatusEvent event = new PStatusEvent
                        (objSource, objParam) ;
                    listener.statusOpened(event) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


}//StatusFrame

