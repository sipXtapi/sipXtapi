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
package org.sipfoundry.sipxconfig.site.user;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.resource.ResourceContext;
import org.sipfoundry.sipxconfig.resource.ResourceTestHelper;
import org.sipfoundry.sipxconfig.resource.ResourceTranslator;

public class ManageUsersResourceTest extends TestCase {
    
    private List users;
    
    private ManageUsersResource resource = new ManageUsersResource();
    
    private ResourceContext context = new ResourceContext();

    public void setUp() {
        users = new ArrayList();
        User user = new User();
        user.setDisplayId("jsmith");
        users.add(user);

        context.setUri("/user");        
    }

    public void testPostResource() throws Exception {
        MockControl translatorControl = MockControl.createStrictControl(ResourceTranslator.class);
        ResourceTranslator translator = (ResourceTranslator) translatorControl.getMock();
        translatorControl.expectAndReturn(translator.read(ResourceTestHelper.ACCEPT_STREAM), users);
        translatorControl.replay();

        MockControl coreControl = MockControl.createStrictControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreControl.getMock();
        coreContext.saveUser((User) users.get(0));
        coreControl.replay();
        
        context.setTranslator(translator);        
        resource.setCoreContext(coreContext);
        assertNull(resource.postResource(context, ResourceTestHelper.ACCEPT_STREAM));
                
        translatorControl.verify();
        coreControl.verify();        
    }

    public void testGetResource() throws Exception {
        MockControl translatorControl = MockControl.createStrictControl(ResourceTranslator.class);
        ResourceTranslator translator = (ResourceTranslator) translatorControl.getMock();
        translatorControl.expectAndReturn(translator.write(users), ResourceTestHelper.RETURN_STREAM);
        translatorControl.replay();

        MockControl coreControl = MockControl.createNiceControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreControl.getMock();
        coreContext.loadUserByTemplateUser(new User());
        coreControl.setDefaultReturnValue(users);
        coreControl.replay();
        
        context.setTranslator(translator);
        resource.setCoreContext(coreContext);
        assertSame(ResourceTestHelper.RETURN_STREAM, resource.getResource(context, ResourceTestHelper.ACCEPT_STREAM));
                
        translatorControl.verify();
        coreControl.verify();        
    }
    
    public void testPutResource() {
        ResourceTestHelper.testPutNotAllowed(resource);
    }

    public void testDeleteResource() {
        ResourceTestHelper.testDeleteNotAllowed(resource);
    }

    public void testOnResource() {
        ResourceTestHelper.testOnResourceNotAllowed(resource);
    }    
}
