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

import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.Snapshot;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class SnapshotPage extends BasePage {

    public abstract void setSnapshotFile(File file);

    public abstract Snapshot getSnapshot();

    public void createSnapshot() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        File file = getSnapshot().perform();
        setSnapshotFile(file);
    }
}
