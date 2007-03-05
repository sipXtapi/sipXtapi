/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.springframework.beans.factory.config.ConfigurableListableBeanFactory;
import org.springframework.beans.factory.config.PropertyResourceConfigurer;
import org.springframework.core.io.ClassPathResource;

public class SkinProperties extends PropertyResourceConfigurer {   

    @Override
    protected void processProperties(ConfigurableListableBeanFactory beanFactory, Properties props) {
        SkinControl skin = (SkinControl) beanFactory.getBean("skin");
        Map<String, String> assets = new HashMap(props.size());
        for (Object key : props.keySet()) {
            assets.put((String) key, (String) props.get(key));
        }
        skin.setAssets(assets);
    }
    
    /**
     * convience method to set location w/o ClassPathResource in bean file.
     * @param path
     */
    public void setLocationPath(String path) {
        ClassPathResource location = new ClassPathResource(path);
        setLocation(location);
    }
}
