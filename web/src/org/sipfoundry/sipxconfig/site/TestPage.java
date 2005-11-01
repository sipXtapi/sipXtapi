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
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminder;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

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
    public static final String TEST_USER_ALIASES = TEST_USER_ALIAS1 + ", " + TEST_USER_ALIAS2;
    public static final String TEST_USER_PIN = "1234";

    public abstract DialPlanContext getDialPlanContext();

    public abstract GatewayContext getGatewayContext();

    public abstract PhoneContext getPhoneContext();

    public abstract CallGroupContext getCallGroupContext();

    public abstract ParkOrbitContext getParkOrbitContext();

    public abstract CoreContext getCoreContext();

    public abstract SipxReplicationContext getSipxReplicationContext();

    public abstract JobContext getJobContext();

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

    public void resetCoreContext(IRequestCycle cycle) {
        // need to reset all data that could potentially have a reference
        // to users
        resetDialPlans(cycle);
        resetPhoneContext(cycle);
        resetCallGroupContext(cycle);
        getCoreContext().clear();
        Visit visit = (Visit) getVisit();
        visit.logout(cycle);
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
        Visit visit = (Visit) getVisit();
        visit.setNavigationVisible(!visit.isNavigationVisible());
    }

    public void hideNavigation(IRequestCycle cycle_) {
        Visit visit = (Visit) getVisit();
        visit.setNavigationVisible(false);
    }

    public void toggleAdmin(IRequestCycle cycle) {
        Visit visit = (Visit) getVisit();
        boolean admin = !visit.isAdmin();
        Integer userId = visit.getUserId();
        if (userId == null) {
            login(cycle);
        } else {
            visit.login(userId, admin);
        }
    }

    public void seedTestUser(IRequestCycle cycle_) {
        // Create a test user that doesn't already exist
        String userName = getUnusedTestUserName();
        String firstName = TEST_USER_FIRSTNAME;
        User user = new User();
        user.setUserName(userName);
        user.setFirstName(firstName);
        user.setLastName(TEST_USER_LASTNAME);
        user.setAliasesString(userName.equals(TEST_USER_USERNAME) ? TEST_USER_ALIASES
                : EMPTY_STRING);
        user.setPin(TEST_USER_PIN, getCoreContext().getAuthorizationRealm());
        getCoreContext().saveUser(user);
    }

    public void loginFirstTestUser(IRequestCycle cycle_) {
        // Find the first test user
        User user = getCoreContext().loadUserByUserName(TEST_USER_USERNAME);
        if (user == null) {
            throw new IllegalStateException("Test user with username = " + TEST_USER_USERNAME
                    + " is not in the database");
        }

        // Log it in
        Visit visit = (Visit) getVisit();
        visit.login(user.getId(), false);
    }

    public void deleteAllUsers(IRequestCycle cycle_) {
        List users = getCoreContext().loadUsers();
        for (Iterator iter = users.iterator(); iter.hasNext();) {
            User user = (User) iter.next();
            getCoreContext().deleteUser(user);
        }
    }

    public String getUnusedTestUserName() {
        String userName = null;
        int count = 0;
        while (true) {
            userName = TEST_USER_USERNAME + getUsernameSuffix(count);
            User user = getCoreContext().loadUserByUserName(userName);
            count++;
            if (user == null) {
                break; // found an unused username
            }
        }
        return userName;
    }

    // Construct a username by appending a numeric suffix to the standard
    // test username. Special case: when the count is 0, append the empty
    // string rather than "0".
    private String getUsernameSuffix(int i) {
        return i != 0 ? Integer.toString(i) : EMPTY_STRING;
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
        CoreContext core = getCoreContext();
        List users = core.loadUsers();
        if (users.isEmpty()) {
            seedTestUser(cycle);
            users = core.loadUsers();
        }
        User user = (User) users.get(0);
        Visit visit = (Visit) getVisit();
        visit.login(user.getId(), true);
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
}
