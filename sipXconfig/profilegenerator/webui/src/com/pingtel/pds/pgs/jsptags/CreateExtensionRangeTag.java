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

import java.rmi.RemoteException;

import javax.ejb.CreateException;
import javax.naming.NamingException;
import javax.servlet.jsp.JspException;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocate;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocateHome;

/**
 * CreateExtensionRangeTag is a tag hander to create an extension range in and existing
 * ExtensionPool
 */
public class CreateExtensionRangeTag extends ExTagSupport {
    private String mExtensionPoolID = null;
    private String mMinExtension = null;
    private String mMaxExtension = null;
    private ExtensionPoolAdvocate mExtensionAdvocateEJBObject = null;

    public void setExtensionpoolid(String extensionpoolid) {
        if (extensionpoolid != null) {
            mExtensionPoolID = extensionpoolid;
        }
    }

    public void setMinextension(String minextension) {
        mMinExtension = minextension;
    }

    public void setMaxextension(String maxextension) {
        mMaxExtension = maxextension;
    }

    public int doStartTag() throws JspException {

        try {
            if (mExtensionAdvocateEJBObject == null) {
                ExtensionPoolAdvocateHome uaHome = (ExtensionPoolAdvocateHome) EJBHomeFactory
                        .getInstance().getHomeInterface(ExtensionPoolAdvocateHome.class,
                                "ExtensionPoolAdvocate");

                mExtensionAdvocateEJBObject = uaHome.create();
            }

            mExtensionAdvocateEJBObject.allocateExtensionsToPool(mExtensionPoolID, mMinExtension,
                    mMaxExtension);
        } catch (CreateException e) {
            throw new JspException(e);
        } catch (RemoteException e) {
            throw new JspException(e);
        } catch (PDSException e) {
            throw new JspException(e);
        } catch (NamingException e) {
            throw new JspException(e);
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        mExtensionPoolID = null;
        mMinExtension = null;
        mMaxExtension = null;

        super.clearProperties();
    }
}