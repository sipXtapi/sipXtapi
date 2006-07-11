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


package org.sipfoundry.sipxphone.awt ;

import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;

import java.awt.* ;
import java.util.* ;

public class PDefaultListModel implements PListModel
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** vector of our data */
    protected Vector m_vElements  = null ;
    /** vector of listeners */
    protected Vector m_vListeners = null ;
    /** hashtable of elements and their respective hints */
    protected Hashtable m_hashElementsHints = null;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public PDefaultListModel()
    {
        m_vElements = new Vector() ;
        m_hashElementsHints = new Hashtable();
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * How many elements do we have in our list?
     */
    public int getSize()
    {
        return m_vElements.size() ;
    }



    /**
     * snag the element at position iIndex
     */
    public Object getElementAt(int iIndex)
    {
        return m_vElements.elementAt(iIndex) ;
    }


    /**
     * inserts an element at the specified index
     * @param objObject element that will  be inserted
     * @param iIndex index at which the new element will be inserted
     */
    public synchronized void insertElementAt(Object objElement, int iIndex)
    {
       insertElementAt( objElement, null, iIndex) ;
    }

     /**
     * inserts an element at the specified index  and
     * associated the hint to it.
     * @param objObject element that will  be inserted
     * @param strHint  hint for the new element that
     *                 will be inserted to the list
     * @param iIndex index at which the new element will be inserted
     */
    public synchronized void insertElementAt
        (Object objElement, String strHint, int iIndex)
    {
        m_vElements.insertElementAt( objElement, iIndex) ;
        if( (strHint != null) && (objElement != null))
            m_hashElementsHints.put( objElement, strHint );
        // Inform anyone who cares about the change to the model


    //int iItems = m_vElements.size() ;

    //we were passing "iItems" as 2nd and 3rd param before.
    //Because of that we were not seeing the "INTERVAL_ADDED" event
    //on the right index.
        fireDataChanged(PListDataEvent.INTERVAL_ADDED, iIndex, iIndex) ;
    }



    /**
     * adds an element to the list.
     * @param objObject element that will  be added
     */
    public synchronized void addElement(Object objElement)
    {
        addElement(objElement, null) ;
    }

    /**
     * adds an element to the list and associated the hint to it.
     * @param objObject element that will  be added
     * @param strHint  hint for the new element that will be added to the list
     */
    public synchronized void addElement(Object objElement, String strHint)
    {
        m_vElements.addElement(objElement) ;

        if( (strHint != null) && (objElement != null))
            m_hashElementsHints.put( objElement, strHint );

        // Inform anyone who cares about the change to the model
        int iItems = m_vElements.size() ;
        fireDataChanged(PListDataEvent.INTERVAL_ADDED, iItems, iItems) ;
    }


    /**
     * adds an array of elements to the list
     */
    public synchronized void addElements(Object objElements[])
    {
        try{
            addElements( objElements, null );
        }catch( IllegalArgumentException e ){
            SysLog.log(e);
        }
    }


    /**
     * add an array of elements and associate the respective hint in
     * strHints array to each element.
     * Make sure the length of objElements and strHints is same.
     *@exception IllegalArgumentException  if the length of objElements
     *            is not same as the length of strHints
     *            (if strHints is not null )
     */
    public synchronized void addElements
         (Object objElements[], String strHints[])
         throws IllegalArgumentException{

         if( strHints != null ){
             if( objElements.length != strHints.length ){
                 throw new IllegalArgumentException
                     ("size of objElements and strHints is not equal" );
             }
         }
         int iBeginIndex ;
         int iEndIndex ;

         iBeginIndex = m_vElements.size() ;
         for (int i=0; i<objElements.length; i++) {
             m_vElements.addElement( objElements[i]) ;
             if( (strHints != null) && (strHints[i] != null))
                 m_hashElementsHints.put( objElements[i], strHints[i]);
        }
         iEndIndex = iBeginIndex + objElements.length ;

         fireDataChanged
             (PListDataEvent.INTERVAL_ADDED, iBeginIndex, iEndIndex) ;
     }


    /**
     * Insert the specified 'new element' immediately after the specified
     * 'existing object'.
     *
     * @param objNewElement The new element that will be added to the list.
     * @param objExistingElement The element the new element will be inserted
     *        after.
     *
     * @exception IllegalArgumentException if the existing element is not a
     *            member of this list.
     *
     */
    public synchronized void insertElementAfter
        (Object objNewElement, Object objExistingElement)
        throws IllegalArgumentException
    {
        insertElementAfter( objNewElement, null, objExistingElement );
    }


    /**
     * Insert the specified 'new element' immediately after the specified
     * 'existing object'.
     *
     * @param objNewElement The new element that will be added to the list.
     * @param strHint  hint for the new element that will be added to the list
     * @param objExistingElement The element the new element will be inserted
     *        after.
     *
     * @exception IllegalArgumentException if the existing element is not a
     *            member of this list.
     *
     */
    public synchronized void insertElementAfter
        (Object objNewElement, String strHint, Object objExistingElement)
        throws IllegalArgumentException
    {
        int iPosition = m_vElements.indexOf(objExistingElement) ;
        if (iPosition == -1) {
            throw new IllegalArgumentException("existing element is not a member of this list") ;
        } else {
            m_vElements.insertElementAt(objNewElement, iPosition+1) ;
            if( (strHint != null) && (objNewElement != null))
                m_hashElementsHints.put( objNewElement, strHint );
            fireDataChanged(PListDataEvent.INTERVAL_ADDED, iPosition+1, iPosition+1) ;
        }
    }



    /**
     * Insert the specified 'new element' immediately before the specified
     * 'existing object'.
     *
     * @param objNewElement The new element that will be added to the list.
     * @param objExistingElement The element the new element will be inserted
     *        before.
     *
     * @exception IllegalArgumentException if the existing element is not a
     *            member of this list.
     *
     */
    public synchronized void insertElementBefore
        (Object objNewElement, Object objExistingElement)
        throws IllegalArgumentException
    {
        insertElementBefore( objNewElement, null, objExistingElement );
    }


     /**
     * Insert the specified 'new element' immediately before the specified
     * 'existing object'.
     *
     * @param objNewElement The new element that will be added to the list.
     * @param strHint  hint for the new element that will be added to the list
     * @param objExistingElement The element the new element will be inserted
     *        before.
     *
     * @exception IllegalArgumentException if the existing element is not a
     *            member of this list.
     *
     */
    public synchronized void insertElementBefore
        (Object objNewElement, String strHint, Object objExistingElement)
        throws IllegalArgumentException
    {
        int iPosition = m_vElements.indexOf(objExistingElement) ;
        if (iPosition == -1) {
            throw new IllegalArgumentException("existing element is not a member of this list") ;
        } else {
            m_vElements.insertElementAt(objNewElement, iPosition) ;
            if( (strHint != null) && (objNewElement != null))
                m_hashElementsHints.put( objNewElement, strHint );
            fireDataChanged(PListDataEvent.INTERVAL_ADDED, iPosition, iPosition) ;
        }
    }


    /**
     * Remove item element at position iIndex from the list model
     */
    public synchronized void removeElementAt(int iIndex)
    {
        if (iIndex != -1) {

            Object objElement = m_vElements.elementAt(iIndex);
            if (objElement != null)
                m_hashElementsHints.remove( objElement );

            m_vElements.removeElementAt(iIndex) ;

            fireDataChanged(PListDataEvent.INTERVAL_REMOVED, iIndex, iIndex) ;
        }
    }


    /**
     * Remove an element from the list model
     */
    public synchronized boolean removeElement(Object objElement)
    {
        boolean bRC = false ;
        int iIndex = m_vElements.indexOf(objElement) ;
        if (iIndex != -1) {
            bRC = true ;
            m_vElements.removeElementAt(iIndex) ;
            if( objElement != null )
                m_hashElementsHints.remove( objElement );
            fireDataChanged(PListDataEvent.INTERVAL_REMOVED, iIndex, iIndex) ;
        }
        return bRC ;
    }


    /**
     * Remove all entries from our list
     */
    public synchronized void removeAllElements()
    {
        int iLength = m_vElements.size() ;
        if (iLength > 0) {
            m_vElements.removeAllElements() ;
            m_hashElementsHints.clear();
            fireDataChanged(PListDataEvent.INTERVAL_REMOVED, 0, iLength) ;
        }
    }


    /**
     * snag any popup information related to the item at position iIndex
     * or null if nothing is available.
     */
    public String getElementPopupTextAt(int iIndex)
    {
        String strHint = null;
        Object objElement = m_vElements.elementAt(iIndex);
        if( objElement != null )
            strHint = (String)(m_hashElementsHints.get( objElement ));
        return strHint ;
    }


    /**
     * stock list data listener interface
     */
    public void addListDataListener(PListDataListener listener)
    {
        // only create listeners if needed
        if (m_vListeners == null) {
            m_vListeners = new Vector() ;
        }

        m_vListeners.addElement(listener) ;

    }


    /**
     * stock list data listener interface
     */
    public void removeListDataListener(PListDataListener listener)
    {
        if (m_vListeners != null) {
            m_vListeners.removeElement(listener) ;

            // null out if no longer needed
            if (m_vListeners.size() == 0)
                m_vListeners = null ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * tell all of our listeners about a particular change
     */
    protected void fireDataChanged(int iType, int iIndex0, int iIndex1)
    {
        if (m_vListeners != null) {
            PListDataEvent event = new PListDataEvent(this, iType, iIndex0, iIndex1) ;

            Enumeration enum = m_vListeners.elements() ;
            while (enum.hasMoreElements()) {
                PListDataListener listener = (PListDataListener) enum.nextElement() ;
                if (listener != null) {
                    try {
                        switch (iType) {
                            case PListDataEvent.CONTENTS_CHANGED:
                                listener.contentsChanged(event) ;
                                break ;
                            case PListDataEvent.INTERVAL_ADDED:
                                listener.intervalAdded(event) ;
                                break ;
                            case PListDataEvent.INTERVAL_REMOVED:
                                listener.intervalRemoved(event) ;
                                break ;
                        }
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }

}
