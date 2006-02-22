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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPattern;

/**
 * PatternsEditor - list of pattersn with ability to edit, remove and delete
 * 
 * FIXME: when there are errors we should not add or delete any patters
 * 
 * FIXME: for loop should take explicit key converter
 */
public abstract class PatternsEditor extends BaseComponent {
    public abstract boolean getAddPattern();

    public abstract void setAddPattern(boolean addPattern);

    public abstract int getIndexToRemove();

    public abstract void setIndexToRemove(int index);

    public abstract List getPatterns();

    public abstract int getIndex();

    public boolean isLast() {
        List patterns = getPatterns();
        return getIndex() == patterns.size() - 1;
    }

    /**
     * Process pattern adds/deletes.
     */
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        // reset components before rewind
        setIndexToRemove(-1);
        setAddPattern(false);
        super.renderComponent(writer, cycle);
        if (!cycle.isRewinding()) {
            // nothing else to do
            return;
        }
        List patterns = getPatterns();
        if (getAddPattern()) {
            patterns.add(new DialPattern());
        }
        int indexToRemove = getIndexToRemove();
        if (indexToRemove >= 0) {
            patterns.remove(indexToRemove);
        }
    }
}
