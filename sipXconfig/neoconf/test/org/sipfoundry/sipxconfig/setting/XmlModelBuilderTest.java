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
import java.util.Collection;
import java.util.Iterator;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;

public class XmlModelBuilderTest extends TestCase {
    private ModelBuilder m_builder;

    protected void setUp() throws Exception {
        m_builder = new XmlModelBuilder("etc");
    }

    public void testSettingPropertySetters() throws IOException {
        InputStream in = getClass().getResourceAsStream("simplemodel.xml");
        SettingSet root = m_builder.buildModel(in, null);
        Setting group = root.getSetting("group");
        assertEquals("Group Profile Name", group.getProfileName());
        assertEquals("Group Label", group.getLabel());
        assertEquals("Group Description", group.getDescription());

        Setting setting = group.getSetting("setting");
        assertEquals("Setting Profile Name", setting.getProfileName());
        assertEquals("Setting Label", setting.getLabel());
        assertEquals("Setting Description", setting.getDescription());
        assertSame(StringSetting.DEFAULT, setting.getType());
    }

    public void testReadingGames() throws IOException {
        InputStream in = getClass().getResourceAsStream("games.xml");
        SettingSet games = m_builder.buildModel(in, null);
        assertEquals("", games.getName());
        assertEquals(2, games.getValues().size());

        SettingSet chess = (SettingSet) games.getSetting("chess");
        assertEquals(chess.getName(), "chess");
        assertEquals("The game of chess", chess.getLabel());
        assertEquals(2, chess.getValues().size());
        Iterator orderPreserved = chess.getValues().iterator();
        SettingSet colors = (SettingSet) orderPreserved.next();
        assertEquals(2, colors.getValues().size());
        SettingSet pieces = (SettingSet) orderPreserved.next();
        assertEquals(6, pieces.getValues().size());

        Setting pawn = pieces.getSetting("pawn");
        SettingType type = pawn.getType();
        assertEquals("enum", type.getName());
        EnumSetting enumType = (EnumSetting) type;
        Collection moves = enumType.getEnums().keySet();
        assertNotNull(moves);
        assertEquals(3, moves.size());
        assertTrue(moves.contains("diagonal one to take another piece"));

        SettingSet cards = (SettingSet) games.getSetting("cards");
        assertEquals(2, cards.getValues().size());

        SettingSet suits = (SettingSet) cards.getSetting("suit");
        assertEquals(4, suits.getValues().size());
        SettingSet card = (SettingSet) cards.getSetting("card");
        assertEquals(13, card.getValues().size());
    }

    /**
     * marginal value, testing a bug...
     */
    public void testIteration() throws IOException {
        InputStream in = getClass().getResourceAsStream("games.xml");
        SettingSet games = m_builder.buildModel(in, null);

        Iterator i = games.getValues().iterator();
        while (i.hasNext()) {
            assertTrue(SettingSet.class.isAssignableFrom(i.next().getClass()));
        }
    }

    public void testInheritance() throws IOException {
        InputStream in = getClass().getResourceAsStream("genders.xml");
        SettingSet root = m_builder.buildModel(in, null);

        Setting human = root.getSetting("human");
        assertEquals("Human", human.getLabel());
        assertEquals("Earthlings", human.getDescription());
        assertNotNull(human.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertNull(human.getSetting("giveBirth"));

        Setting man = root.getSetting("man");
        assertEquals("Man", man.getLabel());
        assertEquals("Earthlings", man.getDescription());
        assertNotNull(man.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertEquals("face", man.getSetting("shave").getValue());
        assertNull(man.getSetting("giveBirth"));

        Setting woman = root.getSetting("woman");
        assertEquals("Woman", woman.getLabel());
        assertEquals("Earthlings", woman.getDescription());
        assertNotNull(woman.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertEquals("legs", woman.getSetting("shave").getValue());
        assertNotNull(woman.getSetting("giveBirth"));

        // test for true clones
        man.getSetting("shave").setValue("back");
        assertEquals("back", man.getSetting("shave").getValue());
        assertNotSame("back", woman.getSetting("shave").getValue());
    }

    public void testFlags() throws Exception {
        InputStream in = getClass().getResourceAsStream("genders.xml");
        SettingSet root = m_builder.buildModel(in, null);
        Setting reason = root.getSetting("man/reason");
        assertFalse(reason.isAdvanced());
        assertTrue(reason.isHidden());
        Setting giveBirth = root.getSetting("woman/giveBirth");
        assertTrue(giveBirth.isAdvanced());
        assertFalse(giveBirth.isHidden());
    }
    
    public void testNullValue() throws Exception {
        InputStream in = getClass().getResourceAsStream("simplemodel.xml");
        SettingSet root = m_builder.buildModel(in, null);
        assertNull(root.getSetting("group/setting").getValue());
    }
    
    public void testLoadModelFileWithDetails() throws Exception {
        InputStream inBase = getClass().getResourceAsStream("basename.xml");
        SettingSet base = m_builder.buildModel(inBase, null);
        IOUtils.closeQuietly(inBase);
        
        InputStream inModel = getClass().getResourceAsStream("basename_model.xml");
        SettingSet model = m_builder.buildModel(inModel, base);
        IOUtils.closeQuietly(inModel);
        
        assertNotNull(model.getSetting("car/color"));
        assertNotNull(model.getSetting("car/model"));

        InputStream inVersion = getClass().getResourceAsStream("basename_model_version.xml");
        SettingSet version = m_builder.buildModel(inVersion, model);
        IOUtils.closeQuietly(inVersion);

        assertNotNull(version.getSetting("car/color"));
        assertNotNull(version.getSetting("car/model"));
        assertNotNull(version.getSetting("car/doodad"));
        assertNull(model.getSetting("car/doodad"));
    }
}
