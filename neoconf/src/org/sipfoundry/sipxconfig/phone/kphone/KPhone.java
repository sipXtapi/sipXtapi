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
package org.sipfoundry.sipxconfig.phone.kphone;

import java.io.File;
import java.io.FileWriter;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.Template;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.phone.AbstractPhone;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.setting.Setting;

public class KPhone extends AbstractPhone {
    
    public static final String FACTORY_ID = "kphone";
    
    private String m_webDirectory;
    
    private VelocityEngine m_velocityEngine;
    
    private String m_templateFilename = FACTORY_ID + "/kphonerc.vm";
    
    public KPhone() {
        // Where to load model relative to etc/sipxpbx directory
        setModelFilename(FACTORY_ID + "/phone.xml");
        
        // Tells superclass what bean to create for lines
        setLineFactoryId(KPhoneLine.FACTORY_ID);
    }

    public String getWebDirectory() {
        return m_webDirectory;
    }
    
    /**
     * KPhone doesn't have a download mechanism built in so for basic remote configuration
     * management, user will download profile from website before starting KPhone
     *  
     * @param webDirectory file path to directory that makes files available via http
     */
    public void setWebDirectory(String webDirectory) {
        m_webDirectory = webDirectory;
    }
    
    public String getProfileFileName() {
        return getWebDirectory() + "/" + getPhoneData().getSerialNumber() + ".kphonerc";
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }
    
    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
    
    public String getTemplateFilename() {
        return m_templateFilename;
    }
    
    public void setTemplateFilename(String templateFilename) {
        m_templateFilename = templateFilename;
    }
    
    public void generateProfiles() {
        FileWriter output = null;
        try {
            File profileFile = new File(getProfileFileName());
            profileFile.getParentFile().mkdirs();
            output = new FileWriter(profileFile);
            generateProfile(output);
        } catch (Exception e) {
            // Although in general it's not a good practice to catch all Exceptions, 
            // Velocity API reports throwing this exception type
            throw new RuntimeException("Could not generate kphone profile", e);
        } finally {
            IOUtils.closeQuietly(output);
        }
    }
    
    void generateProfile(Writer output) throws Exception {
        Template template = getVelocityEngine().getTemplate(getTemplateFilename());
        VelocityContext context = new VelocityContext();

        Line line;
        if (getLines().size() > 0) {
            line = getLine(0);
        } else {
            // Use empty line settings if no lines where added to device 
            line = createLine(new LineData());
        }
        
        // names match names used in kphonerc.vm
        context.put("lineSettings", line.getSettings());
        context.put("phoneSettings", getSettings());
        
        template.merge(context, output);
    }    
    
    public void setDefaults(Setting settings_) {        
    }

    public void restart() {
    }
}
