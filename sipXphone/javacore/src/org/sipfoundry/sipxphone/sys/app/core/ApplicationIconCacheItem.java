/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/ApplicationIconCacheItem.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

 
package org.sipfoundry.sipxphone.sys.app.core ; 

import java.awt.* ;
import java.awt.image.* ;
import java.util.* ;
import java.net.* ;
import java.io.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;

/**
 * This data encapsulation object is stored by the ApplicationIconCache to
 * cache application icons.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ApplicationIconCacheItem implements Serializable
{
    public Object          m_objData ;
    public int             m_iWidth ;
    public int             m_iHeight ;
    public transient Image m_imgRendered ;
        
    public ApplicationIconCacheItem()
    {
        m_objData = null ;
        m_iWidth = -1 ;
        m_iHeight = -1 ;
        m_imgRendered = null ;
    }
        
        
    public ApplicationIconCacheItem(Object objData, int iWidth, int iHeight, Image imgRendered)
    {
        m_objData = objData ;
        m_iWidth = iWidth ;
        m_iHeight = iHeight ;
        m_imgRendered = imgRendered ;
    }
        
        
    public Image getRenderedImage()
    {
        return m_imgRendered ;   
    }        
}    