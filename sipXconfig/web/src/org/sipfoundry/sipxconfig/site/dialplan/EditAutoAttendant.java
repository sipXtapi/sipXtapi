/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Iterator;
import java.util.Map;

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.VxmlGenerator;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditAutoAttendant extends BasePage implements PageRenderListener {

    public static final String PAGE = "EditAutoAttendant";

    public abstract AutoAttendant getAttendant();

    public abstract void setAttendant(AutoAttendant attendant);

    public abstract VxmlGenerator getVxmlGenerator();

    public abstract SelectMap getSelections();

    public abstract DialPlanContext getDialPlanContext();

    public abstract DialPad getAddMenuItemDialPad();

    public abstract void setAddMenuItemDialPad(DialPad dialPad);

    public abstract AttendantMenuAction getAddMenuItemAction();

    public abstract void setAddMenuItemAction(AttendantMenuAction action);

    public void removeMenuItems(IRequestCycle cycle_) {
        Iterator selected = getSelections().getAllSelected().iterator();
        Map menuItems = getAttendant().getMenuItems();
        while (selected.hasNext()) {
            String name = (String) selected.next();
            menuItems.remove(DialPad.getByName(name));
        }
    }

    public void reset(IRequestCycle cycle_) {
        getAttendant().resetToFactoryDefault();
    }

    public void apply(IRequestCycle cycle_) {
        save();
    }

    public void ok(IRequestCycle cycle) {
        if (save()) {
            returnManageAttendants(cycle);
        }
    }

    private boolean save() {
        boolean saved = false;        
        IValidationDelegate validator = TapestryUtils.getValidator(this);        
        AbstractComponent component = (AbstractComponent) getComponent("common");
        StringSizeValidator descriptionValidator = (StringSizeValidator) component.getBeans()
                .getBean("descriptionValidator");
        descriptionValidator.validate(validator);
        if (!validator.getHasErrors()) {
            getDialPlanContext().storeAutoAttendant(getAttendant());
            getVxmlGenerator().generate(getAttendant());
            saved = true;
        }

        return saved;
    }

    public void cancel(IRequestCycle cycle) {
        returnManageAttendants(cycle);
    }

    private void returnManageAttendants(IRequestCycle cycle) {
        cycle.activate(ManageAttendants.PAGE);
        setAttendant(null);
    }

    public void addMenuItem(IRequestCycle cycle_) {
        if (getAddMenuItemAction() == null) {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record("You must select an action for your new attendant menu item",
                    ValidationConstraint.REQUIRED);
        } else {
            AttendantMenuItem menuItem = new AttendantMenuItem(getAddMenuItemAction());
            getAttendant().addMenuItem(getAddMenuItemDialPad(), menuItem);
            selectNextAvailableDialpadKey();
            setAddMenuItemAction(null);
        }
    }

    /**
     * Try to select the next likely dial pad key
     */
    private void selectNextAvailableDialpadKey() {
        // set last desparate attempt
        setAddMenuItemDialPad(DialPad.POUND);

        Map menuItems = getAttendant().getMenuItems();
        for (int i = 0; i < DialPad.KEYS.length; i++) {
            DialPad key = DialPad.KEYS[i];
            // probably not pound
            if (!menuItems.containsKey(key) && key != DialPad.POUND) {
                setAddMenuItemDialPad(DialPad.KEYS[i]);
                break;
            }
        }
    }

    public void pageBeginRender(PageEvent event_) {
        AutoAttendant aa = getAttendant();
        if (aa == null) {
            // add new attendant
            initializeAttendant();
        }
        selectNextAvailableDialpadKey();
    }

    private void initializeAttendant() {
        AutoAttendant aa = new AutoAttendant();
        aa.resetToFactoryDefault();
        setAttendant(aa);
    }
}
