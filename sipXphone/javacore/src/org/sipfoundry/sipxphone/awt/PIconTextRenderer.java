/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/PIconTextRenderer.java#2 $
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

import java.util.* ;
import java.awt.* ;

import org.sipfoundry.sipxphone.sys.* ;


/**
 * 
 *
 */
public class PIconTextRenderer implements PItemRenderer 
{    
//////////////////////////////////////////////////////////////////////////////
// Construction
//// 
    public PIconTextRenderer()
    {
        
    }    
    

//////////////////////////////////////////////////////////////////////////////
// Public Methods
//// 
    public Component getComponent(Object objSource, Object objValue, boolean bSelected)
    {  
        Component compRC = null ;
        
        if ((objValue != null) && (objValue instanceof PIconTextRendererData)) {
            PIconTextRendererData data = (PIconTextRendererData) objValue ;
            
            icIconTextContainer label = new icIconTextContainer(data.getIcon(), data.getText()) ;
            label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;
            label.setOpaque(true) ;
     
            if (bSelected) {
                label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT_SELECTED)) ; 
                label.setBackground(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND_SELECTED)) ;  
            } else {
                label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
                label.setBackground(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;
            }
            compRC = label ;            
        } else {                           

            PLabel label = new PLabel(objValue.toString(), PLabel.ALIGN_EAST) ;
     
            label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;
            label.setOpaque(true) ;
     
            if (bSelected) {
                label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT_SELECTED)) ; 
                label.setBackground(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND_SELECTED)) ;  
            } else {
                label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
                label.setBackground(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;
            }
            compRC = label ;
        }        
        return compRC ;            
    }
    
    
    private class icIconTextContainer extends PContainer
    {        
        protected PLabel m_lblImage ;
        protected PLabel m_lblText ;
        
        public icIconTextContainer(Image icon, String strText)
        {            
            GridBagLayout      gbl = new GridBagLayout() ;
            GridBagConstraints gbc = new GridBagConstraints() ;
            
            setLayout(gbl) ;                        
            
            // Add Text
            gbc.weightx = 1.0 ;
            gbc.weighty = 1.0 ;
            gbc.fill = GridBagConstraints.BOTH ;
            gbc.insets = new Insets(0, 0, 0, 0) ;
            gbc.gridwidth = 1 ;
            m_lblText = new PLabel(strText, PLabel.ALIGN_EAST) ;
            m_lblText.setInsets(new Insets(0, 0, 0, 10)) ;
            add(m_lblText, gbc) ;
                            
            // Add Image
            gbc.weightx = 0.0 ;
            gbc.weighty = 1.0 ;
            gbc.insets = new Insets(0, 0, 0, 0) ;
            gbc.fill = GridBagConstraints.VERTICAL ;
            gbc.gridwidth = GridBagConstraints.REMAINDER ;
            m_lblImage = new PLabel(icon) ;
            add(m_lblImage, gbc) ;                        
        }
        
        public void repaint()
        {
            super.repaint() ;
            if (m_lblImage != null)
                m_lblImage.repaint() ;
            if (m_lblText != null)
                m_lblText.repaint() ;                        
            
        }
        
        
        public void setForeground(Color color)
        {
            if (m_lblImage != null)
                m_lblImage.setForeground(color) ;
            if (m_lblText != null)
                m_lblText.setForeground(color) ;                        
        }
        
        
        public void setBackground(Color color)
        {
            if (m_lblImage != null)
                m_lblImage.setBackground(color) ;
            if (m_lblText != null)
                m_lblText.setBackground(color) ;                        
        }
        
        public void setFont(Font font)
        {
            if (m_lblImage != null)
                m_lblImage.setFont(font) ;                        
            if (m_lblText != null)
                m_lblText.setFont(font) ;                        
                
        }
        
        public void setOpaque(boolean bOpaque)
        {
            if (m_lblImage != null)
                m_lblImage.setOpaque(bOpaque) ;
            if (m_lblText != null)
                m_lblText.setOpaque(bOpaque) ;                        
        }                
    }
}

