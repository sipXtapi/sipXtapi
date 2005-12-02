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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;

/**
 * SelectRuleType
 */
public abstract class SelectRuleType extends BasePage {
    public static final String PAGE = "SelectRuleType";
    
    public static final Map TYPE_2_PAGE = new HashMap();
    public static final DialingRuleType[] TYPES;
    

    static {
        TYPE_2_PAGE.put(DialingRuleType.CUSTOM, "EditCustomDialRule");
        TYPE_2_PAGE.put(DialingRuleType.INTERNAL, "EditInternalDialRule");
        TYPE_2_PAGE.put(DialingRuleType.LOCAL, "EditLocalDialRule");
        TYPE_2_PAGE.put(DialingRuleType.LONG_DISTANCE, "EditLongDistanceDialRule");
        TYPE_2_PAGE.put(DialingRuleType.EMERGENCY, "EditEmergencyDialRule");
        TYPE_2_PAGE.put(DialingRuleType.INTERNATIONAL, "EditInternationalDialRule");
        
        TYPES = (DialingRuleType[]) TYPE_2_PAGE.keySet().toArray(new DialingRuleType[TYPE_2_PAGE.size()]);
    }

    
    public abstract DialingRuleType getRuleType();

    public abstract void setRuleType(DialingRuleType type);

    public void next(IRequestCycle cycle) {
        DialingRuleType ruleType = getRuleType();
        activatePage(cycle, ruleType, null);
    }
    
    public static void activateEditPage(DialingRule rule, IRequestCycle cycle) {
        DialingRuleType ruleType = rule.getType();
        Integer ruleId = rule.getId();
        activatePage(cycle, ruleType, ruleId);
    }

    /**
     * Activate add/edit page for the rule
     * 
     * @param cycle - current request cycle
     * @param ruleType - type of the rule (used to determine the type of the page
     * @param ruleId - id of the rule, can be null if new rule is to be created
     */
    private static void activatePage(IRequestCycle cycle, DialingRuleType ruleType, Integer ruleId) {
        String pageName = (String) TYPE_2_PAGE.get(ruleType);
        EditDialRule page = (EditDialRule) cycle.getPage(pageName);
        page.setRuleId(ruleId);
        page.setRuleType(ruleType);
        page.setRule(null);        
        cycle.activate(page);
    }
    
}
