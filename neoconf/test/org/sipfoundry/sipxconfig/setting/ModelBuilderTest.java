/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.IOException;
import java.io.InputStream;

import junit.framework.TestCase;


public class ModelBuilderTest extends TestCase {

    private String[] m_suitsNames = new String[] { 
        "hearts", "spades", "clubs", "diamonds" 
    };
    
    private String[] m_cardValues = new String[] { 
        "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" 
    };
    
    public void testReadingGames() throws IOException {
        
        XmlModelBuilder builder = new XmlModelBuilder();
        InputStream in = getClass().getResourceAsStream("games.xml");
        SettingModel games = builder.buildModel(in);
        assertEquals(2, games.size());

        SettingModel chess = (SettingModel) games.getMeta(0);       
        assertEquals(2, chess.size());
        SettingModel colors = (SettingModel) chess.getMeta(0);       
        assertEquals(2, colors.size());
        SettingModel pieces = (SettingModel) chess.getMeta(1);       
        assertEquals(6, pieces.size());
        
        SettingModel cards = (SettingModel) games.getMeta(1);       
        assertEquals(2, cards.size());
        SettingModel suits = (SettingModel) cards.getMeta(0);       
        assertEquals(4, suits.size());
        SettingModel card = (SettingModel) cards.getMeta(1);       
        assertEquals(13, card.size());
    }
}
