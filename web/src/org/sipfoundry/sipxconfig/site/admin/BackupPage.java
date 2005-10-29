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
package org.sipfoundry.sipxconfig.site.admin;

import java.io.File;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.AdminManager;
import org.sipfoundry.sipxconfig.admin.BackupPlan;
import org.sipfoundry.sipxconfig.admin.DailyBackupSchedule;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class BackupPage extends BasePage implements PageRenderListener {

    /**
     * Conceivable, available backup limits. Otherwise arbitrary.
     * NOTE : Spring 1.1 couldn't define Integers in lists see
     * DefaultXmlBeanDefinitionParser.java:parsePropertySubelement() 
     */
    public static final List BACKUP_LIMIT_MODEL = Arrays.asList(new Integer[] {
        new Integer(1),
        new Integer(2),
        new Integer(3),
        new Integer(4),
        new Integer(5),
        new Integer(10),
        new Integer(20),
        new Integer(30),
        new Integer(40),
        new Integer(50)
    });

    public abstract AdminManager getAdminContext();

    public abstract List getBackupFiles();

    public abstract void setBackupFiles(List files);
    
    public abstract BackupPlan getBackupPlan();

    public abstract void setBackupPlan(BackupPlan plan);
    
    public void pageBeginRender(PageEvent event_) {
        List urls = getBackupFiles();
        if (urls == null) {
            setBackupFiles(Collections.EMPTY_LIST);
        }
        
        // every plan has at least 1 schedule, thought of having this somewhere in
        // library, but you could argue it's application specific.        
        BackupPlan plan = getBackupPlan();
        if (plan == null) {
            plan = getAdminContext().getBackupPlan();
            if (plan.getSchedules().isEmpty()) {
                DailyBackupSchedule schedule = new DailyBackupSchedule(); 
                plan.addSchedule(schedule);
            }
            setBackupPlan(plan);
        }
    }

    public void submit(IRequestCycle cycle_) {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
    }

    public void backup(IRequestCycle cycle_) {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        AdminManager adminContext = getAdminContext();
        BackupPlan plan = getBackupPlan();
        File[] backupFiles = adminContext.performBackup(plan);
        if (null != backupFiles) {
            setBackupFiles(Arrays.asList(backupFiles));
        } else {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record("Backup operation failed.", ValidationConstraint.CONSISTENCY);
        }
    }
    
    public void ok(IRequestCycle cycle_) {
        AdminManager adminContext = getAdminContext();
        BackupPlan plan = getBackupPlan();
        adminContext.storeBackupPlan(plan);
    }
}
