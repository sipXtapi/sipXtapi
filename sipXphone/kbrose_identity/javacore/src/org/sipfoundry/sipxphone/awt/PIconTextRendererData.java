/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/PIconTextRendererData.java#2 $
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

import java.awt.* ;

/**
 * The PIconTextRenderer class is a simple data wrapping object that the
 * PIconTextRenderer is expecting to receive.  Create one of these objects
 * with a text and image component along with the alignment contraints.
 *
 * @author Robert J. Andreasen, Jr.
 *
 * @see PIconTextRenderer
 * @todo Add alignment constraints
 */
public class PIconTextRendererData
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Text String that will be displayed by the renderer */
    protected String m_strText ;
    /** Image Icon that will be displayed by the renderer */
    protected Image  m_imgIcon ;
    /** Optionally supplied data object */
    protected Object m_objData ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default no argument constructor
     */
    public PIconTextRendererData()
    {
        m_strText = null ;
        m_imgIcon = null ;
        m_objData = null ;
    }


    /**
     * Constructor requiring the text and icon image that will be displayed
     * using the PIconTextRenderer.
     *
     * @param strText Text String that will be displayed by the renderer
     * @param imgIcon Image icon that will be displayed by the renderer
     */
    public PIconTextRendererData(String strText, Image imgIcon)
    {
        m_strText = strText ;
        m_imgIcon = imgIcon ;
        m_objData = null ;
    }


    /**
     * Constructor requiring the text and icon image that will be displayed
     * using the PIconTextRenderer.
     *
     * @param strText Text String that will be displayed by the renderer
     * @param imgIcon Image icon that will be displayed by the renderer
     * @param objData Optionally supplied object data
     */
    public PIconTextRendererData(String strText, Image imgIcon, Object objData)
    {
        m_strText = strText ;
        m_imgIcon = imgIcon ;
        m_objData = objData ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the text that will be displayed by the renderer.
     */
    public String getText()
    {
        return m_strText ;
    }


    /**
     * Set the text that will be displayed by the renderer.
     */
    public void setText(String strText)
    {
        m_strText = strText ;
    }


    /**
     * Get the image icon that will be displayed by the renderer.
     */
    public Image getIcon()
    {
        return m_imgIcon ;
    }


    /**
     * Set the image icon that will be displayed by the renderer.
     */
    public void setIcon(Image imgIcon)
    {
        m_imgIcon = imgIcon ;
    }


    /**
     * Get the optionally supplied object data
     */
    public Object getObjectData()
    {
        return m_objData ;
    }


    /**
     * Set the optionally object data
     */
    public void getObjectData(Object objData)
    {
        m_objData = objData ;
    }

}
