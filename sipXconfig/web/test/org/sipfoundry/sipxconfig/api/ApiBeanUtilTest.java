/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

public class ApiBeanUtilTest extends TestCase {    
    private Set frackProps;
    
    protected void setUp() {
        frackProps = new HashSet();
    }
    
    public void testCopyProperties() {
        Frick frick = new Frick("yaya", "123 hello st.");
        Frack frack = new Frack();
        frackProps.add("name");
        ApiBeanUtil.copyProperties(frack, frick, frackProps, null);
        assertEquals("yaya", frack.getName());
    }
    
    public void testReadableProperties() {
        Object[] props = ApiBeanUtil.getReadableProperties(new Frick()).toArray();
        Arrays.sort(props);
        assertEquals(2, props.length);        
        assertEquals("address", props[0]);
        assertEquals("name", props[1]);
    }
    
    public void testToApiArray() {
        Frick[] fricks = new Frick[] {
                new Frick("a", "b"),
                new Frick("c", "d")
        };
        SimpleBeanBuilder builder = new SimpleBeanBuilder();
        builder.getIgnoreList().add("abode");
        Frack[] fracks = (Frack[]) ApiBeanUtil.toApiArray(builder, fricks, Frack.class);
        assertEquals("a", fracks[0].getName());
        assertEquals(null, fracks[0].getAbode());
        assertEquals("c", fracks[1].getName());
        assertEquals(null, fracks[1].getAbode());
    }
}
