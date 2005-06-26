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
package org.sipfoundry.sipxconfig.gateway;

import java.io.File;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.setting.Setting;


public interface GatewayContext {    
    public static final String CONTEXT_BEAN_NAME = "gatewayContext";

    public abstract List getGateways();
    
    public abstract List getGatewayByIds(Collection gatewayIds);

    public abstract Gateway getGateway(Integer id);

    public abstract void storeGateway(Gateway gateway);

    public abstract void clear();

    public abstract boolean deleteGateway(Integer id);

    public abstract void deleteGateways(Collection selectedRows);
    
    /**
     * Returns the list of gateways available for a specific rule
     * 
     * @param ruleId id of the rule for which gateways are checked
     * @return collection of available gateways
     */
    public abstract Collection getAvailableGateways(Integer ruleId);
    
    public abstract Gateway newGateway(String factoryId);
    
    public Map getFactoryIds();
    
    // TODO: this should be moved to more generic class (phones use this too)
    public Setting loadModelFile(String manufacturer, String basename);
    
    public File getModelFile(String manufacturer, String basename);    
}
