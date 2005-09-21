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
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 * Support for Grandstream BudgeTone / HandyTone
 */
public class GrandstreamPhone extends Phone {
    
    public static final String BEAN_ID = "grandstream";

    public static final String SIP = "sip";

    public static final String EMPTY = "";

    public static final int KOLME = 3;

    public static final int FOUR = 4;

    public static final int VIISI = 5;

    public static final int SIX = 6;

    public static final int SIXTEEN = 16;

    public static final int EIGHT = 8;

    public static final int CR = 0x0d;

    public static final int LF = 0x0a;

    public static final int HEXFF = 0xff;

    public static final int OXIOOOO = 0x10000;

    public static final String EQUALS = "=";

    public static final String ET = "&";
    
    private static final SettingFilter S_REALSETTINGS = new SettingFilter() {
            public boolean acceptSetting(Setting root_, Setting setting) {
                boolean isLeaf = setting.getValues().isEmpty();            
                boolean isVirtual = (setting.getName().startsWith("_"));            
                return isLeaf && !isVirtual;
            }
        };        

    private static final SettingFilter S_IPSETTINGS = new SettingFilter() {
            public boolean acceptSetting(Setting root_, Setting setting) {
                boolean isLeaf = setting.getValues().isEmpty();            
                boolean isBitmapped = (setting.getName().startsWith("__"));            
                return  isLeaf && isBitmapped;
            }
        };        

    private boolean m_isTextFormatEnabled;

    public GrandstreamPhone() {
        super(BEAN_ID);
        init();
    }
    
    public GrandstreamPhone(GrandstreamModel model) {
        super(model); // sexy
        init();
    }
    
    private void init() {
        setPhoneTemplate("grandstream/grandstream.vm");        
    }


    /** 
     * Generate files in text format.  Won't be usable by phone, but you can use 
     * grandstreams config tool to convert manually.  This is mostly for debugging
     * 
     * @param isTextFormatEnabled true to save as text, default is false
     */
    public void setTextFormatEnabled(boolean isTextFormatEnabled) {
        m_isTextFormatEnabled = isTextFormatEnabled;
    }

    public String getModelLabel() {
        return getModel().getLabel();
    }

    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return getTftpRoot() + "/cfg" + phoneFilename.toLowerCase();
    }

    public Object getAdapter(Class c) {
        Object o = null;
        if (c == PhoneSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(c);
            adapter.setSetting(getSettings());
            adapter.addMapping(PhoneSettings.OUTBOUND_PROXY, "sip/P48");
            adapter.addMapping(PhoneSettings.TFTP_SERVER, "upgrade/__TFTPServer-213");
            o = adapter.getImplementation();
        } else {
            o = super.getAdapter(c);
        }
        
        return o;
    }

    public Object getLineAdapter(Line line, Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(line.getSettings());
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "port/P36");
            adapter.addMapping(LineSettings.USER_ID, "port/P35");
            adapter.addMapping(LineSettings.PASSWORD, "port/P34");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "port/P3");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "sip/P47");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;
    }
    public Collection getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        ArrayList linesSettings = new ArrayList(lineCount);

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < lineCount; i++) {
            Line line = createLine();
            line.setPosition(i);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    public void generateProfiles() {
        String outputfile = getPhoneFilename();
        FileOutputStream wtr = null;

        splitIpSettings();

        try {
            wtr = new FileOutputStream(outputfile);
            if (!m_isTextFormatEnabled) {
                String body = generateGsParaBody();
                generateGsParaString(wtr, body);
            } else {
                writeTextFile(wtr);
            }
                
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }
    
    void writeTextFile(OutputStream wtr) throws IOException {
        Collection phoneset = getRealSettings(getSettings());
        Iterator psi = phoneset.iterator();
        while (psi.hasNext()) {
            Setting pset = (Setting) psi.next();
            writeProfileLine(wtr, pset.getName(), pset.getValue());
        }

        Collection lines = getProfileLines();
        Iterator lni = lines.iterator();
        while (lni.hasNext()) {
            Collection lineset = getRealSettings((Setting) lni.next());
            Iterator lsi = lineset.iterator();
            while (lsi.hasNext()) {
                Setting lset = (Setting) lsi.next();
                writeProfileLine(wtr, lset.getName(), lset.getValue());
            }
        }        
    }
    
    void writeProfileLine(OutputStream wtr, String name, String value) throws IOException {
        String line = name + " = " + value + (char) LF;
        wtr.write(line.getBytes());
    }
    
    String generateGsParaBody() {
        StringBuffer paras = new StringBuffer();
        Collection phoneset = getRealSettings(getSettings());
        Iterator psi = phoneset.iterator();
        while (psi.hasNext()) {
            Setting pset = (Setting) psi.next();
            paras.append(pset.getName() + EQUALS + pset.getValue() + ET);
        }

        Collection lines = getProfileLines();
        Iterator lni = lines.iterator();
        while (lni.hasNext()) {
            Collection lineset = getRealSettings((Setting) lni.next());
            Iterator lsi = lineset.iterator();
            while (lsi.hasNext()) {
                Setting lset = (Setting) lsi.next();
                paras.append(lset.getName() + EQUALS + lset.getValue() + ET);
            }
        }
        
        return paras.toString();
    }

    void generateGsParaString(OutputStream wtr, String body) throws IOException {
        byte[] gsheader = new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, CR, LF, CR, LF};
        StringBuffer paras = new StringBuffer();
        
        String serial = getSerialNumber();
        
        for (int si = 0; si < SIX; si++) {
            gsheader[si + SIX] = (byte) Integer.parseInt(serial.substring(si * 2, si * 2 + 2), SIXTEEN);
        }
        
        paras.append(body);

        paras.append("gnkey=0b82");

        if (paras.length() % 2 == 1) {
            paras.append('\000');
        }

        int plen = EIGHT + paras.length() / 2;
        gsheader[2] = (byte) ((plen >> EIGHT) & HEXFF);
        gsheader[KOLME] = (byte) (plen & HEXFF);

        int checksum = 0;
        for (int pi = 0; pi < paras.length(); pi += 2) {
            checksum += (paras.charAt(pi) & HEXFF) << EIGHT;
            checksum += paras.charAt(pi + 1) & HEXFF;
        }
        for (int pi = 0; pi < SIXTEEN; pi += 2) {
            checksum += (gsheader[pi] & HEXFF) << EIGHT;
            checksum += gsheader[pi + 1] & HEXFF;
        }

        checksum = OXIOOOO - (checksum % OXIOOOO);

        gsheader[FOUR] = (byte) ((checksum >> EIGHT) & HEXFF);
        gsheader[VIISI] = (byte) (checksum & HEXFF);

        wtr.write(gsheader);
        wtr.write(paras.toString().getBytes());
    }

    public Collection getRealSettings(Setting root) {
        return SettingUtil.filter(S_REALSETTINGS, root);
    }

    public Collection getIpSettings() {
        return SettingUtil.filter(S_IPSETTINGS, getSettings());
    }

    private void splitIpSettings() {
        Collection bitmaps = getIpSettings();
        Iterator bmi = bitmaps.iterator();
        while (bmi.hasNext()) {
            Setting bset = (Setting) bmi.next();
            String bname = bset.getName();
            int bpoint = bname.indexOf('-');

            if (bpoint < KOLME || bpoint == bname.length() - 1) {
                continue;
            }

            Setting btgt = getSettings().getSetting(bset.getParentPath().substring(1));
            
            int bofs = Integer.parseInt(bname.substring(bpoint + 1));
            
            String ipa = bset.getValue();
            String[] ipn = {EMPTY, EMPTY, EMPTY, EMPTY};

            if (!StringUtils.isBlank(ipa)) {
                ipn = ipa.split("\\.");
            }

            for (int i = 0; i < ipn.length; i++) {
                btgt.getSetting("P" + Integer.toString(bofs + i)).setValue(ipn[i]);
            }
        }
    }

    public void restart() {
        sendCheckSyncToFirstLine();        
    }
}
