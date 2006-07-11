/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dj_deprecated("dojo.reflect is merged into dojo.lang (dojo.lang[type]).  Will be removed by 0.4");
dojo.hostenv.conditionalLoadModule({
	common: ["dojo.reflect.reflection"]
});
dojo.hostenv.moduleLoaded("dojo.reflect.*");
