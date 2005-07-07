/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags;

import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;

import javax.ejb.FinderException;
import javax.naming.NamingException;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceHome;

public class AssignedDeviceTag extends ExTagSupport {
    private String m_userid = null;

    public void setUserid(String userid) {
        m_userid = userid;
    }

    public int doStartTag() throws JspException {
        try {
            DeviceHome dHome = (DeviceHome) EJBHomeFactory.getInstance().getHomeInterface(
                    DeviceHome.class, "Device");

            Collection dCollection = dHome.findByUserID(m_userid);
            Iterator dIterator = dCollection.iterator();
            StringBuffer m_sBuffer = new StringBuffer();

            Map map1 = new TreeMap();
            while (dIterator.hasNext()) {
                Device m_dProp = (Device) dIterator.next();
                map1.put(m_dProp.getShortName(), m_dProp.getID());
            }

            Iterator mapIterator = map1.entrySet().iterator();
            while (mapIterator.hasNext()) {
                Map.Entry me = (Map.Entry) mapIterator.next();
                m_sBuffer.append("<option value=\"" + me.getValue() + "\">" + me.getKey() + "\n");
            }

            pageContext.getOut().println("<select name=\"deviceid\" size=\"");
            pageContext.getOut().println(dCollection.size() < 20 ? dCollection.size() + 1 : 20);
            pageContext.getOut().println("\">");
            pageContext.getOut().println("<option>Select Device to Unassign");
            pageContext.getOut().println(m_sBuffer);
            pageContext.getOut().println("</select>");
        } catch (NamingException e) {
            throw new JspTagException(e);
        } catch (IOException e) {
            throw new JspTagException(e);
        } catch (FinderException e) {
            throw new JspTagException(e);
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_userid = null;

        super.clearProperties();
    }
}
