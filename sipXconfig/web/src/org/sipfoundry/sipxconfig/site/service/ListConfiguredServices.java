/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.service;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.service.ConfiguredService;
import org.sipfoundry.sipxconfig.service.ServiceDescriptor;
import org.sipfoundry.sipxconfig.service.ServiceManager;


public abstract class ListConfiguredServices extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "service/ListConfiguredServices";

    @InjectObject(value = "spring:serviceManager")
    public abstract ServiceManager getServiceManager();

    @InjectObject(value = "spring:serviceDescriptorSource")
    public abstract ModelSource<ServiceDescriptor> getServiceDescriptorSource();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestryContext();

    @Bean()
    public abstract SipxValidationDelegate getValidator();
    
    @Bean()
    public abstract SelectMap getSelections();

    @InitialValue("createServiceSelectionModel()")
    public abstract IPropertySelectionModel getServiceSelectionModel();

    public abstract ConfiguredService getCurrentRow();
    
    public abstract void setConverter(IPrimaryKeyConverter converter);
    
    public abstract ServiceDescriptor getServiceDescriptor();
        
    public void pageBeginRender(PageEvent event) {
        if (getRequestCycle().isRewinding()) {
            setConverter(new ObjectSourceDataSqueezer(getServiceManager(), ConfiguredService.class));
        }
    }
    
    public IPage formSubmit() {
        if (getServiceDescriptor() != null) {
            UnmanagedServicePage page = (UnmanagedServicePage) getRequestCycle().getPage(UnmanagedServicePage.PAGE);
            page.setServiceDescriptor(getServiceDescriptor());
            page.setReturnPage(PAGE);
            return page;
        }
        
        return this;
    }
    
    public IPage edit(Integer serviceId) {
        UnmanagedServicePage page = (UnmanagedServicePage) getRequestCycle().getPage(UnmanagedServicePage.PAGE);
        page.setServiceId(serviceId);
        page.setReturnPage(PAGE);
        return page;
    }
    
    public IPropertySelectionModel createServiceSelectionModel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(getServiceDescriptorSource().getModels());
        model.setLabelExpression("label");

        return getTapestryContext().addExtraOption(model, getMessages(), "label.addNewService");
    }
}
