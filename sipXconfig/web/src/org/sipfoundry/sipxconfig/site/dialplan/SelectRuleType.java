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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.TapestryUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;

/**
 * SelectRuleType
 */
public abstract class SelectRuleType extends BaseComponent {
    public static final Map<DialingRuleType, String> TYPE_2_PAGE = new HashMap<DialingRuleType, String>();
    public static final DialingRuleType[] TYPES;

    static {
        TYPE_2_PAGE.put(DialingRuleType.CUSTOM, "EditCustomDialRule");
        TYPE_2_PAGE.put(DialingRuleType.INTERNAL, "EditInternalDialRule");
        TYPE_2_PAGE.put(DialingRuleType.ATTENDANT, "EditAttendantDialRule");
        TYPE_2_PAGE.put(DialingRuleType.LOCAL, "EditLocalDialRule");
        TYPE_2_PAGE.put(DialingRuleType.LONG_DISTANCE, "EditLongDistanceDialRule");
        TYPE_2_PAGE.put(DialingRuleType.EMERGENCY, "EditEmergencyDialRule");
        TYPE_2_PAGE.put(DialingRuleType.INTERNATIONAL, "EditInternationalDialRule");

        TYPES = TYPE_2_PAGE.keySet().toArray(new DialingRuleType[TYPE_2_PAGE.size()]);
    }

    public abstract DialingRuleType getRuleType();

    public abstract void setRuleType(DialingRuleType type);

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (cycle.isRewinding() && TapestryUtils.getForm(cycle, this).isRewinding()) {
            activateNewRulePage(cycle);
        }
    }

    public void activateNewRulePage(IRequestCycle cycle) {
        DialingRuleType ruleType = getRuleType();
        if (ruleType == null) {
            // nothing to do
            return;
        }
        IPage editDialRulePage = getEditDialRulePage(cycle, ruleType, null);
        cycle.activate(editDialRulePage);
    }

    public static IPage getEditDialRulePage(IRequestCycle cycle, DialingRuleType ruleType,
            Integer ruleId) {
        String pageName = TYPE_2_PAGE.get(ruleType);
        EditDialRule page = (EditDialRule) cycle.getPage(pageName);
        page.setRuleId(ruleId);
        page.setRuleType(ruleType);
        page.setRule(null);
        return page;
    }
}
