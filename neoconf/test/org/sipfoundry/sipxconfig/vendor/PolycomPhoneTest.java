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
package org.sipfoundry.sipxconfig.vendor;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.Iterator;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.Credential;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Organization;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.User;
import org.sipfoundry.sipxconfig.setting.SettingModel;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class PolycomPhoneTest extends XMLTestCase {
        
    public void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
    }
    
    public void testBasicProfile() throws Exception {
        MockControl phoneControl = MockControl.createStrictControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneControl.getMock();
        Organization rootOrg = new Organization();
        rootOrg.setDnsDomain("localhost.localdomain");
        phoneControl.expectAndReturn(phoneContext.loadRootOrganization(), rootOrg);
        phoneControl.replay();

        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber("123abc");
        endpoint.setSettings(new SettingSet());
        endpoint.setPhoneId(PolycomPhone.MODEL_300);
        PolycomPhone phone = new PolycomPhone();
        phone.setSystemDirectory(TestHelper.getSysDirProperties().getProperty("sysdir.etc"));
        phone.setTftpRoot(TestHelper.getTestDirectory());
        phone.setModelId(PolycomPhone.MODEL_300);
        phone.setEndpoint(endpoint);
        phone.setVelocityEngine(TestHelper.getVelocityEngine());

        // create basic data
        Line line = new Line();
        User user = new User();
        Credential credential = new Credential();
        user.setCredential(credential);
        line.setEndpoint(endpoint);
        line.setUser(user);
        
        phone.generateProfiles(phoneContext);
        InputStream expectedPhoneStream = null;
        InputStream actualPhoneStream = null;
        try {            
            expectedPhoneStream = getClass().getResourceAsStream("phone1.cfg");
            assertNotNull(expectedPhoneStream);
            Reader expectedXml = new InputStreamReader(expectedPhoneStream);
            
            actualPhoneStream = phone.getPhoneConfigFile();
            assertNotNull(actualPhoneStream);
            Reader generatedXml = new InputStreamReader(actualPhoneStream);

            Diff phoneDiff = new Diff(expectedXml, generatedXml);
            assertXMLEqual(phoneDiff, true);
        } finally {
            if (expectedPhoneStream != null) {
                expectedPhoneStream.close();
            }
            if (actualPhoneStream != null) {
                actualPhoneStream.close();
            }
        }
        
        phoneControl.verify();
    }
    
    /**
     * G O A L S
     * ------------
     * U I : 
     *  1.) drill thru skeleton of possible settings
     *  2.) current setting already populated with values
     *  3.) populated w/defaults as well
     *  4.) field level data: labels, fieldnames, validation errors, short descriptions
     *  5.) group level data: long descriptions, validation errors, heading labels
     *  6.) model apis condusive to tapestry page syntax 
     * 
     * D O M A I N : 
     *  1.) create domain objects to validate settings
     *  2.) allow end user to effect available settings
     *  3.) build validation library of common field types
     *  4.) model apis condusive to velocity syntax
     *  5.) unittestability 
     *  6.) custom renderers, validators, parsing interpretters
     *  7.) model use enumerations available to java code
     * 
     * D A T A B A S E :
     *  1.) store only what user has entered
     *  2.) CRUD sections at a time
     *  3.) migration plan for settings as domain evolves
     * 
     * Implementation Persectives
     * -----------------------------
     * U I:
     *  MyPage.java
     *    render()
     *      Model m = new Model();
     *      m.populateData(getSettings());
     *    save()
     *      m.save()
     * 
     *  MyPage.page
     *    m.currentRow.label
     *    m.currentRow.value
     *    m.currentRow.defaultValue
     *    
     * D O M A I N:
     * o Most phones will have developer manual that major percentage of
     *   rules can be translated and maintained to this specification file
     * o End users can edit this file to drive UI and model
     * o Using programming language in xml reduces maintenance chore and
     *   avoid extra xml interprettation to java objects (example. cardinality
     *   would define how many are allowed, etc.
     * o This method does not have to be adopted by all phone implementation.
     *   use a toolkit that throw different event types
     *     
     * 
     * model.xml.template
     *    <model>
     *      <name=network types="$enumerationX">
     *    #if $model = xyz
     *         <name=tftpServer default=$property type=serverName>
     *             <label>This does this and should match $property</label>
     *    #for i = 1 to $num
     *         <line default=$property>  
     *    #end     
     *    #end
     * 
     * profile.dat.template
     * 
     *   #for i in model.lines
     *   lines.$i.server = model[$i].value
     *   #end
     * 
     * 
     *  MyPhoneModel
     * 
     *   validateElement(ModelEvent e)
     *     if (e.getLabel() == NETWORK)
     *        ModelElement elem = e.getElement()
     *        if (elem.getSetting().getValue() == xyz)
     *           elem.setValidationError('abc')
     * 
     *   generateProfiles(Model m)
     *      template1.getObject("model", model)
     *      template1.write(file1)
     *      template2.getObject("model", model)
     *      template2.write(file2)
     * 
     *  MyPhoneModelTest
     *      MyPhoneModelTest phone
     *      // polulate phone w/business objects
     *      settings = loadTestSetting(file)
     *      model = phone.getModel(settings)
     *      phone.generateProfile()
     *      assertFileEquals(DiffCallback)
     * 
     *  D A T A B A S E:
     *     List settings = model.getChildSettings(parentId)
     *     model.populateSettings(settings);
     *     
     * 
     *     List changed = model.getChangedSettings()
     *     dao.saveSettings(changed);
     *     List deleted = model.getDeletedSettings()
     *     dao.deleteSettings(changed);
     * 
     */    
    public void testSettingModel() {
        PolycomPhone phone = new PolycomPhone();
        String sysDir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        assertNotNull(sysDir);
        phone.setSystemDirectory(sysDir);
        SettingModel model = phone.getSettingModel();
        assertNotNull(model);
        
        SettingModel lines = (SettingModel) model.getMeta(Phone.LINE_SETTINGS);
        assertNotNull(lines);
        
        Iterator headings = model.values().iterator();
        assertEquals("Registration", nextModel(headings).getLabel());
        assertEquals("Line Settings", nextModel(headings).getLabel());
        assertFalse(headings.hasNext());
    }
    
    private SettingModel nextModel(Iterator i) {
        return (SettingModel) i.next();
    }
}
