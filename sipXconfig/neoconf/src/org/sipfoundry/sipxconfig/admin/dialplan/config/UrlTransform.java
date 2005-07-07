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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.dom4j.Element;

/**
 * UrilTransfomr
 */
public class UrlTransform extends Transform {
    private String m_url;

    public String getUrl() {
        return m_url;
    }

    public void setUrl(String url) {
        m_url = url;
    }

    protected void addChildren(Element transform) {
        if (null != m_url) {
            Element url = transform.addElement("url");
            url.setText(m_url);
        }
    }
}
