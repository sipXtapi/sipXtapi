/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.rmi.RemoteException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class PhoneServiceImpl implements PhoneService {

    private static final String GROUP_RESOURCE_ID = 
        org.sipfoundry.sipxconfig.phone.Phone.GROUP_RESOURCE_ID;
    
    private PhoneContext m_context;

    private SettingDao m_settingDao;

    private PhoneBuilder m_builder;

    private CoreContext m_coreContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhoneContext(PhoneContext context) {
        m_context = context;
    }

    public void setPhoneBuilder(PhoneBuilder builder) {
        m_builder = builder;
    }

    public void addPhone(AddPhone addPhone) throws RemoteException {
        Phone apiPhone = addPhone.getPhone();
        PhoneModel model = requireModelId(apiPhone.getModelId());
        org.sipfoundry.sipxconfig.phone.Phone phone = m_context.newPhone(model);
        ApiBeanUtil.toMyObject(m_builder, phone, apiPhone);
        String[] groups = apiPhone.getGroups();
        for (int i = 0; groups != null && i < groups.length; i++) {
            Group g = m_settingDao.getGroupCreateIfNotFound(GROUP_RESOURCE_ID, groups[i]);
            phone.addGroup(g);
        }
        m_context.storePhone(phone);
    }

    public FindPhoneResponse findPhone(FindPhone findPhone) throws RemoteException {
        FindPhoneResponse response = new FindPhoneResponse();
        org.sipfoundry.sipxconfig.phone.Phone[] myPhones = phoneSearch(findPhone.getSearch());
        Phone[] arrayOfPhones = (Phone[]) ApiBeanUtil
                .toApiArray(m_builder, myPhones, Phone.class);
        response.setPhones(arrayOfPhones);

        return response;
    }

    org.sipfoundry.sipxconfig.phone.Phone[] phoneSearch(PhoneSearch search) {
        Collection phones = Collections.EMPTY_LIST;
        if (search == null) {
            phones = m_context.loadPhones();
        } else if (search.getBySerialNumber() != null) {
            Integer id = m_context.getPhoneIdBySerialNumber(search.getBySerialNumber());
            if (id != null) {
                org.sipfoundry.sipxconfig.phone.Phone phone = m_context.loadPhone(id);
                if (phone != null) {
                    phones = Collections.singleton(phone);
                }
            }
        } else if (search.getByGroup() != null) {
            Group g = m_settingDao.getGroupByName(GROUP_RESOURCE_ID, search.getByGroup());
            if (g != null) {
                phones = m_context.getPhonesByGroupId(g.getId());
            }
        }

        return (org.sipfoundry.sipxconfig.phone.Phone[]) phones
                .toArray(new org.sipfoundry.sipxconfig.phone.Phone[phones.size()]);
    }

    public PhoneModel requireModelId(String modelId) {
        PhoneModel model = PhoneModel.getModel(modelId);
        if (model == null) {
            throw new IllegalArgumentException("phone model doesn't exist: " + modelId);
        }
        return model;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void adminPhone(AdminPhone adminPhone) throws RemoteException {
        org.sipfoundry.sipxconfig.phone.Phone[] myPhones = phoneSearch(adminPhone.getSearch());
        if (Boolean.TRUE.equals(adminPhone.getGenerateProfiles())) {
            m_context.generateProfilesAndRestart(Arrays.asList(myPhones));
        } else if (Boolean.TRUE.equals(adminPhone.getRestart())) {
            m_context.restart(Arrays.asList(myPhones));
        } else {
            for (int i = 0; i < myPhones.length; i++) {

                if (Boolean.TRUE.equals(adminPhone.getDeletePhone())) {
                    m_context.deletePhone(myPhones[i]);
                    continue; // all other edits wouldn't make sense
                }

                if (adminPhone.getEdit() != null) {
                    Phone apiPhone = new Phone();
                    Set properties = ApiBeanUtil.getSpecfiedProperties(adminPhone.getEdit());
                    ApiBeanUtil.setProperties(apiPhone, adminPhone.getEdit());
                    m_builder.toMyObject(myPhones[i], apiPhone, properties);
                }

                if (adminPhone.getRemoveLine() != null) {
                    String username = adminPhone.getRemoveLine();
                    org.sipfoundry.sipxconfig.phone.Line l = myPhones[i].findByUsername(username);
                    if (l != null) {
                        myPhones[i].removeLine(l);
                    }
                }

                if (adminPhone.getAddLine() != null) {
                    String userName = adminPhone.getAddLine().getUserName();
                    org.sipfoundry.sipxconfig.common.User u = m_coreContext
                            .loadUserByUserName(userName);
                    org.sipfoundry.sipxconfig.phone.Line l = myPhones[i].createLine();
                    l.setUser(u);
                    myPhones[i].addLine(l);
                }

                if (adminPhone.getAddGroup() != null) {
                    Group g = m_settingDao.getGroupCreateIfNotFound(GROUP_RESOURCE_ID, adminPhone
                            .getAddGroup());
                    myPhones[i].addGroup(g);
                }

                if (adminPhone.getRemoveGroup() != null) {
                    Group g = m_settingDao.getGroupByName(GROUP_RESOURCE_ID, adminPhone.getRemoveGroup());
                    if (g != null) {
                        DataCollectionUtil.removeByPrimaryKey(myPhones[i].getGroups(), g.getPrimaryKey());
                    }
                }

                m_context.storePhone(myPhones[i]);
            }
        }
    }
}
