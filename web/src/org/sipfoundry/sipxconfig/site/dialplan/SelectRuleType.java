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
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule.Type;

/**
 * SelectRuleType
 */
public abstract class SelectRuleType extends BasePage {
    public static final String PAGE = "SelectRuleType";
    
    public static final Map TYPE_2_PAGE = new HashMap();
    public static final Type[] TYPES;
    

    static {
        TYPE_2_PAGE.put(Type.CUSTOM, EditCustomDialRule.PAGE);
        TYPE_2_PAGE.put(Type.INTERNAL, EditInternalDialRule.PAGE);
        
        TYPES = (Type[]) TYPE_2_PAGE.keySet().toArray(new Type[TYPE_2_PAGE.size()]);
    }

    
    public abstract DialingRule.Type getRuleType();

    public abstract void setRuleType(DialingRule.Type type);

    public void next(IRequestCycle cycle) {
        Type ruleType = getRuleType();
        activatePage(cycle, ruleType, null);
    }
    
    public static void activateEditPage(DialingRule rule, IRequestCycle cycle) {
        Type ruleType = rule.getType();
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
    private static void activatePage(IRequestCycle cycle, Type ruleType, Integer ruleId) {
        String pageName = (String) TYPE_2_PAGE.get(ruleType);
        EditDialRule page = (EditDialRule) cycle.getPage(pageName);
        page.setRuleId(ruleId);
        cycle.activate(page);
    }
}
