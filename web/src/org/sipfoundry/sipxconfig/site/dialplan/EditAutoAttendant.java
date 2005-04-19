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

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.request.IUploadFile;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.VxmlService;
import org.sipfoundry.sipxconfig.common.DialPad;

public abstract class EditAutoAttendant extends BasePage implements PageRenderListener {

    public static final String PAGE = "EditAutoAttendant";

    public abstract AutoAttendant getAttendant();

    public abstract void setAttendant(AutoAttendant attendant);

    public abstract Integer getAttendantId();

    public abstract void setAttendantId(Integer id);

    public abstract IPropertySelectionModel getPromptSelectionModel();

    public abstract void setPromptSelectionModel(IPropertySelectionModel model);

    public abstract IUploadFile getPromptUploadFile();

    public abstract VxmlService getVxmlService();
    
    public abstract String getCurrentDialPadId();
    
    public abstract void setCurrentDialPadId(String dialpadId);
    
    public AttendantMenuItem getCurrentMenuItem() {
        AttendantMenuItem menuItem = (AttendantMenuItem) getAttendant().getMenuItems().get(getCurrentDialPadId());
        return menuItem;
    }
    
    public DialPad getCurrentDialPad() {
        DialPad dialpad = DialPad.getDialPadById(getCurrentDialPadId());        
        return dialpad;
    }

    public void setCurrentDialPad(DialPad dialpad_) {
        // TODO
    }

    public void ok(IRequestCycle cycle_) {
        checkFileUpload();
    }
    
    public void cancel(IRequestCycle cycle_) {
    }

    public void pageBeginRender(PageEvent event_) {
        AutoAttendant aa = getAttendant();        
        if (aa == null) {
            Integer aaId = getAttendantId();
            if (aaId == null) {
                initializeAttendant();
            }            
        }
        
        File promptsDir = new File(getVxmlService().getPromptsDirectory());
        String[] prompts = promptsDir.list();
        if (prompts == null || prompts.length == 0) {
            // no prompts not likely in real system
            prompts = new String[] { 
                "---no prompts---"
            };
        }
        setPromptSelectionModel(new StringPropertySelectionModel(prompts));
    }
    
    private void initializeAttendant() {
        AutoAttendant aa = new AutoAttendant();
        aa.addMenuItem(DialPad.NUM_0, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
        aa.addMenuItem(DialPad.STAR, new AttendantMenuItem(AttendantMenuAction.CANCEL));
        setAttendant(aa);        
    }

    private void checkFileUpload() {
        IUploadFile file = getPromptUploadFile();
        if (file == null) {
            return;
        }
        
        InputStream upload = file.getStream();
        FileOutputStream promptWtr = null;
        try {
            File promptsDir = new File(getVxmlService().getPromptsDirectory());
            promptsDir.mkdirs();
            File promptFile = new File(promptsDir, file.getFileName());
            promptWtr = new FileOutputStream(promptFile);
            CopyUtils.copy(upload, promptWtr);
            getAttendant().setPrompt(promptFile.getName());
        } catch (IOException ioe) {
            throw new RuntimeException("Could not upload file " + file.getFileName(), ioe);
        } finally {
            IOUtils.closeQuietly(promptWtr);
        }
    }
}
