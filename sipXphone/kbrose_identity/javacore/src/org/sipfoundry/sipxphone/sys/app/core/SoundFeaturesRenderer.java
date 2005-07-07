/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/SoundFeaturesRenderer.java#2 $
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
import java.net.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.sip.* ;

import org.sipfoundry.sipxphone.awt.* ;

/**
 * Displays the visual rendering for an icon
 *
 * @author Dan Winsor
 */
public class SoundFeaturesRenderer extends PDefaultItemRenderer
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** font used to display address display name */
    protected Font fontName = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;

    /** font used to display sip address */
    protected Font fontAddress = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * renderer your data onto the passed graphics context
     */
    public void drawItemText(Graphics g)
    {
        setOpaque(true);
        Rectangle rectBounds = new Rectangle(getSize()) ;

        PLabel label = (PLabel) super.getData() ;
        Image imgStatus = label.getImage() ;

        if (imgStatus != null)
        {
            Rectangle rectImgBounds = rectBounds ;
            rectImgBounds.width = 20 ;

            int xOffset = GUIUtils.calcXImageOffset(imgStatus, rectImgBounds, GUIUtils.ALIGN_CENTER) ;
            int yOffset = GUIUtils.calcYImageOffset(imgStatus, rectImgBounds, GUIUtils.ALIGN_CENTER) ;

            g.drawImage(imgStatus, xOffset, yOffset, this) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
}

