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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.MillisDurationFormat;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
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
    
    public abstract List<String> getFolderIds();
    public abstract void setFolderIds(List<String> folderIds);
    
    @Persist(value = "client")
    public abstract String getFolderId();
    public abstract void setFolderId(String folderId);
    
    public IPropertySelectionModel getActionModel() {
        Collection actions = new ArrayList();
        actions.add(new OptGroup(getMessages().getMessage("label.moveTo")));
        for (String folderId : getFolderIds()) {
            if (!folderId.equals(getFolderId())) {
                actions.add(new MoveVoicemailAction(getFolderLabel(folderId), folderId));
            }
        }
        
        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        return model;
    }
    
    public String getFolderLabel() {
        return getFolderLabel(getFolderId());
    }
    
    String getFolderLabel(String folderId) {
        return getMessages().getMessage("tab." + folderId);
    }

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
        
        String userId = getUser().getUserName();
        
        List<String> folderIds = getFolderIds(); 
        if (getFolderIds() == null) {
            folderIds = getVoicemailManager().getFolderIds(userId);
            setFolderIds(folderIds);
            setFolderId(folderIds.get(0));
        }

        List<Voicemail> vm;
        try {
            vm = getVoicemailManager().getVoicemail(userId, getFolderId());
        } catch (UserException e) {
            getValidator().record(new ValidatorException(e.getMessage()));
            vm = Collections.emptyList();
        }
        setVoicemails(vm);
        
        setConverter(new VoicemailSqueezer(getVoicemailManager()));
    }
}
