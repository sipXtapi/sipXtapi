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
package org.sipfoundry.sipxconfig.phone;

import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.job.JobContext;

public class RestartManagerImpl implements RestartManager {
    private static final Log LOG = LogFactory.getLog(RestartManagerImpl.class);

    private JobContext m_jobContext;

    private PhoneContext m_phoneContext;

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void restart(Collection phoneIds) {
        for (Iterator iter = phoneIds.iterator(); iter.hasNext();) {
            Integer id = (Integer) iter.next();
            restart(id);
        }
    }

    public void restart(Integer phoneId) {
        Phone phone = m_phoneContext.loadPhone(phoneId);
        restart(phone);
    }

    private void restart(Phone phone) {
        Serializable jobId = m_jobContext.schedule("Restarting phone " + phone.getSerialNumber());
        try {
            m_jobContext.start(jobId);
            phone.restart();
            m_jobContext.success(jobId);
        } catch (RestartException e) {
            m_jobContext.failure(jobId, null, e);
        } catch (RuntimeException e) {
            m_jobContext.failure(jobId, null, e);
            // do not throw error, job queue will stop running.
            // error gets logged to job error table and sipxconfig.log
            LOG.error(e);
        }
    }
}
