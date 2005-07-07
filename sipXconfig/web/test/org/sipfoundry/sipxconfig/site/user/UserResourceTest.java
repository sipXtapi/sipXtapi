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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.resource.ResourceContext;
import org.sipfoundry.sipxconfig.resource.ResourceTestHelper;
import org.sipfoundry.sipxconfig.resource.ResourceTranslator;

import junit.framework.TestCase;

public class UserResourceTest extends TestCase {
    
    private User user = new User();
    
    private UserResource resource = new UserResource();
    
    private ResourceContext context = new ResourceContext();
    
    protected void setUp() {
        context.setUri("/user/towhee");        
    }

    public void testDeleteResource() {
        MockControl translatorControl = MockControl.createStrictControl(ResourceTranslator.class);
        ResourceTranslator translator = (ResourceTranslator) translatorControl.getMock();
        translatorControl.replay();

        MockControl coreControl = MockControl.createStrictControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreControl.getMock();
        coreControl.expectAndReturn(coreContext.loadUserByDisplayId("towhee"), user);
        coreContext.deleteUser(user);
        coreControl.replay();
        
        context.setTranslator(translator);      
        resource.setCoreContext(coreContext);
        assertNull(resource.deleteResource(context, ResourceTestHelper.ACCEPT_STREAM));
                
        translatorControl.verify();
        coreControl.verify();        
    }

    public void testPostResource() {
        ResourceTestHelper.testPostNotAllowed(resource);
    }

    public void testPutResource() {
        MockControl translatorControl = MockControl.createStrictControl(ResourceTranslator.class);
        ResourceTranslator translator = (ResourceTranslator) translatorControl.getMock();
        translator.merge(ResourceTestHelper.ACCEPT_STREAM, user);
        translatorControl.replay();

        MockControl coreControl = MockControl.createStrictControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreControl.getMock();
        coreControl.expectAndReturn(coreContext.loadUserByDisplayId("towhee"), user);
        coreContext.saveUser(user);
        coreControl.replay();
        
        context.setTranslator(translator);        
        resource.setCoreContext(coreContext);
        assertNull(resource.putResource(context, ResourceTestHelper.ACCEPT_STREAM));
                
        translatorControl.verify();
        coreControl.verify();        
    }

    public void testGetResource() {
        MockControl translatorControl = MockControl.createStrictControl(ResourceTranslator.class);
        ResourceTranslator translator = (ResourceTranslator) translatorControl.getMock();
        translatorControl.expectAndReturn(translator.write(user), ResourceTestHelper.RETURN_STREAM);
        translatorControl.replay();

        MockControl coreControl = MockControl.createStrictControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreControl.getMock();
        coreControl.expectAndReturn(coreContext.loadUserByDisplayId("towhee"), user);
        coreControl.replay();
        
        context.setTranslator(translator);        
        resource.setCoreContext(coreContext);
        assertSame(ResourceTestHelper.RETURN_STREAM, resource.getResource(context, ResourceTestHelper.ACCEPT_STREAM));
                
        translatorControl.verify();
        coreControl.verify();
    }

    public void testOnResource() {
        ResourceTestHelper.testOnResourceNotAllowed(resource);
    }
}
