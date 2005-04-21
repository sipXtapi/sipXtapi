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
import java.util.Map;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.VxmlService;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditAutoAttendant extends BasePage implements PageRenderListener {

    public static final String PAGE = "EditAutoAttendant";

    public abstract AutoAttendant getAttendant();

    public abstract void setAttendant(AutoAttendant attendant);

    public abstract void setPromptSelectionModel(IPropertySelectionModel model);

    public abstract IUploadFile getPromptUploadFile();

    public abstract void setPromptUploadFile(IUploadFile file);

    public abstract VxmlService getVxmlService();
    
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);
    
    public abstract DialPlanContext getDialPlanContext();
    
    public abstract DialPad getAddMenuItemDialPad();
    
    public abstract void setAddMenuItemDialPad(DialPad dialPad);

    public abstract AttendantMenuAction getAddMenuItemAction();

    public abstract void setAddMenuItemAction(AttendantMenuAction action);

    public abstract DialPad getCurrentDialPad();

    public AttendantMenuItem getCurrentMenuItem() {
        AttendantMenuItem menuItem = (AttendantMenuItem) getAttendant().getMenuItems().get(getCurrentDialPad());
        return menuItem;
    }    

    public void ok(IRequestCycle cycle) {        
        IValidationDelegate validator = TapestryUtils.getValidator(this);
        validatePrompt(validator);        
        checkFileUpload();
        if (!validator.getHasErrors()) {
            getDialPlanContext().storeAutoAttendant(getAttendant());
            returnManageAttendants(cycle);
        }        
    }
    
    public void cancel(IRequestCycle cycle) {
        returnManageAttendants(cycle);
    }
    
    private void returnManageAttendants(IRequestCycle cycle) {
        cycle.activate(ManageAttendants.PAGE);
        setAttendant(null);        
    }
    
    private void validatePrompt(IValidationDelegate validator) {
        if (getAttendant().getPrompt() == null) {
            if (!isUploadFileSpecified(getPromptUploadFile())) {
                validator.record("You must select an existing prompt or upload a new one.", 
                        ValidationConstraint.REQUIRED);
            }
        }
    }
    
    public void addMenuItem(IRequestCycle cycle_) {
        // SUBOPTIMAL: although it would be nice to upload file when saving, tapestry
        // cannot preserve state of Upload component and users would lose upload
        // selection everytime a menu item was added. Negative to this method is
        // that hitting cancel after adding 1 or more menu items would leave prompt
        // on server.  Dedicated prompt management page would avoid this altogether.
        checkFileUpload();
        
        IValidationDelegate validator = TapestryUtils.getValidator(this);
        validator.clearErrors();
        if (getAddMenuItemAction() == null) {
            validator.record("You must selection an action for your new attentant menu item", 
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
        
        File promptsDir = new File(getVxmlService().getPromptsDirectory());
        String[] prompts = promptsDir.list();
        if (prompts == null) {
            prompts = new String[0];
        }
        
        IPropertySelectionModel promptsModel = new StringPropertySelectionModel(prompts);
        setPromptSelectionModel(promptsModel);

        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
    
    private void initializeAttendant() {
        AutoAttendant aa = new AutoAttendant();
        aa.addMenuItem(DialPad.NUM_0, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
        aa.addMenuItem(DialPad.STAR, new AttendantMenuItem(AttendantMenuAction.CANCEL));
        setAttendant(aa);
    }
    
    private static boolean isUploadFileSpecified(IUploadFile file) {
        boolean isSpecified = file != null && !StringUtils.isBlank(file.getFileName());
        return isSpecified;
    }

    private void checkFileUpload() {
        IUploadFile file = getPromptUploadFile();
        if (!isUploadFileSpecified(file)) {
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
            setPromptUploadFile(null);
        } catch (IOException ioe) {
            throw new RuntimeException("Could not upload file " + file.getFileName(), ioe);
        } finally {
            IOUtils.closeQuietly(promptWtr);
        }
    }
}
