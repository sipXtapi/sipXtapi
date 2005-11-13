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
package org.sipfoundry.sipxconfig.site.search;

import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRule;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.admin.dialplan.InternationalRule;
import org.sipfoundry.sipxconfig.admin.dialplan.LocalRule;
import org.sipfoundry.sipxconfig.admin.dialplan.LongDistanceRule;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.admin.EditCallGroup;
import org.sipfoundry.sipxconfig.site.admin.EditParkOrbit;
import org.sipfoundry.sipxconfig.site.conference.EditBridge;
import org.sipfoundry.sipxconfig.site.conference.EditConference;
import org.sipfoundry.sipxconfig.site.dialplan.EditAutoAttendant;
import org.sipfoundry.sipxconfig.site.gateway.EditGateway;
import org.sipfoundry.sipxconfig.site.phone.EditPhone;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.site.user.EditUser;

/**
 * This is a class in charge of determining which "edit" page should be used for an entity object
 * (bean). It does not seem to be any elegant way of solving this problem - we have object type
 * and id and we need to find an edit page for it.
 * 
 */
public class EnumEditPageProvider implements EditPageProvider {
    public static final Log LOG = LogFactory.getLog(EnumEditPageProvider.class);

    public static final String RULE_ID = "ruleId";

    public static final Object[] PAGES = {
        User.class, new String[] {
            EditUser.PAGE, "userId"
        }, Group.class, new String[] {
            EditGroup.PAGE, "groupId"
        }, Phone.class, new String[] {
            EditPhone.PAGE, "phoneId"
        }, Gateway.class, new String[] {
            EditGateway.PAGE, "gatewayId"
        }, CallGroup.class, new String[] {
            EditCallGroup.PAGE, "callGroupId"
        }, Bridge.class, new String[] {
            EditBridge.PAGE, "bridgeId"
        }, Conference.class, new String[] {
            EditConference.PAGE, "conferenceId"
        }, ParkOrbit.class, new String[] {
            EditParkOrbit.PAGE, "parkOrbitId"
        }, AutoAttendant.class, new String[] {
            EditAutoAttendant.PAGE, "autoAttendantId"
        }, InternalRule.class, new String[] {
            "EditInternalDialRule", RULE_ID
        }, CustomDialingRule.class, new String[] {
            "EditCustomDialRule", RULE_ID
        }, LocalRule.class, new String[] {
            "EditLocalDialRule", RULE_ID
        }, LongDistanceRule.class, new String[] {
            "EditLongDistanceDialRule", RULE_ID
        }, EmergencyRule.class, new String[] {
            "EditEmergencyDialRule", RULE_ID
        }, InternationalRule.class, new String[] {
            "EditInternationalDialRule", RULE_ID
        }
    };

    private Map m_classToPageInfo;

    public EnumEditPageProvider() {
        m_classToPageInfo = new HashMap(PAGES.length / 2);
        for (int i = 0; i < PAGES.length; i = i + 2) {
            Class klass = (Class) PAGES[i];
            m_classToPageInfo.put(klass.getName(), PAGES[i + 1]);
        }
    }

    public IPage getPage(IRequestCycle cycle, String klass, Object id) {
        try {
            String[] pageInfo = (String[]) m_classToPageInfo.get(klass);
            IPage page = cycle.getPage(pageInfo[0]);
            BeanUtils.setProperty(page, pageInfo[1], id);
            return page;
        } catch (IllegalAccessException e) {
            LOG.error(e);
            return null;
        } catch (InvocationTargetException e) {
            LOG.error(e);
            return null;
        }
    }
}
