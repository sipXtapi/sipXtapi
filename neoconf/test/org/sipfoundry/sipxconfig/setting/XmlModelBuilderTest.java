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
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;


public class XmlModelBuilderTest extends TestCase {

    public void testReadingGames() throws IOException {        
        XmlModelBuilder builder = new XmlModelBuilder();
        InputStream in = getClass().getResourceAsStream("games.xml");
        SettingGroup games = builder.buildModel(in);
        assertNull(games.getName());
        assertEquals(2, games.getValues().size());

        SettingGroup chess = (SettingGroup) games.getSetting("chess");
        assertEquals(chess.getName(), "chess");
        assertEquals("The game of chess", chess.getLabel());
        assertEquals(2, chess.getValues().size());
        Iterator orderPreserved = chess.getValues().iterator(); 
        SettingGroup colors = (SettingGroup) orderPreserved.next();       
        assertEquals(2, colors.getValues().size());
        SettingGroup pieces = (SettingGroup) orderPreserved.next();       
        assertEquals(6, pieces.getValues().size());
        
        Setting pawn = pieces.getSetting("pawn");
        List moves = pawn.getPossibleValues();
        assertNotNull(moves);
        assertEquals(3, moves.size());
        assertTrue(moves.contains("diagonal one to take another piece"));
        
        SettingGroup cards = (SettingGroup) games.getSetting("cards");       
        assertEquals(2, cards.getValues().size());
        
        SettingGroup suits = (SettingGroup) cards.getSetting("suit");       
        assertEquals(4, suits.getValues().size());
        SettingGroup card = (SettingGroup) cards.getSetting("card");       
        assertEquals(13, card.getValues().size());
    }
    
    /**
     * marginal value, testing a bug...
     */
    public void testIteration() throws IOException {
        XmlModelBuilder builder = new XmlModelBuilder();
        InputStream in = getClass().getResourceAsStream("games.xml");
        SettingGroup games = builder.buildModel(in);
        
        Iterator i = games.getValues().iterator();
        while (i.hasNext()) {
            assertTrue(i.next().getClass().isAssignableFrom(SettingGroup.class));
        }       
    }
}
