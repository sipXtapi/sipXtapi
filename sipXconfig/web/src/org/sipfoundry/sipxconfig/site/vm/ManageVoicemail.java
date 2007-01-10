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
package org.sipfoundry.sipxconfig.site.vm;

import java.util.Collections;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.MillisDurationFormat;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.sipfoundry.sipxconfig.vm.VoicemailManager;

public abstract class ManageVoicemail extends UserBasePage {

    @Asset("/images/voicemail-play.png")
    public abstract IAsset getPlayVoicemailAsset();

    @InjectObject(value = "spring:voicemailManager")
    public abstract VoicemailManager getVoicemailManager();

    public abstract SelectMap getSelections();
    public abstract void setSelections(SelectMap selections);

    public abstract Voicemail getVoicemail();

    public abstract List<Voicemail> getVoicemails();

    public abstract void setVoicemails(List<Voicemail> vm);

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    public abstract void setConverter(IPrimaryKeyConverter converter);
    
    @Bean(initializer = "maxField=2")
    public abstract MillisDurationFormat getDurationFormat();

    public ITableColumn getTimestampColumn() {
        return TapestryUtils.createDateColumn("timestamp", getMessages(),
                getExpressionEvaluator(), getLocale());
    }

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        
        SelectMap selections = getSelections();
        if (selections == null) {
            setSelections(new SelectMap());
        }

        List<Voicemail> vm = getVoicemails();
        if (vm == null) {
            try {
                vm = getVoicemailManager().getVoicemail(getUser().getUserName(), "inbox");
            } catch (UserException e) {
                getValidator().record(new ValidatorException(e.getMessage()));
                vm = Collections.emptyList();
            }
            setVoicemails(vm);
        }
        
        setConverter(new VoicemailSqueezer(getVoicemailManager()));
    }
}
