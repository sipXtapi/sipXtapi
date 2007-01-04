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
package org.sipfoundry.sipxconfig.site.speeddial;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.speeddial.Button;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ButtonEditor extends BaseComponent {
    @Parameter(required = true)
    public abstract Button getButton();

    @Parameter(required = true)
    public abstract int getIndex();

    @Parameter(required = true)
    public abstract int getButtonsSize();

    @Parameter(required = true)
    public abstract int getRemoveIndex();

    @Parameter(required = true)
    public abstract int getMoveIndex();

    public abstract void setMoveIndex(int index);

    @Parameter(required = true)
    public abstract int getMoveOffset();

    public abstract void setMoveOffset(int offset);

    public void up() {
        move(-1);
    }

    public void down() {
        move(1);
    }

    private void move(int offset) {
        setMoveIndex(getIndex());
        setMoveOffset(offset);
    }

    public boolean isRenderUp() {
        return getIndex() > 0;
    }

    public boolean isRenderDown() {
        return getIndex() + 1 < getButtonsSize();
    }
}
