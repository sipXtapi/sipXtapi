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

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;

import java.awt.* ;
import java.util.* ;


/**
 * PItemRenderer identifies components that can be used as "rubber stamps" to 
 * paint your data's representation in a list. 
 * For example, to use a {@link PLabel} to display your data, 
 * you would write something like this:
 *
 * <code><pre>
 *  public class MyItemRenderer implements PItemRenderer 
 *  {
 *      public Component getComponent(Object objSource, Object objValue, boolean bSelected)
 *      {
 *          PLabel label = new PLabel() ;
 *           
 *          label.setText(objValue.toString()) ;
 *          label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;
 *          label.setOpaque(true) ;
 * 
 *          if (bSelected) {
 *              label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT_SELECTED)) ; 
 *              label.setBackground(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND_SELECTED)) ;  
 *          } else {
 *              label.setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
 *              label.setBackground(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;
 *          }
 *          return label ;            
 *      }
 *  }
 * </pre></code>
 *
 * @see org.sipfoundry.sipxphone.sys.SystemDefaults
 *
 * @author Robert J. Andreasen, Jr.
 */ 
public interface PItemRenderer
{
    /**
     * Get the component that should be used to render the specified object 
     * value.
     * 
     * @param objSource The source control requesting the component.
     * @param objValue  The data item that needs to be rendered.
     * @param bSelected The selection state of the control or data item.
     *
     * @return The Component that will be rendered to display the specified 
     *         object value.
     */
    public Component getComponent(Object objSource, Object objValue, boolean bSelected) ;
} 
