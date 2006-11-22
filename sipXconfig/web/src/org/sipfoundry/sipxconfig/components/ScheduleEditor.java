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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.sipfoundry.sipxconfig.admin.ScheduledDay;

public abstract class ScheduleEditor extends BaseComponent {

    public abstract IPropertySelectionModel getTypeModel();

    public abstract void setTypeModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getDayOfWeekModel();

    public abstract void setDayOfWeekModel(IPropertySelectionModel model);

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getTypeModel() == null) {
            NewEnumPropertySelectionModel typeModel = new NewEnumPropertySelectionModel();
            typeModel.setEnumType(CronSchedule.Type.class);
            
            LocalizedOptionModelDecorator decoratedModel = new LocalizedOptionModelDecorator();
            decoratedModel.setMessages(getMessages());
            decoratedModel.setModel(typeModel);
            decoratedModel.setResourcePrefix("type.");
            setTypeModel(decoratedModel);
        }
        if (getDayOfWeekModel() == null) {
            EnumPropertySelectionModel dayModel = new EnumPropertySelectionModel();
            dayModel.setOptions(ScheduledDay.DAYS_OF_WEEK);
            setDayOfWeekModel(dayModel);
        }
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (!cycle.isRewinding()) {
            return;
        }
    }
}
