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
package org.sipfoundry.sipxconfig.site;

import java.io.Serializable;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.search.IndexManager;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminder;
import org.sipfoundry.sipxconfig.site.search.EnumEditPageProvider;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.site.upload.EditUpload;
import org.sipfoundry.sipxconfig.upload.UploadManager;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

/**
 * TestPage page
 */
public abstract class TestPage extends BasePage {

    public static final String PAGE = "TestPage";

    public static final int JOBS = 4;
    public static final String EMPTY_STRING = "";
    public static final int SERIAL_NUM_LEN = 12;

    // Data for the primary test user
    // Make sure the username matches SiteTestHelper.java
    public static final String TEST_USER_USERNAME = "testuser";
    public static final String TEST_USER_FIRSTNAME = "Test";
    public static final String TEST_USER_LASTNAME = "User";
    public static final String TEST_USER_ALIAS1 = "testy";
    public static final String TEST_USER_ALIAS2 = "200";
    public static final String TEST_USER_ALIASES = TEST_USER_ALIAS1 + " " + TEST_USER_ALIAS2;
    public static final String TEST_USER_PIN = "1234";
    public static final int MANY_USERS = 10000;

    public abstract DialPlanContext getDialPlanContext();

    public abstract GatewayContext getGatewayContext();

    public abstract PhoneContext getPhoneContext();

    public abstract CallGroupContext getCallGroupContext();

    public abstract ParkOrbitContext getParkOrbitContext();

    public abstract UploadManager getUploadManager();
    
    public abstract CoreContext getCoreContext();

    public abstract SipxReplicationContext getSipxReplicationContext();

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract JobContext getJobContext();

    public abstract IndexManager getIndexManager();
    
    public abstract UserSession getUserSession();
    
    public abstract IEngineService getRestartService();

    public void resetDialPlans(IRequestCycle cycle_) {
        getDialPlanContext().clear();
        getGatewayContext().clear();
    }

    public void resetPhoneContext(IRequestCycle cycle_) {
        getPhoneContext().clear();
    }

    public void resetCallGroupContext(IRequestCycle cycle_) {
        getCallGroupContext().clear();
    }

    public void resetParkOrbitContext(IRequestCycle cycle_) {
        getParkOrbitContext().clear();
    }

    public void resetConferenceBridgeContext(IRequestCycle cycle_) {
        getConferenceBridgeContext().clear();
    }

    public void resetCoreContext(IRequestCycle cycle) {
        // need to reset all data that could potentially have a reference
        // to users
        resetDialPlans(cycle);
        resetPhoneContext(cycle);
        resetCallGroupContext(cycle);
        getCoreContext().clear();
        getUserSession().logout(getRestartService(), cycle);
    }

    public void newGroup(IRequestCycle cycle) {
        String resource = (String) TapestryUtils.assertParameter(String.class, cycle
                .getServiceParameters(), 0);
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.newGroup(resource, PAGE);
        cycle.activate(page);
    }

    public void goToRestartReminderPage(IRequestCycle cycle) {
        RestartReminder page = (RestartReminder) cycle.getPage(RestartReminder.PAGE);
        page.setNextPage(PAGE);
        cycle.activate(page);
    }

    public void toggleNavigation(IRequestCycle cycle_) {
        UserSession userSession = getUserSession();
        userSession.setNavigationVisible(!userSession.isNavigationVisible());
    }

    public void hideNavigation(IRequestCycle cycle_) {
        getUserSession().setNavigationVisible(false);
    }

    public void toggleAdmin(IRequestCycle cycle) {
        UserSession userSession = getUserSession();
        boolean admin = !userSession.isAdmin();
        Integer userId = userSession.getUserId();
        if (userId == null) {
            login(cycle);
        } else {
            userSession.login(userId, admin);
        }
    }

    public void seedTestUser(IRequestCycle cycle_) {
        createTestUserIfMissing();
    }
    
    private User createTestUserIfMissing() {
        String userName = TEST_USER_USERNAME;
        if (null != getCoreContext().loadUserByUserName(TEST_USER_USERNAME)) {
            // we already have test user - get a unique name for a new one
            userName = TEST_USER_USERNAME + System.currentTimeMillis();
        }
        String firstName = TEST_USER_FIRSTNAME;
        User user = new User();
        user.setUserName(userName);
        user.setFirstName(firstName);
        user.setLastName(TEST_USER_LASTNAME);
        user.setAliasesString(userName.equals(TEST_USER_USERNAME) ? TEST_USER_ALIASES
                : EMPTY_STRING);
        user.setPin(TEST_USER_PIN, getCoreContext().getAuthorizationRealm());
        getCoreContext().saveUser(user);
        return user;
    }

    public void populateUsers(IRequestCycle cycle_) {
        long l = System.currentTimeMillis();
        CoreContext coreContext = getCoreContext();
        String authorizationRealm = coreContext.getAuthorizationRealm();
        for (int i = 0; i < MANY_USERS; i++) {
            String firstName = TEST_USER_FIRSTNAME + i;
            User user = new User();
            user.setUserName("xuser" + (l + i));
            user.setFirstName(firstName);
            user.setLastName(TEST_USER_LASTNAME);
            user.setPin(TEST_USER_PIN, authorizationRealm);
            coreContext.saveUser(user);
        }
    }

    public void loginFirstTestUser(IRequestCycle cycle_) {
        // Find the first test user
        User user = getCoreContext().loadUserByUserName(TEST_USER_USERNAME);
        if (user == null) {
            throw new IllegalStateException("Test user with username = " + TEST_USER_USERNAME
                    + " is not in the database");
        }

        // Log it in
        UserSession userSession = getUserSession();
        userSession.login(user.getId(), false);
    }

    public void deleteAllUsers(IRequestCycle cycle_) {
        List users = getCoreContext().loadUsers();
        for (Iterator iter = users.iterator(); iter.hasNext();) {
            User user = (User) iter.next();
            getCoreContext().deleteUser(user);
        }
    }

    public void indexAll(IRequestCycle cycle_) {
        getIndexManager().indexAll();
    }

    public void populateJobs(IRequestCycle cycle_) {
        JobContext jobContext = getJobContext();
        jobContext.clear();
        Serializable[] jobIds = new Serializable[JOBS];
        for (int i = 0; i < jobIds.length; i++) {
            jobIds[i] = jobContext.schedule("test" + i);
            if (i > 0) {
                jobContext.start(jobIds[i]);
            }
        }
        jobContext.success(jobIds[2]);
        jobContext.failure(jobIds[JOBS - 1], "something bad happened", null);
    }

    public void populatePhones(IRequestCycle cycle_) {
        List availablePhoneModels = getPhoneContext().getAvailablePhoneModels();
        for (Iterator i = availablePhoneModels.iterator(); i.hasNext();) {
            PhoneModel model = (PhoneModel) i.next();
            Phone phone = getPhoneContext().newPhone(model);
            phone.setSerialNumber(RandomStringUtils.randomNumeric(SERIAL_NUM_LEN));
            getPhoneContext().storePhone(phone);
        }
    }

    public void login(IRequestCycle cycle) {
        User user = createTestUserIfMissing();
        getUserSession().login(user.getId(), true);
    }

    public void generateDataSet(IRequestCycle cycle) {
        String setName = (String) TapestryUtils.assertParameter(String.class, cycle
                .getServiceParameters(), 0);
        SipxReplicationContext sipxReplicationContext = getSipxReplicationContext();
        sipxReplicationContext.generate(DataSet.getEnum(setName));
    }

    public void throwException(IRequestCycle cycle_) {
        throw new IllegalArgumentException("Just testing");
    }

    public void validateEditPageProvider(IRequestCycle cycle) {
        EnumEditPageProvider provider = new EnumEditPageProvider();
        provider.validatePages(cycle);
    }
    
    public void newUpload(IRequestCycle cycle) {
        EditUpload page = (EditUpload) cycle.getPage(EditUpload.PAGE);
        page.setUploadId(null);
        page.setUploadSpecification(UploadSpecification.UNMANAGED);
        page.activatePageWithCallback(PAGE, cycle);
    }
    
    public void resetUploadManager(IRequestCycle cycle_) {
        getUploadManager().clear();
    }

}
