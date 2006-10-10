package org.sipfoundry.sipxconfig.phone;

import java.io.IOException;
import java.io.InputStream;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class TestPhone extends Phone {
    public static final String BEAN_ID = "testPhone";
    public static final String MODEL_ID = "testPhoneModel";
    
    public TestPhone() {
        super(BEAN_ID);
    }
    
    @Override
    protected Setting loadSettings() {    
        return loadSettings("phone.xml");
    }
    
    Setting loadSettings(String resource) {
        InputStream xmlStream = getClass().getResourceAsStream(resource);
        String sysdir = TestHelper.getSettingModelContextRoot();
        XmlModelBuilder builder = new XmlModelBuilder(sysdir);
        Setting settings;
        try {
            settings = builder.buildModel(xmlStream, null);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        return settings;        
    }
    
    @Override
    protected Setting loadLineSettings() {
        return loadSettings("line.xml");
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        return null;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
    }

}
