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
    
    public void testSettingPropertySetters() throws IOException {
        XmlModelBuilder builder = new XmlModelBuilder();
        InputStream in = getClass().getResourceAsStream("simplemodel.xml");
        SettingGroup root = builder.buildModel(in);
        Setting group = root.getSetting("group");
        assertEquals("Group Profile Name", group.getProfileName());
        assertEquals("Group Label", group.getLabel());
        assertEquals("Group Description", group.getDescription());
        
        Setting setting = group.getSetting("setting");
        assertEquals("Setting Profile Name", setting.getProfileName());
        assertEquals("Setting Label", setting.getLabel());
        assertEquals("Setting Description", setting.getDescription());
    }

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
    
    public void testInheritance() throws IOException {
        XmlModelBuilder builder = new XmlModelBuilder();
        InputStream in = getClass().getResourceAsStream("genders.xml");
        SettingGroup root = builder.buildModel(in);
        
        Setting human = root.getSetting("human");
        assertNotNull(human.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertNull(human.getSetting("giveBirth"));
        
        Setting man = root.getSetting("man");
        assertNotNull(man.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertEquals("face", man.getSetting("shave").getValue());
        assertNull(man.getSetting("giveBirth"));

        Setting woman = root.getSetting("woman");        
        assertNotNull(woman.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertEquals("legs", woman.getSetting("shave").getValue());
        assertNotNull(woman.getSetting("giveBirth"));
        
        // test for true clones
        man.getSetting("shave").setValue("back");
        assertEquals("back", man.getSetting("shave").getValue());        
        assertNotSame("back", woman.getSetting("shave").getValue());        
    }
    
    
}
