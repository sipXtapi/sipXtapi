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
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class LgNortelPhoneDefaults {

    private DeviceDefaults m_defaults;
    private int m_lines;

    public LgNortelPhoneDefaults(DeviceDefaults defaults, int lines) {
        m_defaults = defaults;
        m_lines = lines;
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    @SettingEntry(path = "VOIP/outbound_proxy_server")
    public String getProxyServerAddr() {
        return m_defaults.getProxyServerAddr();
    }

    @SettingEntry(path = "VOIP/outbound_proxy_port")
    public String getProxyServerPort() {
        return m_defaults.getProxyServerSipPort();
    }

    @SettingEntry(path = "VOIP/SIP_service_domain")
    public String getSipServiceDomain() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "VOIP/max_line_num")
    public int getMaxLineNum() {
        return m_lines;
    }
    
    @SettingEntry(path = "VOIP/timezone")
    public int getTimezone() {
        return getZone().getOffsetInHours();
    }

    @SettingEntry(path = "VOIP/dst_start_month")
    public int getStartMonth() {
        return getZone().getStartMonth();
    }

    @SettingEntry(path = "VOIP/dst_start_day")
    public int getStartDay() {
        return getZone().getStartDay();
    }

    @SettingEntry(path = "VOIP/dst_start_day_of_week")
    public int getStartDayOfWeek() {
        return getZone().getStartDayOfWeek();
    }

    @SettingEntry(path = "VOIP/dst_start_week_of_month")
    public int getStartWeekOfMonth() {
        int week = getZone().getStartWeek();
        return adjustWeekOfMonth(week);
    }

    @SettingEntry(path = "VOIP/dst_start_time")
    public int getStartTime() {
        return getZone().getStartTimeInHours();
    }

    @SettingEntry(path = "VOIP/dst_stop_month")
    public int getStopMonth() {
        return getZone().getStopMonth();
    }

    @SettingEntry(path = "VOIP/dst_stop_day")
    public int getStopDay() {
        return getZone().getStopDay();
    }

    @SettingEntry(path = "VOIP/dst_stop_day_of_week")
    public int getStopDayOfWeek() {
        return getZone().getStopDayOfWeek();
    }

    @SettingEntry(path = "VOIP/dst_stop_week_of_month")
    public int getStopWeekOfMonth() {
        int week = getZone().getStopWeek();
        return adjustWeekOfMonth(week);
    }

    @SettingEntry(path = "VOIP/dst_stop_time")
    public int getStopTime() {
        return getZone().getStopTimeInHours();
    }

    /**
     * Adjusts week of month value for LG phones.
     * 
     * In case of LG phones '7' means last month of the week.
     */
    private int adjustWeekOfMonth(int week) {
        return week != DeviceTimeZone.DST_LASTWEEK ? week : 7;
    }
}
