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

import java.awt.* ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;

public class PBorderedContainer extends PContainer
{
    public static final int TAB_BORDER = 0 ;
    public static final int MESSAGEBOX_BORDER = 1 ;
    public static final int APP_BORDER = 2 ;
    
    protected     Component m_comp ; 
        
    protected int         m_iStyle ;
            
    public PBorderedContainer()
    {                
        m_iStyle = TAB_BORDER ; 
        initializeContainer() ;               
        
        setLayout(null) ;          
    }
    
    
    public PBorderedContainer(int iStyle)
    {
        m_iStyle = iStyle ; 
        initializeContainer() ;
                
        setLayout(null) ;
        setOpaque(false) ;
    }
    
    
    protected void initializeContainer()
    {
        AppResourceManager resMgr = AppResourceManager.getInstance() ;
        
        switch (m_iStyle) {
            case TAB_BORDER:
                setBackgroundImage(resMgr.getImage("imgTabFrameBackground")) ;
                setInsets(new Insets(2, 10, 2, 10)) ;
                break ;
            case MESSAGEBOX_BORDER:
                setBackgroundImage(resMgr.getImage("imgDialogFrameBackground")) ;
                // setInsets(new Insets(4, 4, 4, 4)) ;
                setInsets(new Insets(0, 0, 0, 0)) ;
                break ;
            case APP_BORDER:
                setBackgroundImage(resMgr.getImage("imgAppFrameBackground")) ;
                // setInsets(new Insets(8, 8, 8, 8)) ;
                // setInsets(new Insets(2, 10, 2, 0)) ;
                setInsets(new Insets(2, 8, 2, 8)) ;
                break ;                        
        }        
    }


    public Component add(Component comp)
    {
        Rectangle bounds = getBounds() ;
        
        if (m_comp != null) {
            remove(m_comp) ;            
        }
                
        super.add(comp) ;
        m_comp = comp ;
        Rectangle rectBounds = new Rectangle(m_insets.left, m_insets.top, bounds.width-(m_insets.left+m_insets.right), bounds.height-(m_insets.top+m_insets.bottom)) ;
        m_comp.setBounds(rectBounds) ;
        
        return m_comp ;
    }        
}
