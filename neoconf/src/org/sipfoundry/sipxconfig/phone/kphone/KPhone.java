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
        setLineFactoryId(KPhoneLine.FACTORY_ID);
        setModelFilename(FACTORY_ID + "/phone.xml");
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
    
    public void setDefaults(Setting settings_) {        
    }

    public void generateProfiles() {
        FileWriter output = null;
        try {
            output = new FileWriter(getProfileFileName());
            generateProfile(output);
        } catch (Exception e) {
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
            line = createLine(new LineData());
        }
        context.put("lineSettings", line.getSettings());
        context.put("phoneSettings", getSettings());
        template.merge(context, output);
    }       

    /**
     * Not supported
     */
    public void restart() {
    }

    public String getWebDirectory() {
        return m_webDirectory;
    }
    
    public void setWebDirectory(String webDirectory) {
        m_webDirectory = webDirectory;
    }
    
    public String getProfileFileName() {
        return getWebDirectory() + "/" + getPhoneData().getSerialNumber() + ".kphonerc";
    }
}
