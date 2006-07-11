/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

/*
	This is a compiled version of Dojo, built for deployment and not for
	development. To get an editable version, please visit:

		http://dojotoolkit.org

	for documentation and information on getting the source.
*/

var dj_global=this;
function dj_undef(_1,_2){
if(!_2){
_2=dj_global;
}
return (typeof _2[_1]=="undefined");
}
if(dj_undef("djConfig")){
var djConfig={};
}
if(dj_undef("dojo")){
var dojo={};
}
dojo.version={major:0,minor:2,patch:2,flag:"+",revision:Number("$Rev: 3458 $".match(/[0-9]+/)[0]),toString:function(){
with(dojo.version){
return major+"."+minor+"."+patch+flag+" ("+revision+")";
}
}};
dojo.evalProp=function(_3,_4,_5){
return (_4&&!dj_undef(_3,_4)?_4[_3]:(_5?(_4[_3]={}):undefined));
};
dojo.parseObjPath=function(_6,_7,_8){
var _9=(_7?_7:dj_global);
var _a=_6.split(".");
var _b=_a.pop();
for(var i=0,l=_a.length;i<l&&_9;i++){
_9=dojo.evalProp(_a[i],_9,_8);
}
return {obj:_9,prop:_b};
};
dojo.evalObjPath=function(_d,_e){
if(typeof _d!="string"){
return dj_global;
}
if(_d.indexOf(".")==-1){
return dojo.evalProp(_d,dj_global,_e);
}
with(dojo.parseObjPath(_d,dj_global,_e)){
return dojo.evalProp(prop,obj,_e);
}
};
dojo.errorToString=function(_f){
return ((!dj_undef("message",_f))?_f.message:(dj_undef("description",_f)?_f:_f.description));
};
dojo.raise=function(_10,_11){
if(_11){
_10=_10+": "+dojo.errorToString(_11);
}
var he=dojo.hostenv;
if((!dj_undef("hostenv",dojo))&&(!dj_undef("println",dojo.hostenv))){
dojo.hostenv.println("FATAL: "+_10);
}
throw Error(_10);
};
dj_throw=dj_rethrow=function(m,e){
dojo.deprecated("dj_throw and dj_rethrow deprecated, use dojo.raise instead");
dojo.raise(m,e);
};
dojo.debug=function(){
if(!djConfig.isDebug){
return;
}
var _15=arguments;
if(dj_undef("println",dojo.hostenv)){
dojo.raise("dojo.debug not available (yet?)");
}
var _16=dj_global["jum"]&&!dj_global["jum"].isBrowser;
var s=[(_16?"":"DEBUG: ")];
for(var i=0;i<_15.length;++i){
if(!false&&_15[i] instanceof Error){
var msg="["+_15[i].name+": "+dojo.errorToString(_15[i])+(_15[i].fileName?", file: "+_15[i].fileName:"")+(_15[i].lineNumber?", line: "+_15[i].lineNumber:"")+"]";
}else{
try{
var msg=String(_15[i]);
}
catch(e){
if(dojo.render.html.ie){
var msg="[ActiveXObject]";
}else{
var msg="[unknown]";
}
}
}
s.push(msg);
}
if(_16){
jum.debug(s.join(" "));
}else{
dojo.hostenv.println(s.join(" "));
}
};
dojo.debugShallow=function(obj){
if(!djConfig.isDebug){
return;
}
dojo.debug("------------------------------------------------------------");
dojo.debug("Object: "+obj);
var _1b=[];
for(var _1c in obj){
try{
_1b.push(_1c+": "+obj[_1c]);
}
catch(E){
_1b.push(_1c+": ERROR - "+E.message);
}
}
_1b.sort();
for(var i=0;i<_1b.length;i++){
dojo.debug(_1b[i]);
}
dojo.debug("------------------------------------------------------------");
};
var dj_debug=dojo.debug;
function dj_eval(s){
return dj_global.eval?dj_global.eval(s):eval(s);
}
dj_unimplemented=dojo.unimplemented=function(_1f,_20){
var _21="'"+_1f+"' not implemented";
if((!dj_undef(_20))&&(_20)){
_21+=" "+_20;
}
dojo.raise(_21);
};
dj_deprecated=dojo.deprecated=function(_22,_23,_24){
var _25="DEPRECATED: "+_22;
if(_23){
_25+=" "+_23;
}
if(_24){
_25+=" -- will be removed in version: "+_24;
}
dojo.debug(_25);
};
dojo.experimental=function(_26,_27){
var _28="EXPERIMENTAL: "+_26;
_28+=" -- Not yet ready for use.  APIs subject to change without notice.";
if(_27){
_28+=" "+_27;
}
dojo.debug(_28);
};
dojo.inherits=function(_29,_2a){
if(typeof _2a!="function"){
dojo.raise("superclass: "+_2a+" borken");
}
_29.prototype=new _2a();
_29.prototype.constructor=_29;
_29.superclass=_2a.prototype;
_29["super"]=_2a.prototype;
};
dj_inherits=function(_2b,_2c){
dojo.deprecated("dj_inherits deprecated, use dojo.inherits instead");
dojo.inherits(_2b,_2c);
};
dojo.render=(function(){
function vscaffold(_2d,_2e){
var tmp={capable:false,support:{builtin:false,plugin:false},prefixes:_2d};
for(var x in _2e){
tmp[x]=false;
}
return tmp;
}
return {name:"",ver:dojo.version,os:{win:false,linux:false,osx:false},html:vscaffold(["html"],["ie","opera","khtml","safari","moz"]),svg:vscaffold(["svg"],["corel","adobe","batik"]),vml:vscaffold(["vml"],["ie"]),swf:vscaffold(["Swf","Flash","Mm"],["mm"]),swt:vscaffold(["Swt"],["ibm"])};
})();
dojo.hostenv=(function(){
var _31={isDebug:false,allowQueryConfig:false,baseScriptUri:"",baseRelativePath:"",libraryScriptUri:"",iePreventClobber:false,ieClobberMinimal:true,preventBackButtonFix:true,searchIds:[],parseWidgets:true};
if(typeof djConfig=="undefined"){
djConfig=_31;
}else{
for(var _32 in _31){
if(typeof djConfig[_32]=="undefined"){
djConfig[_32]=_31[_32];
}
}
}
var djc=djConfig;
function _def(obj,_35,def){
return (dj_undef(_35,obj)?def:obj[_35]);
}
return {name_:"(unset)",version_:"(unset)",pkgFileName:"__package__",loading_modules_:{},loaded_modules_:{},addedToLoadingCount:[],removedFromLoadingCount:[],inFlightCount:0,modulePrefixes_:{dojo:{name:"dojo",value:"src"}},setModulePrefix:function(_37,_38){
this.modulePrefixes_[_37]={name:_37,value:_38};
},getModulePrefix:function(_39){
var mp=this.modulePrefixes_;
if((mp[_39])&&(mp[_39]["name"])){
return mp[_39].value;
}
return _39;
},getTextStack:[],loadUriStack:[],loadedUris:[],post_load_:false,modulesLoadedListeners:[],getName:function(){
return this.name_;
},getVersion:function(){
return this.version_;
},getText:function(uri){
dojo.unimplemented("getText","uri="+uri);
},getLibraryScriptUri:function(){
dojo.unimplemented("getLibraryScriptUri","");
}};
})();
dojo.hostenv.getBaseScriptUri=function(){
if(djConfig.baseScriptUri.length){
return djConfig.baseScriptUri;
}
var uri=new String(djConfig.libraryScriptUri||djConfig.baseRelativePath);
if(!uri){
dojo.raise("Nothing returned by getLibraryScriptUri(): "+uri);
}
var _3d=uri.lastIndexOf("/");
djConfig.baseScriptUri=djConfig.baseRelativePath;
return djConfig.baseScriptUri;
};
dojo.hostenv.setBaseScriptUri=function(uri){
djConfig.baseScriptUri=uri;
};
dojo.hostenv.loadPath=function(_3f,_40,cb){
if((_3f.charAt(0)=="/")||(_3f.match(/^\w+:/))){
dojo.raise("relpath '"+_3f+"'; must be relative");
}
var uri=this.getBaseScriptUri()+_3f;
if(djConfig.cacheBust&&dojo.render.html.capable){
uri+="?"+String(djConfig.cacheBust).replace(/\W+/g,"");
}
try{
return ((!_40)?this.loadUri(uri,cb):this.loadUriAndCheck(uri,_40,cb));
}
catch(e){
dojo.debug(e);
return false;
}
};
dojo.hostenv.loadUri=function(uri,cb){
if(this.loadedUris[uri]){
return;
}
var _45=this.getText(uri,null,true);
if(_45==null){
return 0;
}
this.loadedUris[uri]=true;
var _46=dj_eval(_45);
return 1;
};
dojo.hostenv.loadUriAndCheck=function(uri,_48,cb){
var ok=true;
try{
ok=this.loadUri(uri,cb);
}
catch(e){
dojo.debug("failed loading ",uri," with error: ",e);
}
return ((ok)&&(this.findModule(_48,false)))?true:false;
};
dojo.loaded=function(){
};
dojo.hostenv.loaded=function(){
this.post_load_=true;
var mll=this.modulesLoadedListeners;
for(var x=0;x<mll.length;x++){
mll[x]();
}
dojo.loaded();
};
dojo.addOnLoad=function(obj,_4e){
if(arguments.length==1){
dojo.hostenv.modulesLoadedListeners.push(obj);
}else{
if(arguments.length>1){
dojo.hostenv.modulesLoadedListeners.push(function(){
obj[_4e]();
});
}
}
};
dojo.hostenv.modulesLoaded=function(){
if(this.post_load_){
return;
}
if((this.loadUriStack.length==0)&&(this.getTextStack.length==0)){
if(this.inFlightCount>0){
dojo.debug("files still in flight!");
return;
}
if(typeof setTimeout=="object"){
setTimeout("dojo.hostenv.loaded();",0);
}else{
dojo.hostenv.loaded();
}
}
};
dojo.hostenv.moduleLoaded=function(_4f){
var _50=dojo.evalObjPath((_4f.split(".").slice(0,-1)).join("."));
this.loaded_modules_[(new String(_4f)).toLowerCase()]=_50;
};
dojo.hostenv._global_omit_module_check=false;
dojo.hostenv.loadModule=function(_51,_52,_53){
if(!_51){
return;
}
_53=this._global_omit_module_check||_53;
var _54=this.findModule(_51,false);
if(_54){
return _54;
}
if(dj_undef(_51,this.loading_modules_)){
this.addedToLoadingCount.push(_51);
}
this.loading_modules_[_51]=1;
var _55=_51.replace(/\./g,"/")+".js";
var _56=_51.split(".");
var _57=_51.split(".");
for(var i=_56.length-1;i>0;i--){
var _59=_56.slice(0,i).join(".");
var _5a=this.getModulePrefix(_59);
if(_5a!=_59){
_56.splice(0,i,_5a);
break;
}
}
var _5b=_56[_56.length-1];
if(_5b=="*"){
_51=(_57.slice(0,-1)).join(".");
while(_56.length){
_56.pop();
_56.push(this.pkgFileName);
_55=_56.join("/")+".js";
if(_55.charAt(0)=="/"){
_55=_55.slice(1);
}
ok=this.loadPath(_55,((!_53)?_51:null));
if(ok){
break;
}
_56.pop();
}
}else{
_55=_56.join("/")+".js";
_51=_57.join(".");
var ok=this.loadPath(_55,((!_53)?_51:null));
if((!ok)&&(!_52)){
_56.pop();
while(_56.length){
_55=_56.join("/")+".js";
ok=this.loadPath(_55,((!_53)?_51:null));
if(ok){
break;
}
_56.pop();
_55=_56.join("/")+"/"+this.pkgFileName+".js";
if(_55.charAt(0)=="/"){
_55=_55.slice(1);
}
ok=this.loadPath(_55,((!_53)?_51:null));
if(ok){
break;
}
}
}
if((!ok)&&(!_53)){
dojo.raise("Could not load '"+_51+"'; last tried '"+_55+"'");
}
}
if(!_53){
_54=this.findModule(_51,false);
if(!_54){
dojo.raise("symbol '"+_51+"' is not defined after loading '"+_55+"'");
}
}
return _54;
};
dojo.hostenv.startPackage=function(_5d){
var _5e=_5d.split(/\./);
if(_5e[_5e.length-1]=="*"){
_5e.pop();
}
return dojo.evalObjPath(_5e.join("."),true);
};
dojo.hostenv.findModule=function(_5f,_60){
var lmn=(new String(_5f)).toLowerCase();
if(this.loaded_modules_[lmn]){
return this.loaded_modules_[lmn];
}
var _62=dojo.evalObjPath(_5f);
if((_5f)&&(typeof _62!="undefined")&&(_62)){
this.loaded_modules_[lmn]=_62;
return _62;
}
if(_60){
dojo.raise("no loaded module named '"+_5f+"'");
}
return null;
};
if(typeof window=="undefined"){
dojo.raise("no window object");
}
(function(){
if(djConfig.allowQueryConfig){
var _63=document.location.toString();
var _64=_63.split("?",2);
if(_64.length>1){
var _65=_64[1];
var _66=_65.split("&");
for(var x in _66){
var sp=_66[x].split("=");
if((sp[0].length>9)&&(sp[0].substr(0,9)=="djConfig.")){
var opt=sp[0].substr(9);
try{
djConfig[opt]=eval(sp[1]);
}
catch(e){
djConfig[opt]=sp[1];
}
}
}
}
}
if(((djConfig["baseScriptUri"]=="")||(djConfig["baseRelativePath"]==""))&&(document&&document.getElementsByTagName)){
var _6a=document.getElementsByTagName("script");
var _6b=/(__package__|dojo|bootstrap1)\.js([\?\.]|$)/i;
for(var i=0;i<_6a.length;i++){
var src=_6a[i].getAttribute("src");
if(!src){
continue;
}
var m=src.match(_6b);
if(m){
root=src.substring(0,m.index);
if(src.indexOf("bootstrap1")>-1){
root+="../";
}
if(!this["djConfig"]){
djConfig={};
}
if(djConfig["baseScriptUri"]==""){
djConfig["baseScriptUri"]=root;
}
if(djConfig["baseRelativePath"]==""){
djConfig["baseRelativePath"]=root;
}
break;
}
}
}
var dr=dojo.render;
var drh=dojo.render.html;
var drs=dojo.render.svg;
var dua=drh.UA=navigator.userAgent;
var dav=drh.AV=navigator.appVersion;
var t=true;
var f=false;
drh.capable=t;
drh.support.builtin=t;
dr.ver=parseFloat(drh.AV);
dr.os.mac=dav.indexOf("Macintosh")>=0;
dr.os.win=dav.indexOf("Windows")>=0;
dr.os.linux=dav.indexOf("X11")>=0;
drh.opera=dua.indexOf("Opera")>=0;
drh.khtml=(dav.indexOf("Konqueror")>=0)||(dav.indexOf("Safari")>=0);
drh.safari=dav.indexOf("Safari")>=0;
var _76=dua.indexOf("Gecko");
drh.mozilla=drh.moz=(_76>=0)&&(!drh.khtml);
if(drh.mozilla){
drh.geckoVersion=dua.substring(_76+6,_76+14);
}
drh.ie=(document.all)&&(!drh.opera);
drh.ie50=drh.ie&&dav.indexOf("MSIE 5.0")>=0;
drh.ie55=drh.ie&&dav.indexOf("MSIE 5.5")>=0;
drh.ie60=drh.ie&&dav.indexOf("MSIE 6.0")>=0;
dr.vml.capable=drh.ie;
drs.capable=f;
drs.support.plugin=f;
drs.support.builtin=f;
drs.adobe=f;
if(document.implementation&&document.implementation.hasFeature&&document.implementation.hasFeature("org.w3c.dom.svg","1.0")){
drs.capable=t;
drs.support.builtin=t;
drs.support.plugin=f;
drs.adobe=f;
}else{
if(navigator.mimeTypes&&navigator.mimeTypes.length>0){
var _77=navigator.mimeTypes["image/svg+xml"]||navigator.mimeTypes["image/svg"]||navigator.mimeTypes["image/svg-xml"];
if(_77){
drs.adobe=_77&&_77.enabledPlugin&&_77.enabledPlugin.description&&(_77.enabledPlugin.description.indexOf("Adobe")>-1);
if(drs.adobe){
drs.capable=t;
drs.support.plugin=t;
}
}
}else{
if(drh.ie&&dr.os.win){
var _77=f;
try{
var _78=new ActiveXObject("Adobe.SVGCtl");
_77=t;
}
catch(e){
}
if(_77){
drs.capable=t;
drs.support.plugin=t;
drs.adobe=t;
}
}else{
drs.capable=f;
drs.support.plugin=f;
drs.adobe=f;
}
}
}
})();
dojo.hostenv.startPackage("dojo.hostenv");
dojo.render.name=dojo.hostenv.name_="browser";
dojo.hostenv.searchIds=[];
var DJ_XMLHTTP_PROGIDS=["Msxml2.XMLHTTP","Microsoft.XMLHTTP","Msxml2.XMLHTTP.4.0"];
dojo.hostenv.getXmlhttpObject=function(){
var _79=null;
var _7a=null;
try{
_79=new XMLHttpRequest();
}
catch(e){
}
if(!_79){
for(var i=0;i<3;++i){
var _7c=DJ_XMLHTTP_PROGIDS[i];
try{
_79=new ActiveXObject(_7c);
}
catch(e){
_7a=e;
}
if(_79){
DJ_XMLHTTP_PROGIDS=[_7c];
break;
}
}
}
if(!_79){
return dojo.raise("XMLHTTP not available",_7a);
}
return _79;
};
dojo.hostenv.getText=function(uri,_7e,_7f){
var _80=this.getXmlhttpObject();
if(_7e){
_80.onreadystatechange=function(){
if((4==_80.readyState)&&(_80["status"])){
if(_80.status==200){
_7e(_80.responseText);
}
}
};
}
_80.open("GET",uri,_7e?true:false);
try{
_80.send(null);
}
catch(e){
if(_7f&&!_7e){
return null;
}else{
throw e;
}
}
if(_7e){
return null;
}
return _80.responseText;
};
dojo.hostenv.defaultDebugContainerId="dojoDebug";
dojo.hostenv._println_buffer=[];
dojo.hostenv._println_safe=false;
dojo.hostenv.println=function(_81){
if(!dojo.hostenv._println_safe){
dojo.hostenv._println_buffer.push(_81);
}else{
try{
var _82=document.getElementById(djConfig.debugContainerId?djConfig.debugContainerId:dojo.hostenv.defaultDebugContainerId);
if(!_82){
_82=document.getElementsByTagName("body")[0]||document.body;
}
var div=document.createElement("div");
div.appendChild(document.createTextNode(_81));
_82.appendChild(div);
}
catch(e){
try{
document.write("<div>"+_81+"</div>");
}
catch(e2){
window.status=_81;
}
}
}
};
dojo.addOnLoad(function(){
dojo.hostenv._println_safe=true;
while(dojo.hostenv._println_buffer.length>0){
dojo.hostenv.println(dojo.hostenv._println_buffer.shift());
}
});
function dj_addNodeEvtHdlr(_84,_85,fp,_87){
var _88=_84["on"+_85]||function(){
};
_84["on"+_85]=function(){
fp.apply(_84,arguments);
_88.apply(_84,arguments);
};
return true;
}
dj_load_init=function(){
if(arguments.callee.initialized){
return;
}
arguments.callee.initialized=true;
if(dojo.render.html.ie){
dojo.hostenv.makeWidgets();
}
dojo.hostenv.modulesLoaded();
};
dj_addNodeEvtHdlr(window,"load",dj_load_init);
dojo.hostenv.makeWidgets=function(){
var _89=[];
if(djConfig.searchIds&&djConfig.searchIds.length>0){
_89=_89.concat(djConfig.searchIds);
}
if(dojo.hostenv.searchIds&&dojo.hostenv.searchIds.length>0){
_89=_89.concat(dojo.hostenv.searchIds);
}
if((djConfig.parseWidgets)||(_89.length>0)){
if(dojo.evalObjPath("dojo.widget.Parse")){
try{
var _8a=new dojo.xml.Parse();
if(_89.length>0){
for(var x=0;x<_89.length;x++){
var _8c=document.getElementById(_89[x]);
if(!_8c){
continue;
}
var _8d=_8a.parseElement(_8c,null,true);
dojo.widget.getParser().createComponents(_8d);
}
}else{
if(djConfig.parseWidgets){
var _8d=_8a.parseElement(document.getElementsByTagName("body")[0]||document.body,null,true);
dojo.widget.getParser().createComponents(_8d);
}
}
}
catch(e){
dojo.debug("auto-build-widgets error:",e);
}
}
}
};
dojo.hostenv.modulesLoadedListeners.push(function(){
if(!dojo.render.html.ie){
dojo.hostenv.makeWidgets();
}
});
try{
if(dojo.render.html.ie){
document.namespaces.add("v","urn:schemas-microsoft-com:vml");
document.createStyleSheet().addRule("v\\:*","behavior:url(#default#VML)");
}
}
catch(e){
}
dojo.hostenv.writeIncludes=function(){
};
dojo.hostenv.byId=dojo.byId=function(id,doc){
if(id&&(typeof id=="string"||id instanceof String)){
if(!doc){
doc=document;
}
return doc.getElementById(id);
}
return id;
};
dojo.hostenv.byIdArray=dojo.byIdArray=function(){
var ids=[];
for(var i=0;i<arguments.length;i++){
if((arguments[i] instanceof Array)||(typeof arguments[i]=="array")){
for(var j=0;j<arguments[i].length;j++){
ids=ids.concat(dojo.hostenv.byIdArray(arguments[i][j]));
}
}else{
ids.push(dojo.hostenv.byId(arguments[i]));
}
}
return ids;
};
dojo.hostenv.conditionalLoadModule=function(_93){
var _94=_93["common"]||[];
var _95=(_93[dojo.hostenv.name_])?_94.concat(_93[dojo.hostenv.name_]||[]):_94.concat(_93["default"]||[]);
for(var x=0;x<_95.length;x++){
var _97=_95[x];
if(_97.constructor==Array){
dojo.hostenv.loadModule.apply(dojo.hostenv,_97);
}else{
dojo.hostenv.loadModule(_97);
}
}
};
dojo.hostenv.require=dojo.hostenv.loadModule;
dojo.require=function(){
dojo.hostenv.loadModule.apply(dojo.hostenv,arguments);
};
dojo.requireAfter=dojo.require;
dojo.requireIf=function(){
if((arguments[0]===true)||(arguments[0]=="common")||(arguments[0]&&dojo.render[arguments[0]].capable)){
var _98=[];
for(var i=1;i<arguments.length;i++){
_98.push(arguments[i]);
}
dojo.require.apply(dojo,_98);
}
};
dojo.requireAfterIf=dojo.requireIf;
dojo.conditionalRequire=dojo.requireIf;
dojo.requireAll=function(){
for(var i=0;i<arguments.length;i++){
dojo.require(arguments[i]);
}
};
dojo.kwCompoundRequire=function(){
dojo.hostenv.conditionalLoadModule.apply(dojo.hostenv,arguments);
};
dojo.hostenv.provide=dojo.hostenv.startPackage;
dojo.provide=function(){
return dojo.hostenv.startPackage.apply(dojo.hostenv,arguments);
};
dojo.setModulePrefix=function(_9b,_9c){
return dojo.hostenv.setModulePrefix(_9b,_9c);
};
dojo.profile={start:function(){
},end:function(){
},stop:function(){
},dump:function(){
}};
dojo.exists=function(obj,_9e){
var p=_9e.split(".");
for(var i=0;i<p.length;i++){
if(!(obj[p[i]])){
return false;
}
obj=obj[p[i]];
}
return true;
};
dojo.provide("dojo.string.common");
dojo.require("dojo.string");
dojo.string.trim=function(str,wh){
if(!str.replace){
return str;
}
if(!str.length){
return str;
}
var re=(wh>0)?(/^\s+/):(wh<0)?(/\s+$/):(/^\s+|\s+$/g);
return str.replace(re,"");
};
dojo.string.trimStart=function(str){
return dojo.string.trim(str,1);
};
dojo.string.trimEnd=function(str){
return dojo.string.trim(str,-1);
};
dojo.string.repeat=function(str,_a7,_a8){
var out="";
for(var i=0;i<_a7;i++){
out+=str;
if(_a8&&i<_a7-1){
out+=_a8;
}
}
return out;
};
dojo.string.pad=function(str,len,c,dir){
var out=String(str);
if(!c){
c="0";
}
if(!dir){
dir=1;
}
while(out.length<len){
if(dir>0){
out=c+out;
}else{
out+=c;
}
}
return out;
};
dojo.string.padLeft=function(str,len,c){
return dojo.string.pad(str,len,c,1);
};
dojo.string.padRight=function(str,len,c){
return dojo.string.pad(str,len,c,-1);
};
dojo.provide("dojo.string");
dojo.require("dojo.string.common");
dojo.provide("dojo.lang.common");
dojo.require("dojo.lang");
dojo.lang.mixin=function(obj,_b7){
var _b8={};
for(var x in _b7){
if(typeof _b8[x]=="undefined"||_b8[x]!=_b7[x]){
obj[x]=_b7[x];
}
}
if(dojo.render.html.ie&&dojo.lang.isFunction(_b7["toString"])&&_b7["toString"]!=obj["toString"]){
obj.toString=_b7.toString;
}
return obj;
};
dojo.lang.extend=function(_ba,_bb){
this.mixin(_ba.prototype,_bb);
};
dojo.lang.find=function(arr,val,_be,_bf){
if(!dojo.lang.isArrayLike(arr)&&dojo.lang.isArrayLike(val)){
var a=arr;
arr=val;
val=a;
}
var _c1=dojo.lang.isString(arr);
if(_c1){
arr=arr.split("");
}
if(_bf){
var _c2=-1;
var i=arr.length-1;
var end=-1;
}else{
var _c2=1;
var i=0;
var end=arr.length;
}
if(_be){
while(i!=end){
if(arr[i]===val){
return i;
}
i+=_c2;
}
}else{
while(i!=end){
if(arr[i]==val){
return i;
}
i+=_c2;
}
}
return -1;
};
dojo.lang.indexOf=dojo.lang.find;
dojo.lang.findLast=function(arr,val,_c7){
return dojo.lang.find(arr,val,_c7,true);
};
dojo.lang.lastIndexOf=dojo.lang.findLast;
dojo.lang.inArray=function(arr,val){
return dojo.lang.find(arr,val)>-1;
};
dojo.lang.isObject=function(wh){
return typeof wh=="object"||dojo.lang.isArray(wh)||dojo.lang.isFunction(wh);
};
dojo.lang.isArray=function(wh){
return (wh instanceof Array||typeof wh=="array");
};
dojo.lang.isArrayLike=function(wh){
if(dojo.lang.isString(wh)){
return false;
}
if(dojo.lang.isFunction(wh)){
return false;
}
if(dojo.lang.isArray(wh)){
return true;
}
if(typeof wh!="undefined"&&wh&&dojo.lang.isNumber(wh.length)&&isFinite(wh.length)){
return true;
}
return false;
};
dojo.lang.isFunction=function(wh){
return (wh instanceof Function||typeof wh=="function");
};
dojo.lang.isString=function(wh){
return (wh instanceof String||typeof wh=="string");
};
dojo.lang.isAlien=function(wh){
return !dojo.lang.isFunction()&&/\{\s*\[native code\]\s*\}/.test(String(wh));
};
dojo.lang.isBoolean=function(wh){
return (wh instanceof Boolean||typeof wh=="boolean");
};
dojo.lang.isNumber=function(wh){
return (wh instanceof Number||typeof wh=="number");
};
dojo.lang.isUndefined=function(wh){
return ((wh==undefined)&&(typeof wh=="undefined"));
};
dojo.provide("dojo.lang.type");
dojo.require("dojo.lang.common");
dojo.lang.whatAmI=function(wh){
try{
if(dojo.lang.isArray(wh)){
return "array";
}
if(dojo.lang.isFunction(wh)){
return "function";
}
if(dojo.lang.isString(wh)){
return "string";
}
if(dojo.lang.isNumber(wh)){
return "number";
}
if(dojo.lang.isBoolean(wh)){
return "boolean";
}
if(dojo.lang.isAlien(wh)){
return "alien";
}
if(dojo.lang.isUndefined(wh)){
return "undefined";
}
for(var _d4 in dojo.lang.whatAmI.custom){
if(dojo.lang.whatAmI.custom[_d4](wh)){
return _d4;
}
}
if(dojo.lang.isObject(wh)){
return "object";
}
}
catch(E){
}
return "unknown";
};
dojo.lang.whatAmI.custom={};
dojo.lang.isNumeric=function(wh){
return (!isNaN(wh)&&isFinite(wh)&&(wh!=null)&&!dojo.lang.isBoolean(wh)&&!dojo.lang.isArray(wh));
};
dojo.lang.isBuiltIn=function(wh){
return (dojo.lang.isArray(wh)||dojo.lang.isFunction(wh)||dojo.lang.isString(wh)||dojo.lang.isNumber(wh)||dojo.lang.isBoolean(wh)||(wh==null)||(wh instanceof Error)||(typeof wh=="error"));
};
dojo.lang.isPureObject=function(wh){
return ((wh!=null)&&dojo.lang.isObject(wh)&&wh.constructor==Object);
};
dojo.lang.isOfType=function(_d8,_d9){
if(dojo.lang.isArray(_d9)){
var _da=_d9;
for(var i in _da){
var _dc=_da[i];
if(dojo.lang.isOfType(_d8,_dc)){
return true;
}
}
return false;
}else{
if(dojo.lang.isString(_d9)){
_d9=_d9.toLowerCase();
}
switch(_d9){
case Array:
case "array":
return dojo.lang.isArray(_d8);
break;
case Function:
case "function":
return dojo.lang.isFunction(_d8);
break;
case String:
case "string":
return dojo.lang.isString(_d8);
break;
case Number:
case "number":
return dojo.lang.isNumber(_d8);
break;
case "numeric":
return dojo.lang.isNumeric(_d8);
break;
case Boolean:
case "boolean":
return dojo.lang.isBoolean(_d8);
break;
case Object:
case "object":
return dojo.lang.isObject(_d8);
break;
case "pureobject":
return dojo.lang.isPureObject(_d8);
break;
case "builtin":
return dojo.lang.isBuiltIn(_d8);
break;
case "alien":
return dojo.lang.isAlien(_d8);
break;
case "undefined":
return dojo.lang.isUndefined(_d8);
break;
case null:
case "null":
return (_d8===null);
break;
case "optional":
return ((_d8===null)||dojo.lang.isUndefined(_d8));
break;
default:
if(dojo.lang.isFunction(_d9)){
return (_d8 instanceof _d9);
}else{
dojo.raise("dojo.lang.isOfType() was passed an invalid type");
}
break;
}
}
dojo.raise("If we get here, it means a bug was introduced above.");
};
dojo.lang.getObject=function(str){
var _de=str.split("."),i=0,obj=dj_global;
do{
obj=obj[_de[i++]];
}while(i<_de.length&&obj);
return (obj!=dj_global)?obj:null;
};
dojo.lang.doesObjectExist=function(str){
var _e0=str.split("."),i=0,obj=dj_global;
do{
obj=obj[_e0[i++]];
}while(i<_e0.length&&obj);
return (obj&&obj!=dj_global);
};
dojo.lang.getConstructor=function(obj){
return obj.constructor;
};
dojo.lang.isConstructedBy=function(obj,_e3){
return dojo.lang.getConstructor(obj)==_e3;
};
dojo.lang.isSubOf=function(obj,_e5){
return obj instanceof _e5;
};
dojo.lang.isBaseOf=function(_e6,obj){
return obj instanceof _e6;
};
dojo.lang.createInstance=function(_e8){
var o=null;
var f=_e8;
if(typeof (f)=="string"){
f=dojo.lang.getObject(_e8);
}
if(typeof (f)=="function"){
try{
o=new f();
}
catch(e){
}
}
return o;
};
dojo.provide("dojo.lang.extras");
dojo.require("dojo.lang.common");
dojo.require("dojo.lang.type");
dojo.lang.setTimeout=function(_eb,_ec){
var _ed=window,argsStart=2;
if(!dojo.lang.isFunction(_eb)){
_ed=_eb;
_eb=_ec;
_ec=arguments[2];
argsStart++;
}
if(dojo.lang.isString(_eb)){
_eb=_ed[_eb];
}
var _ee=[];
for(var i=argsStart;i<arguments.length;i++){
_ee.push(arguments[i]);
}
return setTimeout(function(){
_eb.apply(_ed,_ee);
},_ec);
};
dojo.lang.getNameInObj=function(ns,_f1){
if(!ns){
ns=dj_global;
}
for(var x in ns){
if(ns[x]===_f1){
return new String(x);
}
}
return null;
};
dojo.lang.shallowCopy=function(obj){
var ret={},key;
for(key in obj){
if(dojo.lang.isUndefined(ret[key])){
ret[key]=obj[key];
}
}
return ret;
};
dojo.lang.firstValued=function(){
for(var i=0;i<arguments.length;i++){
if(typeof arguments[i]!="undefined"){
return arguments[i];
}
}
return undefined;
};
dojo.lang.getObjPathValue=function(_f6,_f7,_f8){
with(dojo.parseObjPath(_f6,_f7,_f8)){
return dojo.evalProp(prop,obj,_f8);
}
};
dojo.lang.setObjPathValue=function(_f9,_fa,_fb,_fc){
if(arguments.length<4){
_fc=true;
}
with(dojo.parseObjPath(_f9,_fb,_fc)){
if(obj&&(_fc||(prop in obj))){
obj[prop]=_fa;
}
}
};
dojo.provide("dojo.io.IO");
dojo.require("dojo.string");
dojo.require("dojo.lang.extras");
dojo.io.transports=[];
dojo.io.hdlrFuncNames=["load","error","timeout"];
dojo.io.Request=function(url,_fe,_ff,_100){
if((arguments.length==1)&&(arguments[0].constructor==Object)){
this.fromKwArgs(arguments[0]);
}else{
this.url=url;
if(_fe){
this.mimetype=_fe;
}
if(_ff){
this.transport=_ff;
}
if(arguments.length>=4){
this.changeUrl=_100;
}
}
};
dojo.lang.extend(dojo.io.Request,{url:"",mimetype:"text/plain",method:"GET",content:undefined,transport:undefined,changeUrl:undefined,formNode:undefined,sync:false,bindSuccess:false,useCache:false,preventCache:false,load:function(type,data,evt){
},error:function(type,_105){
},timeout:function(type){
},handle:function(){
},timeoutSeconds:0,abort:function(){
},fromKwArgs:function(_107){
if(_107["url"]){
_107.url=_107.url.toString();
}
if(_107["formNode"]){
_107.formNode=dojo.byId(_107.formNode);
}
if(!_107["method"]&&_107["formNode"]&&_107["formNode"].method){
_107.method=_107["formNode"].method;
}
if(!_107["handle"]&&_107["handler"]){
_107.handle=_107.handler;
}
if(!_107["load"]&&_107["loaded"]){
_107.load=_107.loaded;
}
if(!_107["changeUrl"]&&_107["changeURL"]){
_107.changeUrl=_107.changeURL;
}
_107.encoding=dojo.lang.firstValued(_107["encoding"],djConfig["bindEncoding"],"");
_107.sendTransport=dojo.lang.firstValued(_107["sendTransport"],djConfig["ioSendTransport"],false);
var _108=dojo.lang.isFunction;
for(var x=0;x<dojo.io.hdlrFuncNames.length;x++){
var fn=dojo.io.hdlrFuncNames[x];
if(_108(_107[fn])){
continue;
}
if(_108(_107["handle"])){
_107[fn]=_107.handle;
}
}
dojo.lang.mixin(this,_107);
}});
dojo.io.Error=function(msg,type,num){
this.message=msg;
this.type=type||"unknown";
this.number=num||0;
};
dojo.io.transports.addTransport=function(name){
this.push(name);
this[name]=dojo.io[name];
};
dojo.io.bind=function(_10f){
if(!(_10f instanceof dojo.io.Request)){
try{
_10f=new dojo.io.Request(_10f);
}
catch(e){
dojo.debug(e);
}
}
var _110="";
if(_10f["transport"]){
_110=_10f["transport"];
if(!this[_110]){
return _10f;
}
}else{
for(var x=0;x<dojo.io.transports.length;x++){
var tmp=dojo.io.transports[x];
if((this[tmp])&&(this[tmp].canHandle(_10f))){
_110=tmp;
}
}
if(_110==""){
return _10f;
}
}
this[_110].bind(_10f);
_10f.bindSuccess=true;
return _10f;
};
dojo.io.queueBind=function(_113){
if(!(_113 instanceof dojo.io.Request)){
try{
_113=new dojo.io.Request(_113);
}
catch(e){
dojo.debug(e);
}
}
var _114=_113.load;
_113.load=function(){
dojo.io._queueBindInFlight=false;
var ret=_114.apply(this,arguments);
dojo.io._dispatchNextQueueBind();
return ret;
};
var _116=_113.error;
_113.error=function(){
dojo.io._queueBindInFlight=false;
var ret=_116.apply(this,arguments);
dojo.io._dispatchNextQueueBind();
return ret;
};
dojo.io._bindQueue.push(_113);
dojo.io._dispatchNextQueueBind();
return _113;
};
dojo.io._dispatchNextQueueBind=function(){
if(!dojo.io._queueBindInFlight){
dojo.io._queueBindInFlight=true;
if(dojo.io._bindQueue.length>0){
dojo.io.bind(dojo.io._bindQueue.shift());
}else{
dojo.io._queueBindInFlight=false;
}
}
};
dojo.io._bindQueue=[];
dojo.io._queueBindInFlight=false;
dojo.io.argsFromMap=function(map,_119){
var enc=/utf/i.test(_119||"")?encodeURIComponent:dojo.string.encodeAscii;
var _11b=[];
var _11c=function(elt){
_11b.push(enc(name)+"="+enc(elt));
};
var _11e=new Object();
for(var name in map){
if(!_11e[name]){
var _120=map[name];
if(dojo.lang.isArray(_120)){
dojo.lang.forEach(_120,_11c);
}else{
_11c(_120);
}
}
}
return _11b.join("&");
};
dojo.io.setIFrameSrc=function(_121,src,_123){
try{
var r=dojo.render.html;
if(!_123){
if(r.safari){
_121.location=src;
}else{
frames[_121.name].location=src;
}
}else{
var idoc;
if(r.ie){
idoc=_121.contentWindow.document;
}else{
if(r.moz){
idoc=_121.contentWindow;
}else{
if(r.safari){
idoc=_121.document;
}
}
}
idoc.location.replace(src);
}
}
catch(e){
dojo.debug(e);
dojo.debug("setIFrameSrc: "+e);
}
};
dojo.provide("dojo.lang.array");
dojo.require("dojo.lang.common");
dojo.lang.has=function(obj,name){
try{
return (typeof obj[name]!="undefined");
}
catch(e){
return false;
}
};
dojo.lang.isEmpty=function(obj){
if(dojo.lang.isObject(obj)){
var tmp={};
var _12a=0;
for(var x in obj){
if(obj[x]&&(!tmp[x])){
_12a++;
break;
}
}
return (_12a==0);
}else{
if(dojo.lang.isArrayLike(obj)||dojo.lang.isString(obj)){
return obj.length==0;
}
}
};
dojo.lang.map=function(arr,obj,_12e){
var _12f=dojo.lang.isString(arr);
if(_12f){
arr=arr.split("");
}
if(dojo.lang.isFunction(obj)&&(!_12e)){
_12e=obj;
obj=dj_global;
}else{
if(dojo.lang.isFunction(obj)&&_12e){
var _130=obj;
obj=_12e;
_12e=_130;
}
}
if(Array.map){
var _131=Array.map(arr,_12e,obj);
}else{
var _131=[];
for(var i=0;i<arr.length;++i){
_131.push(_12e.call(obj,arr[i]));
}
}
if(_12f){
return _131.join("");
}else{
return _131;
}
};
dojo.lang.forEach=function(_133,_134,_135){
if(dojo.lang.isString(_133)){
_133=_133.split("");
}
if(Array.forEach){
Array.forEach(_133,_134,_135);
}else{
if(!_135){
_135=dj_global;
}
for(var i=0,l=_133.length;i<l;i++){
_134.call(_135,_133[i],i,_133);
}
}
};
dojo.lang._everyOrSome=function(_137,arr,_139,_13a){
if(dojo.lang.isString(arr)){
arr=arr.split("");
}
if(Array.every){
return Array[(_137)?"every":"some"](arr,_139,_13a);
}else{
if(!_13a){
_13a=dj_global;
}
for(var i=0,l=arr.length;i<l;i++){
var _13c=_139.call(_13a,arr[i],i,arr);
if((_137)&&(!_13c)){
return false;
}else{
if((!_137)&&(_13c)){
return true;
}
}
}
return (_137)?true:false;
}
};
dojo.lang.every=function(arr,_13e,_13f){
return this._everyOrSome(true,arr,_13e,_13f);
};
dojo.lang.some=function(arr,_141,_142){
return this._everyOrSome(false,arr,_141,_142);
};
dojo.lang.filter=function(arr,_144,_145){
var _146=dojo.lang.isString(arr);
if(_146){
arr=arr.split("");
}
if(Array.filter){
var _147=Array.filter(arr,_144,_145);
}else{
if(!_145){
if(arguments.length>=3){
dojo.raise("thisObject doesn't exist!");
}
_145=dj_global;
}
var _147=[];
for(var i=0;i<arr.length;i++){
if(_144.call(_145,arr[i],i,arr)){
_147.push(arr[i]);
}
}
}
if(_146){
return _147.join("");
}else{
return _147;
}
};
dojo.lang.unnest=function(){
var out=[];
for(var i=0;i<arguments.length;i++){
if(dojo.lang.isArrayLike(arguments[i])){
var add=dojo.lang.unnest.apply(this,arguments[i]);
out=out.concat(add);
}else{
out.push(arguments[i]);
}
}
return out;
};
dojo.lang.toArray=function(_14c,_14d){
var _14e=[];
for(var i=_14d||0;i<_14c.length;i++){
_14e.push(_14c[i]);
}
return _14e;
};
dojo.provide("dojo.lang.func");
dojo.require("dojo.lang.common");
dojo.require("dojo.lang.type");
dojo.lang.hitch=function(_150,_151){
if(dojo.lang.isString(_151)){
var fcn=_150[_151];
}else{
var fcn=_151;
}
return function(){
return fcn.apply(_150,arguments);
};
};
dojo.lang.anonCtr=0;
dojo.lang.anon={};
dojo.lang.nameAnonFunc=function(_153,_154){
var nso=(_154||dojo.lang.anon);
if((dj_global["djConfig"])&&(djConfig["slowAnonFuncLookups"]==true)){
for(var x in nso){
if(nso[x]===_153){
return x;
}
}
}
var ret="__"+dojo.lang.anonCtr++;
while(typeof nso[ret]!="undefined"){
ret="__"+dojo.lang.anonCtr++;
}
nso[ret]=_153;
return ret;
};
dojo.lang.forward=function(_158){
return function(){
return this[_158].apply(this,arguments);
};
};
dojo.lang.curry=function(ns,func){
var _15b=[];
ns=ns||dj_global;
if(dojo.lang.isString(func)){
func=ns[func];
}
for(var x=2;x<arguments.length;x++){
_15b.push(arguments[x]);
}
var _15d=func.length-_15b.length;
function gather(_15e,_15f,_160){
var _161=_160;
var _162=_15f.slice(0);
for(var x=0;x<_15e.length;x++){
_162.push(_15e[x]);
}
_160=_160-_15e.length;
if(_160<=0){
var res=func.apply(ns,_162);
_160=_161;
return res;
}else{
return function(){
return gather(arguments,_162,_160);
};
}
}
return gather([],_15b,_15d);
};
dojo.lang.curryArguments=function(ns,func,args,_168){
var _169=[];
var x=_168||0;
for(x=_168;x<args.length;x++){
_169.push(args[x]);
}
return dojo.lang.curry.apply(dojo.lang,[ns,func].concat(_169));
};
dojo.lang.tryThese=function(){
for(var x=0;x<arguments.length;x++){
try{
if(typeof arguments[x]=="function"){
var ret=(arguments[x]());
if(ret){
return ret;
}
}
}
catch(e){
dojo.debug(e);
}
}
};
dojo.lang.delayThese=function(farr,cb,_16f,_170){
if(!farr.length){
if(typeof _170=="function"){
_170();
}
return;
}
if((typeof _16f=="undefined")&&(typeof cb=="number")){
_16f=cb;
cb=function(){
};
}else{
if(!cb){
cb=function(){
};
if(!_16f){
_16f=0;
}
}
}
setTimeout(function(){
(farr.shift())();
cb();
dojo.lang.delayThese(farr,cb,_16f,_170);
},_16f);
};
dojo.provide("dojo.string.extras");
dojo.require("dojo.string.common");
dojo.require("dojo.lang");
dojo.string.paramString=function(str,_172,_173){
for(var name in _172){
var re=new RegExp("\\%\\{"+name+"\\}","g");
str=str.replace(re,_172[name]);
}
if(_173){
str=str.replace(/%\{([^\}\s]+)\}/g,"");
}
return str;
};
dojo.string.capitalize=function(str){
if(!dojo.lang.isString(str)){
return "";
}
if(arguments.length==0){
str=this;
}
var _177=str.split(" ");
var _178="";
var len=_177.length;
for(var i=0;i<len;i++){
var word=_177[i];
word=word.charAt(0).toUpperCase()+word.substring(1,word.length);
_178+=word;
if(i<len-1){
_178+=" ";
}
}
return new String(_178);
};
dojo.string.isBlank=function(str){
if(!dojo.lang.isString(str)){
return true;
}
return (dojo.string.trim(str).length==0);
};
dojo.string.encodeAscii=function(str){
if(!dojo.lang.isString(str)){
return str;
}
var ret="";
var _17f=escape(str);
var _180,re=/%u([0-9A-F]{4})/i;
while((_180=_17f.match(re))){
var num=Number("0x"+_180[1]);
var _182=escape("&#"+num+";");
ret+=_17f.substring(0,_180.index)+_182;
_17f=_17f.substring(_180.index+_180[0].length);
}
ret+=_17f.replace(/\+/g,"%2B");
return ret;
};
dojo.string.escape=function(type,str){
var args=[];
for(var i=1;i<arguments.length;i++){
args.push(arguments[i]);
}
switch(type.toLowerCase()){
case "xml":
case "html":
case "xhtml":
return dojo.string.escapeXml.apply(this,args);
case "sql":
return dojo.string.escapeSql.apply(this,args);
case "regexp":
case "regex":
return dojo.string.escapeRegExp.apply(this,args);
case "javascript":
case "jscript":
case "js":
return dojo.string.escapeJavaScript.apply(this,args);
case "ascii":
return dojo.string.encodeAscii.apply(this,args);
default:
return str;
}
};
dojo.string.escapeXml=function(str,_188){
str=str.replace(/&/gm,"&amp;").replace(/</gm,"&lt;").replace(/>/gm,"&gt;").replace(/"/gm,"&quot;");
if(!_188){
str=str.replace(/'/gm,"&#39;");
}
return str;
};
dojo.string.escapeSql=function(str){
return str.replace(/'/gm,"''");
};
dojo.string.escapeRegExp=function(str){
return str.replace(/\\/gm,"\\\\").replace(/([\f\b\n\t\r[\^$|?*+(){}])/gm,"\\$1");
};
dojo.string.escapeJavaScript=function(str){
return str.replace(/(["'\f\b\n\t\r])/gm,"\\$1");
};
dojo.string.summary=function(str,len){
if(!len||str.length<=len){
return str;
}else{
return str.substring(0,len).replace(/\.+$/,"")+"...";
}
};
dojo.string.endsWith=function(str,end,_190){
if(_190){
str=str.toLowerCase();
end=end.toLowerCase();
}
if((str.length-end.length)<0){
return false;
}
return str.lastIndexOf(end)==str.length-end.length;
};
dojo.string.endsWithAny=function(str){
for(var i=1;i<arguments.length;i++){
if(dojo.string.endsWith(str,arguments[i])){
return true;
}
}
return false;
};
dojo.string.startsWith=function(str,_194,_195){
if(_195){
str=str.toLowerCase();
_194=_194.toLowerCase();
}
return str.indexOf(_194)==0;
};
dojo.string.startsWithAny=function(str){
for(var i=1;i<arguments.length;i++){
if(dojo.string.startsWith(str,arguments[i])){
return true;
}
}
return false;
};
dojo.string.has=function(str){
for(var i=1;i<arguments.length;i++){
if(str.indexOf(arguments[i])>-1){
return true;
}
}
return false;
};
dojo.string.normalizeNewlines=function(text,_19b){
if(_19b=="\n"){
text=text.replace(/\r\n/g,"\n");
text=text.replace(/\r/g,"\n");
}else{
if(_19b=="\r"){
text=text.replace(/\r\n/g,"\r");
text=text.replace(/\n/g,"\r");
}else{
text=text.replace(/([^\r])\n/g,"$1\r\n");
text=text.replace(/\r([^\n])/g,"\r\n$1");
}
}
return text;
};
dojo.string.splitEscaped=function(str,_19d){
var _19e=[];
for(var i=0,prevcomma=0;i<str.length;i++){
if(str.charAt(i)=="\\"){
i++;
continue;
}
if(str.charAt(i)==_19d){
_19e.push(str.substring(prevcomma,i));
prevcomma=i+1;
}
}
_19e.push(str.substr(prevcomma));
return _19e;
};
dojo.provide("dojo.dom");
dojo.require("dojo.lang.array");
dojo.dom.ELEMENT_NODE=1;
dojo.dom.ATTRIBUTE_NODE=2;
dojo.dom.TEXT_NODE=3;
dojo.dom.CDATA_SECTION_NODE=4;
dojo.dom.ENTITY_REFERENCE_NODE=5;
dojo.dom.ENTITY_NODE=6;
dojo.dom.PROCESSING_INSTRUCTION_NODE=7;
dojo.dom.COMMENT_NODE=8;
dojo.dom.DOCUMENT_NODE=9;
dojo.dom.DOCUMENT_TYPE_NODE=10;
dojo.dom.DOCUMENT_FRAGMENT_NODE=11;
dojo.dom.NOTATION_NODE=12;
dojo.dom.dojoml="http://www.dojotoolkit.org/2004/dojoml";
dojo.dom.xmlns={svg:"http://www.w3.org/2000/svg",smil:"http://www.w3.org/2001/SMIL20/",mml:"http://www.w3.org/1998/Math/MathML",cml:"http://www.xml-cml.org",xlink:"http://www.w3.org/1999/xlink",xhtml:"http://www.w3.org/1999/xhtml",xul:"http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul",xbl:"http://www.mozilla.org/xbl",fo:"http://www.w3.org/1999/XSL/Format",xsl:"http://www.w3.org/1999/XSL/Transform",xslt:"http://www.w3.org/1999/XSL/Transform",xi:"http://www.w3.org/2001/XInclude",xforms:"http://www.w3.org/2002/01/xforms",saxon:"http://icl.com/saxon",xalan:"http://xml.apache.org/xslt",xsd:"http://www.w3.org/2001/XMLSchema",dt:"http://www.w3.org/2001/XMLSchema-datatypes",xsi:"http://www.w3.org/2001/XMLSchema-instance",rdf:"http://www.w3.org/1999/02/22-rdf-syntax-ns#",rdfs:"http://www.w3.org/2000/01/rdf-schema#",dc:"http://purl.org/dc/elements/1.1/",dcq:"http://purl.org/dc/qualifiers/1.0","soap-env":"http://schemas.xmlsoap.org/soap/envelope/",wsdl:"http://schemas.xmlsoap.org/wsdl/",AdobeExtensions:"http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/"};
dojo.dom.isNode=function(wh){
if(typeof Element=="object"){
try{
return wh instanceof Element;
}
catch(E){
}
}else{
return wh&&!isNaN(wh.nodeType);
}
};
dojo.dom.getTagName=function(node){
dojo.deprecated("dojo.dom.getTagName","use node.tagName instead","0.4");
var _1a2=node.tagName;
if(_1a2.substr(0,5).toLowerCase()!="dojo:"){
if(_1a2.substr(0,4).toLowerCase()=="dojo"){
return "dojo:"+_1a2.substring(4).toLowerCase();
}
var djt=node.getAttribute("dojoType")||node.getAttribute("dojotype");
if(djt){
return "dojo:"+djt.toLowerCase();
}
if((node.getAttributeNS)&&(node.getAttributeNS(this.dojoml,"type"))){
return "dojo:"+node.getAttributeNS(this.dojoml,"type").toLowerCase();
}
try{
djt=node.getAttribute("dojo:type");
}
catch(e){
}
if(djt){
return "dojo:"+djt.toLowerCase();
}
if((!dj_global["djConfig"])||(!djConfig["ignoreClassNames"])){
var _1a4=node.className||node.getAttribute("class");
if((_1a4)&&(_1a4.indexOf)&&(_1a4.indexOf("dojo-")!=-1)){
var _1a5=_1a4.split(" ");
for(var x=0;x<_1a5.length;x++){
if((_1a5[x].length>5)&&(_1a5[x].indexOf("dojo-")>=0)){
return "dojo:"+_1a5[x].substr(5).toLowerCase();
}
}
}
}
}
return _1a2.toLowerCase();
};
dojo.dom.getUniqueId=function(){
do{
var id="dj_unique_"+(++arguments.callee._idIncrement);
}while(document.getElementById(id));
return id;
};
dojo.dom.getUniqueId._idIncrement=0;
dojo.dom.firstElement=dojo.dom.getFirstChildElement=function(_1a8,_1a9){
var node=_1a8.firstChild;
while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE){
node=node.nextSibling;
}
if(_1a9&&node&&node.tagName&&node.tagName.toLowerCase()!=_1a9.toLowerCase()){
node=dojo.dom.nextElement(node,_1a9);
}
return node;
};
dojo.dom.lastElement=dojo.dom.getLastChildElement=function(_1ab,_1ac){
var node=_1ab.lastChild;
while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE){
node=node.previousSibling;
}
if(_1ac&&node&&node.tagName&&node.tagName.toLowerCase()!=_1ac.toLowerCase()){
node=dojo.dom.prevElement(node,_1ac);
}
return node;
};
dojo.dom.nextElement=dojo.dom.getNextSiblingElement=function(node,_1af){
if(!node){
return null;
}
do{
node=node.nextSibling;
}while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE);
if(node&&_1af&&_1af.toLowerCase()!=node.tagName.toLowerCase()){
return dojo.dom.nextElement(node,_1af);
}
return node;
};
dojo.dom.prevElement=dojo.dom.getPreviousSiblingElement=function(node,_1b1){
if(!node){
return null;
}
if(_1b1){
_1b1=_1b1.toLowerCase();
}
do{
node=node.previousSibling;
}while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE);
if(node&&_1b1&&_1b1.toLowerCase()!=node.tagName.toLowerCase()){
return dojo.dom.prevElement(node,_1b1);
}
return node;
};
dojo.dom.moveChildren=function(_1b2,_1b3,trim){
var _1b5=0;
if(trim){
while(_1b2.hasChildNodes()&&_1b2.firstChild.nodeType==dojo.dom.TEXT_NODE){
_1b2.removeChild(_1b2.firstChild);
}
while(_1b2.hasChildNodes()&&_1b2.lastChild.nodeType==dojo.dom.TEXT_NODE){
_1b2.removeChild(_1b2.lastChild);
}
}
while(_1b2.hasChildNodes()){
_1b3.appendChild(_1b2.firstChild);
_1b5++;
}
return _1b5;
};
dojo.dom.copyChildren=function(_1b6,_1b7,trim){
var _1b9=_1b6.cloneNode(true);
return this.moveChildren(_1b9,_1b7,trim);
};
dojo.dom.removeChildren=function(node){
var _1bb=node.childNodes.length;
while(node.hasChildNodes()){
node.removeChild(node.firstChild);
}
return _1bb;
};
dojo.dom.replaceChildren=function(node,_1bd){
dojo.dom.removeChildren(node);
node.appendChild(_1bd);
};
dojo.dom.removeNode=function(node){
if(node&&node.parentNode){
return node.parentNode.removeChild(node);
}
};
dojo.dom.getAncestors=function(node,_1c0,_1c1){
var _1c2=[];
var _1c3=dojo.lang.isFunction(_1c0);
while(node){
if(!_1c3||_1c0(node)){
_1c2.push(node);
}
if(_1c1&&_1c2.length>0){
return _1c2[0];
}
node=node.parentNode;
}
if(_1c1){
return null;
}
return _1c2;
};
dojo.dom.getAncestorsByTag=function(node,tag,_1c6){
tag=tag.toLowerCase();
return dojo.dom.getAncestors(node,function(el){
return ((el.tagName)&&(el.tagName.toLowerCase()==tag));
},_1c6);
};
dojo.dom.getFirstAncestorByTag=function(node,tag){
return dojo.dom.getAncestorsByTag(node,tag,true);
};
dojo.dom.isDescendantOf=function(node,_1cb,_1cc){
if(_1cc&&node){
node=node.parentNode;
}
while(node){
if(node==_1cb){
return true;
}
node=node.parentNode;
}
return false;
};
dojo.dom.innerXML=function(node){
if(node.innerXML){
return node.innerXML;
}else{
if(typeof XMLSerializer!="undefined"){
return (new XMLSerializer()).serializeToString(node);
}
}
};
dojo.dom.createDocumentFromText=function(str,_1cf){
if(!_1cf){
_1cf="text/xml";
}
if(typeof DOMParser!="undefined"){
var _1d0=new DOMParser();
return _1d0.parseFromString(str,_1cf);
}else{
if(typeof ActiveXObject!="undefined"){
var _1d1=new ActiveXObject("Microsoft.XMLDOM");
if(_1d1){
_1d1.async=false;
_1d1.loadXML(str);
return _1d1;
}else{
dojo.debug("toXml didn't work?");
}
}else{
if(document.createElement){
var tmp=document.createElement("xml");
tmp.innerHTML=str;
if(document.implementation&&document.implementation.createDocument){
var _1d3=document.implementation.createDocument("foo","",null);
for(var i=0;i<tmp.childNodes.length;i++){
_1d3.importNode(tmp.childNodes.item(i),true);
}
return _1d3;
}
return tmp.document&&tmp.document.firstChild?tmp.document.firstChild:tmp;
}
}
}
return null;
};
dojo.dom.prependChild=function(node,_1d6){
if(_1d6.firstChild){
_1d6.insertBefore(node,_1d6.firstChild);
}else{
_1d6.appendChild(node);
}
return true;
};
dojo.dom.insertBefore=function(node,ref,_1d9){
if(_1d9!=true&&(node===ref||node.nextSibling===ref)){
return false;
}
var _1da=ref.parentNode;
_1da.insertBefore(node,ref);
return true;
};
dojo.dom.insertAfter=function(node,ref,_1dd){
var pn=ref.parentNode;
if(ref==pn.lastChild){
if((_1dd!=true)&&(node===ref)){
return false;
}
pn.appendChild(node);
}else{
return this.insertBefore(node,ref.nextSibling,_1dd);
}
return true;
};
dojo.dom.insertAtPosition=function(node,ref,_1e1){
if((!node)||(!ref)||(!_1e1)){
return false;
}
switch(_1e1.toLowerCase()){
case "before":
return dojo.dom.insertBefore(node,ref);
case "after":
return dojo.dom.insertAfter(node,ref);
case "first":
if(ref.firstChild){
return dojo.dom.insertBefore(node,ref.firstChild);
}else{
ref.appendChild(node);
return true;
}
break;
default:
ref.appendChild(node);
return true;
}
};
dojo.dom.insertAtIndex=function(node,_1e3,_1e4){
var _1e5=_1e3.childNodes;
if(!_1e5.length){
_1e3.appendChild(node);
return true;
}
var _1e6=null;
for(var i=0;i<_1e5.length;i++){
var _1e8=_1e5.item(i)["getAttribute"]?parseInt(_1e5.item(i).getAttribute("dojoinsertionindex")):-1;
if(_1e8<_1e4){
_1e6=_1e5.item(i);
}
}
if(_1e6){
return dojo.dom.insertAfter(node,_1e6);
}else{
return dojo.dom.insertBefore(node,_1e5.item(0));
}
};
dojo.dom.textContent=function(node,text){
if(text){
dojo.dom.replaceChildren(node,document.createTextNode(text));
return text;
}else{
var _1eb="";
if(node==null){
return _1eb;
}
for(var i=0;i<node.childNodes.length;i++){
switch(node.childNodes[i].nodeType){
case 1:
case 5:
_1eb+=dojo.dom.textContent(node.childNodes[i]);
break;
case 3:
case 2:
case 4:
_1eb+=node.childNodes[i].nodeValue;
break;
default:
break;
}
}
return _1eb;
}
};
dojo.dom.collectionToArray=function(_1ed){
dojo.deprecated("dojo.dom.collectionToArray","use dojo.lang.toArray instead","0.4");
return dojo.lang.toArray(_1ed);
};
dojo.dom.hasParent=function(node){
return node&&node.parentNode&&dojo.dom.isNode(node.parentNode);
};
dojo.dom.isTag=function(node){
if(node&&node.tagName){
var arr=dojo.lang.toArray(arguments,1);
return arr[dojo.lang.find(node.tagName,arr)]||"";
}
return "";
};
dojo.provide("dojo.undo.browser");
dojo.require("dojo.io");
try{
if((!djConfig["preventBackButtonFix"])&&(!dojo.hostenv.post_load_)){
document.write("<iframe style='border: 0px; width: 1px; height: 1px; position: absolute; bottom: 0px; right: 0px; visibility: visible;' name='djhistory' id='djhistory' src='"+(dojo.hostenv.getBaseScriptUri()+"iframe_history.html")+"'></iframe>");
}
}
catch(e){
}
dojo.undo.browser={initialHref:window.location.href,initialHash:window.location.hash,moveForward:false,historyStack:[],forwardStack:[],historyIframe:null,bookmarkAnchor:null,locationTimer:null,setInitialState:function(args){
this.initialState={"url":this.initialHref,"kwArgs":args,"urlHash":this.initialHash};
},addToHistory:function(args){
var hash=null;
if(!this.historyIframe){
this.historyIframe=window.frames["djhistory"];
}
if(!this.bookmarkAnchor){
this.bookmarkAnchor=document.createElement("a");
(document.body||document.getElementsByTagName("body")[0]).appendChild(this.bookmarkAnchor);
this.bookmarkAnchor.style.display="none";
}
if((!args["changeUrl"])||(dojo.render.html.ie)){
var url=dojo.hostenv.getBaseScriptUri()+"iframe_history.html?"+(new Date()).getTime();
this.moveForward=true;
dojo.io.setIFrameSrc(this.historyIframe,url,false);
}
if(args["changeUrl"]){
this.changingUrl=true;
hash="#"+((args["changeUrl"]!==true)?args["changeUrl"]:(new Date()).getTime());
setTimeout("window.location.href = '"+hash+"'; dojo.undo.browser.changingUrl = false;",1);
this.bookmarkAnchor.href=hash;
if(dojo.render.html.ie){
var _1f5=args["back"]||args["backButton"]||args["handle"];
var tcb=function(_1f7){
if(window.location.hash!=""){
setTimeout("window.location.href = '"+hash+"';",1);
}
_1f5.apply(this,[_1f7]);
};
if(args["back"]){
args.back=tcb;
}else{
if(args["backButton"]){
args.backButton=tcb;
}else{
if(args["handle"]){
args.handle=tcb;
}
}
}
this.forwardStack=[];
var _1f8=args["forward"]||args["forwardButton"]||args["handle"];
var tfw=function(_1fa){
if(window.location.hash!=""){
window.location.href=hash;
}
if(_1f8){
_1f8.apply(this,[_1fa]);
}
};
if(args["forward"]){
args.forward=tfw;
}else{
if(args["forwardButton"]){
args.forwardButton=tfw;
}else{
if(args["handle"]){
args.handle=tfw;
}
}
}
}else{
if(dojo.render.html.moz){
if(!this.locationTimer){
this.locationTimer=setInterval("dojo.undo.browser.checkLocation();",200);
}
}
}
}
this.historyStack.push({"url":url,"kwArgs":args,"urlHash":hash});
},checkLocation:function(){
if(!this.changingUrl){
var hsl=this.historyStack.length;
if((window.location.hash==this.initialHash)||(window.location.href==this.initialHref)&&(hsl==1)){
this.handleBackButton();
return;
}
if(this.forwardStack.length>0){
if(this.forwardStack[this.forwardStack.length-1].urlHash==window.location.hash){
this.handleForwardButton();
return;
}
}
if((hsl>=2)&&(this.historyStack[hsl-2])){
if(this.historyStack[hsl-2].urlHash==window.location.hash){
this.handleBackButton();
return;
}
}
}
},iframeLoaded:function(evt,_1fd){
var _1fe=this._getUrlQuery(_1fd.href);
if(_1fe==null){
if(this.historyStack.length==1){
this.handleBackButton();
}
return;
}
if(this.moveForward){
this.moveForward=false;
return;
}
if(this.historyStack.length>=2&&_1fe==this._getUrlQuery(this.historyStack[this.historyStack.length-2].url)){
this.handleBackButton();
}else{
if(this.forwardStack.length>0&&_1fe==this._getUrlQuery(this.forwardStack[this.forwardStack.length-1].url)){
this.handleForwardButton();
}
}
},handleBackButton:function(){
var _1ff=this.historyStack.pop();
if(!_1ff){
return;
}
var last=this.historyStack[this.historyStack.length-1];
if(!last&&this.historyStack.length==0){
last=this.initialState;
}
if(last){
if(last.kwArgs["back"]){
last.kwArgs["back"]();
}else{
if(last.kwArgs["backButton"]){
last.kwArgs["backButton"]();
}else{
if(last.kwArgs["handle"]){
last.kwArgs.handle("back");
}
}
}
}
this.forwardStack.push(_1ff);
},handleForwardButton:function(){
var last=this.forwardStack.pop();
if(!last){
return;
}
if(last.kwArgs["forward"]){
last.kwArgs.forward();
}else{
if(last.kwArgs["forwardButton"]){
last.kwArgs.forwardButton();
}else{
if(last.kwArgs["handle"]){
last.kwArgs.handle("forward");
}
}
}
this.historyStack.push(last);
},_getUrlQuery:function(url){
var _203=url.split("?");
if(_203.length<2){
return null;
}else{
return _203[1];
}
}};
dojo.provide("dojo.io.BrowserIO");
dojo.require("dojo.io");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.func");
dojo.require("dojo.string.extras");
dojo.require("dojo.dom");
dojo.require("dojo.undo.browser");
dojo.io.checkChildrenForFile=function(node){
var _205=false;
var _206=node.getElementsByTagName("input");
dojo.lang.forEach(_206,function(_207){
if(_205){
return;
}
if(_207.getAttribute("type")=="file"){
_205=true;
}
});
return _205;
};
dojo.io.formHasFile=function(_208){
return dojo.io.checkChildrenForFile(_208);
};
dojo.io.updateNode=function(node,_20a){
node=dojo.byId(node);
var args=_20a;
if(dojo.lang.isString(_20a)){
args={url:_20a};
}
args.mimetype="text/html";
args.load=function(t,d,e){
while(node.firstChild){
if(dojo["event"]){
try{
dojo.event.browser.clean(node.firstChild);
}
catch(e){
}
}
node.removeChild(node.firstChild);
}
node.innerHTML=d;
};
dojo.io.bind(args);
};
dojo.io.formFilter=function(node){
var type=(node.type||"").toLowerCase();
return !node.disabled&&node.name&&!dojo.lang.inArray(type,["file","submit","image","reset","button"]);
};
dojo.io.encodeForm=function(_211,_212,_213){
if((!_211)||(!_211.tagName)||(!_211.tagName.toLowerCase()=="form")){
dojo.raise("Attempted to encode a non-form element.");
}
if(!_213){
_213=dojo.io.formFilter;
}
var enc=/utf/i.test(_212||"")?encodeURIComponent:dojo.string.encodeAscii;
var _215=[];
for(var i=0;i<_211.elements.length;i++){
var elm=_211.elements[i];
if(!elm||elm.tagName.toLowerCase()=="fieldset"||!_213(elm)){
continue;
}
var name=enc(elm.name);
var type=elm.type.toLowerCase();
if(type=="select-multiple"){
for(var j=0;j<elm.options.length;j++){
if(elm.options[j].selected){
_215.push(name+"="+enc(elm.options[j].value));
}
}
}else{
if(dojo.lang.inArray(type,["radio","checkbox"])){
if(elm.checked){
_215.push(name+"="+enc(elm.value));
}
}else{
_215.push(name+"="+enc(elm.value));
}
}
}
var _21b=_211.getElementsByTagName("input");
for(var i=0;i<_21b.length;i++){
var _21c=_21b[i];
if(_21c.type.toLowerCase()=="image"&&_21c.form==_211&&_213(_21c)){
var name=enc(_21c.name);
_215.push(name+"="+enc(_21c.value));
_215.push(name+".x=0");
_215.push(name+".y=0");
}
}
return _215.join("&")+"&";
};
dojo.io.FormBind=function(args){
this.bindArgs={};
if(args&&args.formNode){
this.init(args);
}else{
if(args){
this.init({formNode:args});
}
}
};
dojo.lang.extend(dojo.io.FormBind,{form:null,bindArgs:null,clickedButton:null,init:function(args){
var form=dojo.byId(args.formNode);
if(!form||!form.tagName||form.tagName.toLowerCase()!="form"){
throw new Error("FormBind: Couldn't apply, invalid form");
}else{
if(this.form==form){
return;
}else{
if(this.form){
throw new Error("FormBind: Already applied to a form");
}
}
}
dojo.lang.mixin(this.bindArgs,args);
this.form=form;
this.connect(form,"onsubmit","submit");
for(var i=0;i<form.elements.length;i++){
var node=form.elements[i];
if(node&&node.type&&dojo.lang.inArray(node.type.toLowerCase(),["submit","button"])){
this.connect(node,"onclick","click");
}
}
var _222=form.getElementsByTagName("input");
for(var i=0;i<_222.length;i++){
var _223=_222[i];
if(_223.type.toLowerCase()=="image"&&_223.form==form){
this.connect(_223,"onclick","click");
}
}
},onSubmit:function(form){
return true;
},submit:function(e){
e.preventDefault();
if(this.onSubmit(this.form)){
dojo.io.bind(dojo.lang.mixin(this.bindArgs,{formFilter:dojo.lang.hitch(this,"formFilter")}));
}
},click:function(e){
var node=e.currentTarget;
if(node.disabled){
return;
}
this.clickedButton=node;
},formFilter:function(node){
var type=(node.type||"").toLowerCase();
var _22a=false;
if(node.disabled||!node.name){
_22a=false;
}else{
if(dojo.lang.inArray(type,["submit","button","image"])){
if(!this.clickedButton){
this.clickedButton=node;
}
_22a=node==this.clickedButton;
}else{
_22a=!dojo.lang.inArray(type,["file","submit","reset","button"]);
}
}
return _22a;
},connect:function(_22b,_22c,_22d){
if(dojo.evalObjPath("dojo.event.connect")){
dojo.event.connect(_22b,_22c,this,_22d);
}else{
var fcn=dojo.lang.hitch(this,_22d);
_22b[_22c]=function(e){
if(!e){
e=window.event;
}
if(!e.currentTarget){
e.currentTarget=e.srcElement;
}
if(!e.preventDefault){
e.preventDefault=function(){
window.event.returnValue=false;
};
}
fcn(e);
};
}
}});
dojo.io.XMLHTTPTransport=new function(){
var _230=this;
var _231={};
this.useCache=false;
this.preventCache=false;
function getCacheKey(url,_233,_234){
return url+"|"+_233+"|"+_234.toLowerCase();
}
function addToCache(url,_236,_237,http){
_231[getCacheKey(url,_236,_237)]=http;
}
function getFromCache(url,_23a,_23b){
return _231[getCacheKey(url,_23a,_23b)];
}
this.clearCache=function(){
_231={};
};
function doLoad(_23c,http,url,_23f,_240){
if((http.status==200)||(http.status==304)||(location.protocol=="file:"&&(http.status==0||http.status==undefined))||(location.protocol=="chrome:"&&(http.status==0||http.status==undefined))){
var ret;
if(_23c.method.toLowerCase()=="head"){
var _242=http.getAllResponseHeaders();
ret={};
ret.toString=function(){
return _242;
};
var _243=_242.split(/[\r\n]+/g);
for(var i=0;i<_243.length;i++){
var pair=_243[i].match(/^([^:]+)\s*:\s*(.+)$/i);
if(pair){
ret[pair[1]]=pair[2];
}
}
}else{
if(_23c.mimetype=="text/javascript"){
try{
ret=dj_eval(http.responseText);
}
catch(e){
dojo.debug(e);
dojo.debug(http.responseText);
ret=null;
}
}else{
if(_23c.mimetype=="text/json"){
try{
ret=dj_eval("("+http.responseText+")");
}
catch(e){
dojo.debug(e);
dojo.debug(http.responseText);
ret=false;
}
}else{
if((_23c.mimetype=="application/xml")||(_23c.mimetype=="text/xml")){
ret=http.responseXML;
if(!ret||typeof ret=="string"){
ret=dojo.dom.createDocumentFromText(http.responseText);
}
}else{
ret=http.responseText;
}
}
}
}
if(_240){
addToCache(url,_23f,_23c.method,http);
}
_23c[(typeof _23c.load=="function")?"load":"handle"]("load",ret,http,_23c);
}else{
var _246=new dojo.io.Error("XMLHttpTransport Error: "+http.status+" "+http.statusText);
_23c[(typeof _23c.error=="function")?"error":"handle"]("error",_246,http,_23c);
}
}
function setHeaders(http,_248){
if(_248["headers"]){
for(var _249 in _248["headers"]){
if(_249.toLowerCase()=="content-type"&&!_248["contentType"]){
_248["contentType"]=_248["headers"][_249];
}else{
http.setRequestHeader(_249,_248["headers"][_249]);
}
}
}
}
this.inFlight=[];
this.inFlightTimer=null;
this.startWatchingInFlight=function(){
if(!this.inFlightTimer){
this.inFlightTimer=setInterval("dojo.io.XMLHTTPTransport.watchInFlight();",10);
}
};
this.watchInFlight=function(){
var now=null;
for(var x=this.inFlight.length-1;x>=0;x--){
var tif=this.inFlight[x];
if(!tif){
this.inFlight.splice(x,1);
continue;
}
if(4==tif.http.readyState){
this.inFlight.splice(x,1);
doLoad(tif.req,tif.http,tif.url,tif.query,tif.useCache);
}else{
if(tif.startTime){
if(!now){
now=(new Date()).getTime();
}
if(tif.startTime+(tif.req.timeoutSeconds*1000)<now){
if(typeof tif.http.abort=="function"){
tif.http.abort();
}
this.inFlight.splice(x,1);
tif.req[(typeof tif.req.timeout=="function")?"timeout":"handle"]("timeout",null,tif.http,tif.req);
}
}
}
}
if(this.inFlight.length==0){
clearInterval(this.inFlightTimer);
this.inFlightTimer=null;
}
};
var _24d=dojo.hostenv.getXmlhttpObject()?true:false;
this.canHandle=function(_24e){
return _24d&&dojo.lang.inArray((_24e["mimetype"].toLowerCase()||""),["text/plain","text/html","application/xml","text/xml","text/javascript","text/json"])&&dojo.lang.inArray(_24e["method"].toLowerCase(),["post","get","head"])&&!(_24e["formNode"]&&dojo.io.formHasFile(_24e["formNode"]));
};
this.multipartBoundary="45309FFF-BD65-4d50-99C9-36986896A96F";
this.bind=function(_24f){
if(!_24f["url"]){
if(!_24f["formNode"]&&(_24f["backButton"]||_24f["back"]||_24f["changeUrl"]||_24f["watchForURL"])&&(!djConfig.preventBackButtonFix)){
dj_deprecated("Using dojo.io.XMLHTTPTransport.bind() to add to browser history without doing an IO request is deprecated. Use dojo.undo.browser.addToHistory() instead.");
dojo.undo.browser.addToHistory(_24f);
return true;
}
}
var url=_24f.url;
var _251="";
if(_24f["formNode"]){
var ta=_24f.formNode.getAttribute("action");
if((ta)&&(!_24f["url"])){
url=ta;
}
var tp=_24f.formNode.getAttribute("method");
if((tp)&&(!_24f["method"])){
_24f.method=tp;
}
_251+=dojo.io.encodeForm(_24f.formNode,_24f.encoding,_24f["formFilter"]);
}
if(url.indexOf("#")>-1){
dojo.debug("Warning: dojo.io.bind: stripping hash values from url:",url);
url=url.split("#")[0];
}
if(_24f["file"]){
_24f.method="post";
}
if(!_24f["method"]){
_24f.method="get";
}
if(_24f.method.toLowerCase()=="get"){
_24f.multipart=false;
}else{
if(_24f["file"]){
_24f.multipart=true;
}else{
if(!_24f["multipart"]){
_24f.multipart=false;
}
}
}
if(_24f["backButton"]||_24f["back"]||_24f["changeUrl"]){
dojo.undo.browser.addToHistory(_24f);
}
var _254=_24f["content"]||{};
if(_24f.sendTransport){
_254["dojo.transport"]="xmlhttp";
}
do{
if(_24f.postContent){
_251=_24f.postContent;
break;
}
if(_254){
_251+=dojo.io.argsFromMap(_254,_24f.encoding);
}
if(_24f.method.toLowerCase()=="get"||!_24f.multipart){
break;
}
var t=[];
if(_251.length){
var q=_251.split("&");
for(var i=0;i<q.length;++i){
if(q[i].length){
var p=q[i].split("=");
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+p[0]+"\"","",p[1]);
}
}
}
if(_24f.file){
if(dojo.lang.isArray(_24f.file)){
for(var i=0;i<_24f.file.length;++i){
var o=_24f.file[i];
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+o.name+"\"; filename=\""+("fileName" in o?o.fileName:o.name)+"\"","Content-Type: "+("contentType" in o?o.contentType:"application/octet-stream"),"",o.content);
}
}else{
var o=_24f.file;
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+o.name+"\"; filename=\""+("fileName" in o?o.fileName:o.name)+"\"","Content-Type: "+("contentType" in o?o.contentType:"application/octet-stream"),"",o.content);
}
}
if(t.length){
t.push("--"+this.multipartBoundary+"--","");
_251=t.join("\r\n");
}
}while(false);
var _25a=_24f["sync"]?false:true;
var _25b=_24f["preventCache"]||(this.preventCache==true&&_24f["preventCache"]!=false);
var _25c=_24f["useCache"]==true||(this.useCache==true&&_24f["useCache"]!=false);
if(!_25b&&_25c){
var _25d=getFromCache(url,_251,_24f.method);
if(_25d){
doLoad(_24f,_25d,url,_251,false);
return;
}
}
var http=dojo.hostenv.getXmlhttpObject(_24f);
var _25f=false;
if(_25a){
var _260=this.inFlight.push({"req":_24f,"http":http,"url":url,"query":_251,"useCache":_25c,"startTime":_24f.timeoutSeconds?(new Date()).getTime():0});
this.startWatchingInFlight();
}
if(_24f.method.toLowerCase()=="post"){
http.open("POST",url,_25a);
setHeaders(http,_24f);
http.setRequestHeader("Content-Type",_24f.multipart?("multipart/form-data; boundary="+this.multipartBoundary):(_24f.contentType||"application/x-www-form-urlencoded"));
try{
http.send(_251);
}
catch(e){
if(typeof http.abort=="function"){
http.abort();
}
doLoad(_24f,{status:404},url,_251,_25c);
}
}else{
var _261=url;
if(_251!=""){
_261+=(_261.indexOf("?")>-1?"&":"?")+_251;
}
if(_25b){
_261+=(dojo.string.endsWithAny(_261,"?","&")?"":(_261.indexOf("?")>-1?"&":"?"))+"dojo.preventCache="+new Date().valueOf();
}
http.open(_24f.method.toUpperCase(),_261,_25a);
setHeaders(http,_24f);
try{
http.send(null);
}
catch(e){
if(typeof http.abort=="function"){
http.abort();
}
doLoad(_24f,{status:404},url,_251,_25c);
}
}
if(!_25a){
doLoad(_24f,http,url,_251,_25c);
}
_24f.abort=function(){
return http.abort();
};
return;
};
dojo.io.transports.addTransport("XMLHTTPTransport");
};
dojo.provide("dojo.io.cookie");
dojo.io.cookie.setCookie=function(name,_263,days,path,_266,_267){
var _268=-1;
if(typeof days=="number"&&days>=0){
var d=new Date();
d.setTime(d.getTime()+(days*24*60*60*1000));
_268=d.toGMTString();
}
_263=escape(_263);
document.cookie=name+"="+_263+";"+(_268!=-1?" expires="+_268+";":"")+(path?"path="+path:"")+(_266?"; domain="+_266:"")+(_267?"; secure":"");
};
dojo.io.cookie.set=dojo.io.cookie.setCookie;
dojo.io.cookie.getCookie=function(name){
var idx=document.cookie.lastIndexOf(name+"=");
if(idx==-1){
return null;
}
value=document.cookie.substring(idx+name.length+1);
var end=value.indexOf(";");
if(end==-1){
end=value.length;
}
value=value.substring(0,end);
value=unescape(value);
return value;
};
dojo.io.cookie.get=dojo.io.cookie.getCookie;
dojo.io.cookie.deleteCookie=function(name){
dojo.io.cookie.setCookie(name,"-",0);
};
dojo.io.cookie.setObjectCookie=function(name,obj,days,path,_272,_273,_274){
if(arguments.length==5){
_274=_272;
_272=null;
_273=null;
}
var _275=[],cookie,value="";
if(!_274){
cookie=dojo.io.cookie.getObjectCookie(name);
}
if(days>=0){
if(!cookie){
cookie={};
}
for(var prop in obj){
if(prop==null){
delete cookie[prop];
}else{
if(typeof obj[prop]=="string"||typeof obj[prop]=="number"){
cookie[prop]=obj[prop];
}
}
}
prop=null;
for(var prop in cookie){
_275.push(escape(prop)+"="+escape(cookie[prop]));
}
value=_275.join("&");
}
dojo.io.cookie.setCookie(name,value,days,path,_272,_273);
};
dojo.io.cookie.getObjectCookie=function(name){
var _278=null,cookie=dojo.io.cookie.getCookie(name);
if(cookie){
_278={};
var _279=cookie.split("&");
for(var i=0;i<_279.length;i++){
var pair=_279[i].split("=");
var _27c=pair[1];
if(isNaN(_27c)){
_27c=unescape(pair[1]);
}
_278[unescape(pair[0])]=_27c;
}
}
return _278;
};
dojo.io.cookie.isSupported=function(){
if(typeof navigator.cookieEnabled!="boolean"){
dojo.io.cookie.setCookie("__TestingYourBrowserForCookieSupport__","CookiesAllowed",90,null);
var _27d=dojo.io.cookie.getCookie("__TestingYourBrowserForCookieSupport__");
navigator.cookieEnabled=(_27d=="CookiesAllowed");
if(navigator.cookieEnabled){
this.deleteCookie("__TestingYourBrowserForCookieSupport__");
}
}
return navigator.cookieEnabled;
};
if(!dojo.io.cookies){
dojo.io.cookies=dojo.io.cookie;
}
dojo.hostenv.conditionalLoadModule({common:["dojo.io"],rhino:["dojo.io.RhinoIO"],browser:["dojo.io.BrowserIO","dojo.io.cookie"],dashboard:["dojo.io.BrowserIO","dojo.io.cookie"]});
dojo.hostenv.moduleLoaded("dojo.io.*");
dojo.provide("dojo.event");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.extras");
dojo.require("dojo.lang.func");
dojo.event=new function(){
this.canTimeout=dojo.lang.isFunction(dj_global["setTimeout"])||dojo.lang.isAlien(dj_global["setTimeout"]);
function interpolateArgs(args){
var dl=dojo.lang;
var ao={srcObj:dj_global,srcFunc:null,adviceObj:dj_global,adviceFunc:null,aroundObj:null,aroundFunc:null,adviceType:(args.length>2)?args[0]:"after",precedence:"last",once:false,delay:null,rate:0,adviceMsg:false};
switch(args.length){
case 0:
return;
case 1:
return;
case 2:
ao.srcFunc=args[0];
ao.adviceFunc=args[1];
break;
case 3:
if((dl.isObject(args[0]))&&(dl.isString(args[1]))&&(dl.isString(args[2]))){
ao.adviceType="after";
ao.srcObj=args[0];
ao.srcFunc=args[1];
ao.adviceFunc=args[2];
}else{
if((dl.isString(args[1]))&&(dl.isString(args[2]))){
ao.srcFunc=args[1];
ao.adviceFunc=args[2];
}else{
if((dl.isObject(args[0]))&&(dl.isString(args[1]))&&(dl.isFunction(args[2]))){
ao.adviceType="after";
ao.srcObj=args[0];
ao.srcFunc=args[1];
var _281=dojo.lang.nameAnonFunc(args[2],ao.adviceObj);
ao.adviceFunc=_281;
}else{
if((dl.isFunction(args[0]))&&(dl.isObject(args[1]))&&(dl.isString(args[2]))){
ao.adviceType="after";
ao.srcObj=dj_global;
var _281=dojo.lang.nameAnonFunc(args[0],ao.srcObj);
ao.srcFunc=_281;
ao.adviceObj=args[1];
ao.adviceFunc=args[2];
}
}
}
}
break;
case 4:
if((dl.isObject(args[0]))&&(dl.isObject(args[2]))){
ao.adviceType="after";
ao.srcObj=args[0];
ao.srcFunc=args[1];
ao.adviceObj=args[2];
ao.adviceFunc=args[3];
}else{
if((dl.isString(args[0]))&&(dl.isString(args[1]))&&(dl.isObject(args[2]))){
ao.adviceType=args[0];
ao.srcObj=dj_global;
ao.srcFunc=args[1];
ao.adviceObj=args[2];
ao.adviceFunc=args[3];
}else{
if((dl.isString(args[0]))&&(dl.isFunction(args[1]))&&(dl.isObject(args[2]))){
ao.adviceType=args[0];
ao.srcObj=dj_global;
var _281=dojo.lang.nameAnonFunc(args[1],dj_global);
ao.srcFunc=_281;
ao.adviceObj=args[2];
ao.adviceFunc=args[3];
}else{
if(dl.isObject(args[1])){
ao.srcObj=args[1];
ao.srcFunc=args[2];
ao.adviceObj=dj_global;
ao.adviceFunc=args[3];
}else{
if(dl.isObject(args[2])){
ao.srcObj=dj_global;
ao.srcFunc=args[1];
ao.adviceObj=args[2];
ao.adviceFunc=args[3];
}else{
ao.srcObj=ao.adviceObj=ao.aroundObj=dj_global;
ao.srcFunc=args[1];
ao.adviceFunc=args[2];
ao.aroundFunc=args[3];
}
}
}
}
}
break;
case 6:
ao.srcObj=args[1];
ao.srcFunc=args[2];
ao.adviceObj=args[3];
ao.adviceFunc=args[4];
ao.aroundFunc=args[5];
ao.aroundObj=dj_global;
break;
default:
ao.srcObj=args[1];
ao.srcFunc=args[2];
ao.adviceObj=args[3];
ao.adviceFunc=args[4];
ao.aroundObj=args[5];
ao.aroundFunc=args[6];
ao.once=args[7];
ao.delay=args[8];
ao.rate=args[9];
ao.adviceMsg=args[10];
break;
}
if((typeof ao.srcFunc).toLowerCase()!="string"){
ao.srcFunc=dojo.lang.getNameInObj(ao.srcObj,ao.srcFunc);
}
if((typeof ao.adviceFunc).toLowerCase()!="string"){
ao.adviceFunc=dojo.lang.getNameInObj(ao.adviceObj,ao.adviceFunc);
}
if((ao.aroundObj)&&((typeof ao.aroundFunc).toLowerCase()!="string")){
ao.aroundFunc=dojo.lang.getNameInObj(ao.aroundObj,ao.aroundFunc);
}
if(!ao.srcObj){
dojo.raise("bad srcObj for srcFunc: "+ao.srcFunc);
}
if(!ao.adviceObj){
dojo.raise("bad adviceObj for adviceFunc: "+ao.adviceFunc);
}
return ao;
}
this.connect=function(){
if(arguments.length==1){
var ao=arguments[0];
}else{
var ao=interpolateArgs(arguments);
}
if(dojo.lang.isArray(ao.srcObj)&&ao.srcObj!=""){
var _283={};
for(var x in ao){
_283[x]=ao[x];
}
var mjps=[];
dojo.lang.forEach(ao.srcObj,function(src){
if((dojo.render.html.capable)&&(dojo.lang.isString(src))){
src=dojo.byId(src);
}
_283.srcObj=src;
mjps.push(dojo.event.connect.call(dojo.event,_283));
});
return mjps;
}
var mjp=dojo.event.MethodJoinPoint.getForMethod(ao.srcObj,ao.srcFunc);
if(ao.adviceFunc){
var mjp2=dojo.event.MethodJoinPoint.getForMethod(ao.adviceObj,ao.adviceFunc);
}
mjp.kwAddAdvice(ao);
return mjp;
};
this.log=function(a1,a2){
var _28b;
if((arguments.length==1)&&(typeof a1=="object")){
_28b=a1;
}else{
_28b={srcObj:a1,srcFunc:a2};
}
_28b.adviceFunc=function(){
var _28c=[];
for(var x=0;x<arguments.length;x++){
_28c.push(arguments[x]);
}
dojo.debug("("+_28b.srcObj+")."+_28b.srcFunc,":",_28c.join(", "));
};
this.kwConnect(_28b);
};
this.connectBefore=function(){
var args=["before"];
for(var i=0;i<arguments.length;i++){
args.push(arguments[i]);
}
return this.connect.apply(this,args);
};
this.connectAround=function(){
var args=["around"];
for(var i=0;i<arguments.length;i++){
args.push(arguments[i]);
}
return this.connect.apply(this,args);
};
this._kwConnectImpl=function(_292,_293){
var fn=(_293)?"disconnect":"connect";
if(typeof _292["srcFunc"]=="function"){
_292.srcObj=_292["srcObj"]||dj_global;
var _295=dojo.lang.nameAnonFunc(_292.srcFunc,_292.srcObj);
_292.srcFunc=_295;
}
if(typeof _292["adviceFunc"]=="function"){
_292.adviceObj=_292["adviceObj"]||dj_global;
var _295=dojo.lang.nameAnonFunc(_292.adviceFunc,_292.adviceObj);
_292.adviceFunc=_295;
}
return dojo.event[fn]((_292["type"]||_292["adviceType"]||"after"),_292["srcObj"]||dj_global,_292["srcFunc"],_292["adviceObj"]||_292["targetObj"]||dj_global,_292["adviceFunc"]||_292["targetFunc"],_292["aroundObj"],_292["aroundFunc"],_292["once"],_292["delay"],_292["rate"],_292["adviceMsg"]||false);
};
this.kwConnect=function(_296){
return this._kwConnectImpl(_296,false);
};
this.disconnect=function(){
var ao=interpolateArgs(arguments);
if(!ao.adviceFunc){
return;
}
var mjp=dojo.event.MethodJoinPoint.getForMethod(ao.srcObj,ao.srcFunc);
return mjp.removeAdvice(ao.adviceObj,ao.adviceFunc,ao.adviceType,ao.once);
};
this.kwDisconnect=function(_299){
return this._kwConnectImpl(_299,true);
};
};
dojo.event.MethodInvocation=function(_29a,obj,args){
this.jp_=_29a;
this.object=obj;
this.args=[];
for(var x=0;x<args.length;x++){
this.args[x]=args[x];
}
this.around_index=-1;
};
dojo.event.MethodInvocation.prototype.proceed=function(){
this.around_index++;
if(this.around_index>=this.jp_.around.length){
return this.jp_.object[this.jp_.methodname].apply(this.jp_.object,this.args);
}else{
var ti=this.jp_.around[this.around_index];
var mobj=ti[0]||dj_global;
var meth=ti[1];
return mobj[meth].call(mobj,this);
}
};
dojo.event.MethodJoinPoint=function(obj,_2a2){
this.object=obj||dj_global;
this.methodname=_2a2;
this.methodfunc=this.object[_2a2];
this.before=[];
this.after=[];
this.around=[];
};
dojo.event.MethodJoinPoint.getForMethod=function(obj,_2a4){
if(!obj){
obj=dj_global;
}
if(!obj[_2a4]){
obj[_2a4]=function(){
};
}else{
if((!dojo.lang.isFunction(obj[_2a4]))&&(!dojo.lang.isAlien(obj[_2a4]))){
return null;
}
}
var _2a5=_2a4+"$joinpoint";
var _2a6=_2a4+"$joinpoint$method";
var _2a7=obj[_2a5];
if(!_2a7){
var _2a8=false;
if(dojo.event["browser"]){
if((obj["attachEvent"])||(obj["nodeType"])||(obj["addEventListener"])){
_2a8=true;
dojo.event.browser.addClobberNodeAttrs(obj,[_2a5,_2a6,_2a4]);
}
}
obj[_2a6]=obj[_2a4];
_2a7=obj[_2a5]=new dojo.event.MethodJoinPoint(obj,_2a6);
obj[_2a4]=function(){
var args=[];
if((_2a8)&&(!arguments.length)){
var evt=null;
try{
if(obj.ownerDocument){
evt=obj.ownerDocument.parentWindow.event;
}else{
if(obj.documentElement){
evt=obj.documentElement.ownerDocument.parentWindow.event;
}else{
evt=window.event;
}
}
}
catch(e){
evt=window.event;
}
if(evt){
args.push(dojo.event.browser.fixEvent(evt,this));
}
}else{
for(var x=0;x<arguments.length;x++){
if((x==0)&&(_2a8)&&(dojo.event.browser.isEvent(arguments[x]))){
args.push(dojo.event.browser.fixEvent(arguments[x],this));
}else{
args.push(arguments[x]);
}
}
}
return _2a7.run.apply(_2a7,args);
};
}
return _2a7;
};
dojo.lang.extend(dojo.event.MethodJoinPoint,{unintercept:function(){
this.object[this.methodname]=this.methodfunc;
},run:function(){
var obj=this.object||dj_global;
var args=arguments;
var _2ae=[];
for(var x=0;x<args.length;x++){
_2ae[x]=args[x];
}
var _2b0=function(marr){
if(!marr){
dojo.debug("Null argument to unrollAdvice()");
return;
}
var _2b2=marr[0]||dj_global;
var _2b3=marr[1];
if(!_2b2[_2b3]){
dojo.raise("function \""+_2b3+"\" does not exist on \""+_2b2+"\"");
}
var _2b4=marr[2]||dj_global;
var _2b5=marr[3];
var msg=marr[6];
var _2b7;
var to={args:[],jp_:this,object:obj,proceed:function(){
return _2b2[_2b3].apply(_2b2,to.args);
}};
to.args=_2ae;
var _2b9=parseInt(marr[4]);
var _2ba=((!isNaN(_2b9))&&(marr[4]!==null)&&(typeof marr[4]!="undefined"));
if(marr[5]){
var rate=parseInt(marr[5]);
var cur=new Date();
var _2bd=false;
if((marr["last"])&&((cur-marr.last)<=rate)){
if(dojo.event.canTimeout){
if(marr["delayTimer"]){
clearTimeout(marr.delayTimer);
}
var tod=parseInt(rate*2);
var mcpy=dojo.lang.shallowCopy(marr);
marr.delayTimer=setTimeout(function(){
mcpy[5]=0;
_2b0(mcpy);
},tod);
}
return;
}else{
marr.last=cur;
}
}
if(_2b5){
_2b4[_2b5].call(_2b4,to);
}else{
if((_2ba)&&((dojo.render.html)||(dojo.render.svg))){
dj_global["setTimeout"](function(){
if(msg){
_2b2[_2b3].call(_2b2,to);
}else{
_2b2[_2b3].apply(_2b2,args);
}
},_2b9);
}else{
if(msg){
_2b2[_2b3].call(_2b2,to);
}else{
_2b2[_2b3].apply(_2b2,args);
}
}
}
};
if(this.before.length>0){
dojo.lang.forEach(this.before,_2b0);
}
var _2c0;
if(this.around.length>0){
var mi=new dojo.event.MethodInvocation(this,obj,args);
_2c0=mi.proceed();
}else{
if(this.methodfunc){
_2c0=this.object[this.methodname].apply(this.object,args);
}
}
if(this.after.length>0){
dojo.lang.forEach(this.after,_2b0);
}
return (this.methodfunc)?_2c0:null;
},getArr:function(kind){
var arr=this.after;
if((typeof kind=="string")&&(kind.indexOf("before")!=-1)){
arr=this.before;
}else{
if(kind=="around"){
arr=this.around;
}
}
return arr;
},kwAddAdvice:function(args){
this.addAdvice(args["adviceObj"],args["adviceFunc"],args["aroundObj"],args["aroundFunc"],args["adviceType"],args["precedence"],args["once"],args["delay"],args["rate"],args["adviceMsg"]);
},addAdvice:function(_2c5,_2c6,_2c7,_2c8,_2c9,_2ca,once,_2cc,rate,_2ce){
var arr=this.getArr(_2c9);
if(!arr){
dojo.raise("bad this: "+this);
}
var ao=[_2c5,_2c6,_2c7,_2c8,_2cc,rate,_2ce];
if(once){
if(this.hasAdvice(_2c5,_2c6,_2c9,arr)>=0){
return;
}
}
if(_2ca=="first"){
arr.unshift(ao);
}else{
arr.push(ao);
}
},hasAdvice:function(_2d1,_2d2,_2d3,arr){
if(!arr){
arr=this.getArr(_2d3);
}
var ind=-1;
for(var x=0;x<arr.length;x++){
if((arr[x][0]==_2d1)&&(arr[x][1]==_2d2)){
ind=x;
}
}
return ind;
},removeAdvice:function(_2d7,_2d8,_2d9,once){
var arr=this.getArr(_2d9);
var ind=this.hasAdvice(_2d7,_2d8,_2d9,arr);
if(ind==-1){
return false;
}
while(ind!=-1){
arr.splice(ind,1);
if(once){
break;
}
ind=this.hasAdvice(_2d7,_2d8,_2d9,arr);
}
return true;
}});
dojo.require("dojo.event");
dojo.provide("dojo.event.topic");
dojo.event.topic=new function(){
this.topics={};
this.getTopic=function(_2dd){
if(!this.topics[_2dd]){
this.topics[_2dd]=new this.TopicImpl(_2dd);
}
return this.topics[_2dd];
};
this.registerPublisher=function(_2de,obj,_2e0){
var _2de=this.getTopic(_2de);
_2de.registerPublisher(obj,_2e0);
};
this.subscribe=function(_2e1,obj,_2e3){
var _2e1=this.getTopic(_2e1);
_2e1.subscribe(obj,_2e3);
};
this.unsubscribe=function(_2e4,obj,_2e6){
var _2e4=this.getTopic(_2e4);
_2e4.unsubscribe(obj,_2e6);
};
this.publish=function(_2e7,_2e8){
var _2e7=this.getTopic(_2e7);
var args=[];
if(arguments.length==2&&(dojo.lang.isArray(_2e8)||_2e8.callee)){
args=_2e8;
}else{
var args=[];
for(var x=1;x<arguments.length;x++){
args.push(arguments[x]);
}
}
_2e7.sendMessage.apply(_2e7,args);
};
};
dojo.event.topic.TopicImpl=function(_2eb){
this.topicName=_2eb;
var self=this;
self.subscribe=function(_2ed,_2ee){
var tf=_2ee||_2ed;
var to=(!_2ee)?dj_global:_2ed;
dojo.event.kwConnect({srcObj:self,srcFunc:"sendMessage",adviceObj:to,adviceFunc:tf});
};
self.unsubscribe=function(_2f1,_2f2){
var tf=(!_2f2)?_2f1:_2f2;
var to=(!_2f2)?null:_2f1;
dojo.event.kwDisconnect({srcObj:self,srcFunc:"sendMessage",adviceObj:to,adviceFunc:tf});
};
self.registerPublisher=function(_2f5,_2f6){
dojo.event.connect(_2f5,_2f6,self,"sendMessage");
};
self.sendMessage=function(_2f7){
};
};
dojo.provide("dojo.event.browser");
dojo.require("dojo.event");
dojo_ie_clobber=new function(){
this.clobberNodes=[];
function nukeProp(node,prop){
try{
node[prop]=null;
}
catch(e){
}
try{
delete node[prop];
}
catch(e){
}
try{
node.removeAttribute(prop);
}
catch(e){
}
}
this.clobber=function(_2fa){
var na;
var tna;
if(_2fa){
tna=_2fa.all||_2fa.getElementsByTagName("*");
na=[_2fa];
for(var x=0;x<tna.length;x++){
if(tna[x]["__doClobber__"]){
na.push(tna[x]);
}
}
}else{
try{
window.onload=null;
}
catch(e){
}
na=(this.clobberNodes.length)?this.clobberNodes:document.all;
}
tna=null;
var _2fe={};
for(var i=na.length-1;i>=0;i=i-1){
var el=na[i];
if(el["__clobberAttrs__"]){
for(var j=0;j<el.__clobberAttrs__.length;j++){
nukeProp(el,el.__clobberAttrs__[j]);
}
nukeProp(el,"__clobberAttrs__");
nukeProp(el,"__doClobber__");
}
}
na=null;
};
};
if(dojo.render.html.ie){
window.onunload=function(){
dojo_ie_clobber.clobber();
try{
if((dojo["widget"])&&(dojo.widget["manager"])){
dojo.widget.manager.destroyAll();
}
}
catch(e){
}
try{
window.onload=null;
}
catch(e){
}
try{
window.onunload=null;
}
catch(e){
}
dojo_ie_clobber.clobberNodes=[];
};
}
dojo.event.browser=new function(){
var _302=0;
this.clean=function(node){
if(dojo.render.html.ie){
dojo_ie_clobber.clobber(node);
}
};
this.addClobberNode=function(node){
if(!node["__doClobber__"]){
node.__doClobber__=true;
dojo_ie_clobber.clobberNodes.push(node);
node.__clobberAttrs__=[];
}
};
this.addClobberNodeAttrs=function(node,_306){
this.addClobberNode(node);
for(var x=0;x<_306.length;x++){
node.__clobberAttrs__.push(_306[x]);
}
};
this.removeListener=function(node,_309,fp,_30b){
if(!_30b){
var _30b=false;
}
_309=_309.toLowerCase();
if(_309.substr(0,2)=="on"){
_309=_309.substr(2);
}
if(node.removeEventListener){
node.removeEventListener(_309,fp,_30b);
}
};
this.addListener=function(node,_30d,fp,_30f,_310){
if(!node){
return;
}
if(!_30f){
var _30f=false;
}
_30d=_30d.toLowerCase();
if(_30d.substr(0,2)!="on"){
_30d="on"+_30d;
}
if(!_310){
var _311=function(evt){
if(!evt){
evt=window.event;
}
var ret=fp(dojo.event.browser.fixEvent(evt,this));
if(_30f){
dojo.event.browser.stopEvent(evt);
}
return ret;
};
}else{
_311=fp;
}
if(node.addEventListener){
node.addEventListener(_30d.substr(2),_311,_30f);
return _311;
}else{
if(typeof node[_30d]=="function"){
var _314=node[_30d];
node[_30d]=function(e){
_314(e);
return _311(e);
};
}else{
node[_30d]=_311;
}
if(dojo.render.html.ie){
this.addClobberNodeAttrs(node,[_30d]);
}
return _311;
}
};
this.isEvent=function(obj){
return (typeof obj!="undefined")&&(typeof Event!="undefined")&&(obj.eventPhase);
};
this.currentEvent=null;
this.callListener=function(_317,_318){
if(typeof _317!="function"){
dojo.raise("listener not a function: "+_317);
}
dojo.event.browser.currentEvent.currentTarget=_318;
return _317.call(_318,dojo.event.browser.currentEvent);
};
this.stopPropagation=function(){
dojo.event.browser.currentEvent.cancelBubble=true;
};
this.preventDefault=function(){
dojo.event.browser.currentEvent.returnValue=false;
};
this.keys={KEY_BACKSPACE:8,KEY_TAB:9,KEY_ENTER:13,KEY_SHIFT:16,KEY_CTRL:17,KEY_ALT:18,KEY_PAUSE:19,KEY_CAPS_LOCK:20,KEY_ESCAPE:27,KEY_SPACE:32,KEY_PAGE_UP:33,KEY_PAGE_DOWN:34,KEY_END:35,KEY_HOME:36,KEY_LEFT_ARROW:37,KEY_UP_ARROW:38,KEY_RIGHT_ARROW:39,KEY_DOWN_ARROW:40,KEY_INSERT:45,KEY_DELETE:46,KEY_LEFT_WINDOW:91,KEY_RIGHT_WINDOW:92,KEY_SELECT:93,KEY_F1:112,KEY_F2:113,KEY_F3:114,KEY_F4:115,KEY_F5:116,KEY_F6:117,KEY_F7:118,KEY_F8:119,KEY_F9:120,KEY_F10:121,KEY_F11:122,KEY_F12:123,KEY_NUM_LOCK:144,KEY_SCROLL_LOCK:145};
this.revKeys=[];
for(var key in this.keys){
this.revKeys[this.keys[key]]=key;
}
this.fixEvent=function(evt,_31b){
if((!evt)&&(window["event"])){
var evt=window.event;
}
if((evt["type"])&&(evt["type"].indexOf("key")==0)){
evt.keys=this.revKeys;
for(var key in this.keys){
evt[key]=this.keys[key];
}
if((dojo.render.html.ie)&&(evt["type"]=="keypress")){
evt.charCode=evt.keyCode;
}
}
if(dojo.render.html.ie){
if(!evt.target){
evt.target=evt.srcElement;
}
if(!evt.currentTarget){
evt.currentTarget=(_31b?_31b:evt.srcElement);
}
if(!evt.layerX){
evt.layerX=evt.offsetX;
}
if(!evt.layerY){
evt.layerY=evt.offsetY;
}
if(evt.fromElement){
evt.relatedTarget=evt.fromElement;
}
if(evt.toElement){
evt.relatedTarget=evt.toElement;
}
this.currentEvent=evt;
evt.callListener=this.callListener;
evt.stopPropagation=this.stopPropagation;
evt.preventDefault=this.preventDefault;
}
return evt;
};
this.stopEvent=function(ev){
if(window.event){
ev.returnValue=false;
ev.cancelBubble=true;
}else{
ev.preventDefault();
ev.stopPropagation();
}
};
};
dojo.hostenv.conditionalLoadModule({common:["dojo.event","dojo.event.topic"],browser:["dojo.event.browser"],dashboard:["dojo.event.browser"]});
dojo.hostenv.moduleLoaded("dojo.event.*");
dojo.provide("dojo.lang.assert");
dojo.require("dojo.lang.common");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.type");
dojo.lang.assert=function(_31e,_31f){
if(!_31e){
var _320="An assert statement failed.\n"+"The method dojo.lang.assert() was called with a 'false' value.\n";
if(_31f){
_320+="Here's the assert message:\n"+_31f+"\n";
}
throw new Error(_320);
}
};
dojo.lang.assertType=function(_321,type,_323){
if(!dojo.lang.isOfType(_321,type)){
if(!_323){
if(!dojo.lang.assertType._errorMessage){
dojo.lang.assertType._errorMessage="Type mismatch: dojo.lang.assertType() failed.";
}
_323=dojo.lang.assertType._errorMessage;
}
dojo.lang.assert(false,_323);
}
};
dojo.lang.assertValidKeywords=function(_324,_325,_326){
var key;
if(!_326){
if(!dojo.lang.assertValidKeywords._errorMessage){
dojo.lang.assertValidKeywords._errorMessage="In dojo.lang.assertValidKeywords(), found invalid keyword:";
}
_326=dojo.lang.assertValidKeywords._errorMessage;
}
if(dojo.lang.isArray(_325)){
for(key in _324){
if(!dojo.lang.inArray(_325,key)){
dojo.lang.assert(false,_326+" "+key);
}
}
}else{
for(key in _324){
if(!(key in _325)){
dojo.lang.assert(false,_326+" "+key);
}
}
}
};
dojo.provide("dojo.AdapterRegistry");
dojo.require("dojo.lang.func");
dojo.require("dojo.lang.type");
dojo.AdapterRegistry=function(){
this.pairs=[];
};
dojo.lang.extend(dojo.AdapterRegistry,{register:function(name,_329,wrap,_32b){
if(_32b){
this.pairs.unshift([name,_329,wrap]);
}else{
this.pairs.push([name,_329,wrap]);
}
},match:function(){
for(var i=0;i<this.pairs.length;i++){
var pair=this.pairs[i];
if(pair[1].apply(this,arguments)){
return pair[2].apply(this,arguments);
}
}
throw new Error("No match found");
},unregister:function(name){
for(var i=0;i<this.pairs.length;i++){
var pair=this.pairs[i];
if(pair[0]==name){
this.pairs.splice(i,1);
return true;
}
}
return false;
}});
dojo.provide("dojo.lang.repr");
dojo.require("dojo.lang.common");
dojo.require("dojo.AdapterRegistry");
dojo.lang.reprRegistry=new dojo.AdapterRegistry();
dojo.lang.registerRepr=function(name,_332,wrap,_334){
dojo.lang.reprRegistry.register(name,_332,wrap,_334);
};
dojo.lang.repr=function(obj){
if(typeof (obj)=="undefined"){
return "undefined";
}else{
if(obj===null){
return "null";
}
}
try{
if(typeof (obj["__repr__"])=="function"){
return obj["__repr__"]();
}else{
if((typeof (obj["repr"])=="function")&&(obj.repr!=arguments.callee)){
return obj["repr"]();
}
}
return dojo.lang.reprRegistry.match(obj);
}
catch(e){
if(typeof (obj.NAME)=="string"&&(obj.toString==Function.prototype.toString||obj.toString==Object.prototype.toString)){
return o.NAME;
}
}
if(typeof (obj)=="function"){
obj=(obj+"").replace(/^\s+/,"");
var idx=obj.indexOf("{");
if(idx!=-1){
obj=obj.substr(0,idx)+"{...}";
}
}
return obj+"";
};
dojo.lang.reprArrayLike=function(arr){
try{
var na=dojo.lang.map(arr,dojo.lang.repr);
return "["+na.join(", ")+"]";
}
catch(e){
}
};
dojo.lang.reprString=function(str){
return ("\""+str.replace(/(["\\])/g,"\\$1")+"\"").replace(/[\f]/g,"\\f").replace(/[\b]/g,"\\b").replace(/[\n]/g,"\\n").replace(/[\t]/g,"\\t").replace(/[\r]/g,"\\r");
};
dojo.lang.reprNumber=function(num){
return num+"";
};
(function(){
var m=dojo.lang;
m.registerRepr("arrayLike",m.isArrayLike,m.reprArrayLike);
m.registerRepr("string",m.isString,m.reprString);
m.registerRepr("numbers",m.isNumber,m.reprNumber);
m.registerRepr("boolean",m.isBoolean,m.reprNumber);
})();
dojo.hostenv.conditionalLoadModule({common:["dojo.lang","dojo.lang.common","dojo.lang.assert","dojo.lang.array","dojo.lang.type","dojo.lang.func","dojo.lang.extras","dojo.lang.repr"]});
dojo.hostenv.moduleLoaded("dojo.lang.*");
dojo.provide("dojo.widget.Manager");
dojo.require("dojo.lang.array");
dojo.require("dojo.event.*");
dojo.widget.manager=new function(){
this.widgets=[];
this.widgetIds=[];
this.topWidgets={};
var _33c={};
var _33d=[];
this.getUniqueId=function(_33e){
return _33e+"_"+(_33c[_33e]!=undefined?++_33c[_33e]:_33c[_33e]=0);
};
this.add=function(_33f){
dojo.profile.start("dojo.widget.manager.add");
this.widgets.push(_33f);
if(!_33f.extraArgs["id"]){
_33f.extraArgs["id"]=_33f.extraArgs["ID"];
}
if(_33f.widgetId==""){
if(_33f["id"]){
_33f.widgetId=_33f["id"];
}else{
if(_33f.extraArgs["id"]){
_33f.widgetId=_33f.extraArgs["id"];
}else{
_33f.widgetId=this.getUniqueId(_33f.widgetType);
}
}
}
if(this.widgetIds[_33f.widgetId]){
dojo.debug("widget ID collision on ID: "+_33f.widgetId);
}
this.widgetIds[_33f.widgetId]=_33f;
dojo.profile.end("dojo.widget.manager.add");
};
this.destroyAll=function(){
for(var x=this.widgets.length-1;x>=0;x--){
try{
this.widgets[x].destroy(true);
delete this.widgets[x];
}
catch(e){
}
}
};
this.remove=function(_341){
var tw=this.widgets[_341].widgetId;
delete this.widgetIds[tw];
this.widgets.splice(_341,1);
};
this.removeById=function(id){
for(var i=0;i<this.widgets.length;i++){
if(this.widgets[i].widgetId==id){
this.remove(i);
break;
}
}
};
this.getWidgetById=function(id){
return this.widgetIds[id];
};
this.getWidgetsByType=function(type){
var lt=type.toLowerCase();
var ret=[];
dojo.lang.forEach(this.widgets,function(x){
if(x.widgetType.toLowerCase()==lt){
ret.push(x);
}
});
return ret;
};
this.getWidgetsOfType=function(id){
dj_deprecated("getWidgetsOfType is depecrecated, use getWidgetsByType");
return dojo.widget.manager.getWidgetsByType(id);
};
this.getWidgetsByFilter=function(_34b){
var ret=[];
dojo.lang.forEach(this.widgets,function(x){
if(_34b(x)){
ret.push(x);
}
});
return ret;
};
this.getAllWidgets=function(){
return this.widgets.concat();
};
this.getWidgetByNode=function(node){
var w=this.getAllWidgets();
for(var i=0;i<w.length;i++){
if(w[i].domNode==node){
return w[i];
}
}
return null;
};
this.byId=this.getWidgetById;
this.byType=this.getWidgetsByType;
this.byFilter=this.getWidgetsByFilter;
this.byNode=this.getWidgetByNode;
var _351={};
var _352=["dojo.widget"];
for(var i=0;i<_352.length;i++){
_352[_352[i]]=true;
}
this.registerWidgetPackage=function(_354){
if(!_352[_354]){
_352[_354]=true;
_352.push(_354);
}
};
this.getWidgetPackageList=function(){
return dojo.lang.map(_352,function(elt){
return (elt!==true?elt:undefined);
});
};
this.getImplementation=function(_356,_357,_358){
var impl=this.getImplementationName(_356);
if(impl){
var ret=new impl(_357);
return ret;
}
};
this.getImplementationName=function(_35b){
var _35c=_35b.toLowerCase();
var impl=_351[_35c];
if(impl){
return impl;
}
if(!_33d.length){
for(var _35e in dojo.render){
if(dojo.render[_35e]["capable"]===true){
var _35f=dojo.render[_35e].prefixes;
for(var i=0;i<_35f.length;i++){
_33d.push(_35f[i].toLowerCase());
}
}
}
_33d.push("");
}
for(var i=0;i<_352.length;i++){
var _361=dojo.evalObjPath(_352[i]);
if(!_361){
continue;
}
for(var j=0;j<_33d.length;j++){
if(!_361[_33d[j]]){
continue;
}
for(var _363 in _361[_33d[j]]){
if(_363.toLowerCase()!=_35c){
continue;
}
_351[_35c]=_361[_33d[j]][_363];
return _351[_35c];
}
}
for(var j=0;j<_33d.length;j++){
for(var _363 in _361){
if(_363.toLowerCase()!=(_33d[j]+_35c)){
continue;
}
_351[_35c]=_361[_363];
return _351[_35c];
}
}
}
throw new Error("Could not locate \""+_35b+"\" class");
};
this.resizing=false;
this.onResized=function(){
if(this.resizing){
return;
}
try{
this.resizing=true;
for(var id in this.topWidgets){
var _365=this.topWidgets[id];
if(_365.onResized){
_365.onResized();
}
}
}
catch(e){
}
finally{
this.resizing=false;
}
};
if(typeof window!="undefined"){
dojo.addOnLoad(this,"onResized");
dojo.event.connect(window,"onresize",this,"onResized");
}
};
dojo.widget.getUniqueId=function(){
return dojo.widget.manager.getUniqueId.apply(dojo.widget.manager,arguments);
};
dojo.widget.addWidget=function(){
return dojo.widget.manager.add.apply(dojo.widget.manager,arguments);
};
dojo.widget.destroyAllWidgets=function(){
return dojo.widget.manager.destroyAll.apply(dojo.widget.manager,arguments);
};
dojo.widget.removeWidget=function(){
return dojo.widget.manager.remove.apply(dojo.widget.manager,arguments);
};
dojo.widget.removeWidgetById=function(){
return dojo.widget.manager.removeById.apply(dojo.widget.manager,arguments);
};
dojo.widget.getWidgetById=function(){
return dojo.widget.manager.getWidgetById.apply(dojo.widget.manager,arguments);
};
dojo.widget.getWidgetsByType=function(){
return dojo.widget.manager.getWidgetsByType.apply(dojo.widget.manager,arguments);
};
dojo.widget.getWidgetsByFilter=function(){
return dojo.widget.manager.getWidgetsByFilter.apply(dojo.widget.manager,arguments);
};
dojo.widget.byId=function(){
return dojo.widget.manager.getWidgetById.apply(dojo.widget.manager,arguments);
};
dojo.widget.byType=function(){
return dojo.widget.manager.getWidgetsByType.apply(dojo.widget.manager,arguments);
};
dojo.widget.byFilter=function(){
return dojo.widget.manager.getWidgetsByFilter.apply(dojo.widget.manager,arguments);
};
dojo.widget.byNode=function(){
return dojo.widget.manager.getWidgetByNode.apply(dojo.widget.manager,arguments);
};
dojo.widget.all=function(n){
var _367=dojo.widget.manager.getAllWidgets.apply(dojo.widget.manager,arguments);
if(arguments.length>0){
return _367[n];
}
return _367;
};
dojo.widget.registerWidgetPackage=function(){
return dojo.widget.manager.registerWidgetPackage.apply(dojo.widget.manager,arguments);
};
dojo.widget.getWidgetImplementation=function(){
return dojo.widget.manager.getImplementation.apply(dojo.widget.manager,arguments);
};
dojo.widget.getWidgetImplementationName=function(){
return dojo.widget.manager.getImplementationName.apply(dojo.widget.manager,arguments);
};
dojo.widget.widgets=dojo.widget.manager.widgets;
dojo.widget.widgetIds=dojo.widget.manager.widgetIds;
dojo.widget.root=dojo.widget.manager.root;
dojo.provide("dojo.graphics.color");
dojo.require("dojo.lang.array");
dojo.graphics.color.Color=function(r,g,b,a){
if(dojo.lang.isArray(r)){
this.r=r[0];
this.g=r[1];
this.b=r[2];
this.a=r[3]||1;
}else{
if(dojo.lang.isString(r)){
var rgb=dojo.graphics.color.extractRGB(r);
this.r=rgb[0];
this.g=rgb[1];
this.b=rgb[2];
this.a=g||1;
}else{
if(r instanceof dojo.graphics.color.Color){
this.r=r.r;
this.b=r.b;
this.g=r.g;
this.a=r.a;
}else{
this.r=r;
this.g=g;
this.b=b;
this.a=a;
}
}
}
};
dojo.graphics.color.Color.fromArray=function(arr){
return new dojo.graphics.color.Color(arr[0],arr[1],arr[2],arr[3]);
};
dojo.lang.extend(dojo.graphics.color.Color,{toRgb:function(_36e){
if(_36e){
return this.toRgba();
}else{
return [this.r,this.g,this.b];
}
},toRgba:function(){
return [this.r,this.g,this.b,this.a];
},toHex:function(){
return dojo.graphics.color.rgb2hex(this.toRgb());
},toCss:function(){
return "rgb("+this.toRgb().join()+")";
},toString:function(){
return this.toHex();
},toHsv:function(){
return dojo.graphics.color.rgb2hsv(this.toRgb());
},toHsl:function(){
return dojo.graphics.color.rgb2hsl(this.toRgb());
},blend:function(_36f,_370){
return dojo.graphics.color.blend(this.toRgb(),new Color(_36f).toRgb(),_370);
}});
dojo.graphics.color.named={white:[255,255,255],black:[0,0,0],red:[255,0,0],green:[0,255,0],blue:[0,0,255],navy:[0,0,128],gray:[128,128,128],silver:[192,192,192]};
dojo.graphics.color.blend=function(a,b,_373){
if(typeof a=="string"){
return dojo.graphics.color.blendHex(a,b,_373);
}
if(!_373){
_373=0;
}else{
if(_373>1){
_373=1;
}else{
if(_373<-1){
_373=-1;
}
}
}
var c=new Array(3);
for(var i=0;i<3;i++){
var half=Math.abs(a[i]-b[i])/2;
c[i]=Math.floor(Math.min(a[i],b[i])+half+(half*_373));
}
return c;
};
dojo.graphics.color.blendHex=function(a,b,_379){
return dojo.graphics.color.rgb2hex(dojo.graphics.color.blend(dojo.graphics.color.hex2rgb(a),dojo.graphics.color.hex2rgb(b),_379));
};
dojo.graphics.color.extractRGB=function(_37a){
var hex="0123456789abcdef";
_37a=_37a.toLowerCase();
if(_37a.indexOf("rgb")==0){
var _37c=_37a.match(/rgba*\((\d+), *(\d+), *(\d+)/i);
var ret=_37c.splice(1,3);
return ret;
}else{
var _37e=dojo.graphics.color.hex2rgb(_37a);
if(_37e){
return _37e;
}else{
return dojo.graphics.color.named[_37a]||[255,255,255];
}
}
};
dojo.graphics.color.hex2rgb=function(hex){
var _380="0123456789ABCDEF";
var rgb=new Array(3);
if(hex.indexOf("#")==0){
hex=hex.substring(1);
}
hex=hex.toUpperCase();
if(hex.replace(new RegExp("["+_380+"]","g"),"")!=""){
return null;
}
if(hex.length==3){
rgb[0]=hex.charAt(0)+hex.charAt(0);
rgb[1]=hex.charAt(1)+hex.charAt(1);
rgb[2]=hex.charAt(2)+hex.charAt(2);
}else{
rgb[0]=hex.substring(0,2);
rgb[1]=hex.substring(2,4);
rgb[2]=hex.substring(4);
}
for(var i=0;i<rgb.length;i++){
rgb[i]=_380.indexOf(rgb[i].charAt(0))*16+_380.indexOf(rgb[i].charAt(1));
}
return rgb;
};
dojo.graphics.color.rgb2hex=function(r,g,b){
if(dojo.lang.isArray(r)){
g=r[1]||0;
b=r[2]||0;
r=r[0]||0;
}
function pad(x){
while(x.length<2){
x="0"+x;
}
return x;
}
var ret=dojo.lang.map([r,g,b],function(x){
var s=x.toString(16);
while(s.length<2){
s="0"+s;
}
return s;
});
ret.unshift("#");
return ret.join("");
};
dojo.graphics.color.rgb2hsv=function(r,g,b){
if(dojo.lang.isArray(r)){
b=r[2]||0;
g=r[1]||0;
r=r[0]||0;
}
var h=null;
var s=null;
var v=null;
var min=Math.min(r,g,b);
v=Math.max(r,g,b);
var _391=v-min;
s=(v==0)?0:_391/v;
if(s==0){
h=0;
}else{
if(r==v){
h=60*(g-b)/_391;
}else{
if(g==v){
h=120+60*(b-r)/_391;
}else{
if(b==v){
h=240+60*(r-g)/_391;
}
}
}
if(h<0){
h+=360;
}
}
h=(h==0)?360:Math.ceil((h/360)*255);
s=Math.ceil(s*255);
return [h,s,v];
};
dojo.graphics.color.hsv2rgb=function(h,s,v){
if(dojo.lang.isArray(h)){
v=h[2]||0;
s=h[1]||0;
h=h[0]||0;
}
h=(h/255)*360;
if(h==360){
h=0;
}
s=s/255;
v=v/255;
var r=null;
var g=null;
var b=null;
if(s==0){
r=v;
g=v;
b=v;
}else{
var _398=h/60;
var i=Math.floor(_398);
var f=_398-i;
var p=v*(1-s);
var q=v*(1-(s*f));
var t=v*(1-(s*(1-f)));
switch(i){
case 0:
r=v;
g=t;
b=p;
break;
case 1:
r=q;
g=v;
b=p;
break;
case 2:
r=p;
g=v;
b=t;
break;
case 3:
r=p;
g=q;
b=v;
break;
case 4:
r=t;
g=p;
b=v;
break;
case 5:
r=v;
g=p;
b=q;
break;
}
}
r=Math.ceil(r*255);
g=Math.ceil(g*255);
b=Math.ceil(b*255);
return [r,g,b];
};
dojo.graphics.color.rgb2hsl=function(r,g,b){
if(dojo.lang.isArray(r)){
b=r[2]||0;
g=r[1]||0;
r=r[0]||0;
}
r/=255;
g/=255;
b/=255;
var h=null;
var s=null;
var l=null;
var min=Math.min(r,g,b);
var max=Math.max(r,g,b);
var _3a6=max-min;
l=(min+max)/2;
s=0;
if((l>0)&&(l<1)){
s=_3a6/((l<0.5)?(2*l):(2-2*l));
}
h=0;
if(_3a6>0){
if((max==r)&&(max!=g)){
h+=(g-b)/_3a6;
}
if((max==g)&&(max!=b)){
h+=(2+(b-r)/_3a6);
}
if((max==b)&&(max!=r)){
h+=(4+(r-g)/_3a6);
}
h*=60;
}
h=(h==0)?360:Math.ceil((h/360)*255);
s=Math.ceil(s*255);
l=Math.ceil(l*255);
return [h,s,l];
};
dojo.graphics.color.hsl2rgb=function(h,s,l){
if(dojo.lang.isArray(h)){
l=h[2]||0;
s=h[1]||0;
h=h[0]||0;
}
h=(h/255)*360;
if(h==360){
h=0;
}
s=s/255;
l=l/255;
while(h<0){
h+=360;
}
while(h>360){
h-=360;
}
if(h<120){
r=(120-h)/60;
g=h/60;
b=0;
}else{
if(h<240){
r=0;
g=(240-h)/60;
b=(h-120)/60;
}else{
r=(h-240)/60;
g=0;
b=(360-h)/60;
}
}
r=Math.min(r,1);
g=Math.min(g,1);
b=Math.min(b,1);
r=2*s*r+(1-s);
g=2*s*g+(1-s);
b=2*s*b+(1-s);
if(l<0.5){
r=l*r;
g=l*g;
b=l*b;
}else{
r=(1-l)*r+2*l-1;
g=(1-l)*g+2*l-1;
b=(1-l)*b+2*l-1;
}
r=Math.ceil(r*255);
g=Math.ceil(g*255);
b=Math.ceil(b*255);
return [r,g,b];
};
dojo.graphics.color.hsl2hex=function(h,s,l){
var rgb=dojo.graphics.color.hsl2rgb(h,s,l);
return dojo.graphics.color.rgb2hex(rgb[0],rgb[1],rgb[2]);
};
dojo.graphics.color.hex2hsl=function(hex){
var rgb=dojo.graphics.color.hex2rgb(hex);
return dojo.graphics.color.rgb2hsl(rgb[0],rgb[1],rgb[2]);
};
dojo.provide("dojo.uri.Uri");
dojo.uri=new function(){
this.joinPath=function(){
var arr=[];
for(var i=0;i<arguments.length;i++){
arr.push(arguments[i]);
}
return arr.join("/").replace(/\/{2,}/g,"/").replace(/((https*|ftps*):)/i,"$1/");
};
this.dojoUri=function(uri){
return new dojo.uri.Uri(dojo.hostenv.getBaseScriptUri(),uri);
};
this.Uri=function(){
var uri=arguments[0];
for(var i=1;i<arguments.length;i++){
if(!arguments[i]){
continue;
}
var _3b5=new dojo.uri.Uri(arguments[i].toString());
var _3b6=new dojo.uri.Uri(uri.toString());
if(_3b5.path==""&&_3b5.scheme==null&&_3b5.authority==null&&_3b5.query==null){
if(_3b5.fragment!=null){
_3b6.fragment=_3b5.fragment;
}
_3b5=_3b6;
}else{
if(_3b5.scheme==null){
_3b5.scheme=_3b6.scheme;
if(_3b5.authority==null){
_3b5.authority=_3b6.authority;
if(_3b5.path.charAt(0)!="/"){
var path=_3b6.path.substring(0,_3b6.path.lastIndexOf("/")+1)+_3b5.path;
var segs=path.split("/");
for(var j=0;j<segs.length;j++){
if(segs[j]=="."){
if(j==segs.length-1){
segs[j]="";
}else{
segs.splice(j,1);
j--;
}
}else{
if(j>0&&!(j==1&&segs[0]=="")&&segs[j]==".."&&segs[j-1]!=".."){
if(j==segs.length-1){
segs.splice(j,1);
segs[j-1]="";
}else{
segs.splice(j-1,2);
j-=2;
}
}
}
}
_3b5.path=segs.join("/");
}
}
}
}
uri="";
if(_3b5.scheme!=null){
uri+=_3b5.scheme+":";
}
if(_3b5.authority!=null){
uri+="//"+_3b5.authority;
}
uri+=_3b5.path;
if(_3b5.query!=null){
uri+="?"+_3b5.query;
}
if(_3b5.fragment!=null){
uri+="#"+_3b5.fragment;
}
}
this.uri=uri.toString();
var _3ba="^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?$";
var r=this.uri.match(new RegExp(_3ba));
this.scheme=r[2]||(r[1]?"":null);
this.authority=r[4]||(r[3]?"":null);
this.path=r[5];
this.query=r[7]||(r[6]?"":null);
this.fragment=r[9]||(r[8]?"":null);
if(this.authority!=null){
_3ba="^((([^:]+:)?([^@]+))@)?([^:]*)(:([0-9]+))?$";
r=this.authority.match(new RegExp(_3ba));
this.user=r[3]||null;
this.password=r[4]||null;
this.host=r[5];
this.port=r[7]||null;
}
this.toString=function(){
return this.uri;
};
};
};
dojo.provide("dojo.style");
dojo.require("dojo.graphics.color");
dojo.require("dojo.uri.Uri");
dojo.style.boxSizing={marginBox:"margin-box",borderBox:"border-box",paddingBox:"padding-box",contentBox:"content-box"};
dojo.style.getBoxSizing=function(node){
node=dojo.byId(node);
if(dojo.render.html.ie||dojo.render.html.opera){
var cm=document["compatMode"];
if(cm=="BackCompat"||cm=="QuirksMode"){
return dojo.style.boxSizing.borderBox;
}else{
return dojo.style.boxSizing.contentBox;
}
}else{
if(arguments.length==0){
node=document.documentElement;
}
var _3be=dojo.style.getStyle(node,"-moz-box-sizing");
if(!_3be){
_3be=dojo.style.getStyle(node,"box-sizing");
}
return (_3be?_3be:dojo.style.boxSizing.contentBox);
}
};
dojo.style.isBorderBox=function(node){
return (dojo.style.getBoxSizing(node)==dojo.style.boxSizing.borderBox);
};
dojo.style.getUnitValue=function(node,_3c1,_3c2){
node=dojo.byId(node);
var _3c3={value:0,units:"px"};
var s=dojo.style.getComputedStyle(node,_3c1);
if(s==""||(s=="auto"&&_3c2)){
return _3c3;
}
if(dojo.lang.isUndefined(s)){
_3c3.value=NaN;
}else{
var _3c5=s.match(/(\-?[\d.]+)([a-z%]*)/i);
if(!_3c5){
_3c3.value=NaN;
}else{
_3c3.value=Number(_3c5[1]);
_3c3.units=_3c5[2].toLowerCase();
}
}
return _3c3;
};
dojo.style.getPixelValue=function(node,_3c7,_3c8){
node=dojo.byId(node);
var _3c9=dojo.style.getUnitValue(node,_3c7,_3c8);
if(isNaN(_3c9.value)){
return 0;
}
if((_3c9.value)&&(_3c9.units!="px")){
return NaN;
}
return _3c9.value;
};
dojo.style.getNumericStyle=dojo.style.getPixelValue;
dojo.style.isPositionAbsolute=function(node){
node=dojo.byId(node);
return (dojo.style.getComputedStyle(node,"position")=="absolute");
};
dojo.style.getMarginWidth=function(node){
node=dojo.byId(node);
var _3cc=dojo.style.isPositionAbsolute(node);
var left=dojo.style.getPixelValue(node,"margin-left",_3cc);
var _3ce=dojo.style.getPixelValue(node,"margin-right",_3cc);
return left+_3ce;
};
dojo.style.getBorderWidth=function(node){
node=dojo.byId(node);
var left=(dojo.style.getStyle(node,"border-left-style")=="none"?0:dojo.style.getPixelValue(node,"border-left-width"));
var _3d1=(dojo.style.getStyle(node,"border-right-style")=="none"?0:dojo.style.getPixelValue(node,"border-right-width"));
return left+_3d1;
};
dojo.style.getPaddingWidth=function(node){
node=dojo.byId(node);
var left=dojo.style.getPixelValue(node,"padding-left",true);
var _3d4=dojo.style.getPixelValue(node,"padding-right",true);
return left+_3d4;
};
dojo.style.getContentWidth=function(node){
node=dojo.byId(node);
return node.offsetWidth-dojo.style.getPaddingWidth(node)-dojo.style.getBorderWidth(node);
};
dojo.style.getInnerWidth=function(node){
node=dojo.byId(node);
return node.offsetWidth;
};
dojo.style.getOuterWidth=function(node){
node=dojo.byId(node);
return dojo.style.getInnerWidth(node)+dojo.style.getMarginWidth(node);
};
dojo.style.setOuterWidth=function(node,_3d9){
node=dojo.byId(node);
if(!dojo.style.isBorderBox(node)){
_3d9-=dojo.style.getPaddingWidth(node)+dojo.style.getBorderWidth(node);
}
_3d9-=dojo.style.getMarginWidth(node);
if(!isNaN(_3d9)&&_3d9>0){
node.style.width=_3d9+"px";
return true;
}else{
return false;
}
};
dojo.style.getContentBoxWidth=dojo.style.getContentWidth;
dojo.style.getBorderBoxWidth=dojo.style.getInnerWidth;
dojo.style.getMarginBoxWidth=dojo.style.getOuterWidth;
dojo.style.setMarginBoxWidth=dojo.style.setOuterWidth;
dojo.style.getMarginHeight=function(node){
node=dojo.byId(node);
var _3db=dojo.style.isPositionAbsolute(node);
var top=dojo.style.getPixelValue(node,"margin-top",_3db);
var _3dd=dojo.style.getPixelValue(node,"margin-bottom",_3db);
return top+_3dd;
};
dojo.style.getBorderHeight=function(node){
node=dojo.byId(node);
var top=(dojo.style.getStyle(node,"border-top-style")=="none"?0:dojo.style.getPixelValue(node,"border-top-width"));
var _3e0=(dojo.style.getStyle(node,"border-bottom-style")=="none"?0:dojo.style.getPixelValue(node,"border-bottom-width"));
return top+_3e0;
};
dojo.style.getPaddingHeight=function(node){
node=dojo.byId(node);
var top=dojo.style.getPixelValue(node,"padding-top",true);
var _3e3=dojo.style.getPixelValue(node,"padding-bottom",true);
return top+_3e3;
};
dojo.style.getContentHeight=function(node){
node=dojo.byId(node);
return node.offsetHeight-dojo.style.getPaddingHeight(node)-dojo.style.getBorderHeight(node);
};
dojo.style.getInnerHeight=function(node){
node=dojo.byId(node);
return node.offsetHeight;
};
dojo.style.getOuterHeight=function(node){
node=dojo.byId(node);
return dojo.style.getInnerHeight(node)+dojo.style.getMarginHeight(node);
};
dojo.style.setOuterHeight=function(node,_3e8){
node=dojo.byId(node);
if(!dojo.style.isBorderBox(node)){
_3e8-=dojo.style.getPaddingHeight(node)+dojo.style.getBorderHeight(node);
}
_3e8-=dojo.style.getMarginHeight(node);
if(!isNaN(_3e8)&&_3e8>0){
node.style.height=_3e8+"px";
return true;
}else{
return false;
}
};
dojo.style.setContentWidth=function(node,_3ea){
node=dojo.byId(node);
if(dojo.style.isBorderBox(node)){
_3ea+=dojo.style.getPaddingWidth(node)+dojo.style.getBorderWidth(node);
}
if(!isNaN(_3ea)&&_3ea>0){
node.style.width=_3ea+"px";
return true;
}else{
return false;
}
};
dojo.style.setContentHeight=function(node,_3ec){
node=dojo.byId(node);
if(dojo.style.isBorderBox(node)){
_3ec+=dojo.style.getPaddingHeight(node)+dojo.style.getBorderHeight(node);
}
if(!isNaN(_3ec)&&_3ec>0){
node.style.height=_3ec+"px";
return true;
}else{
return false;
}
};
dojo.style.getContentBoxHeight=dojo.style.getContentHeight;
dojo.style.getBorderBoxHeight=dojo.style.getInnerHeight;
dojo.style.getMarginBoxHeight=dojo.style.getOuterHeight;
dojo.style.setMarginBoxHeight=dojo.style.setOuterHeight;
dojo.style.getTotalOffset=function(node,type,_3ef){
node=dojo.byId(node);
var _3f0=(type=="top")?"offsetTop":"offsetLeft";
var _3f1=(type=="top")?"scrollTop":"scrollLeft";
var _3f2=(type=="top")?"y":"x";
var _3f3=0;
if(node["offsetParent"]){
var _3f4;
if(dojo.render.html.safari&&node.style.getPropertyValue("position")=="absolute"&&node.parentNode==document.body){
_3f4=document.body;
}else{
_3f4=document.body.parentNode;
}
if(_3ef&&node.parentNode!=document.body){
_3f3-=dojo.style.sumAncestorProperties(node,_3f1);
}
do{
var n=node[_3f0];
_3f3+=isNaN(n)?0:n;
node=node.offsetParent;
}while(node!=_3f4&&node!=null);
}else{
if(node[_3f2]){
var n=node[_3f2];
_3f3+=isNaN(n)?0:n;
}
}
_3f3+=(type=="top")?dojo.html.getScrollTop():dojo.html.getScrollLeft();
return _3f3;
};
dojo.style.sumAncestorProperties=function(node,prop){
node=dojo.byId(node);
if(!node){
return 0;
}
var _3f8=0;
while(node){
var val=node[prop];
if(val){
_3f8+=val-0;
}
node=node.parentNode;
}
return _3f8;
};
dojo.style.totalOffsetLeft=function(node,_3fb){
node=dojo.byId(node);
return dojo.style.getTotalOffset(node,"left",_3fb);
};
dojo.style.getAbsoluteX=dojo.style.totalOffsetLeft;
dojo.style.totalOffsetTop=function(node,_3fd){
node=dojo.byId(node);
return dojo.style.getTotalOffset(node,"top",_3fd);
};
dojo.style.getAbsoluteY=dojo.style.totalOffsetTop;
dojo.style.getAbsolutePosition=function(node,_3ff){
node=dojo.byId(node);
var _400=[dojo.style.getAbsoluteX(node,_3ff),dojo.style.getAbsoluteY(node,_3ff)];
_400.x=_400[0];
_400.y=_400[1];
return _400;
};
dojo.style.styleSheet=null;
dojo.style.insertCssRule=function(_401,_402,_403){
if(!dojo.style.styleSheet){
if(document.createStyleSheet){
dojo.style.styleSheet=document.createStyleSheet();
}else{
if(document.styleSheets[0]){
dojo.style.styleSheet=document.styleSheets[0];
}else{
return null;
}
}
}
if(arguments.length<3){
if(dojo.style.styleSheet.cssRules){
_403=dojo.style.styleSheet.cssRules.length;
}else{
if(dojo.style.styleSheet.rules){
_403=dojo.style.styleSheet.rules.length;
}else{
return null;
}
}
}
if(dojo.style.styleSheet.insertRule){
var rule=_401+" { "+_402+" }";
return dojo.style.styleSheet.insertRule(rule,_403);
}else{
if(dojo.style.styleSheet.addRule){
return dojo.style.styleSheet.addRule(_401,_402,_403);
}else{
return null;
}
}
};
dojo.style.removeCssRule=function(_405){
if(!dojo.style.styleSheet){
dojo.debug("no stylesheet defined for removing rules");
return false;
}
if(dojo.render.html.ie){
if(!_405){
_405=dojo.style.styleSheet.rules.length;
dojo.style.styleSheet.removeRule(_405);
}
}else{
if(document.styleSheets[0]){
if(!_405){
_405=dojo.style.styleSheet.cssRules.length;
}
dojo.style.styleSheet.deleteRule(_405);
}
}
return true;
};
dojo.style.insertCssFile=function(URI,doc,_408){
if(!URI){
return;
}
if(!doc){
doc=document;
}
var _409=dojo.hostenv.getText(URI);
_409=dojo.style.fixPathsInCssText(_409,URI);
if(_408){
var _40a=doc.getElementsByTagName("style");
var _40b="";
for(var i=0;i<_40a.length;i++){
_40b=(_40a[i].styleSheet&&_40a[i].styleSheet.cssText)?_40a[i].styleSheet.cssText:_40a[i].innerHTML;
if(_409==_40b){
return;
}
}
}
var _40d=dojo.style.insertCssText(_409);
if(_40d&&djConfig.isDebug){
_40d.setAttribute("dbgHref",URI);
}
return _40d;
};
dojo.style.insertCssText=function(_40e,doc,URI){
if(!_40e){
return;
}
if(!doc){
doc=document;
}
if(URI){
_40e=dojo.style.fixPathsInCssText(_40e,URI);
}
var _411=doc.createElement("style");
_411.setAttribute("type","text/css");
if(_411.styleSheet){
_411.styleSheet.cssText=_40e;
}else{
var _412=doc.createTextNode(_40e);
_411.appendChild(_412);
}
var head=doc.getElementsByTagName("head")[0];
if(head){
head.appendChild(_411);
}
return _411;
};
dojo.style.fixPathsInCssText=function(_414,URI){
if(!_414||!URI){
return;
}
var pos=0;
var str="";
var url="";
while(pos!=-1){
pos=0;
url="";
pos=_414.indexOf("url(",pos);
if(pos<0){
break;
}
str+=_414.slice(0,pos+4);
_414=_414.substring(pos+4,_414.length);
url+=_414.match(/^[\t\s\w()\/.\\'"-:#=&?]*\)/)[0];
_414=_414.substring(url.length-1,_414.length);
url=url.replace(/^[\s\t]*(['"]?)([\w()\/.\\'"-:#=&?]*)\1[\s\t]*?\)/,"$2");
if(url.search(/(file|https?|ftps?):\/\//)==-1){
url=(new dojo.uri.Uri(URI,url).toString());
}
str+=url;
}
return str+_414;
};
dojo.style.getBackgroundColor=function(node){
node=dojo.byId(node);
var _41a;
do{
_41a=dojo.style.getStyle(node,"background-color");
if(_41a.toLowerCase()=="rgba(0, 0, 0, 0)"){
_41a="transparent";
}
if(node==document.getElementsByTagName("body")[0]){
node=null;
break;
}
node=node.parentNode;
}while(node&&dojo.lang.inArray(_41a,["transparent",""]));
if(_41a=="transparent"){
_41a=[255,255,255,0];
}else{
_41a=dojo.graphics.color.extractRGB(_41a);
}
return _41a;
};
dojo.style.getComputedStyle=function(node,_41c,_41d){
node=dojo.byId(node);
var _41c=dojo.style.toSelectorCase(_41c);
var _41e=dojo.style.toCamelCase(_41c);
if(!node||!node.style){
return _41d;
}else{
if(document.defaultView){
try{
var cs=document.defaultView.getComputedStyle(node,"");
if(cs){
return cs.getPropertyValue(_41c);
}
}
catch(e){
if(node.style.getPropertyValue){
return node.style.getPropertyValue(_41c);
}else{
return _41d;
}
}
}else{
if(node.currentStyle){
return node.currentStyle[_41e];
}
}
}
if(node.style.getPropertyValue){
return node.style.getPropertyValue(_41c);
}else{
return _41d;
}
};
dojo.style.getStyleProperty=function(node,_421){
node=dojo.byId(node);
return (node&&node.style?node.style[dojo.style.toCamelCase(_421)]:undefined);
};
dojo.style.getStyle=function(node,_423){
node=dojo.byId(node);
var _424=dojo.style.getStyleProperty(node,_423);
return (_424?_424:dojo.style.getComputedStyle(node,_423));
};
dojo.style.setStyle=function(node,_426,_427){
node=dojo.byId(node);
if(node&&node.style){
var _428=dojo.style.toCamelCase(_426);
node.style[_428]=_427;
}
};
dojo.style.toCamelCase=function(_429){
var arr=_429.split("-"),cc=arr[0];
for(var i=1;i<arr.length;i++){
cc+=arr[i].charAt(0).toUpperCase()+arr[i].substring(1);
}
return cc;
};
dojo.style.toSelectorCase=function(_42c){
return _42c.replace(/([A-Z])/g,"-$1").toLowerCase();
};
dojo.style.setOpacity=function setOpacity(node,_42e,_42f){
node=dojo.byId(node);
var h=dojo.render.html;
if(!_42f){
if(_42e>=1){
if(h.ie){
dojo.style.clearOpacity(node);
return;
}else{
_42e=0.999999;
}
}else{
if(_42e<0){
_42e=0;
}
}
}
if(h.ie){
if(node.nodeName.toLowerCase()=="tr"){
var tds=node.getElementsByTagName("td");
for(var x=0;x<tds.length;x++){
tds[x].style.filter="Alpha(Opacity="+_42e*100+")";
}
}
node.style.filter="Alpha(Opacity="+_42e*100+")";
}else{
if(h.moz){
node.style.opacity=_42e;
node.style.MozOpacity=_42e;
}else{
if(h.safari){
node.style.opacity=_42e;
node.style.KhtmlOpacity=_42e;
}else{
node.style.opacity=_42e;
}
}
}
};
dojo.style.getOpacity=function getOpacity(node){
node=dojo.byId(node);
if(dojo.render.html.ie){
var opac=(node.filters&&node.filters.alpha&&typeof node.filters.alpha.opacity=="number"?node.filters.alpha.opacity:100)/100;
}else{
var opac=node.style.opacity||node.style.MozOpacity||node.style.KhtmlOpacity||1;
}
return opac>=0.999999?1:Number(opac);
};
dojo.style.clearOpacity=function clearOpacity(node){
node=dojo.byId(node);
var h=dojo.render.html;
if(h.ie){
if(node.filters&&node.filters.alpha){
node.style.filter="";
}
}else{
if(h.moz){
node.style.opacity=1;
node.style.MozOpacity=1;
}else{
if(h.safari){
node.style.opacity=1;
node.style.KhtmlOpacity=1;
}else{
node.style.opacity=1;
}
}
}
};
dojo.style._toggle=function(node,_438,_439){
node=dojo.byId(node);
_439(node,!_438(node));
return _438(node);
};
dojo.style.setShowing=function(node,_43b){
dojo.style.setStyle(node,"display",(_43b?"":"none"));
};
dojo.style.isShowing=function(node){
return (dojo.style.getStyleProperty(node,"display")!="none");
};
dojo.style.toggleShowing=function(node){
return dojo.style._toggle(node,dojo.style.isShowing,dojo.style.setShowing);
};
dojo.style.show=function(node){
dojo.style.setShowing(node,true);
};
dojo.style.hide=function(node){
dojo.style.setShowing(node,false);
};
dojo.style.displayMap={tr:"",td:"",th:"",img:"inline",span:"inline",input:"inline",button:"inline"};
dojo.style.suggestDisplayByTagName=function(node){
node=dojo.byId(node);
if(node&&node.tagName){
var tag=node.tagName.toLowerCase();
return (tag in dojo.style.displayMap?dojo.style.displayMap[tag]:"block");
}
};
dojo.style.setDisplay=function(node,_443){
dojo.style.setStyle(node,"display",(dojo.lang.isString(_443)?_443:(_443?dojo.style.suggestDisplayByTagName(node):"none")));
};
dojo.style.isDisplayed=function(node){
return (dojo.style.getComputedStyle(node,"display")!="none");
};
dojo.style.toggleDisplay=function(node){
return dojo.style._toggle(node,dojo.style.isDisplayed,dojo.style.setDisplay);
};
dojo.style.setVisibility=function(node,_447){
dojo.style.setStyle(node,"visibility",(dojo.lang.isString(_447)?_447:(_447?"visible":"hidden")));
};
dojo.style.isVisible=function(node){
return (dojo.style.getComputedStyle(node,"visibility")!="hidden");
};
dojo.style.toggleVisibility=function(node){
return dojo.style._toggle(node,dojo.style.isVisible,dojo.style.setVisibility);
};
dojo.style.toCoordinateArray=function(_44a,_44b){
if(dojo.lang.isArray(_44a)){
while(_44a.length<4){
_44a.push(0);
}
while(_44a.length>4){
_44a.pop();
}
var ret=_44a;
}else{
var node=dojo.byId(_44a);
var ret=[dojo.style.getAbsoluteX(node,_44b),dojo.style.getAbsoluteY(node,_44b),dojo.style.getInnerWidth(node),dojo.style.getInnerHeight(node)];
}
ret.x=ret[0];
ret.y=ret[1];
ret.w=ret[2];
ret.h=ret[3];
return ret;
};
dojo.provide("dojo.math.curves");
dojo.require("dojo.math");
dojo.math.curves={Line:function(_44e,end){
this.start=_44e;
this.end=end;
this.dimensions=_44e.length;
for(var i=0;i<_44e.length;i++){
_44e[i]=Number(_44e[i]);
}
for(var i=0;i<end.length;i++){
end[i]=Number(end[i]);
}
this.getValue=function(n){
var _452=new Array(this.dimensions);
for(var i=0;i<this.dimensions;i++){
_452[i]=((this.end[i]-this.start[i])*n)+this.start[i];
}
return _452;
};
return this;
},Bezier:function(pnts){
this.getValue=function(step){
if(step>=1){
return this.p[this.p.length-1];
}
if(step<=0){
return this.p[0];
}
var _456=new Array(this.p[0].length);
for(var k=0;j<this.p[0].length;k++){
_456[k]=0;
}
for(var j=0;j<this.p[0].length;j++){
var C=0;
var D=0;
for(var i=0;i<this.p.length;i++){
C+=this.p[i][j]*this.p[this.p.length-1][0]*dojo.math.bernstein(step,this.p.length,i);
}
for(var l=0;l<this.p.length;l++){
D+=this.p[this.p.length-1][0]*dojo.math.bernstein(step,this.p.length,l);
}
_456[j]=C/D;
}
return _456;
};
this.p=pnts;
return this;
},CatmullRom:function(pnts,c){
this.getValue=function(step){
var _460=step*(this.p.length-1);
var node=Math.floor(_460);
var _462=_460-node;
var i0=node-1;
if(i0<0){
i0=0;
}
var i=node;
var i1=node+1;
if(i1>=this.p.length){
i1=this.p.length-1;
}
var i2=node+2;
if(i2>=this.p.length){
i2=this.p.length-1;
}
var u=_462;
var u2=_462*_462;
var u3=_462*_462*_462;
var _46a=new Array(this.p[0].length);
for(var k=0;k<this.p[0].length;k++){
var x1=(-this.c*this.p[i0][k])+((2-this.c)*this.p[i][k])+((this.c-2)*this.p[i1][k])+(this.c*this.p[i2][k]);
var x2=(2*this.c*this.p[i0][k])+((this.c-3)*this.p[i][k])+((3-2*this.c)*this.p[i1][k])+(-this.c*this.p[i2][k]);
var x3=(-this.c*this.p[i0][k])+(this.c*this.p[i1][k]);
var x4=this.p[i][k];
_46a[k]=x1*u3+x2*u2+x3*u+x4;
}
return _46a;
};
if(!c){
this.c=0.7;
}else{
this.c=c;
}
this.p=pnts;
return this;
},Arc:function(_470,end,ccw){
var _473=dojo.math.points.midpoint(_470,end);
var _474=dojo.math.points.translate(dojo.math.points.invert(_473),_470);
var rad=Math.sqrt(Math.pow(_474[0],2)+Math.pow(_474[1],2));
var _476=dojo.math.radToDeg(Math.atan(_474[1]/_474[0]));
if(_474[0]<0){
_476-=90;
}else{
_476+=90;
}
dojo.math.curves.CenteredArc.call(this,_473,rad,_476,_476+(ccw?-180:180));
},CenteredArc:function(_477,_478,_479,end){
this.center=_477;
this.radius=_478;
this.start=_479||0;
this.end=end;
this.getValue=function(n){
var _47c=new Array(2);
var _47d=dojo.math.degToRad(this.start+((this.end-this.start)*n));
_47c[0]=this.center[0]+this.radius*Math.sin(_47d);
_47c[1]=this.center[1]-this.radius*Math.cos(_47d);
return _47c;
};
return this;
},Circle:function(_47e,_47f){
dojo.math.curves.CenteredArc.call(this,_47e,_47f,0,360);
return this;
},Path:function(){
var _480=[];
var _481=[];
var _482=[];
var _483=0;
this.add=function(_484,_485){
if(_485<0){
dojo.raise("dojo.math.curves.Path.add: weight cannot be less than 0");
}
_480.push(_484);
_481.push(_485);
_483+=_485;
computeRanges();
};
this.remove=function(_486){
for(var i=0;i<_480.length;i++){
if(_480[i]==_486){
_480.splice(i,1);
_483-=_481.splice(i,1)[0];
break;
}
}
computeRanges();
};
this.removeAll=function(){
_480=[];
_481=[];
_483=0;
};
this.getValue=function(n){
var _489=false,value=0;
for(var i=0;i<_482.length;i++){
var r=_482[i];
if(n>=r[0]&&n<r[1]){
var subN=(n-r[0])/r[2];
value=_480[i].getValue(subN);
_489=true;
break;
}
}
if(!_489){
value=_480[_480.length-1].getValue(1);
}
for(j=0;j<i;j++){
value=dojo.math.points.translate(value,_480[j].getValue(1));
}
return value;
};
function computeRanges(){
var _48d=0;
for(var i=0;i<_481.length;i++){
var end=_48d+_481[i]/_483;
var len=end-_48d;
_482[i]=[_48d,end,len];
_48d=end;
}
}
return this;
}};
dojo.provide("dojo.animation.AnimationEvent");
dojo.require("dojo.lang");
dojo.animation.AnimationEvent=function(anim,type,_493,_494,_495,_496,dur,pct,fps){
this.type=type;
this.animation=anim;
this.coords=_493;
this.x=_493[0];
this.y=_493[1];
this.z=_493[2];
this.startTime=_494;
this.currentTime=_495;
this.endTime=_496;
this.duration=dur;
this.percent=pct;
this.fps=fps;
};
dojo.lang.extend(dojo.animation.AnimationEvent,{coordsAsInts:function(){
var _49a=new Array(this.coords.length);
for(var i=0;i<this.coords.length;i++){
_49a[i]=Math.round(this.coords[i]);
}
return _49a;
}});
dojo.provide("dojo.animation.Animation");
dojo.require("dojo.animation.AnimationEvent");
dojo.require("dojo.lang.func");
dojo.require("dojo.math");
dojo.require("dojo.math.curves");
dojo.animation.Animation=function(_49c,_49d,_49e,_49f,rate){
if(dojo.lang.isArray(_49c)){
_49c=new dojo.math.curves.Line(_49c[0],_49c[1]);
}
this.curve=_49c;
this.duration=_49d;
this.repeatCount=_49f||0;
this.rate=rate||25;
if(_49e){
if(dojo.lang.isFunction(_49e.getValue)){
this.accel=_49e;
}else{
var i=0.35*_49e+0.5;
this.accel=new dojo.math.curves.CatmullRom([[0],[i],[1]],0.45);
}
}
};
dojo.lang.extend(dojo.animation.Animation,{curve:null,duration:0,repeatCount:0,accel:null,onBegin:null,onAnimate:null,onEnd:null,onPlay:null,onPause:null,onStop:null,handler:null,_animSequence:null,_startTime:null,_endTime:null,_lastFrame:null,_timer:null,_percent:0,_active:false,_paused:false,_startRepeatCount:0,play:function(_4a2){
if(_4a2){
clearTimeout(this._timer);
this._active=false;
this._paused=false;
this._percent=0;
}else{
if(this._active&&!this._paused){
return;
}
}
this._startTime=new Date().valueOf();
if(this._paused){
this._startTime-=(this.duration*this._percent/100);
}
this._endTime=this._startTime+this.duration;
this._lastFrame=this._startTime;
var e=new dojo.animation.AnimationEvent(this,null,this.curve.getValue(this._percent),this._startTime,this._startTime,this._endTime,this.duration,this._percent,0);
this._active=true;
this._paused=false;
if(this._percent==0){
if(!this._startRepeatCount){
this._startRepeatCount=this.repeatCount;
}
e.type="begin";
if(typeof this.handler=="function"){
this.handler(e);
}
if(typeof this.onBegin=="function"){
this.onBegin(e);
}
}
e.type="play";
if(typeof this.handler=="function"){
this.handler(e);
}
if(typeof this.onPlay=="function"){
this.onPlay(e);
}
if(this._animSequence){
this._animSequence._setCurrent(this);
}
this._cycle();
},pause:function(){
clearTimeout(this._timer);
if(!this._active){
return;
}
this._paused=true;
var e=new dojo.animation.AnimationEvent(this,"pause",this.curve.getValue(this._percent),this._startTime,new Date().valueOf(),this._endTime,this.duration,this._percent,0);
if(typeof this.handler=="function"){
this.handler(e);
}
if(typeof this.onPause=="function"){
this.onPause(e);
}
},playPause:function(){
if(!this._active||this._paused){
this.play();
}else{
this.pause();
}
},gotoPercent:function(pct,_4a6){
clearTimeout(this._timer);
this._active=true;
this._paused=true;
this._percent=pct;
if(_4a6){
this.play();
}
},stop:function(_4a7){
clearTimeout(this._timer);
var step=this._percent/100;
if(_4a7){
step=1;
}
var e=new dojo.animation.AnimationEvent(this,"stop",this.curve.getValue(step),this._startTime,new Date().valueOf(),this._endTime,this.duration,this._percent,Math.round(fps));
if(typeof this.handler=="function"){
this.handler(e);
}
if(typeof this.onStop=="function"){
this.onStop(e);
}
this._active=false;
this._paused=false;
},status:function(){
if(this._active){
return this._paused?"paused":"playing";
}else{
return "stopped";
}
},_cycle:function(){
clearTimeout(this._timer);
if(this._active){
var curr=new Date().valueOf();
var step=(curr-this._startTime)/(this._endTime-this._startTime);
fps=1000/(curr-this._lastFrame);
this._lastFrame=curr;
if(step>=1){
step=1;
this._percent=100;
}else{
this._percent=step*100;
}
if(this.accel&&this.accel.getValue){
step=this.accel.getValue(step);
}
var e=new dojo.animation.AnimationEvent(this,"animate",this.curve.getValue(step),this._startTime,curr,this._endTime,this.duration,this._percent,Math.round(fps));
if(typeof this.handler=="function"){
this.handler(e);
}
if(typeof this.onAnimate=="function"){
this.onAnimate(e);
}
if(step<1){
this._timer=setTimeout(dojo.lang.hitch(this,"_cycle"),this.rate);
}else{
e.type="end";
this._active=false;
if(typeof this.handler=="function"){
this.handler(e);
}
if(typeof this.onEnd=="function"){
this.onEnd(e);
}
if(this.repeatCount>0){
this.repeatCount--;
this.play(true);
}else{
if(this.repeatCount==-1){
this.play(true);
}else{
if(this._startRepeatCount){
this.repeatCount=this._startRepeatCount;
this._startRepeatCount=0;
}
if(this._animSequence){
this._animSequence._playNext();
}
}
}
}
}
}});
dojo.provide("dojo.animation");
dojo.require("dojo.animation.Animation");
dojo.provide("dojo.fx.html");
dojo.require("dojo.style");
dojo.require("dojo.math.curves");
dojo.require("dojo.lang.func");
dojo.require("dojo.animation");
dojo.require("dojo.event.*");
dojo.require("dojo.graphics.color");
dojo.fx.duration=300;
dojo.fx.html._makeFadeable=function(node){
if(dojo.render.html.ie){
if((node.style.zoom.length==0)&&(dojo.style.getStyle(node,"zoom")=="normal")){
node.style.zoom="1";
}
if((node.style.width.length==0)&&(dojo.style.getStyle(node,"width")=="auto")){
node.style.width="auto";
}
}
};
dojo.fx.html.fadeOut=function(node,_4af,_4b0,_4b1){
return dojo.fx.html.fade(node,_4af,dojo.style.getOpacity(node),0,_4b0,_4b1);
};
dojo.fx.html.fadeIn=function(node,_4b3,_4b4,_4b5){
return dojo.fx.html.fade(node,_4b3,dojo.style.getOpacity(node),1,_4b4,_4b5);
};
dojo.fx.html.fadeHide=function(node,_4b7,_4b8,_4b9){
node=dojo.byId(node);
if(!_4b7){
_4b7=150;
}
return dojo.fx.html.fadeOut(node,_4b7,function(node){
node.style.display="none";
if(typeof _4b8=="function"){
_4b8(node);
}
});
};
dojo.fx.html.fadeShow=function(node,_4bc,_4bd,_4be){
node=dojo.byId(node);
if(!_4bc){
_4bc=150;
}
node.style.display="block";
return dojo.fx.html.fade(node,_4bc,0,1,_4bd,_4be);
};
dojo.fx.html.fade=function(node,_4c0,_4c1,_4c2,_4c3,_4c4){
node=dojo.byId(node);
dojo.fx.html._makeFadeable(node);
var anim=new dojo.animation.Animation(new dojo.math.curves.Line([_4c1],[_4c2]),_4c0||dojo.fx.duration,0);
dojo.event.connect(anim,"onAnimate",function(e){
dojo.style.setOpacity(node,e.x);
});
if(_4c3){
dojo.event.connect(anim,"onEnd",function(e){
_4c3(node,anim);
});
}
if(!_4c4){
anim.play(true);
}
return anim;
};
dojo.fx.html.slideTo=function(node,_4c9,_4ca,_4cb,_4cc){
if(!dojo.lang.isNumber(_4c9)){
var tmp=_4c9;
_4c9=_4ca;
_4ca=tmp;
}
node=dojo.byId(node);
var top=node.offsetTop;
var left=node.offsetLeft;
var pos=dojo.style.getComputedStyle(node,"position");
if(pos=="relative"||pos=="static"){
top=parseInt(dojo.style.getComputedStyle(node,"top"))||0;
left=parseInt(dojo.style.getComputedStyle(node,"left"))||0;
}
return dojo.fx.html.slide(node,_4c9,[left,top],_4ca,_4cb,_4cc);
};
dojo.fx.html.slideBy=function(node,_4d2,_4d3,_4d4,_4d5){
if(!dojo.lang.isNumber(_4d2)){
var tmp=_4d2;
_4d2=_4d3;
_4d3=tmp;
}
node=dojo.byId(node);
var top=node.offsetTop;
var left=node.offsetLeft;
var pos=dojo.style.getComputedStyle(node,"position");
if(pos=="relative"||pos=="static"){
top=parseInt(dojo.style.getComputedStyle(node,"top"))||0;
left=parseInt(dojo.style.getComputedStyle(node,"left"))||0;
}
return dojo.fx.html.slideTo(node,_4d2,[left+_4d3[0],top+_4d3[1]],_4d4,_4d5);
};
dojo.fx.html.slide=function(node,_4db,_4dc,_4dd,_4de,_4df){
if(!dojo.lang.isNumber(_4db)){
var tmp=_4db;
_4db=_4dd;
_4dd=_4dc;
_4dc=tmp;
}
node=dojo.byId(node);
if(dojo.style.getComputedStyle(node,"position")=="static"){
node.style.position="relative";
}
var anim=new dojo.animation.Animation(new dojo.math.curves.Line(_4dc,_4dd),_4db||dojo.fx.duration,0);
dojo.event.connect(anim,"onAnimate",function(e){
with(node.style){
left=e.x+"px";
top=e.y+"px";
}
});
if(_4de){
dojo.event.connect(anim,"onEnd",function(e){
_4de(node,anim);
});
}
if(!_4df){
anim.play(true);
}
return anim;
};
dojo.fx.html.colorFadeIn=function(node,_4e5,_4e6,_4e7,_4e8,_4e9){
if(!dojo.lang.isNumber(_4e5)){
var tmp=_4e5;
_4e5=_4e6;
_4e6=tmp;
}
node=dojo.byId(node);
var _4eb=dojo.style.getBackgroundColor(node);
var bg=dojo.style.getStyle(node,"background-color").toLowerCase();
var _4ed=bg=="transparent"||bg=="rgba(0, 0, 0, 0)";
while(_4eb.length>3){
_4eb.pop();
}
var rgb=new dojo.graphics.color.Color(_4e6).toRgb();
var anim=dojo.fx.html.colorFade(node,_4e5||dojo.fx.duration,_4e6,_4eb,_4e8,true);
dojo.event.connect(anim,"onEnd",function(e){
if(_4ed){
node.style.backgroundColor="transparent";
}
});
if(_4e7>0){
node.style.backgroundColor="rgb("+rgb.join(",")+")";
if(!_4e9){
setTimeout(function(){
anim.play(true);
},_4e7);
}
}else{
if(!_4e9){
anim.play(true);
}
}
return anim;
};
dojo.fx.html.highlight=dojo.fx.html.colorFadeIn;
dojo.fx.html.colorFadeFrom=dojo.fx.html.colorFadeIn;
dojo.fx.html.colorFadeOut=function(node,_4f2,_4f3,_4f4,_4f5,_4f6){
if(!dojo.lang.isNumber(_4f2)){
var tmp=_4f2;
_4f2=_4f3;
_4f3=tmp;
}
node=dojo.byId(node);
var _4f8=new dojo.graphics.color.Color(dojo.style.getBackgroundColor(node)).toRgb();
var rgb=new dojo.graphics.color.Color(_4f3).toRgb();
var anim=dojo.fx.html.colorFade(node,_4f2||dojo.fx.duration,_4f8,rgb,_4f5,_4f4>0||_4f6);
if(_4f4>0){
node.style.backgroundColor="rgb("+_4f8.join(",")+")";
if(!_4f6){
setTimeout(function(){
anim.play(true);
},_4f4);
}
}
return anim;
};
dojo.fx.html.unhighlight=dojo.fx.html.colorFadeOut;
dojo.fx.html.colorFadeTo=dojo.fx.html.colorFadeOut;
dojo.fx.html.colorFade=function(node,_4fc,_4fd,_4fe,_4ff,_500){
if(!dojo.lang.isNumber(_4fc)){
var tmp=_4fc;
_4fc=_4fe;
_4fe=_4fd;
_4fd=tmp;
}
node=dojo.byId(node);
var _502=new dojo.graphics.color.Color(_4fd).toRgb();
var _503=new dojo.graphics.color.Color(_4fe).toRgb();
var anim=new dojo.animation.Animation(new dojo.math.curves.Line(_502,_503),_4fc||dojo.fx.duration,0);
dojo.event.connect(anim,"onAnimate",function(e){
node.style.backgroundColor="rgb("+e.coordsAsInts().join(",")+")";
});
if(_4ff){
dojo.event.connect(anim,"onEnd",function(e){
_4ff(node,anim);
});
}
if(!_500){
anim.play(true);
}
return anim;
};
dojo.fx.html.wipeIn=function(node,_508,_509,_50a){
node=dojo.byId(node);
var _50b=dojo.style.getStyle(node,"overflow");
if(_50b=="visible"){
node.style.overflow="hidden";
}
node.style.height=0;
dojo.style.show(node);
var anim=dojo.fx.html.wipe(node,_508,0,node.scrollHeight,null,true);
dojo.event.connect(anim,"onEnd",function(){
node.style.overflow=_50b;
node.style.height="auto";
if(_509){
_509(node,anim);
}
});
if(!_50a){
anim.play();
}
return anim;
};
dojo.fx.html.wipeOut=function(node,_50e,_50f,_510){
node=dojo.byId(node);
var _511=dojo.style.getStyle(node,"overflow");
if(_511=="visible"){
node.style.overflow="hidden";
}
var anim=dojo.fx.html.wipe(node,_50e,node.offsetHeight,0,null,true);
dojo.event.connect(anim,"onEnd",function(){
dojo.style.hide(node);
node.style.overflow=_511;
if(_50f){
_50f(node,anim);
}
});
if(!_510){
anim.play();
}
return anim;
};
dojo.fx.html.wipe=function(node,_514,_515,_516,_517,_518){
node=dojo.byId(node);
var anim=new dojo.animation.Animation([[_515],[_516]],_514||dojo.fx.duration,0);
dojo.event.connect(anim,"onAnimate",function(e){
node.style.height=e.x+"px";
});
dojo.event.connect(anim,"onEnd",function(){
if(_517){
_517(node,anim);
}
});
if(!_518){
anim.play();
}
return anim;
};
dojo.fx.html.wiper=function(node,_51c){
this.node=dojo.byId(node);
if(_51c){
dojo.event.connect(dojo.byId(_51c),"onclick",this,"toggle");
}
};
dojo.lang.extend(dojo.fx.html.wiper,{duration:dojo.fx.duration,_anim:null,toggle:function(){
if(!this._anim){
var type="wipe"+(dojo.style.isVisible(this.node)?"Hide":"Show");
this._anim=dojo.fx[type](this.node,this.duration,dojo.lang.hitch(this,"_callback"));
}
},_callback:function(){
this._anim=null;
}});
dojo.fx.html.explode=function(_51e,_51f,_520,_521,_522){
var _523=dojo.style.toCoordinateArray(_51e);
var _524=document.createElement("div");
with(_524.style){
position="absolute";
border="1px solid black";
display="none";
}
document.body.appendChild(_524);
_51f=dojo.byId(_51f);
with(_51f.style){
visibility="hidden";
display="block";
}
var _525=dojo.style.toCoordinateArray(_51f);
with(_51f.style){
display="none";
visibility="visible";
}
var anim=new dojo.animation.Animation(new dojo.math.curves.Line(_523,_525),_520||dojo.fx.duration,0);
dojo.event.connect(anim,"onBegin",function(e){
_524.style.display="block";
});
dojo.event.connect(anim,"onAnimate",function(e){
with(_524.style){
left=e.x+"px";
top=e.y+"px";
width=e.coords[2]+"px";
height=e.coords[3]+"px";
}
});
dojo.event.connect(anim,"onEnd",function(){
_51f.style.display="block";
_524.parentNode.removeChild(_524);
if(_521){
_521(_51f,anim);
}
});
if(!_522){
anim.play();
}
return anim;
};
dojo.fx.html.implode=function(_529,end,_52b,_52c,_52d){
var _52e=dojo.style.toCoordinateArray(_529);
var _52f=dojo.style.toCoordinateArray(end);
_529=dojo.byId(_529);
var _530=document.createElement("div");
with(_530.style){
position="absolute";
border="1px solid black";
display="none";
}
document.body.appendChild(_530);
var anim=new dojo.animation.Animation(new dojo.math.curves.Line(_52e,_52f),_52b||dojo.fx.duration,0);
dojo.event.connect(anim,"onBegin",function(e){
_529.style.display="none";
_530.style.display="block";
});
dojo.event.connect(anim,"onAnimate",function(e){
with(_530.style){
left=e.x+"px";
top=e.y+"px";
width=e.coords[2]+"px";
height=e.coords[3]+"px";
}
});
dojo.event.connect(anim,"onEnd",function(){
_530.parentNode.removeChild(_530);
if(_52c){
_52c(_529,anim);
}
});
if(!_52d){
anim.play();
}
return anim;
};
dojo.fx.html.Exploder=function(_534,_535){
_534=dojo.byId(_534);
_535=dojo.byId(_535);
var _536=this;
this.waitToHide=500;
this.timeToShow=100;
this.waitToShow=200;
this.timeToHide=70;
this.autoShow=false;
this.autoHide=false;
var _537=null;
var _538=null;
var _539=null;
var _53a=null;
var _53b=null;
var _53c=null;
this.showing=false;
this.onBeforeExplode=null;
this.onAfterExplode=null;
this.onBeforeImplode=null;
this.onAfterImplode=null;
this.onExploding=null;
this.onImploding=null;
this.timeShow=function(){
clearTimeout(_539);
_539=setTimeout(_536.show,_536.waitToShow);
};
this.show=function(){
clearTimeout(_539);
clearTimeout(_53a);
if((_538&&_538.status()=="playing")||(_537&&_537.status()=="playing")||_536.showing){
return;
}
if(typeof _536.onBeforeExplode=="function"){
_536.onBeforeExplode(_534,_535);
}
_537=dojo.fx.html.explode(_534,_535,_536.timeToShow,function(e){
_536.showing=true;
if(typeof _536.onAfterExplode=="function"){
_536.onAfterExplode(_534,_535);
}
});
if(typeof _536.onExploding=="function"){
dojo.event.connect(_537,"onAnimate",this,"onExploding");
}
};
this.timeHide=function(){
clearTimeout(_539);
clearTimeout(_53a);
if(_536.showing){
_53a=setTimeout(_536.hide,_536.waitToHide);
}
};
this.hide=function(){
clearTimeout(_539);
clearTimeout(_53a);
if(_537&&_537.status()=="playing"){
return;
}
_536.showing=false;
if(typeof _536.onBeforeImplode=="function"){
_536.onBeforeImplode(_534,_535);
}
_538=dojo.fx.html.implode(_535,_534,_536.timeToHide,function(e){
if(typeof _536.onAfterImplode=="function"){
_536.onAfterImplode(_534,_535);
}
});
if(typeof _536.onImploding=="function"){
dojo.event.connect(_538,"onAnimate",this,"onImploding");
}
};
dojo.event.connect(_534,"onclick",function(e){
if(_536.showing){
_536.hide();
}else{
_536.show();
}
});
dojo.event.connect(_534,"onmouseover",function(e){
if(_536.autoShow){
_536.timeShow();
}
});
dojo.event.connect(_534,"onmouseout",function(e){
if(_536.autoHide){
_536.timeHide();
}
});
dojo.event.connect(_535,"onmouseover",function(e){
clearTimeout(_53a);
});
dojo.event.connect(_535,"onmouseout",function(e){
if(_536.autoHide){
_536.timeHide();
}
});
dojo.event.connect(document.documentElement||document.body,"onclick",function(e){
function isDesc(node,_546){
while(node){
if(node==_546){
return true;
}
node=node.parentNode;
}
return false;
}
if(_536.autoHide&&_536.showing&&!isDesc(e.target,_535)&&!isDesc(e.target,_534)){
_536.hide();
}
});
return this;
};
dojo.fx.html.toggle={};
dojo.fx.html.toggle.plain={show:function(node,_548,_549,_54a){
dojo.style.show(node);
if(dojo.lang.isFunction(_54a)){
_54a();
}
},hide:function(node,_54c,_54d,_54e){
dojo.style.hide(node);
if(dojo.lang.isFunction(_54e)){
_54e();
}
}};
dojo.fx.html.toggle.fade={show:function(node,_550,_551,_552){
dojo.fx.html.fadeShow(node,_550,_552);
},hide:function(node,_554,_555,_556){
dojo.fx.html.fadeHide(node,_554,_556);
}};
dojo.fx.html.toggle.wipe={show:function(node,_558,_559,_55a){
dojo.fx.html.wipeIn(node,_558,_55a);
},hide:function(node,_55c,_55d,_55e){
dojo.fx.html.wipeOut(node,_55c,_55e);
}};
dojo.fx.html.toggle.explode={show:function(node,_560,_561,_562){
dojo.fx.html.explode(_561||[0,0,0,0],node,_560,_562);
},hide:function(node,_564,_565,_566){
dojo.fx.html.implode(node,_565||[0,0,0,0],_564,_566);
}};
dojo.lang.mixin(dojo.fx,dojo.fx.html);

