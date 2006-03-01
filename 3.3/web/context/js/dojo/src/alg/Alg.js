/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.alg.Alg");
dojo.require("dojo.lang.array");
dj_deprecated("dojo.alg.Alg is deprecated, use dojo.lang instead");

dojo.alg = dojo.lang;

dojo.alg.for_each = dojo.alg.forEach; // burst compat

dojo.alg.for_each_call = dojo.alg.map; // burst compat

dojo.alg.inArr = dojo.lang.inArray; // burst compat
