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

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.phone.PhoneModel;


public interface GatewayManager {    
    public static final String CONTEXT_BEAN_NAME = "gatewayContext";

    public abstract List getGateways();
    
    public abstract List getGatewayByIds(Collection gatewayIds);

    public abstract Gateway getGateway(Integer id);

    public abstract void storeGateway(Gateway gateway);

    public abstract void clear();

    public abstract boolean deleteGateway(Integer id);

    public abstract void deleteGateways(Collection selectedRows);
    
    public void propagateGateways(Collection selectedRows);
    
    /**
     * Returns the list of gateways available for a specific rule
     * 
     * @param ruleId id of the rule for which gateways are checked
     * @return collection of available gateways
     */
    public abstract Collection getAvailableGateways(Integer ruleId);
    
    public abstract Gateway newGateway(PhoneModel model);
    
    public List getAvailableGatewayModels();

    public abstract void propagateAllGateways();
}
