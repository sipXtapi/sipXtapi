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

public interface GatewayContext {
    public static final String CONTEXT_BEAN_NAME = "gatewayContext";

    List getGateways();

    Collection getAllGatewayIds();

    List getGatewayByIds(Collection gatewayIds);

    Gateway getGateway(Integer id);

    void storeGateway(Gateway gateway);

    void clear();

    boolean deleteGateway(Integer id);

    void deleteGateways(Collection selectedRows);

    /**
     * Returns the list of gateways available for a specific rule
     * 
     * @param ruleId id of the rule for which gateways are checked
     * @return collection of available gateways
     */
    Collection getAvailableGateways(Integer ruleId);

    Gateway newGateway(PhoneModel model);

    List getAvailableGatewayModels();
}
