package org.sipfoundry.sipxconfig.phone.snom;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.phone.PhoneSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class SnomPhone extends GenericPhone {

	public static final String FACTORY_ID = "snom";

	public SnomPhone() {
	  setLineFactoryId(SnomLine.FACTORY_ID);
	  setPhoneTemplate("snom/snom.vm");
	  setModelFile("snom/phone.xml");
	}

	public Setting getSettingModel() {
	  File sysDir = new File(getPhoneContext().getSystemDirectory());
	  File modelDefsFile = new File(sysDir, getModelFile());
	  Setting all = new XmlModelBuilder(sysDir).buildModel(modelDefsFile);
	  Setting model = all.getSetting(getModel().getModelId());
	  return model;
	}

	public String getPhoneFilename() {
	  String phoneFilename = getModel() + "-" + getPhoneData().getSerialNumber().toUpperCase();
	  return getWebDirectory() + "/" + phoneFilename + ".htm";
	}

	public String getPhoneFactoryIds() {
	  return FACTORY_ID;
	}

	public SnomModel getModel() {
	  return SnomModel.getModel(getPhoneData().getFactoryId());
	}

	public int getMaxLineCount() {
	  return getModel().getMaxLines();
	}

	public Object getAdapter(Class c) {
	  Object o = null;
	  if (c == PhoneSettings.class) {
		SettingBeanAdapter adapter = new SettingBeanAdapter(c);
		adapter.setSetting(getSettings());
		o = adapter.getImplementation();
	  }
	  else {
		o = super.getAdapter(c);
	  }
	  return o;
	}

	public Collection getProfileLines() {
	  ArrayList linesSettings = new ArrayList(getMaxLineCount());

	  Collection lines = getLines();
	  int i = 0;
	  Iterator ilines = lines.iterator();
	  for (; ilines.hasNext() && (i < getMaxLineCount()); i++) {
		linesSettings.add(((Line) ilines.next()).getSettings());
	  }

	  for (; i < getMaxLineCount(); i++) {
		SnomLine line = new SnomLine();
		line.setPhone(this);
		line.setLineData(new LineData());
		line.getLineData().setPosition(i);
		line.setDefaults(new PhoneDefaults());
		linesSettings.add(line.getSettings());
	  }

	  return linesSettings;
	}
}
