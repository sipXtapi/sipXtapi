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
var dojo;
if(dj_undef("dojo")){
dojo={};
}
dojo.version={major:0,minor:2,patch:2,flag:"+",revision:Number("$Rev: 2889 $".match(/[0-9]+/)[0]),toString:function(){
with(dojo.version){
return major+"."+minor+"."+patch+flag+" ("+revision+")";
}
}};
dojo.evalObjPath=function(_3,_4){
if(typeof _3!="string"){
return dj_global;
}
if(_3.indexOf(".")==-1){
if((dj_undef(_3,dj_global))&&(_4)){
dj_global[_3]={};
}
return dj_global[_3];
}
var _5=_3.split(/\./);
var _6=dj_global;
for(var i=0;i<_5.length;++i){
if(!_4){
_6=_6[_5[i]];
if((typeof _6=="undefined")||(!_6)){
return _6;
}
}else{
if(dj_undef(_5[i],_6)){
_6[_5[i]]={};
}
_6=_6[_5[i]];
}
}
return _6;
};
dojo.errorToString=function(_8){
return ((!dj_undef("message",_8))?_8.message:(dj_undef("description",_8)?_8:_8.description));
};
dojo.raise=function(_9,_a){
if(_a){
_9=_9+": "+dojo.errorToString(_a);
}
var he=dojo.hostenv;
if((!dj_undef("hostenv",dojo))&&(!dj_undef("println",dojo.hostenv))){
dojo.hostenv.println("FATAL: "+_9);
}
throw Error(_9);
};
dj_throw=dj_rethrow=function(m,e){
dojo.deprecated("dj_throw and dj_rethrow deprecated, use dojo.raise instead");
dojo.raise(m,e);
};
dojo.debug=function(){
if(!djConfig.isDebug){
return;
}
var _e=arguments;
if(dj_undef("println",dojo.hostenv)){
dojo.raise("dojo.debug not available (yet?)");
}
var _f=dj_global["jum"]&&!dj_global["jum"].isBrowser;
var s=[(_f?"":"DEBUG: ")];
for(var i=0;i<_e.length;++i){
if(!false&&_e[i] instanceof Error){
var msg="["+_e[i].name+": "+dojo.errorToString(_e[i])+(_e[i].fileName?", file: "+_e[i].fileName:"")+(_e[i].lineNumber?", line: "+_e[i].lineNumber:"")+"]";
}else{
try{
var msg=String(_e[i]);
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
if(_f){
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
var _14=[];
for(var _15 in obj){
try{
_14.push(_15+": "+obj[_15]);
}
catch(E){
_14.push(_15+": ERROR - "+E.message);
}
}
_14.sort();
for(var i=0;i<_14.length;i++){
dojo.debug(_14[i]);
}
dojo.debug("------------------------------------------------------------");
};
var dj_debug=dojo.debug;
function dj_eval(s){
return dj_global.eval?dj_global.eval(s):eval(s);
}
dj_unimplemented=dojo.unimplemented=function(_18,_19){
var _1a="'"+_18+"' not implemented";
if((!dj_undef(_19))&&(_19)){
_1a+=" "+_19;
}
dojo.raise(_1a);
};
dj_deprecated=dojo.deprecated=function(_1b,_1c,_1d){
var _1e="DEPRECATED: "+_1b;
if(_1c){
_1e+=" "+_1c;
}
if(_1d){
_1e+=" -- will be removed in version: "+_1d;
}
dojo.debug(_1e);
};
dojo.inherits=function(_1f,_20){
if(typeof _20!="function"){
dojo.raise("superclass: "+_20+" borken");
}
_1f.prototype=new _20();
_1f.prototype.constructor=_1f;
_1f.superclass=_20.prototype;
_1f["super"]=_20.prototype;
};
dj_inherits=function(_21,_22){
dojo.deprecated("dj_inherits deprecated, use dojo.inherits instead");
dojo.inherits(_21,_22);
};
dojo.render=(function(){
function vscaffold(_23,_24){
var tmp={capable:false,support:{builtin:false,plugin:false},prefixes:_23};
for(var x in _24){
tmp[x]=false;
}
return tmp;
}
return {name:"",ver:dojo.version,os:{win:false,linux:false,osx:false},html:vscaffold(["html"],["ie","opera","khtml","safari","moz"]),svg:vscaffold(["svg"],["corel","adobe","batik"]),vml:vscaffold(["vml"],["ie"]),swf:vscaffold(["Swf","Flash","Mm"],["mm"]),swt:vscaffold(["Swt"],["ibm"])};
})();
dojo.hostenv=(function(){
var _27={isDebug:false,allowQueryConfig:false,baseScriptUri:"",baseRelativePath:"",libraryScriptUri:"",iePreventClobber:false,ieClobberMinimal:true,preventBackButtonFix:true,searchIds:[],parseWidgets:true};
if(typeof djConfig=="undefined"){
djConfig=_27;
}else{
for(var _28 in _27){
if(typeof djConfig[_28]=="undefined"){
djConfig[_28]=_27[_28];
}
}
}
var djc=djConfig;
function _def(obj,_2b,def){
return (dj_undef(_2b,obj)?def:obj[_2b]);
}
return {name_:"(unset)",version_:"(unset)",pkgFileName:"__package__",loading_modules_:{},loaded_modules_:{},addedToLoadingCount:[],removedFromLoadingCount:[],inFlightCount:0,modulePrefixes_:{dojo:{name:"dojo",value:"src"}},setModulePrefix:function(_2d,_2e){
this.modulePrefixes_[_2d]={name:_2d,value:_2e};
},getModulePrefix:function(_2f){
var mp=this.modulePrefixes_;
if((mp[_2f])&&(mp[_2f]["name"])){
return mp[_2f].value;
}
return _2f;
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
var _33=uri.lastIndexOf("/");
djConfig.baseScriptUri=djConfig.baseRelativePath;
return djConfig.baseScriptUri;
};
dojo.hostenv.setBaseScriptUri=function(uri){
djConfig.baseScriptUri=uri;
};
dojo.hostenv.loadPath=function(_35,_36,cb){
if((_35.charAt(0)=="/")||(_35.match(/^\w+:/))){
dojo.raise("relpath '"+_35+"'; must be relative");
}
var uri=this.getBaseScriptUri()+_35;
if(djConfig.cacheBust&&dojo.render.html.capable){
uri+="?"+String(djConfig.cacheBust).replace(/\W+/g,"");
}
try{
return ((!_36)?this.loadUri(uri,cb):this.loadUriAndCheck(uri,_36,cb));
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
var _3b=this.getText(uri,null,true);
if(_3b==null){
return 0;
}
this.loadedUris[uri]=true;
var _3c=dj_eval(_3b);
return 1;
};
dojo.hostenv.loadUriAndCheck=function(uri,_3e,cb){
var ok=true;
try{
ok=this.loadUri(uri,cb);
}
catch(e){
dojo.debug("failed loading ",uri," with error: ",e);
}
return ((ok)&&(this.findModule(_3e,false)))?true:false;
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
dojo.addOnLoad=function(obj,_44){
if(arguments.length==1){
dojo.hostenv.modulesLoadedListeners.push(obj);
}else{
if(arguments.length>1){
dojo.hostenv.modulesLoadedListeners.push(function(){
obj[_44]();
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
dojo.hostenv.moduleLoaded=function(_45){
var _46=dojo.evalObjPath((_45.split(".").slice(0,-1)).join("."));
this.loaded_modules_[(new String(_45)).toLowerCase()]=_46;
};
dojo.hostenv._global_omit_module_check=false;
dojo.hostenv.loadModule=function(_47,_48,_49){
if(!_47){
return;
}
_49=this._global_omit_module_check||_49;
var _4a=this.findModule(_47,false);
if(_4a){
return _4a;
}
if(dj_undef(_47,this.loading_modules_)){
this.addedToLoadingCount.push(_47);
}
this.loading_modules_[_47]=1;
var _4b=_47.replace(/\./g,"/")+".js";
var _4c=_47.split(".");
var _4d=_47.split(".");
for(var i=_4c.length-1;i>0;i--){
var _4f=_4c.slice(0,i).join(".");
var _50=this.getModulePrefix(_4f);
if(_50!=_4f){
_4c.splice(0,i,_50);
break;
}
}
var _51=_4c[_4c.length-1];
if(_51=="*"){
_47=(_4d.slice(0,-1)).join(".");
while(_4c.length){
_4c.pop();
_4c.push(this.pkgFileName);
_4b=_4c.join("/")+".js";
if(_4b.charAt(0)=="/"){
_4b=_4b.slice(1);
}
ok=this.loadPath(_4b,((!_49)?_47:null));
if(ok){
break;
}
_4c.pop();
}
}else{
_4b=_4c.join("/")+".js";
_47=_4d.join(".");
var ok=this.loadPath(_4b,((!_49)?_47:null));
if((!ok)&&(!_48)){
_4c.pop();
while(_4c.length){
_4b=_4c.join("/")+".js";
ok=this.loadPath(_4b,((!_49)?_47:null));
if(ok){
break;
}
_4c.pop();
_4b=_4c.join("/")+"/"+this.pkgFileName+".js";
if(_4b.charAt(0)=="/"){
_4b=_4b.slice(1);
}
ok=this.loadPath(_4b,((!_49)?_47:null));
if(ok){
break;
}
}
}
if((!ok)&&(!_49)){
dojo.raise("Could not load '"+_47+"'; last tried '"+_4b+"'");
}
}
if(!_49){
_4a=this.findModule(_47,false);
if(!_4a){
dojo.raise("symbol '"+_47+"' is not defined after loading '"+_4b+"'");
}
}
return _4a;
};
dojo.hostenv.startPackage=function(_53){
var _54=_53.split(/\./);
if(_54[_54.length-1]=="*"){
_54.pop();
}
return dojo.evalObjPath(_54.join("."),true);
};
dojo.hostenv.findModule=function(_55,_56){
var lmn=(new String(_55)).toLowerCase();
if(this.loaded_modules_[lmn]){
return this.loaded_modules_[lmn];
}
var _58=dojo.evalObjPath(_55);
if((_55)&&(typeof _58!="undefined")&&(_58)){
this.loaded_modules_[lmn]=_58;
return _58;
}
if(_56){
dojo.raise("no loaded module named '"+_55+"'");
}
return null;
};
if(typeof window=="undefined"){
dojo.raise("no window object");
}
(function(){
if(djConfig.allowQueryConfig){
var _59=document.location.toString();
var _5a=_59.split("?",2);
if(_5a.length>1){
var _5b=_5a[1];
var _5c=_5b.split("&");
for(var x in _5c){
var sp=_5c[x].split("=");
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
var _60=document.getElementsByTagName("script");
var _61=/(__package__|dojo|bootstrap1)\.js([\?\.]|$)/i;
for(var i=0;i<_60.length;i++){
var src=_60[i].getAttribute("src");
if(!src){
continue;
}
var m=src.match(_61);
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
var _6c=dua.indexOf("Gecko");
drh.mozilla=drh.moz=(_6c>=0)&&(!drh.khtml);
if(drh.mozilla){
drh.geckoVersion=dua.substring(_6c+6,_6c+14);
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
var _6d=navigator.mimeTypes["image/svg+xml"]||navigator.mimeTypes["image/svg"]||navigator.mimeTypes["image/svg-xml"];
if(_6d){
drs.adobe=_6d&&_6d.enabledPlugin&&_6d.enabledPlugin.description&&(_6d.enabledPlugin.description.indexOf("Adobe")>-1);
if(drs.adobe){
drs.capable=t;
drs.support.plugin=t;
}
}
}else{
if(drh.ie&&dr.os.win){
var _6d=f;
try{
var _6e=new ActiveXObject("Adobe.SVGCtl");
_6d=t;
}
catch(e){
}
if(_6d){
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
dojo.hostenv.name_="browser";
dojo.hostenv.searchIds=[];
var DJ_XMLHTTP_PROGIDS=["Msxml2.XMLHTTP","Microsoft.XMLHTTP","Msxml2.XMLHTTP.4.0"];
dojo.hostenv.getXmlhttpObject=function(){
var _6f=null;
var _70=null;
try{
_6f=new XMLHttpRequest();
}
catch(e){
}
if(!_6f){
for(var i=0;i<3;++i){
var _72=DJ_XMLHTTP_PROGIDS[i];
try{
_6f=new ActiveXObject(_72);
}
catch(e){
_70=e;
}
if(_6f){
DJ_XMLHTTP_PROGIDS=[_72];
break;
}
}
}
if(!_6f){
return dojo.raise("XMLHTTP not available",_70);
}
return _6f;
};
dojo.hostenv.getText=function(uri,_74,_75){
var _76=this.getXmlhttpObject();
if(_74){
_76.onreadystatechange=function(){
if((4==_76.readyState)&&(_76["status"])){
if(_76.status==200){
_74(_76.responseText);
}
}
};
}
_76.open("GET",uri,_74?true:false);
_76.send(null);
if(_74){
return null;
}
return _76.responseText;
};
dojo.hostenv.defaultDebugContainerId="dojoDebug";
dojo.hostenv._println_buffer=[];
dojo.hostenv._println_safe=false;
dojo.hostenv.println=function(_77){
if(!dojo.hostenv._println_safe){
dojo.hostenv._println_buffer.push(_77);
}else{
try{
var _78=document.getElementById(djConfig.debugContainerId?djConfig.debugContainerId:dojo.hostenv.defaultDebugContainerId);
if(!_78){
_78=document.getElementsByTagName("body")[0]||document.body;
}
var div=document.createElement("div");
div.appendChild(document.createTextNode(_77));
_78.appendChild(div);
}
catch(e){
try{
document.write("<div>"+_77+"</div>");
}
catch(e2){
window.status=_77;
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
function dj_addNodeEvtHdlr(_7a,_7b,fp,_7d){
var _7e=_7a["on"+_7b]||function(){
};
_7a["on"+_7b]=function(){
fp.apply(_7a,arguments);
_7e.apply(_7a,arguments);
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
var _7f=[];
if(djConfig.searchIds&&djConfig.searchIds.length>0){
_7f=_7f.concat(djConfig.searchIds);
}
if(dojo.hostenv.searchIds&&dojo.hostenv.searchIds.length>0){
_7f=_7f.concat(dojo.hostenv.searchIds);
}
if((djConfig.parseWidgets)||(_7f.length>0)){
if(dojo.evalObjPath("dojo.widget.Parse")){
try{
var _80=new dojo.xml.Parse();
if(_7f.length>0){
for(var x=0;x<_7f.length;x++){
var _82=document.getElementById(_7f[x]);
if(!_82){
continue;
}
var _83=_80.parseElement(_82,null,true);
dojo.widget.getParser().createComponents(_83);
}
}else{
if(djConfig.parseWidgets){
var _83=_80.parseElement(document.getElementsByTagName("body")[0]||document.body,null,true);
dojo.widget.getParser().createComponents(_83);
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
document.write("<style>v:*{ behavior:url(#default#VML); }</style>");
document.write("<xml:namespace ns=\"urn:schemas-microsoft-com:vml\" prefix=\"v\"/>");
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
dojo.hostenv.conditionalLoadModule=function(_89){
var _8a=_89["common"]||[];
var _8b=(_89[dojo.hostenv.name_])?_8a.concat(_89[dojo.hostenv.name_]||[]):_8a.concat(_89["default"]||[]);
for(var x=0;x<_8b.length;x++){
var _8d=_8b[x];
if(_8d.constructor==Array){
dojo.hostenv.loadModule.apply(dojo.hostenv,_8d);
}else{
dojo.hostenv.loadModule(_8d);
}
}
};
dojo.hostenv.require=dojo.hostenv.loadModule;
dojo.require=function(){
dojo.hostenv.loadModule.apply(dojo.hostenv,arguments);
};
dojo.requireAfter=dojo.require;
dojo.requireIf=function(){
if((arguments[0]===true)||(arguments[0]=="common")||(dojo.render[arguments[0]].capable)){
var _8e=[];
for(var i=1;i<arguments.length;i++){
_8e.push(arguments[i]);
}
dojo.require.apply(dojo,_8e);
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
dojo.setModulePrefix=function(_91,_92){
return dojo.hostenv.setModulePrefix(_91,_92);
};
dojo.profile={start:function(){
},end:function(){
},stop:function(){
},dump:function(){
}};
dojo.exists=function(obj,_94){
var p=_94.split(".");
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
dojo.string.repeat=function(str,_9d,_9e){
var out="";
for(var i=0;i<_9d;i++){
out+=str;
if(_9e&&i<_9d-1){
out+=_9e;
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
dojo.lang.mixin=function(obj,_ad){
var _ae={};
for(var x in _ad){
if(typeof _ae[x]=="undefined"||_ae[x]!=_ad[x]){
obj[x]=_ad[x];
}
}
if(dojo.render.html.ie&&dojo.lang.isFunction(_ad["toString"])&&_ad["toString"]!=obj["toString"]){
obj.toString=_ad.toString;
}
return obj;
};
dojo.lang.extend=function(_b0,_b1){
this.mixin(_b0.prototype,_b1);
};
dojo.lang.find=function(arr,val,_b4,_b5){
if(!dojo.lang.isArrayLike(arr)&&dojo.lang.isArrayLike(val)){
var a=arr;
arr=val;
val=a;
}
var _b7=dojo.lang.isString(arr);
if(_b7){
arr=arr.split("");
}
if(_b5){
var _b8=-1;
var i=arr.length-1;
var end=-1;
}else{
var _b8=1;
var i=0;
var end=arr.length;
}
if(_b4){
while(i!=end){
if(arr[i]===val){
return i;
}
i+=_b8;
}
}else{
while(i!=end){
if(arr[i]==val){
return i;
}
i+=_b8;
}
}
return -1;
};
dojo.lang.indexOf=dojo.lang.find;
dojo.lang.findLast=function(arr,val,_bd){
return dojo.lang.find(arr,val,_bd,true);
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
for(var _ca in dojo.lang.whatAmI.custom){
if(dojo.lang.whatAmI.custom[_ca](wh)){
return _ca;
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
dojo.lang.isOfType=function(_ce,_cf){
if(dojo.lang.isArray(_cf)){
var _d0=_cf;
for(var i in _d0){
var _d2=_d0[i];
if(dojo.lang.isOfType(_ce,_d2)){
return true;
}
}
return false;
}else{
if(dojo.lang.isString(_cf)){
_cf=_cf.toLowerCase();
}
switch(_cf){
case Array:
case "array":
return dojo.lang.isArray(_ce);
break;
case Function:
case "function":
return dojo.lang.isFunction(_ce);
break;
case String:
case "string":
return dojo.lang.isString(_ce);
break;
case Number:
case "number":
return dojo.lang.isNumber(_ce);
break;
case "numeric":
return dojo.lang.isNumeric(_ce);
break;
case Boolean:
case "boolean":
return dojo.lang.isBoolean(_ce);
break;
case Object:
case "object":
return dojo.lang.isObject(_ce);
break;
case "pureobject":
return dojo.lang.isPureObject(_ce);
break;
case "builtin":
return dojo.lang.isBuiltIn(_ce);
break;
case "alien":
return dojo.lang.isAlien(_ce);
break;
case "undefined":
return dojo.lang.isUndefined(_ce);
break;
case null:
case "null":
return (_ce===null);
break;
case "optional":
return ((_ce===null)||dojo.lang.isUndefined(_ce));
break;
default:
if(dojo.lang.isFunction(_cf)){
return (_ce instanceof _cf);
}else{
dojo.raise("dojo.lang.isOfType() was passed an invalid type");
}
break;
}
}
dojo.raise("If we get here, it means a bug was introduced above.");
};
dojo.provide("dojo.lang.extras");
dojo.require("dojo.lang.common");
dojo.require("dojo.lang.type");
dojo.lang.setTimeout=function(_d3,_d4){
var _d5=window,argsStart=2;
if(!dojo.lang.isFunction(_d3)){
_d5=_d3;
_d3=_d4;
_d4=arguments[2];
argsStart++;
}
if(dojo.lang.isString(_d3)){
_d3=_d5[_d3];
}
var _d6=[];
for(var i=argsStart;i<arguments.length;i++){
_d6.push(arguments[i]);
}
return setTimeout(function(){
_d3.apply(_d5,_d6);
},_d4);
};
dojo.lang.getNameInObj=function(ns,_d9){
if(!ns){
ns=dj_global;
}
for(var x in ns){
if(ns[x]===_d9){
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
dojo.provide("dojo.io.IO");
dojo.require("dojo.string");
dojo.require("dojo.lang.extras");
dojo.io.transports=[];
dojo.io.hdlrFuncNames=["load","error"];
dojo.io.Request=function(url,_df,_e0,_e1){
if((arguments.length==1)&&(arguments[0].constructor==Object)){
this.fromKwArgs(arguments[0]);
}else{
this.url=url;
if(_df){
this.mimetype=_df;
}
if(_e0){
this.transport=_e0;
}
if(arguments.length>=4){
this.changeUrl=_e1;
}
}
};
dojo.lang.extend(dojo.io.Request,{url:"",mimetype:"text/plain",method:"GET",content:undefined,transport:undefined,changeUrl:undefined,formNode:undefined,sync:false,bindSuccess:false,useCache:false,preventCache:false,load:function(_e2,_e3,evt){
},error:function(_e5,_e6){
},handle:function(){
},abort:function(){
},fromKwArgs:function(_e7){
if(_e7["url"]){
_e7.url=_e7.url.toString();
}
if(_e7["formNode"]){
_e7.formNode=dojo.byId(_e7.formNode);
}
if(!_e7["method"]&&_e7["formNode"]&&_e7["formNode"].method){
_e7.method=_e7["formNode"].method;
}
if(!_e7["handle"]&&_e7["handler"]){
_e7.handle=_e7.handler;
}
if(!_e7["load"]&&_e7["loaded"]){
_e7.load=_e7.loaded;
}
if(!_e7["changeUrl"]&&_e7["changeURL"]){
_e7.changeUrl=_e7.changeURL;
}
_e7.encoding=dojo.lang.firstValued(_e7["encoding"],djConfig["bindEncoding"],"");
_e7.sendTransport=dojo.lang.firstValued(_e7["sendTransport"],djConfig["ioSendTransport"],false);
var _e8=dojo.lang.isFunction;
for(var x=0;x<dojo.io.hdlrFuncNames.length;x++){
var fn=dojo.io.hdlrFuncNames[x];
if(_e8(_e7[fn])){
continue;
}
if(_e8(_e7["handle"])){
_e7[fn]=_e7.handle;
}
}
dojo.lang.mixin(this,_e7);
}});
dojo.io.Error=function(msg,_ec,num){
this.message=msg;
this.type=_ec||"unknown";
this.number=num||0;
};
dojo.io.transports.addTransport=function(_ee){
this.push(_ee);
this[_ee]=dojo.io[_ee];
};
dojo.io.bind=function(_ef){
if(!(_ef instanceof dojo.io.Request)){
try{
_ef=new dojo.io.Request(_ef);
}
catch(e){
dojo.debug(e);
}
}
var _f0="";
if(_ef["transport"]){
_f0=_ef["transport"];
if(!this[_f0]){
return _ef;
}
}else{
for(var x=0;x<dojo.io.transports.length;x++){
var tmp=dojo.io.transports[x];
if((this[tmp])&&(this[tmp].canHandle(_ef))){
_f0=tmp;
}
}
if(_f0==""){
return _ef;
}
}
this[_f0].bind(_ef);
_ef.bindSuccess=true;
return _ef;
};
dojo.io.queueBind=function(_f3){
if(!(_f3 instanceof dojo.io.Request)){
try{
_f3=new dojo.io.Request(_f3);
}
catch(e){
dojo.debug(e);
}
}
var _f4=_f3.load;
_f3.load=function(){
dojo.io._queueBindInFlight=false;
var ret=_f4.apply(this,arguments);
dojo.io._dispatchNextQueueBind();
return ret;
};
var _f6=_f3.error;
_f3.error=function(){
dojo.io._queueBindInFlight=false;
var ret=_f6.apply(this,arguments);
dojo.io._dispatchNextQueueBind();
return ret;
};
dojo.io._bindQueue.push(_f3);
dojo.io._dispatchNextQueueBind();
return _f3;
};
dojo.io._dispatchNextQueueBind=function(){
if(!dojo.io._queueBindInFlight){
dojo.io._queueBindInFlight=true;
dojo.io.bind(dojo.io._bindQueue.shift());
}
};
dojo.io._bindQueue=[];
dojo.io._queueBindInFlight=false;
dojo.io.argsFromMap=function(map,_f9){
var _fa=new Object();
var _fb="";
var enc=/utf/i.test(_f9||"")?encodeURIComponent:dojo.string.encodeAscii;
for(var x in map){
if(!_fa[x]){
_fb+=enc(x)+"="+enc(map[x])+"&";
}
}
return _fb;
};
dojo.provide("dojo.lang.array");
dojo.require("dojo.lang.common");
dojo.lang.has=function(obj,_ff){
return (typeof obj[_ff]!=="undefined");
};
dojo.lang.isEmpty=function(obj){
if(dojo.lang.isObject(obj)){
var tmp={};
var _102=0;
for(var x in obj){
if(obj[x]&&(!tmp[x])){
_102++;
break;
}
}
return (_102==0);
}else{
if(dojo.lang.isArrayLike(obj)||dojo.lang.isString(obj)){
return obj.length==0;
}
}
};
dojo.lang.forEach=function(arr,_105,_106){
var _107=dojo.lang.isString(arr);
if(_107){
arr=arr.split("");
}
var il=arr.length;
for(var i=0;i<((_106)?il:arr.length);i++){
if(_105(arr[i],i,arr)=="break"){
break;
}
}
};
dojo.lang.map=function(arr,obj,_10c){
var _10d=dojo.lang.isString(arr);
if(_10d){
arr=arr.split("");
}
if(dojo.lang.isFunction(obj)&&(!_10c)){
_10c=obj;
obj=dj_global;
}else{
if(dojo.lang.isFunction(obj)&&_10c){
var _10e=obj;
obj=_10c;
_10c=_10e;
}
}
if(Array.map){
var _10f=Array.map(arr,_10c,obj);
}else{
var _10f=[];
for(var i=0;i<arr.length;++i){
_10f.push(_10c.call(obj,arr[i]));
}
}
if(_10d){
return _10f.join("");
}else{
return _10f;
}
};
dojo.lang.every=function(arr,_112,_113){
var _114=dojo.lang.isString(arr);
if(_114){
arr=arr.split("");
}
if(Array.every){
return Array.every(arr,_112,_113);
}else{
if(!_113){
if(arguments.length>=3){
dojo.raise("thisObject doesn't exist!");
}
_113=dj_global;
}
for(var i=0;i<arr.length;i++){
if(!_112.call(_113,arr[i],i,arr)){
return false;
}
}
return true;
}
};
dojo.lang.some=function(arr,_117,_118){
var _119=dojo.lang.isString(arr);
if(_119){
arr=arr.split("");
}
if(Array.some){
return Array.some(arr,_117,_118);
}else{
if(!_118){
if(arguments.length>=3){
dojo.raise("thisObject doesn't exist!");
}
_118=dj_global;
}
for(var i=0;i<arr.length;i++){
if(_117.call(_118,arr[i],i,arr)){
return true;
}
}
return false;
}
};
dojo.lang.filter=function(arr,_11c,_11d){
var _11e=dojo.lang.isString(arr);
if(_11e){
arr=arr.split("");
}
if(Array.filter){
var _11f=Array.filter(arr,_11c,_11d);
}else{
if(!_11d){
if(arguments.length>=3){
dojo.raise("thisObject doesn't exist!");
}
_11d=dj_global;
}
var _11f=[];
for(var i=0;i<arr.length;i++){
if(_11c.call(_11d,arr[i],i,arr)){
_11f.push(arr[i]);
}
}
}
if(_11e){
return _11f.join("");
}else{
return _11f;
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
dojo.lang.toArray=function(_124,_125){
var _126=[];
for(var i=_125||0;i<_124.length;i++){
_126.push(_124[i]);
}
return _126;
};
dojo.provide("dojo.lang.func");
dojo.require("dojo.lang.common");
dojo.require("dojo.lang.type");
dojo.lang.hitch=function(_128,_129){
if(dojo.lang.isString(_129)){
var fcn=_128[_129];
}else{
var fcn=_129;
}
return function(){
return fcn.apply(_128,arguments);
};
};
dojo.lang.anonCtr=0;
dojo.lang.anon={};
dojo.lang.nameAnonFunc=function(_12b,_12c){
var nso=(_12c||dojo.lang.anon);
if((dj_global["djConfig"])&&(djConfig["slowAnonFuncLookups"]==true)){
for(var x in nso){
if(nso[x]===_12b){
return x;
}
}
}
var ret="__"+dojo.lang.anonCtr++;
while(typeof nso[ret]!="undefined"){
ret="__"+dojo.lang.anonCtr++;
}
nso[ret]=_12b;
return ret;
};
dojo.lang.forward=function(_130){
return function(){
return this[_130].apply(this,arguments);
};
};
dojo.lang.curry=function(ns,func){
var _133=[];
ns=ns||dj_global;
if(dojo.lang.isString(func)){
func=ns[func];
}
for(var x=2;x<arguments.length;x++){
_133.push(arguments[x]);
}
var _135=func.length-_133.length;
function gather(_136,_137,_138){
var _139=_138;
var _13a=_137.slice(0);
for(var x=0;x<_136.length;x++){
_13a.push(_136[x]);
}
_138=_138-_136.length;
if(_138<=0){
var res=func.apply(ns,_13a);
_138=_139;
return res;
}else{
return function(){
return gather(arguments,_13a,_138);
};
}
}
return gather([],_133,_135);
};
dojo.lang.curryArguments=function(ns,func,args,_140){
var _141=[];
var x=_140||0;
for(x=_140;x<args.length;x++){
_141.push(args[x]);
}
return dojo.lang.curry.apply(dojo.lang,[ns,func].concat(_141));
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
dojo.lang.delayThese=function(farr,cb,_147,_148){
if(!farr.length){
if(typeof _148=="function"){
_148();
}
return;
}
if((typeof _147=="undefined")&&(typeof cb=="number")){
_147=cb;
cb=function(){
};
}else{
if(!cb){
cb=function(){
};
if(!_147){
_147=0;
}
}
}
setTimeout(function(){
(farr.shift())();
cb();
dojo.lang.delayThese(farr,cb,_147,_148);
},_147);
};
dojo.provide("dojo.string.extras");
dojo.require("dojo.string.common");
dojo.require("dojo.lang");
dojo.string.paramString=function(str,_14a,_14b){
for(var name in _14a){
var re=new RegExp("\\%\\{"+name+"\\}","g");
str=str.replace(re,_14a[name]);
}
if(_14b){
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
var _14f=str.split(" ");
var _150="";
var len=_14f.length;
for(var i=0;i<len;i++){
var word=_14f[i];
word=word.charAt(0).toUpperCase()+word.substring(1,word.length);
_150+=word;
if(i<len-1){
_150+=" ";
}
}
return new String(_150);
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
var _157=escape(str);
var _158,re=/%u([0-9A-F]{4})/i;
while((_158=_157.match(re))){
var num=Number("0x"+_158[1]);
var _15a=escape("&#"+num+";");
ret+=_157.substring(0,_158.index)+_15a;
_157=_157.substring(_158.index+_158[0].length);
}
ret+=_157.replace(/\+/g,"%2B");
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
dojo.string.escapeXml=function(str,_160){
str=str.replace(/&/gm,"&amp;").replace(/</gm,"&lt;").replace(/>/gm,"&gt;").replace(/"/gm,"&quot;");
if(!_160){
str=str.replace(/'/gm,"&#39;");
}
return str;
};
dojo.string.escapeSql=function(str){
return str.replace(/'/gm,"''");
};
dojo.string.escapeRegExp=function(str){
return str.replace(/\\/gm,"\\\\").replace(/([\f\b\n\t\r])/gm,"\\$1");
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
dojo.string.endsWith=function(str,end,_168){
if(_168){
str=str.toLowerCase();
end=end.toLowerCase();
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
dojo.string.startsWith=function(str,_16c,_16d){
if(_16d){
str=str.toLowerCase();
_16c=_16c.toLowerCase();
}
return str.indexOf(_16c)==0;
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
if(str.indexOf(arguments[i]>-1)){
return true;
}
}
return false;
};
dojo.string.normalizeNewlines=function(text,_173){
if(_173=="\n"){
text=text.replace(/\r\n/g,"\n");
text=text.replace(/\r/g,"\n");
}else{
if(_173=="\r"){
text=text.replace(/\r\n/g,"\r");
text=text.replace(/\n/g,"\r");
}else{
text=text.replace(/([^\r])\n/g,"$1\r\n");
text=text.replace(/\r([^\n])/g,"\r\n$1");
}
}
return text;
};
dojo.string.splitEscaped=function(str,_175){
var _176=[];
for(var i=0,prevcomma=0;i<str.length;i++){
if(str.charAt(i)=="\\"){
i++;
continue;
}
if(str.charAt(i)==_175){
_176.push(str.substring(prevcomma,i));
prevcomma=i+1;
}
}
_176.push(str.substr(prevcomma));
return _176;
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
var _17a=node.tagName;
if(_17a.substr(0,5).toLowerCase()!="dojo:"){
if(_17a.substr(0,4).toLowerCase()=="dojo"){
return "dojo:"+_17a.substring(4).toLowerCase();
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
var _17c=node.className||node.getAttribute("class");
if((_17c)&&(_17c.indexOf)&&(_17c.indexOf("dojo-")!=-1)){
var _17d=_17c.split(" ");
for(var x=0;x<_17d.length;x++){
if((_17d[x].length>5)&&(_17d[x].indexOf("dojo-")>=0)){
return "dojo:"+_17d[x].substr(5).toLowerCase();
}
}
}
}
}
return _17a.toLowerCase();
};
dojo.dom.getUniqueId=function(){
do{
var id="dj_unique_"+(++arguments.callee._idIncrement);
}while(document.getElementById(id));
return id;
};
dojo.dom.getUniqueId._idIncrement=0;
dojo.dom.firstElement=dojo.dom.getFirstChildElement=function(_180,_181){
var node=_180.firstChild;
while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE){
node=node.nextSibling;
}
if(_181&&node&&node.tagName&&node.tagName.toLowerCase()!=_181.toLowerCase()){
node=dojo.dom.nextElement(node,_181);
}
return node;
};
dojo.dom.lastElement=dojo.dom.getLastChildElement=function(_183,_184){
var node=_183.lastChild;
while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE){
node=node.previousSibling;
}
if(_184&&node&&node.tagName&&node.tagName.toLowerCase()!=_184.toLowerCase()){
node=dojo.dom.prevElement(node,_184);
}
return node;
};
dojo.dom.nextElement=dojo.dom.getNextSiblingElement=function(node,_187){
if(!node){
return null;
}
do{
node=node.nextSibling;
}while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE);
if(node&&_187&&_187.toLowerCase()!=node.tagName.toLowerCase()){
return dojo.dom.nextElement(node,_187);
}
return node;
};
dojo.dom.prevElement=dojo.dom.getPreviousSiblingElement=function(node,_189){
if(!node){
return null;
}
if(_189){
_189=_189.toLowerCase();
}
do{
node=node.previousSibling;
}while(node&&node.nodeType!=dojo.dom.ELEMENT_NODE);
if(node&&_189&&_189.toLowerCase()!=node.tagName.toLowerCase()){
return dojo.dom.prevElement(node,_189);
}
return node;
};
dojo.dom.moveChildren=function(_18a,_18b,trim){
var _18d=0;
if(trim){
while(_18a.hasChildNodes()&&_18a.firstChild.nodeType==dojo.dom.TEXT_NODE){
_18a.removeChild(_18a.firstChild);
}
while(_18a.hasChildNodes()&&_18a.lastChild.nodeType==dojo.dom.TEXT_NODE){
_18a.removeChild(_18a.lastChild);
}
}
while(_18a.hasChildNodes()){
_18b.appendChild(_18a.firstChild);
_18d++;
}
return _18d;
};
dojo.dom.copyChildren=function(_18e,_18f,trim){
var _191=_18e.cloneNode(true);
return this.moveChildren(_191,_18f,trim);
};
dojo.dom.removeChildren=function(node){
var _193=node.childNodes.length;
while(node.hasChildNodes()){
node.removeChild(node.firstChild);
}
return _193;
};
dojo.dom.replaceChildren=function(node,_195){
dojo.dom.removeChildren(node);
node.appendChild(_195);
};
dojo.dom.removeNode=function(node){
if(node&&node.parentNode){
return node.parentNode.removeChild(node);
}
};
dojo.dom.getAncestors=function(node,_198,_199){
var _19a=[];
var _19b=dojo.lang.isFunction(_198);
while(node){
if(!_19b||_198(node)){
_19a.push(node);
}
if(_199&&_19a.length>0){
return _19a[0];
}
node=node.parentNode;
}
if(_199){
return null;
}
return _19a;
};
dojo.dom.getAncestorsByTag=function(node,tag,_19e){
tag=tag.toLowerCase();
return dojo.dom.getAncestors(node,function(el){
return ((el.tagName)&&(el.tagName.toLowerCase()==tag));
},_19e);
};
dojo.dom.getFirstAncestorByTag=function(node,tag){
return dojo.dom.getAncestorsByTag(node,tag,true);
};
dojo.dom.isDescendantOf=function(node,_1a3,_1a4){
if(_1a4&&node){
node=node.parentNode;
}
while(node){
if(node==_1a3){
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
dojo.dom.createDocumentFromText=function(str,_1a7){
if(!_1a7){
_1a7="text/xml";
}
if(typeof DOMParser!="undefined"){
var _1a8=new DOMParser();
return _1a8.parseFromString(str,_1a7);
}else{
if(typeof ActiveXObject!="undefined"){
var _1a9=new ActiveXObject("Microsoft.XMLDOM");
if(_1a9){
_1a9.async=false;
_1a9.loadXML(str);
return _1a9;
}else{
dojo.debug("toXml didn't work?");
}
}else{
if(document.createElement){
var tmp=document.createElement("xml");
tmp.innerHTML=str;
if(document.implementation&&document.implementation.createDocument){
var _1ab=document.implementation.createDocument("foo","",null);
for(var i=0;i<tmp.childNodes.length;i++){
_1ab.importNode(tmp.childNodes.item(i),true);
}
return _1ab;
}
return tmp.document&&tmp.document.firstChild?tmp.document.firstChild:tmp;
}
}
}
return null;
};
dojo.dom.prependChild=function(node,_1ae){
if(_1ae.firstChild){
_1ae.insertBefore(node,_1ae.firstChild);
}else{
_1ae.appendChild(node);
}
return true;
};
dojo.dom.insertBefore=function(node,ref,_1b1){
if(_1b1!=true&&(node===ref||node.nextSibling===ref)){
return false;
}
var _1b2=ref.parentNode;
_1b2.insertBefore(node,ref);
return true;
};
dojo.dom.insertAfter=function(node,ref,_1b5){
var pn=ref.parentNode;
if(ref==pn.lastChild){
if((_1b5!=true)&&(node===ref)){
return false;
}
pn.appendChild(node);
}else{
return this.insertBefore(node,ref.nextSibling,_1b5);
}
return true;
};
dojo.dom.insertAtPosition=function(node,ref,_1b9){
if((!node)||(!ref)||(!_1b9)){
return false;
}
switch(_1b9.toLowerCase()){
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
dojo.dom.insertAtIndex=function(node,_1bb,_1bc){
var _1bd=_1bb.childNodes;
if(!_1bd.length){
_1bb.appendChild(node);
return true;
}
var _1be=null;
for(var i=0;i<_1bd.length;i++){
var _1c0=_1bd.item(i)["getAttribute"]?parseInt(_1bd.item(i).getAttribute("dojoinsertionindex")):-1;
if(_1c0<_1bc){
_1be=_1bd.item(i);
}
}
if(_1be){
return dojo.dom.insertAfter(node,_1be);
}else{
return dojo.dom.insertBefore(node,_1bd.item(0));
}
};
dojo.dom.textContent=function(node,text){
if(text){
dojo.dom.replaceChildren(node,document.createTextNode(text));
return text;
}else{
var _1c3="";
if(node==null){
return _1c3;
}
for(var i=0;i<node.childNodes.length;i++){
switch(node.childNodes[i].nodeType){
case 1:
case 5:
_1c3+=dojo.dom.textContent(node.childNodes[i]);
break;
case 3:
case 2:
case 4:
_1c3+=node.childNodes[i].nodeValue;
break;
default:
break;
}
}
return _1c3;
}
};
dojo.dom.collectionToArray=function(_1c5){
dojo.deprecated("dojo.dom.collectionToArray","use dojo.lang.toArray instead");
return dojo.lang.toArray(_1c5);
};
dojo.dom.hasParent=function(node){
if(!node||!node.parentNode||(node.parentNode&&!node.parentNode.tagName)){
return false;
}
return true;
};
dojo.dom.isTag=function(node){
if(node&&node.tagName){
var arr=dojo.lang.toArray(arguments,1);
return arr[dojo.lang.find(node.tagName,arr)]||"";
}
return "";
};
dojo.provide("dojo.io.BrowserIO");
dojo.require("dojo.io");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.func");
dojo.require("dojo.string.extras");
dojo.require("dojo.dom");
try{
if((!djConfig["preventBackButtonFix"])&&(!dojo.hostenv.post_load_)){
document.write("<iframe style='border: 0px; width: 1px; height: 1px; position: absolute; bottom: 0px; right: 0px; visibility: visible;' name='djhistory' id='djhistory' src='"+(dojo.hostenv.getBaseScriptUri()+"iframe_history.html")+"'></iframe>");
}
}
catch(e){
}
dojo.io.checkChildrenForFile=function(node){
var _1ca=false;
var _1cb=node.getElementsByTagName("input");
dojo.lang.forEach(_1cb,function(_1cc){
if(_1ca){
return;
}
if(_1cc.getAttribute("type")=="file"){
_1ca=true;
}
});
return _1ca;
};
dojo.io.formHasFile=function(_1cd){
return dojo.io.checkChildrenForFile(_1cd);
};
dojo.io.formFilter=function(node){
var type=(node.type||"").toLowerCase();
return !node.disabled&&node.name&&!dojo.lang.inArray(type,["file","submit","image","reset","button"]);
};
dojo.io.encodeForm=function(_1d0,_1d1,_1d2){
if((!_1d0)||(!_1d0.tagName)||(!_1d0.tagName.toLowerCase()=="form")){
dojo.raise("Attempted to encode a non-form element.");
}
if(!_1d2){
_1d2=dojo.io.formFilter;
}
var enc=/utf/i.test(_1d1||"")?encodeURIComponent:dojo.string.encodeAscii;
var _1d4=[];
for(var i=0;i<_1d0.elements.length;i++){
var elm=_1d0.elements[i];
if(!elm||elm.tagName.toLowerCase()=="fieldset"||!_1d2(elm)){
continue;
}
var name=enc(elm.name);
var type=elm.type.toLowerCase();
if(type=="select-multiple"){
for(var j=0;j<elm.options.length;j++){
if(elm.options[j].selected){
_1d4.push(name+"="+enc(elm.options[j].value));
}
}
}else{
if(dojo.lang.inArray(type,["radio","checkbox"])){
if(elm.checked){
_1d4.push(name+"="+enc(elm.value));
}
}else{
_1d4.push(name+"="+enc(elm.value));
}
}
}
var _1da=_1d0.getElementsByTagName("input");
for(var i=0;i<_1da.length;i++){
var _1db=_1da[i];
if(_1db.type.toLowerCase()=="image"&&_1db.form==_1d0&&_1d2(_1db)){
var name=enc(_1db.name);
_1d4.push(name+"="+enc(_1db.value));
_1d4.push(name+".x=0");
_1d4.push(name+".y=0");
}
}
return _1d4.join("&")+"&";
};
dojo.io.setIFrameSrc=function(_1dc,src,_1de){
try{
var r=dojo.render.html;
if(!_1de){
if(r.safari){
_1dc.location=src;
}else{
frames[_1dc.name].location=src;
}
}else{
var idoc;
if(r.ie){
idoc=_1dc.contentWindow.document;
}else{
if(r.moz){
idoc=_1dc.contentWindow;
}else{
if(r.safari){
idoc=_1dc.document;
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
var _1e6=form.getElementsByTagName("input");
for(var i=0;i<_1e6.length;i++){
var _1e7=_1e6[i];
if(_1e7.type.toLowerCase()=="image"&&_1e7.form==form){
this.connect(_1e7,"onclick","click");
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
var _1ee=false;
if(node.disabled||!node.name){
_1ee=false;
}else{
if(dojo.lang.inArray(type,["submit","button","image"])){
if(!this.clickedButton){
this.clickedButton=node;
}
_1ee=node==this.clickedButton;
}else{
_1ee=!dojo.lang.inArray(type,["file","submit","reset","button"]);
}
}
return _1ee;
},connect:function(_1ef,_1f0,_1f1){
if(dojo.evalObjPath("dojo.event.connect")){
dojo.event.connect(_1ef,_1f0,this,_1f1);
}else{
var fcn=dojo.lang.hitch(this,_1f1);
_1ef[_1f0]=function(e){
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
var _1f4=this;
this.initialHref=window.location.href;
this.initialHash=window.location.hash;
this.moveForward=false;
var _1f5={};
this.useCache=false;
this.preventCache=false;
this.historyStack=[];
this.forwardStack=[];
this.historyIframe=null;
this.bookmarkAnchor=null;
this.locationTimer=null;
function getCacheKey(url,_1f7,_1f8){
return url+"|"+_1f7+"|"+_1f8.toLowerCase();
}
function addToCache(url,_1fa,_1fb,http){
_1f5[getCacheKey(url,_1fa,_1fb)]=http;
}
function getFromCache(url,_1fe,_1ff){
return _1f5[getCacheKey(url,_1fe,_1ff)];
}
this.clearCache=function(){
_1f5={};
};
function doLoad(_200,http,url,_203,_204){
if((http.status==200)||(location.protocol=="file:"&&(http.status==0||http.status==undefined))){
var ret;
if(_200.method.toLowerCase()=="head"){
var _206=http.getAllResponseHeaders();
ret={};
ret.toString=function(){
return _206;
};
var _207=_206.split(/[\r\n]+/g);
for(var i=0;i<_207.length;i++){
var pair=_207[i].match(/^([^:]+)\s*:\s*(.+)$/i);
if(pair){
ret[pair[1]]=pair[2];
}
}
}else{
if(_200.mimetype=="text/javascript"){
try{
ret=dj_eval(http.responseText);
}
catch(e){
dojo.debug(e);
dojo.debug(http.responseText);
ret=null;
}
}else{
if(_200.mimetype=="text/json"){
try{
ret=dj_eval("("+http.responseText+")");
}
catch(e){
dojo.debug(e);
dojo.debug(http.responseText);
ret=false;
}
}else{
if((_200.mimetype=="application/xml")||(_200.mimetype=="text/xml")){
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
if(_204){
addToCache(url,_203,_200.method,http);
}
_200[(typeof _200.load=="function")?"load":"handle"]("load",ret,http);
}else{
var _20a=new dojo.io.Error("XMLHttpTransport Error: "+http.status+" "+http.statusText);
_200[(typeof _200.error=="function")?"error":"handle"]("error",_20a,http);
}
}
function setHeaders(http,_20c){
if(_20c["headers"]){
for(var _20d in _20c["headers"]){
if(_20d.toLowerCase()=="content-type"&&!_20c["contentType"]){
_20c["contentType"]=_20c["headers"][_20d];
}else{
http.setRequestHeader(_20d,_20c["headers"][_20d]);
}
}
}
}
this.addToHistory=function(args){
var _20f=args["back"]||args["backButton"]||args["handle"];
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
hash="#"+((args["changeUrl"]!==true)?args["changeUrl"]:(new Date()).getTime());
setTimeout("window.location.href = '"+hash+"';",1);
this.bookmarkAnchor.href=hash;
if(dojo.render.html.ie){
var _212=_20f;
var lh=null;
var hsl=this.historyStack.length-1;
if(hsl>=0){
while(!this.historyStack[hsl]["urlHash"]){
hsl--;
}
lh=this.historyStack[hsl]["urlHash"];
}
if(lh){
_20f=function(){
if(window.location.hash!=""){
setTimeout("window.location.href = '"+lh+"';",1);
}
_212();
};
}
this.forwardStack=[];
var _215=args["forward"]||args["forwardButton"];
var tfw=function(){
if(window.location.hash!=""){
window.location.href=hash;
}
if(_215){
_215();
}
};
if(args["forward"]){
args.forward=tfw;
}else{
if(args["forwardButton"]){
args.forwardButton=tfw;
}
}
}else{
if(dojo.render.html.moz){
if(!this.locationTimer){
this.locationTimer=setInterval("dojo.io.XMLHTTPTransport.checkLocation();",200);
}
}
}
}
this.historyStack.push({"url":url,"callback":_20f,"kwArgs":args,"urlHash":hash});
};
this.checkLocation=function(){
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
};
this.iframeLoaded=function(evt,_219){
var isp=_219.href.split("?");
if(isp.length<2){
if(this.historyStack.length==1){
this.handleBackButton();
}
return;
}
var _21b=isp[1];
if(this.moveForward){
this.moveForward=false;
return;
}
var last=this.historyStack.pop();
if(!last){
if(this.forwardStack.length>0){
var next=this.forwardStack[this.forwardStack.length-1];
if(_21b==next.url.split("?")[1]){
this.handleForwardButton();
}
}
return;
}
this.historyStack.push(last);
if(this.historyStack.length>=2){
if(isp[1]==this.historyStack[this.historyStack.length-2].url.split("?")[1]){
this.handleBackButton();
}
}else{
this.handleBackButton();
}
};
this.handleBackButton=function(){
var last=this.historyStack.pop();
if(!last){
return;
}
if(last["callback"]){
last.callback();
}else{
if(last.kwArgs["backButton"]){
last.kwArgs["backButton"]();
}else{
if(last.kwArgs["back"]){
last.kwArgs["back"]();
}else{
if(last.kwArgs["handle"]){
last.kwArgs.handle("back");
}
}
}
}
this.forwardStack.push(last);
};
this.handleForwardButton=function(){
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
};
this.inFlight=[];
this.inFlightTimer=null;
this.startWatchingInFlight=function(){
if(!this.inFlightTimer){
this.inFlightTimer=setInterval("dojo.io.XMLHTTPTransport.watchInFlight();",10);
}
};
this.watchInFlight=function(){
for(var x=this.inFlight.length-1;x>=0;x--){
var tif=this.inFlight[x];
if(!tif){
this.inFlight.splice(x,1);
continue;
}
if(4==tif.http.readyState){
this.inFlight.splice(x,1);
doLoad(tif.req,tif.http,tif.url,tif.query,tif.useCache);
if(this.inFlight.length==0){
clearInterval(this.inFlightTimer);
this.inFlightTimer=null;
}
}
}
};
var _222=dojo.hostenv.getXmlhttpObject()?true:false;
this.canHandle=function(_223){
return _222&&dojo.lang.inArray((_223["mimetype"]||"".toLowerCase()),["text/plain","text/html","application/xml","text/xml","text/javascript","text/json"])&&dojo.lang.inArray(_223["method"].toLowerCase(),["post","get","head"])&&!(_223["formNode"]&&dojo.io.formHasFile(_223["formNode"]));
};
this.multipartBoundary="45309FFF-BD65-4d50-99C9-36986896A96F";
this.bind=function(_224){
if(!_224["url"]){
if(!_224["formNode"]&&(_224["backButton"]||_224["back"]||_224["changeUrl"]||_224["watchForURL"])&&(!djConfig.preventBackButtonFix)){
this.addToHistory(_224);
return true;
}
}
var url=_224.url;
var _226="";
if(_224["formNode"]){
var ta=_224.formNode.getAttribute("action");
if((ta)&&(!_224["url"])){
url=ta;
}
var tp=_224.formNode.getAttribute("method");
if((tp)&&(!_224["method"])){
_224.method=tp;
}
_226+=dojo.io.encodeForm(_224.formNode,_224.encoding,_224["formFilter"]);
}
if(url.indexOf("#")>-1){
dojo.debug("Warning: dojo.io.bind: stripping hash values from url:",url);
url=url.split("#")[0];
}
if(_224["file"]){
_224.method="post";
}
if(!_224["method"]){
_224.method="get";
}
if(_224.method.toLowerCase()=="get"){
_224.multipart=false;
}else{
if(_224["file"]){
_224.multipart=true;
}else{
if(!_224["multipart"]){
_224.multipart=false;
}
}
}
if(_224["backButton"]||_224["back"]||_224["changeUrl"]){
this.addToHistory(_224);
}
var _229=_224["content"]||{};
if(_224.sendTransport){
_229["dojo.transport"]="xmlhttp";
}
do{
if(_224.postContent){
_226=_224.postContent;
break;
}
if(_229){
_226+=dojo.io.argsFromMap(_229,_224.encoding);
}
if(_224.method.toLowerCase()=="get"||!_224.multipart){
break;
}
var t=[];
if(_226.length){
var q=_226.split("&");
for(var i=0;i<q.length;++i){
if(q[i].length){
var p=q[i].split("=");
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+p[0]+"\"","",p[1]);
}
}
}
if(_224.file){
if(dojo.lang.isArray(_224.file)){
for(var i=0;i<_224.file.length;++i){
var o=_224.file[i];
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+o.name+"\"; filename=\""+("fileName" in o?o.fileName:o.name)+"\"","Content-Type: "+("contentType" in o?o.contentType:"application/octet-stream"),"",o.content);
}
}else{
var o=_224.file;
t.push("--"+this.multipartBoundary,"Content-Disposition: form-data; name=\""+o.name+"\"; filename=\""+("fileName" in o?o.fileName:o.name)+"\"","Content-Type: "+("contentType" in o?o.contentType:"application/octet-stream"),"",o.content);
}
}
if(t.length){
t.push("--"+this.multipartBoundary+"--","");
_226=t.join("\r\n");
}
}while(false);
var _22f=_224["sync"]?false:true;
var _230=_224["preventCache"]||(this.preventCache==true&&_224["preventCache"]!=false);
var _231=_224["useCache"]==true||(this.useCache==true&&_224["useCache"]!=false);
if(!_230&&_231){
var _232=getFromCache(url,_226,_224.method);
if(_232){
doLoad(_224,_232,url,_226,false);
return;
}
}
var http=dojo.hostenv.getXmlhttpObject(_224);
var _234=false;
if(_22f){
this.inFlight.push({"req":_224,"http":http,"url":url,"query":_226,"useCache":_231});
this.startWatchingInFlight();
}
if(_224.method.toLowerCase()=="post"){
http.open("POST",url,_22f);
setHeaders(http,_224);
http.setRequestHeader("Content-Type",_224.multipart?("multipart/form-data; boundary="+this.multipartBoundary):(_224.contentType||"application/x-www-form-urlencoded"));
http.send(_226);
}else{
var _235=url;
if(_226!=""){
_235+=(_235.indexOf("?")>-1?"&":"?")+_226;
}
if(_230){
_235+=(dojo.string.endsWithAny(_235,"?","&")?"":(_235.indexOf("?")>-1?"&":"?"))+"dojo.preventCache="+new Date().valueOf();
}
http.open(_224.method.toUpperCase(),_235,_22f);
setHeaders(http,_224);
http.send(null);
}
if(!_22f){
doLoad(_224,http,url,_226,_231);
}
_224.abort=function(){
return http.abort();
};
return;
};
dojo.io.transports.addTransport("XMLHTTPTransport");
};
dojo.provide("dojo.event");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.extras");
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
var _239=dojo.lang.nameAnonFunc(args[2],ao.adviceObj);
ao.adviceFunc=_239;
}else{
if((dl.isFunction(args[0]))&&(dl.isObject(args[1]))&&(dl.isString(args[2]))){
ao.adviceType="after";
ao.srcObj=dj_global;
var _239=dojo.lang.nameAnonFunc(args[0],ao.srcObj);
ao.srcFunc=_239;
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
var _239=dojo.lang.nameAnonFunc(args[1],dj_global);
ao.srcFunc=_239;
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
var ao=interpolateArgs(arguments);
var mjp=dojo.event.MethodJoinPoint.getForMethod(ao.srcObj,ao.srcFunc);
if(ao.adviceFunc){
var mjp2=dojo.event.MethodJoinPoint.getForMethod(ao.adviceObj,ao.adviceFunc);
}
mjp.kwAddAdvice(ao);
return mjp;
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
this._kwConnectImpl=function(_241,_242){
var fn=(_242)?"disconnect":"connect";
if(typeof _241["srcFunc"]=="function"){
_241.srcObj=_241["srcObj"]||dj_global;
var _244=dojo.lang.nameAnonFunc(_241.srcFunc,_241.srcObj);
_241.srcFunc=_244;
}
if(typeof _241["adviceFunc"]=="function"){
_241.adviceObj=_241["adviceObj"]||dj_global;
var _244=dojo.lang.nameAnonFunc(_241.adviceFunc,_241.adviceObj);
_241.adviceFunc=_244;
}
return dojo.event[fn]((_241["type"]||_241["adviceType"]||"after"),_241["srcObj"]||dj_global,_241["srcFunc"],_241["adviceObj"]||_241["targetObj"]||dj_global,_241["adviceFunc"]||_241["targetFunc"],_241["aroundObj"],_241["aroundFunc"],_241["once"],_241["delay"],_241["rate"],_241["adviceMsg"]||false);
};
this.kwConnect=function(_245){
return this._kwConnectImpl(_245,false);
};
this.disconnect=function(){
var ao=interpolateArgs(arguments);
if(!ao.adviceFunc){
return;
}
var mjp=dojo.event.MethodJoinPoint.getForMethod(ao.srcObj,ao.srcFunc);
return mjp.removeAdvice(ao.adviceObj,ao.adviceFunc,ao.adviceType,ao.once);
};
this.kwDisconnect=function(_248){
return this._kwConnectImpl(_248,true);
};
};
dojo.event.MethodInvocation=function(_249,obj,args){
this.jp_=_249;
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
dojo.event.MethodJoinPoint=function(obj,_251){
this.object=obj||dj_global;
this.methodname=_251;
this.methodfunc=this.object[_251];
this.before=[];
this.after=[];
this.around=[];
};
dojo.event.MethodJoinPoint.getForMethod=function(obj,_253){
if(!obj){
obj=dj_global;
}
if(!obj[_253]){
obj[_253]=function(){
};
}else{
if((!dojo.lang.isFunction(obj[_253]))&&(!dojo.lang.isAlien(obj[_253]))){
return null;
}
}
var _254=_253+"$joinpoint";
var _255=_253+"$joinpoint$method";
var _256=obj[_254];
if(!_256){
var _257=false;
if(dojo.event["browser"]){
if((obj["attachEvent"])||(obj["nodeType"])||(obj["addEventListener"])){
_257=true;
dojo.event.browser.addClobberNodeAttrs(obj,[_254,_255,_253]);
}
}
obj[_255]=obj[_253];
_256=obj[_254]=new dojo.event.MethodJoinPoint(obj,_255);
obj[_253]=function(){
var args=[];
if((_257)&&(!arguments.length)){
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
catch(E){
evt=window.event;
}
if(evt){
args.push(dojo.event.browser.fixEvent(evt));
}
}else{
for(var x=0;x<arguments.length;x++){
if((x==0)&&(_257)&&(dojo.event.browser.isEvent(arguments[x]))){
args.push(dojo.event.browser.fixEvent(arguments[x]));
}else{
args.push(arguments[x]);
}
}
}
return _256.run.apply(_256,args);
};
}
return _256;
};
dojo.lang.extend(dojo.event.MethodJoinPoint,{unintercept:function(){
this.object[this.methodname]=this.methodfunc;
},run:function(){
var obj=this.object||dj_global;
var args=arguments;
var _25d=[];
for(var x=0;x<args.length;x++){
_25d[x]=args[x];
}
var _25f=function(marr){
if(!marr){
dojo.debug("Null argument to unrollAdvice()");
return;
}
var _261=marr[0]||dj_global;
var _262=marr[1];
if(!_261[_262]){
dojo.raise("function \""+_262+"\" does not exist on \""+_261+"\"");
}
var _263=marr[2]||dj_global;
var _264=marr[3];
var msg=marr[6];
var _266;
var to={args:[],jp_:this,object:obj,proceed:function(){
return _261[_262].apply(_261,to.args);
}};
to.args=_25d;
var _268=parseInt(marr[4]);
var _269=((!isNaN(_268))&&(marr[4]!==null)&&(typeof marr[4]!="undefined"));
if(marr[5]){
var rate=parseInt(marr[5]);
var cur=new Date();
var _26c=false;
if((marr["last"])&&((cur-marr.last)<=rate)){
if(dojo.event.canTimeout){
if(marr["delayTimer"]){
clearTimeout(marr.delayTimer);
}
var tod=parseInt(rate*2);
var mcpy=dojo.lang.shallowCopy(marr);
marr.delayTimer=setTimeout(function(){
mcpy[5]=0;
_25f(mcpy);
},tod);
}
return;
}else{
marr.last=cur;
}
}
if(_264){
_263[_264].call(_263,to);
}else{
if((_269)&&((dojo.render.html)||(dojo.render.svg))){
dj_global["setTimeout"](function(){
if(msg){
_261[_262].call(_261,to);
}else{
_261[_262].apply(_261,args);
}
},_268);
}else{
if(msg){
_261[_262].call(_261,to);
}else{
_261[_262].apply(_261,args);
}
}
}
};
if(this.before.length>0){
dojo.lang.forEach(this.before,_25f,true);
}
var _26f;
if(this.around.length>0){
var mi=new dojo.event.MethodInvocation(this,obj,args);
_26f=mi.proceed();
}else{
if(this.methodfunc){
_26f=this.object[this.methodname].apply(this.object,args);
}
}
if(this.after.length>0){
dojo.lang.forEach(this.after,_25f,true);
}
return (this.methodfunc)?_26f:null;
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
},addAdvice:function(_274,_275,_276,_277,_278,_279,once,_27b,rate,_27d){
var arr=this.getArr(_278);
if(!arr){
dojo.raise("bad this: "+this);
}
var ao=[_274,_275,_276,_277,_27b,rate,_27d];
if(once){
if(this.hasAdvice(_274,_275,_278,arr)>=0){
return;
}
}
if(_279=="first"){
arr.unshift(ao);
}else{
arr.push(ao);
}
},hasAdvice:function(_280,_281,_282,arr){
if(!arr){
arr=this.getArr(_282);
}
var ind=-1;
for(var x=0;x<arr.length;x++){
if((arr[x][0]==_280)&&(arr[x][1]==_281)){
ind=x;
}
}
return ind;
},removeAdvice:function(_286,_287,_288,once){
var arr=this.getArr(_288);
var ind=this.hasAdvice(_286,_287,_288,arr);
if(ind==-1){
return false;
}
while(ind!=-1){
arr.splice(ind,1);
if(once){
break;
}
ind=this.hasAdvice(_286,_287,_288,arr);
}
return true;
}});
dojo.require("dojo.event");
dojo.provide("dojo.event.topic");
dojo.event.topic=new function(){
this.topics={};
this.getTopic=function(_28c){
if(!this.topics[_28c]){
this.topics[_28c]=new this.TopicImpl(_28c);
}
return this.topics[_28c];
};
this.registerPublisher=function(_28d,obj,_28f){
var _28d=this.getTopic(_28d);
_28d.registerPublisher(obj,_28f);
};
this.subscribe=function(_290,obj,_292){
var _290=this.getTopic(_290);
_290.subscribe(obj,_292);
};
this.unsubscribe=function(_293,obj,_295){
var _293=this.getTopic(_293);
_293.unsubscribe(obj,_295);
};
this.publish=function(_296,_297){
var _296=this.getTopic(_296);
var args=[];
if(arguments.length==2&&(dojo.lang.isArray(_297)||_297.callee)){
args=_297;
}else{
var args=[];
for(var x=1;x<arguments.length;x++){
args.push(arguments[x]);
}
}
_296.sendMessage.apply(_296,args);
};
};
dojo.event.topic.TopicImpl=function(_29a){
this.topicName=_29a;
var self=this;
self.subscribe=function(_29c,_29d){
var tf=_29d||_29c;
var to=(!_29d)?dj_global:_29c;
dojo.event.kwConnect({srcObj:self,srcFunc:"sendMessage",adviceObj:to,adviceFunc:tf});
};
self.unsubscribe=function(_2a0,_2a1){
var tf=(!_2a1)?_2a0:_2a1;
var to=(!_2a1)?null:_2a0;
dojo.event.kwDisconnect({srcObj:self,srcFunc:"sendMessage",adviceObj:to,adviceFunc:tf});
};
self.registerPublisher=function(_2a4,_2a5){
dojo.event.connect(_2a4,_2a5,self,"sendMessage");
};
self.sendMessage=function(_2a6){
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
this.clobber=function(_2a9){
var na;
var tna;
if(_2a9){
tna=_2a9.getElementsByTagName("*");
na=[_2a9];
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
var _2ad={};
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
var _2b1=0;
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
this.addClobberNodeAttrs=function(node,_2b5){
this.addClobberNode(node);
for(var x=0;x<_2b5.length;x++){
node.__clobberAttrs__.push(_2b5[x]);
}
};
this.removeListener=function(node,_2b8,fp,_2ba){
if(!_2ba){
var _2ba=false;
}
_2b8=_2b8.toLowerCase();
if(_2b8.substr(0,2)=="on"){
_2b8=_2b8.substr(2);
}
if(node.removeEventListener){
node.removeEventListener(_2b8,fp,_2ba);
}
};
this.addListener=function(node,_2bc,fp,_2be,_2bf){
if(!node){
return;
}
if(!_2be){
var _2be=false;
}
_2bc=_2bc.toLowerCase();
if(_2bc.substr(0,2)!="on"){
_2bc="on"+_2bc;
}
if(!_2bf){
var _2c0=function(evt){
if(!evt){
evt=window.event;
}
var ret=fp(dojo.event.browser.fixEvent(evt));
if(_2be){
dojo.event.browser.stopEvent(evt);
}
return ret;
};
}else{
_2c0=fp;
}
if(node.addEventListener){
node.addEventListener(_2bc.substr(2),_2c0,_2be);
return _2c0;
}else{
if(typeof node[_2bc]=="function"){
var _2c3=node[_2bc];
node[_2bc]=function(e){
_2c3(e);
return _2c0(e);
};
}else{
node[_2bc]=_2c0;
}
if(dojo.render.html.ie){
this.addClobberNodeAttrs(node,[_2bc]);
}
return _2c0;
}
};
this.isEvent=function(obj){
return (typeof obj!="undefined")&&(typeof Event!="undefined")&&(obj.eventPhase);
};
this.currentEvent=null;
this.callListener=function(_2c6,_2c7){
if(typeof _2c6!="function"){
dojo.raise("listener not a function: "+_2c6);
}
dojo.event.browser.currentEvent.currentTarget=_2c7;
return _2c6.call(_2c7,dojo.event.browser.currentEvent);
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
this.fixEvent=function(evt){
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
evt.currentTarget=evt.srcElement;
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
dojo.lang.assert=function(_2cc,_2cd){
if(!_2cc){
var _2ce="An assert statement failed.\n"+"The method dojo.lang.assert() was called with a 'false' value.\n";
if(_2cd){
_2ce+="Here's the assert message:\n"+_2cd+"\n";
}
throw new Error(_2ce);
}
};
dojo.lang.assertType=function(_2cf,type,_2d1){
if(!dojo.lang.isOfType(_2cf,type)){
if(!_2d1){
if(!dojo.lang.assertType._errorMessage){
dojo.lang.assertType._errorMessage="Type mismatch: dojo.lang.assertType() failed.";
}
_2d1=dojo.lang.assertType._errorMessage;
}
dojo.lang.assert(false,_2d1);
}
};
dojo.lang.assertValidKeywords=function(_2d2,_2d3,_2d4){
var key;
if(!_2d4){
if(!dojo.lang.assertValidKeywords._errorMessage){
dojo.lang.assertValidKeywords._errorMessage="In dojo.lang.assertValidKeywords(), found invalid keyword:";
}
_2d4=dojo.lang.assertValidKeywords._errorMessage;
}
if(dojo.lang.isArray(_2d3)){
for(key in _2d2){
if(!dojo.lang.inArray(_2d3,key)){
dojo.lang.assert(false,_2d4+" "+key);
}
}
}else{
for(key in _2d2){
if(!(key in _2d3)){
dojo.lang.assert(false,_2d4+" "+key);
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
dojo.lang.extend(dojo.AdapterRegistry,{register:function(name,_2d7,wrap,_2d9){
if(_2d9){
this.pairs.unshift([name,_2d7,wrap]);
}else{
this.pairs.push([name,_2d7,wrap]);
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
dojo.lang.registerRepr=function(name,_2e0,wrap,_2e2){
dojo.lang.reprRegistry.register(name,_2e0,wrap,_2e2);
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
var _2ea={};
var _2eb=[];
this.getUniqueId=function(_2ec){
return _2ec+"_"+(_2ea[_2ec]!=undefined?++_2ea[_2ec]:_2ea[_2ec]=0);
};
this.add=function(_2ed){
dojo.profile.start("dojo.widget.manager.add");
this.widgets.push(_2ed);
if(_2ed.widgetId==""){
if(_2ed["id"]){
_2ed.widgetId=_2ed["id"];
}else{
if(_2ed.extraArgs["id"]){
_2ed.widgetId=_2ed.extraArgs["id"];
}else{
_2ed.widgetId=this.getUniqueId(_2ed.widgetType);
}
}
}
if(this.widgetIds[_2ed.widgetId]){
dojo.debug("widget ID collision on ID: "+_2ed.widgetId);
}
this.widgetIds[_2ed.widgetId]=_2ed;
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
this.remove=function(_2ef){
var tw=this.widgets[_2ef].widgetId;
delete this.widgetIds[tw];
this.widgets.splice(_2ef,1);
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
this.getWidgetsByFilter=function(_2f9){
var ret=[];
dojo.lang.forEach(this.widgets,function(x){
if(_2f9(x)){
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
var _2ff={};
var _300=["dojo.widget"];
for(var i=0;i<_300.length;i++){
_300[_300[i]]=true;
}
this.registerWidgetPackage=function(_302){
if(!_300[_302]){
_300[_302]=true;
_300.push(_302);
}
};
this.getWidgetPackageList=function(){
return dojo.lang.map(_300,function(elt){
return (elt!==true?elt:undefined);
});
};
this.getImplementation=function(_304,_305,_306){
var impl=this.getImplementationName(_304);
if(impl){
var ret=new impl(_305);
return ret;
}
};
this.getImplementationName=function(_309){
var _30a=_309.toLowerCase();
var impl=_2ff[_30a];
if(impl){
return impl;
}
if(!_2eb.length){
for(var _30c in dojo.render){
if(dojo.render[_30c]["capable"]===true){
var _30d=dojo.render[_30c].prefixes;
for(var i=0;i<_30d.length;i++){
_2eb.push(_30d[i].toLowerCase());
}
}
}
_2eb.push("");
}
for(var i=0;i<_300.length;i++){
var _30f=dojo.evalObjPath(_300[i]);
if(!_30f){
continue;
}
for(var j=0;j<_2eb.length;j++){
if(!_30f[_2eb[j]]){
continue;
}
for(var _311 in _30f[_2eb[j]]){
if(_311.toLowerCase()!=_30a){
continue;
}
_2ff[_30a]=_30f[_2eb[j]][_311];
return _2ff[_30a];
}
}
for(var j=0;j<_2eb.length;j++){
for(var _311 in _30f){
if(_311.toLowerCase()!=(_2eb[j]+_30a)){
continue;
}
_2ff[_30a]=_30f[_311];
return _2ff[_30a];
}
}
}
throw new Error("Could not locate \""+_309+"\" class");
};
this.resizing=false;
this.onResized=function(){
if(this.resizing){
return;
}
try{
this.resizing=true;
for(var id in this.topWidgets){
var _313=this.topWidgets[id];
if(_313.onResized){
_313.onResized();
}
}
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
var _315=dojo.widget.manager.getAllWidgets.apply(dojo.widget.manager,arguments);
if(arguments.length>0){
return _315[n];
}
return _315;
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
dojo.provide("dojo.math");
dojo.math.degToRad=function(x){
return (x*Math.PI)/180;
};
dojo.math.radToDeg=function(x){
return (x*180)/Math.PI;
};
dojo.math.factorial=function(n){
if(n<1){
return 0;
}
var _319=1;
for(var i=1;i<=n;i++){
_319*=i;
}
return _319;
};
dojo.math.permutations=function(n,k){
if(n==0||k==0){
return 1;
}
return (dojo.math.factorial(n)/dojo.math.factorial(n-k));
};
dojo.math.combinations=function(n,r){
if(n==0||r==0){
return 1;
}
return (dojo.math.factorial(n)/(dojo.math.factorial(n-r)*dojo.math.factorial(r)));
};
dojo.math.bernstein=function(t,n,i){
return (dojo.math.combinations(n,i)*Math.pow(t,i)*Math.pow(1-t,n-i));
};
dojo.math.gaussianRandom=function(){
var k=2;
do{
var i=2*Math.random()-1;
var j=2*Math.random()-1;
k=i*i+j*j;
}while(k>=1);
k=Math.sqrt((-2*Math.log(k))/k);
return i*k;
};
dojo.math.mean=function(){
var _325=dojo.lang.isArray(arguments[0])?arguments[0]:arguments;
var mean=0;
for(var i=0;i<_325.length;i++){
mean+=_325[i];
}
return mean/_325.length;
};
dojo.math.round=function(_328,_329){
if(!_329){
var _32a=1;
}else{
var _32a=Math.pow(10,_329);
}
return Math.round(_328*_32a)/_32a;
};
dojo.math.sd=function(){
var _32b=dojo.lang.isArray(arguments[0])?arguments[0]:arguments;
return Math.sqrt(dojo.math.variance(_32b));
};
dojo.math.variance=function(){
var _32c=dojo.lang.isArray(arguments[0])?arguments[0]:arguments;
var mean=0,squares=0;
for(var i=0;i<_32c.length;i++){
mean+=_32c[i];
squares+=Math.pow(_32c[i],2);
}
return (squares/_32c.length)-Math.pow(mean/_32c.length,2);
};
dojo.math.range=function(a,b,step){
if(arguments.length<2){
b=a;
a=0;
}
if(arguments.length<3){
step=1;
}
var _332=[];
if(step>0){
for(var i=a;i<b;i+=step){
_332.push(i);
}
}else{
if(step<0){
for(var i=a;i>b;i+=step){
_332.push(i);
}
}else{
throw new Error("dojo.math.range: step must be non-zero");
}
}
return _332;
};
dojo.provide("dojo.graphics.color");
dojo.require("dojo.lang.array");
dojo.require("dojo.math");
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
dojo.lang.extend(dojo.graphics.color.Color,{toRgb:function(_33a){
if(_33a){
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
},blend:function(_33b,_33c){
return dojo.graphics.color.blend(this.toRgb(),new Color(_33b).toRgb(),_33c);
}});
dojo.graphics.color.named={white:[255,255,255],black:[0,0,0],red:[255,0,0],green:[0,255,0],blue:[0,0,255],navy:[0,0,128],gray:[128,128,128],silver:[192,192,192]};
dojo.graphics.color.blend=function(a,b,_33f){
if(typeof a=="string"){
return dojo.graphics.color.blendHex(a,b,_33f);
}
if(!_33f){
_33f=0;
}else{
if(_33f>1){
_33f=1;
}else{
if(_33f<-1){
_33f=-1;
}
}
}
var c=new Array(3);
for(var i=0;i<3;i++){
var half=Math.abs(a[i]-b[i])/2;
c[i]=Math.floor(Math.min(a[i],b[i])+half+(half*_33f));
}
return c;
};
dojo.graphics.color.blendHex=function(a,b,_345){
return dojo.graphics.color.rgb2hex(dojo.graphics.color.blend(dojo.graphics.color.hex2rgb(a),dojo.graphics.color.hex2rgb(b),_345));
};
dojo.graphics.color.extractRGB=function(_346){
var hex="0123456789abcdef";
_346=_346.toLowerCase();
if(_346.indexOf("rgb")==0){
var _348=_346.match(/rgba*\((\d+), *(\d+), *(\d+)/i);
var ret=_348.splice(1,3);
return ret;
}else{
var _34a=dojo.graphics.color.hex2rgb(_346);
if(_34a){
return _34a;
}else{
return dojo.graphics.color.named[_346]||[255,255,255];
}
}
};
dojo.graphics.color.hex2rgb=function(hex){
var _34c="0123456789ABCDEF";
var rgb=new Array(3);
if(hex.indexOf("#")==0){
hex=hex.substring(1);
}
hex=hex.toUpperCase();
if(hex.replace(new RegExp("["+_34c+"]","g"),"")!=""){
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
rgb[i]=_34c.indexOf(rgb[i].charAt(0))*16+_34c.indexOf(rgb[i].charAt(1));
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
var _35d=v-min;
s=(v==0)?0:_35d/v;
if(s==0){
h=0;
}else{
if(r==v){
h=60*(g-b)/_35d;
}else{
if(g==v){
h=120+60*(b-r)/_35d;
}else{
if(b==v){
h=240+60*(r-g)/_35d;
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
var _364=h/60;
var i=Math.floor(_364);
var f=_364-i;
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
var _372=max-min;
l=(min+max)/2;
s=0;
if((l>0)&&(l<1)){
s=_372/((l<0.5)?(2*l):(2-2*l));
}
h=0;
if(_372>0){
if((max==r)&&(max!=g)){
h+=(g-b)/_372;
}
if((max==g)&&(max!=b)){
h+=(2+(b-r)/_372);
}
if((max==b)&&(max!=r)){
h+=(4+(r-g)/_372);
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
dojo.provide("dojo.style");
dojo.require("dojo.graphics.color");
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
var _37e=dojo.style.getStyle(node,"-moz-box-sizing");
if(!_37e){
_37e=dojo.style.getStyle(node,"box-sizing");
}
return (_37e?_37e:dojo.style.boxSizing.contentBox);
}
};
dojo.style.isBorderBox=function(node){
return (dojo.style.getBoxSizing(node)==dojo.style.boxSizing.borderBox);
};
dojo.style.getUnitValue=function(node,_381,_382){
node=dojo.byId(node);
var _383={value:0,units:"px"};
var s=dojo.style.getComputedStyle(node,_381);
if(s==""||(s=="auto"&&_382)){
return _383;
}
if(dojo.lang.isUndefined(s)){
_383.value=NaN;
}else{
var _385=s.match(/(\-?[\d.]+)([a-z%]*)/i);
if(!_385){
_383.value=NaN;
}else{
_383.value=Number(_385[1]);
_383.units=_385[2].toLowerCase();
}
}
return _383;
};
dojo.style.getPixelValue=function(node,_387,_388){
node=dojo.byId(node);
var _389=dojo.style.getUnitValue(node,_387,_388);
if(isNaN(_389.value)){
return 0;
}
if((_389.value)&&(_389.units!="px")){
return NaN;
}
return _389.value;
};
dojo.style.getNumericStyle=dojo.style.getPixelValue;
dojo.style.isPositionAbsolute=function(node){
node=dojo.byId(node);
return (dojo.style.getComputedStyle(node,"position")=="absolute");
};
dojo.style.getMarginWidth=function(node){
node=dojo.byId(node);
var _38c=dojo.style.isPositionAbsolute(node);
var left=dojo.style.getPixelValue(node,"margin-left",_38c);
var _38e=dojo.style.getPixelValue(node,"margin-right",_38c);
return left+_38e;
};
dojo.style.getBorderWidth=function(node){
node=dojo.byId(node);
var left=(dojo.style.getStyle(node,"border-left-style")=="none"?0:dojo.style.getPixelValue(node,"border-left-width"));
var _391=(dojo.style.getStyle(node,"border-right-style")=="none"?0:dojo.style.getPixelValue(node,"border-right-width"));
return left+_391;
};
dojo.style.getPaddingWidth=function(node){
node=dojo.byId(node);
var left=dojo.style.getPixelValue(node,"padding-left",true);
var _394=dojo.style.getPixelValue(node,"padding-right",true);
return left+_394;
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
dojo.style.setOuterWidth=function(node,_399){
node=dojo.byId(node);
if(!dojo.style.isBorderBox(node)){
_399-=dojo.style.getPaddingWidth(node)+dojo.style.getBorderWidth(node);
}
_399-=dojo.style.getMarginWidth(node);
if(!isNaN(_399)&&_399>0){
node.style.width=_399+"px";
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
var _39b=dojo.style.isPositionAbsolute(node);
var top=dojo.style.getPixelValue(node,"margin-top",_39b);
var _39d=dojo.style.getPixelValue(node,"margin-bottom",_39b);
return top+_39d;
};
dojo.style.getBorderHeight=function(node){
node=dojo.byId(node);
var top=(dojo.style.getStyle(node,"border-top-style")=="none"?0:dojo.style.getPixelValue(node,"border-top-width"));
var _3a0=(dojo.style.getStyle(node,"border-bottom-style")=="none"?0:dojo.style.getPixelValue(node,"border-bottom-width"));
return top+_3a0;
};
dojo.style.getPaddingHeight=function(node){
node=dojo.byId(node);
var top=dojo.style.getPixelValue(node,"padding-top",true);
var _3a3=dojo.style.getPixelValue(node,"padding-bottom",true);
return top+_3a3;
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
dojo.style.setOuterHeight=function(node,_3a8){
node=dojo.byId(node);
if(!dojo.style.isBorderBox(node)){
_3a8-=dojo.style.getPaddingHeight(node)+dojo.style.getBorderHeight(node);
}
_3a8-=dojo.style.getMarginHeight(node);
if(!isNaN(_3a8)&&_3a8>0){
node.style.height=_3a8+"px";
return true;
}else{
return false;
}
};
dojo.style.setContentWidth=function(node,_3aa){
node=dojo.byId(node);
if(dojo.style.isBorderBox(node)){
_3aa+=dojo.style.getPaddingWidth(node)+dojo.style.getBorderWidth(node);
}
if(!isNaN(_3aa)&&_3aa>0){
node.style.width=_3aa+"px";
return true;
}else{
return false;
}
};
dojo.style.setContentHeight=function(node,_3ac){
node=dojo.byId(node);
if(dojo.style.isBorderBox(node)){
_3ac+=dojo.style.getPaddingHeight(node)+dojo.style.getBorderHeight(node);
}
if(!isNaN(_3ac)&&_3ac>0){
node.style.height=_3ac+"px";
return true;
}else{
return false;
}
};
dojo.style.getContentBoxHeight=dojo.style.getContentHeight;
dojo.style.getBorderBoxHeight=dojo.style.getInnerHeight;
dojo.style.getMarginBoxHeight=dojo.style.getOuterHeight;
dojo.style.setMarginBoxHeight=dojo.style.setOuterHeight;
dojo.style.getTotalOffset=function(node,type,_3af){
node=dojo.byId(node);
var _3b0=(type=="top")?"offsetTop":"offsetLeft";
var _3b1=(type=="top")?"scrollTop":"scrollLeft";
var _3b2=(type=="top")?"y":"x";
var _3b3=0;
if(node["offsetParent"]){
if(dojo.render.html.safari&&node.style.getPropertyValue("position")=="absolute"&&node.parentNode==document.body){
var _3b4=document.body;
}else{
var _3b4=document.body.parentNode;
}
if(_3af&&node.parentNode!=document.body){
_3b3-=dojo.style.sumAncestorProperties(node,_3b1);
}
do{
var n=node[_3b0];
_3b3+=isNaN(n)?0:n;
node=node.offsetParent;
}while(node!=_3b4&&node!=null);
}else{
if(node[_3b2]){
var n=node[_3b2];
_3b3+=isNaN(n)?0:n;
}
}
return _3b3;
};
dojo.style.sumAncestorProperties=function(node,prop){
node=dojo.byId(node);
if(!node){
return 0;
}
var _3b8=0;
while(node){
var val=node[prop];
if(val){
_3b8+=val-0;
}
node=node.parentNode;
}
return _3b8;
};
dojo.style.totalOffsetLeft=function(node,_3bb){
node=dojo.byId(node);
return dojo.style.getTotalOffset(node,"left",_3bb);
};
dojo.style.getAbsoluteX=dojo.style.totalOffsetLeft;
dojo.style.totalOffsetTop=function(node,_3bd){
node=dojo.byId(node);
return dojo.style.getTotalOffset(node,"top",_3bd);
};
dojo.style.getAbsoluteY=dojo.style.totalOffsetTop;
dojo.style.getAbsolutePosition=function(node,_3bf){
node=dojo.byId(node);
var _3c0=[dojo.style.getAbsoluteX(node,_3bf),dojo.style.getAbsoluteY(node,_3bf)];
_3c0.x=_3c0[0];
_3c0.y=_3c0[1];
return _3c0;
};
dojo.style.styleSheet=null;
dojo.style.insertCssRule=function(_3c1,_3c2,_3c3){
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
_3c3=dojo.style.styleSheet.cssRules.length;
}else{
if(dojo.style.styleSheet.rules){
_3c3=dojo.style.styleSheet.rules.length;
}else{
return null;
}
}
}
if(dojo.style.styleSheet.insertRule){
var rule=_3c1+" { "+_3c2+" }";
return dojo.style.styleSheet.insertRule(rule,_3c3);
}else{
if(dojo.style.styleSheet.addRule){
return dojo.style.styleSheet.addRule(_3c1,_3c2,_3c3);
}else{
return null;
}
}
};
dojo.style.removeCssRule=function(_3c5){
if(!dojo.style.styleSheet){
dojo.debug("no stylesheet defined for removing rules");
return false;
}
if(dojo.render.html.ie){
if(!_3c5){
_3c5=dojo.style.styleSheet.rules.length;
dojo.style.styleSheet.removeRule(_3c5);
}
}else{
if(document.styleSheets[0]){
if(!_3c5){
_3c5=dojo.style.styleSheet.cssRules.length;
}
dojo.style.styleSheet.deleteRule(_3c5);
}
}
return true;
};
dojo.style.getBackgroundColor=function(node){
node=dojo.byId(node);
var _3c7;
do{
_3c7=dojo.style.getStyle(node,"background-color");
if(_3c7.toLowerCase()=="rgba(0, 0, 0, 0)"){
_3c7="transparent";
}
if(node==document.getElementsByTagName("body")[0]){
node=null;
break;
}
node=node.parentNode;
}while(node&&dojo.lang.inArray(_3c7,["transparent",""]));
if(_3c7=="transparent"){
_3c7=[255,255,255,0];
}else{
_3c7=dojo.graphics.color.extractRGB(_3c7);
}
return _3c7;
};
dojo.style.getComputedStyle=function(node,_3c9,_3ca){
node=dojo.byId(node);
var _3cb=_3ca;
if(node.style.getPropertyValue){
_3cb=node.style.getPropertyValue(_3c9);
}
if(!_3cb){
if(document.defaultView){
var cs=document.defaultView.getComputedStyle(node,"");
if(cs){
_3cb=cs.getPropertyValue(_3c9);
}
}else{
if(node.currentStyle){
_3cb=node.currentStyle[dojo.style.toCamelCase(_3c9)];
}
}
}
return _3cb;
};
dojo.style.getStyle=function(node,_3ce){
node=dojo.byId(node);
var _3cf=dojo.style.toCamelCase(_3ce);
var _3d0=node.style[_3cf];
return (_3d0?_3d0:dojo.style.getComputedStyle(node,_3ce,_3d0));
};
dojo.style.toCamelCase=function(_3d1){
var arr=_3d1.split("-"),cc=arr[0];
for(var i=1;i<arr.length;i++){
cc+=arr[i].charAt(0).toUpperCase()+arr[i].substring(1);
}
return cc;
};
dojo.style.toSelectorCase=function(_3d4){
return _3d4.replace(/([A-Z])/g,"-$1").toLowerCase();
};
dojo.style.setOpacity=function setOpacity(node,_3d6,_3d7){
node=dojo.byId(node);
var h=dojo.render.html;
if(!_3d7){
if(_3d6>=1){
if(h.ie){
dojo.style.clearOpacity(node);
return;
}else{
_3d6=0.999999;
}
}else{
if(_3d6<0){
_3d6=0;
}
}
}
if(h.ie){
if(node.nodeName.toLowerCase()=="tr"){
var tds=node.getElementsByTagName("td");
for(var x=0;x<tds.length;x++){
tds[x].style.filter="Alpha(Opacity="+_3d6*100+")";
}
}
node.style.filter="Alpha(Opacity="+_3d6*100+")";
}else{
if(h.moz){
node.style.opacity=_3d6;
node.style.MozOpacity=_3d6;
}else{
if(h.safari){
node.style.opacity=_3d6;
node.style.KhtmlOpacity=_3d6;
}else{
node.style.opacity=_3d6;
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
dojo.style.isVisible=function(node){
node=dojo.byId(node);
return dojo.style.getComputedStyle(node||this.domNode,"display")!="none";
};
dojo.style.show=function(node){
node=dojo.byId(node);
if(node.style){
node.style.display=dojo.lang.inArray(["tr","td","th"],node.tagName.toLowerCase())?"":"block";
}
};
dojo.style.hide=function(node){
node=dojo.byId(node);
if(node.style){
node.style.display="none";
}
};
dojo.style.toggleVisible=function(node){
if(dojo.style.isVisible(node)){
dojo.style.hide(node);
return false;
}else{
dojo.style.show(node);
return true;
}
};
dojo.style.toCoordinateArray=function(_3e3,_3e4){
if(dojo.lang.isArray(_3e3)){
while(_3e3.length<4){
_3e3.push(0);
}
while(_3e3.length>4){
_3e3.pop();
}
var ret=_3e3;
}else{
var node=dojo.byId(_3e3);
var ret=[dojo.style.getAbsoluteX(node,_3e4),dojo.style.getAbsoluteY(node,_3e4),dojo.style.getInnerWidth(node),dojo.style.getInnerHeight(node)];
}
ret.x=ret[0];
ret.y=ret[1];
ret.w=ret[2];
ret.h=ret[3];
return ret;
};
dojo.provide("dojo.animation.AnimationEvent");
dojo.require("dojo.lang");
dojo.animation.AnimationEvent=function(anim,type,_3e9,_3ea,_3eb,_3ec,dur,pct,fps){
this.type=type;
this.animation=anim;
this.coords=_3e9;
this.x=_3e9[0];
this.y=_3e9[1];
this.z=_3e9[2];
this.startTime=_3ea;
this.currentTime=_3eb;
this.endTime=_3ec;
this.duration=dur;
this.percent=pct;
this.fps=fps;
};
dojo.lang.extend(dojo.animation.AnimationEvent,{coordsAsInts:function(){
var _3f0=new Array(this.coords.length);
for(var i=0;i<this.coords.length;i++){
_3f0[i]=Math.round(this.coords[i]);
}
return _3f0;
}});
dojo.provide("dojo.math.curves");
dojo.require("dojo.math");
dojo.math.curves={Line:function(_3f2,end){
this.start=_3f2;
this.end=end;
this.dimensions=_3f2.length;
for(var i=0;i<_3f2.length;i++){
_3f2[i]=Number(_3f2[i]);
}
for(var i=0;i<end.length;i++){
end[i]=Number(end[i]);
}
this.getValue=function(n){
var _3f6=new Array(this.dimensions);
for(var i=0;i<this.dimensions;i++){
_3f6[i]=((this.end[i]-this.start[i])*n)+this.start[i];
}
return _3f6;
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
var _3fa=new Array(this.p[0].length);
for(var k=0;j<this.p[0].length;k++){
_3fa[k]=0;
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
_3fa[j]=C/D;
}
return _3fa;
};
this.p=pnts;
return this;
},CatmullRom:function(pnts,c){
this.getValue=function(step){
var _404=step*(this.p.length-1);
var node=Math.floor(_404);
var _406=_404-node;
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
var u=_406;
var u2=_406*_406;
var u3=_406*_406*_406;
var _40e=new Array(this.p[0].length);
for(var k=0;k<this.p[0].length;k++){
var x1=(-this.c*this.p[i0][k])+((2-this.c)*this.p[i][k])+((this.c-2)*this.p[i1][k])+(this.c*this.p[i2][k]);
var x2=(2*this.c*this.p[i0][k])+((this.c-3)*this.p[i][k])+((3-2*this.c)*this.p[i1][k])+(-this.c*this.p[i2][k]);
var x3=(-this.c*this.p[i0][k])+(this.c*this.p[i1][k]);
var x4=this.p[i][k];
_40e[k]=x1*u3+x2*u2+x3*u+x4;
}
return _40e;
};
if(!c){
this.c=0.7;
}else{
this.c=c;
}
this.p=pnts;
return this;
},Arc:function(_414,end,ccw){
var _417=dojo.math.points.midpoint(_414,end);
var _418=dojo.math.points.translate(dojo.math.points.invert(_417),_414);
var rad=Math.sqrt(Math.pow(_418[0],2)+Math.pow(_418[1],2));
var _41a=dojo.math.radToDeg(Math.atan(_418[1]/_418[0]));
if(_418[0]<0){
_41a-=90;
}else{
_41a+=90;
}
dojo.math.curves.CenteredArc.call(this,_417,rad,_41a,_41a+(ccw?-180:180));
},CenteredArc:function(_41b,_41c,_41d,end){
this.center=_41b;
this.radius=_41c;
this.start=_41d||0;
this.end=end;
this.getValue=function(n){
var _420=new Array(2);
var _421=dojo.math.degToRad(this.start+((this.end-this.start)*n));
_420[0]=this.center[0]+this.radius*Math.sin(_421);
_420[1]=this.center[1]-this.radius*Math.cos(_421);
return _420;
};
return this;
},Circle:function(_422,_423){
dojo.math.curves.CenteredArc.call(this,_422,_423,0,360);
return this;
},Path:function(){
var _424=[];
var _425=[];
var _426=[];
var _427=0;
this.add=function(_428,_429){
if(_429<0){
dojo.raise("dojo.math.curves.Path.add: weight cannot be less than 0");
}
_424.push(_428);
_425.push(_429);
_427+=_429;
computeRanges();
};
this.remove=function(_42a){
for(var i=0;i<_424.length;i++){
if(_424[i]==_42a){
_424.splice(i,1);
_427-=_425.splice(i,1)[0];
break;
}
}
computeRanges();
};
this.removeAll=function(){
_424=[];
_425=[];
_427=0;
};
this.getValue=function(n){
var _42d=false,value=0;
for(var i=0;i<_426.length;i++){
var r=_426[i];
if(n>=r[0]&&n<r[1]){
var subN=(n-r[0])/r[2];
value=_424[i].getValue(subN);
_42d=true;
break;
}
}
if(!_42d){
value=_424[_424.length-1].getValue(1);
}
for(j=0;j<i;j++){
value=dojo.math.points.translate(value,_424[j].getValue(1));
}
return value;
};
function computeRanges(){
var _431=0;
for(var i=0;i<_425.length;i++){
var end=_431+_425[i]/_427;
var len=end-_431;
_426[i]=[_431,end,len];
_431=end;
}
}
return this;
}};
dojo.provide("dojo.animation.Animation");
dojo.require("dojo.animation.AnimationEvent");
dojo.require("dojo.lang.func");
dojo.require("dojo.math");
dojo.require("dojo.math.curves");
dojo.animation.Animation=function(_435,_436,_437,_438,rate){
if(dojo.lang.isArray(_435)){
_435=new dojo.math.curves.Line(_435[0],_435[1]);
}
this.curve=_435;
this.duration=_436;
this.repeatCount=_438||0;
this.rate=rate||25;
if(_437){
if(dojo.lang.isFunction(_437.getValue)){
this.accel=_437;
}else{
var i=0.35*_437+0.5;
this.accel=new dojo.math.curves.CatmullRom([[0],[i],[1]],0.45);
}
}
};
dojo.lang.extend(dojo.animation.Animation,{curve:null,duration:0,repeatCount:0,accel:null,onBegin:null,onAnimate:null,onEnd:null,onPlay:null,onPause:null,onStop:null,handler:null,_animSequence:null,_startTime:null,_endTime:null,_lastFrame:null,_timer:null,_percent:0,_active:false,_paused:false,_startRepeatCount:0,play:function(_43b){
if(_43b){
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
},gotoPercent:function(pct,_43f){
clearTimeout(this._timer);
this._active=true;
this._paused=true;
this._percent=pct;
if(_43f){
this.play();
}
},stop:function(_440){
clearTimeout(this._timer);
var step=this._percent/100;
if(_440){
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
dojo.fx.html.fadeOut=function(node,_448,_449,_44a){
return dojo.fx.html.fade(node,_448,dojo.style.getOpacity(node),0,_449,_44a);
};
dojo.fx.html.fadeIn=function(node,_44c,_44d,_44e){
return dojo.fx.html.fade(node,_44c,dojo.style.getOpacity(node),1,_44d,_44e);
};
dojo.fx.html.fadeHide=function(node,_450,_451,_452){
node=dojo.byId(node);
if(!_450){
_450=150;
}
return dojo.fx.html.fadeOut(node,_450,function(node){
node.style.display="none";
if(typeof _451=="function"){
_451(node);
}
});
};
dojo.fx.html.fadeShow=function(node,_455,_456,_457){
node=dojo.byId(node);
if(!_455){
_455=150;
}
node.style.display="block";
return dojo.fx.html.fade(node,_455,0,1,_456,_457);
};
dojo.fx.html.fade=function(node,_459,_45a,_45b,_45c,_45d){
node=dojo.byId(node);
dojo.fx.html._makeFadeable(node);
var anim=new dojo.animation.Animation(new dojo.math.curves.Line([_45a],[_45b]),_459||dojo.fx.duration,0);
dojo.event.connect(anim,"onAnimate",function(e){
dojo.style.setOpacity(node,e.x);
});
if(_45c){
dojo.event.connect(anim,"onEnd",function(e){
_45c(node,anim);
});
}
if(!_45d){
anim.play(true);
}
return anim;
};
dojo.fx.html.slideTo=function(node,_462,_463,_464,_465){
if(!dojo.lang.isNumber(_462)){
var tmp=_462;
_462=_463;
_463=tmp;
}
node=dojo.byId(node);
var top=node.offsetTop;
var left=node.offsetLeft;
var pos=dojo.style.getComputedStyle(node,"position");
if(pos=="relative"||pos=="static"){
top=parseInt(dojo.style.getComputedStyle(node,"top"))||0;
left=parseInt(dojo.style.getComputedStyle(node,"left"))||0;
}
return dojo.fx.html.slide(node,_462,[left,top],_463,_464,_465);
};
dojo.fx.html.slideBy=function(node,_46b,_46c,_46d,_46e){
if(!dojo.lang.isNumber(_46b)){
var tmp=_46b;
_46b=_46c;
_46c=tmp;
}
node=dojo.byId(node);
var top=node.offsetTop;
var left=node.offsetLeft;
var pos=dojo.style.getComputedStyle(node,"position");
if(pos=="relative"||pos=="static"){
top=parseInt(dojo.style.getComputedStyle(node,"top"))||0;
left=parseInt(dojo.style.getComputedStyle(node,"left"))||0;
}
return dojo.fx.html.slideTo(node,_46b,[left+_46c[0],top+_46c[1]],_46d,_46e);
};
dojo.fx.html.slide=function(node,_474,_475,_476,_477,_478){
if(!dojo.lang.isNumber(_474)){
var tmp=_474;
_474=_476;
_476=_475;
_475=tmp;
}
node=dojo.byId(node);
if(dojo.style.getComputedStyle(node,"position")=="static"){
node.style.position="relative";
}
var anim=new dojo.animation.Animation(new dojo.math.curves.Line(_475,_476),_474||dojo.fx.duration,0);
dojo.event.connect(anim,"onAnimate",function(e){
with(node.style){
left=e.x+"px";
top=e.y+"px";
}
});
if(_477){
dojo.event.connect(anim,"onEnd",function(e){
_477(node,anim);
});
}
if(!_478){
anim.play(true);
}
return anim;
};
dojo.fx.html.colorFadeIn=function(node,_47e,_47f,_480,_481,_482){
if(!dojo.lang.isNumber(_47e)){
var tmp=_47e;
_47e=_47f;
_47f=tmp;
}
node=dojo.byId(node);
var _484=dojo.style.getBackgroundColor(node);
var bg=dojo.style.getStyle(node,"background-color").toLowerCase();
var _486=bg=="transparent"||bg=="rgba(0, 0, 0, 0)";
while(_484.length>3){
_484.pop();
}
var rgb=new dojo.graphics.color.Color(_47f).toRgb();
var anim=dojo.fx.html.colorFade(node,_47e||dojo.fx.duration,_47f,_484,_481,true);
dojo.event.connect(anim,"onEnd",function(e){
if(_486){
node.style.backgroundColor="transparent";
}
});
if(_480>0){
node.style.backgroundColor="rgb("+rgb.join(",")+")";
if(!_482){
setTimeout(function(){
anim.play(true);
},_480);
}
}else{
if(!_482){
anim.play(true);
}
}
return anim;
};
dojo.fx.html.highlight=dojo.fx.html.colorFadeIn;
dojo.fx.html.colorFadeFrom=dojo.fx.html.colorFadeIn;
dojo.fx.html.colorFadeOut=function(node,_48b,_48c,_48d,_48e,_48f){
if(!dojo.lang.isNumber(_48b)){
var tmp=_48b;
_48b=_48c;
_48c=tmp;
}
node=dojo.byId(node);
var _491=new dojo.graphics.color.Color(dojo.style.getBackgroundColor(node)).toRgb();
var rgb=new dojo.graphics.color.Color(_48c).toRgb();
var anim=dojo.fx.html.colorFade(node,_48b||dojo.fx.duration,_491,rgb,_48e,_48d>0||_48f);
if(_48d>0){
node.style.backgroundColor="rgb("+_491.join(",")+")";
if(!_48f){
setTimeout(function(){
anim.play(true);
},_48d);
}
}
return anim;
};
dojo.fx.html.unhighlight=dojo.fx.html.colorFadeOut;
dojo.fx.html.colorFadeTo=dojo.fx.html.colorFadeOut;
dojo.fx.html.colorFade=function(node,_495,_496,_497,_498,_499){
if(!dojo.lang.isNumber(_495)){
var tmp=_495;
_495=_497;
_497=_496;
_496=tmp;
}
node=dojo.byId(node);
var _49b=new dojo.graphics.color.Color(_496).toRgb();
var _49c=new dojo.graphics.color.Color(_497).toRgb();
var anim=new dojo.animation.Animation(new dojo.math.curves.Line(_49b,_49c),_495||dojo.fx.duration,0);
dojo.event.connect(anim,"onAnimate",function(e){
node.style.backgroundColor="rgb("+e.coordsAsInts().join(",")+")";
});
if(_498){
dojo.event.connect(anim,"onEnd",function(e){
_498(node,anim);
});
}
if(!_499){
anim.play(true);
}
return anim;
};
dojo.fx.html.wipeIn=function(node,_4a1,_4a2,_4a3){
node=dojo.byId(node);
var _4a4=dojo.style.getStyle(node,"overflow");
if(_4a4=="visible"){
node.style.overflow="hidden";
}
node.style.height=0;
dojo.style.show(node);
var anim=dojo.fx.html.wipe(node,_4a1,0,node.scrollHeight,null,true);
dojo.event.connect(anim,"onEnd",function(){
node.style.overflow=_4a4;
node.style.height="auto";
if(_4a2){
_4a2(node,anim);
}
});
if(!_4a3){
anim.play();
}
return anim;
};
dojo.fx.html.wipeOut=function(node,_4a7,_4a8,_4a9){
node=dojo.byId(node);
var _4aa=dojo.style.getStyle(node,"overflow");
if(_4aa=="visible"){
node.style.overflow="hidden";
}
var anim=dojo.fx.html.wipe(node,_4a7,node.offsetHeight,0,null,true);
dojo.event.connect(anim,"onEnd",function(){
dojo.style.hide(node);
node.style.overflow=_4aa;
if(_4a8){
_4a8(node,anim);
}
});
if(!_4a9){
anim.play();
}
return anim;
};
dojo.fx.html.wipe=function(node,_4ad,_4ae,_4af,_4b0,_4b1){
node=dojo.byId(node);
var anim=new dojo.animation.Animation([[_4ae],[_4af]],_4ad||dojo.fx.duration,0);
dojo.event.connect(anim,"onAnimate",function(e){
node.style.height=e.x+"px";
});
dojo.event.connect(anim,"onEnd",function(){
if(_4b0){
_4b0(node,anim);
}
});
if(!_4b1){
anim.play();
}
return anim;
};
dojo.fx.html.wiper=function(node,_4b5){
this.node=dojo.byId(node);
if(_4b5){
dojo.event.connect(dojo.byId(_4b5),"onclick",this,"toggle");
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
dojo.fx.html.explode=function(_4b7,_4b8,_4b9,_4ba,_4bb){
var _4bc=dojo.style.toCoordinateArray(_4b7);
var _4bd=document.createElement("div");
with(_4bd.style){
position="absolute";
border="1px solid black";
display="none";
}
document.body.appendChild(_4bd);
_4b8=dojo.byId(_4b8);
with(_4b8.style){
visibility="hidden";
display="block";
}
var _4be=dojo.style.toCoordinateArray(_4b8);
with(_4b8.style){
display="none";
visibility="visible";
}
var anim=new dojo.animation.Animation(new dojo.math.curves.Line(_4bc,_4be),_4b9||dojo.fx.duration,0);
dojo.event.connect(anim,"onBegin",function(e){
_4bd.style.display="block";
});
dojo.event.connect(anim,"onAnimate",function(e){
with(_4bd.style){
left=e.x+"px";
top=e.y+"px";
width=e.coords[2]+"px";
height=e.coords[3]+"px";
}
});
dojo.event.connect(anim,"onEnd",function(){
_4b8.style.display="block";
_4bd.parentNode.removeChild(_4bd);
if(_4ba){
_4ba(_4b8,anim);
}
});
if(!_4bb){
anim.play();
}
return anim;
};
dojo.fx.html.implode=function(_4c2,end,_4c4,_4c5,_4c6){
var _4c7=dojo.style.toCoordinateArray(_4c2);
var _4c8=dojo.style.toCoordinateArray(end);
_4c2=dojo.byId(_4c2);
var _4c9=document.createElement("div");
with(_4c9.style){
position="absolute";
border="1px solid black";
display="none";
}
document.body.appendChild(_4c9);
var anim=new dojo.animation.Animation(new dojo.math.curves.Line(_4c7,_4c8),_4c4||dojo.fx.duration,0);
dojo.event.connect(anim,"onBegin",function(e){
_4c2.style.display="none";
_4c9.style.display="block";
});
dojo.event.connect(anim,"onAnimate",function(e){
with(_4c9.style){
left=e.x+"px";
top=e.y+"px";
width=e.coords[2]+"px";
height=e.coords[3]+"px";
}
});
dojo.event.connect(anim,"onEnd",function(){
_4c9.parentNode.removeChild(_4c9);
if(_4c5){
_4c5(_4c2,anim);
}
});
if(!_4c6){
anim.play();
}
return anim;
};
dojo.fx.html.Exploder=function(_4cd,_4ce){
_4cd=dojo.byId(_4cd);
_4ce=dojo.byId(_4ce);
var _4cf=this;
this.waitToHide=500;
this.timeToShow=100;
this.waitToShow=200;
this.timeToHide=70;
this.autoShow=false;
this.autoHide=false;
var _4d0=null;
var _4d1=null;
var _4d2=null;
var _4d3=null;
var _4d4=null;
var _4d5=null;
this.showing=false;
this.onBeforeExplode=null;
this.onAfterExplode=null;
this.onBeforeImplode=null;
this.onAfterImplode=null;
this.onExploding=null;
this.onImploding=null;
this.timeShow=function(){
clearTimeout(_4d2);
_4d2=setTimeout(_4cf.show,_4cf.waitToShow);
};
this.show=function(){
clearTimeout(_4d2);
clearTimeout(_4d3);
if((_4d1&&_4d1.status()=="playing")||(_4d0&&_4d0.status()=="playing")||_4cf.showing){
return;
}
if(typeof _4cf.onBeforeExplode=="function"){
_4cf.onBeforeExplode(_4cd,_4ce);
}
_4d0=dojo.fx.html.explode(_4cd,_4ce,_4cf.timeToShow,function(e){
_4cf.showing=true;
if(typeof _4cf.onAfterExplode=="function"){
_4cf.onAfterExplode(_4cd,_4ce);
}
});
if(typeof _4cf.onExploding=="function"){
dojo.event.connect(_4d0,"onAnimate",this,"onExploding");
}
};
this.timeHide=function(){
clearTimeout(_4d2);
clearTimeout(_4d3);
if(_4cf.showing){
_4d3=setTimeout(_4cf.hide,_4cf.waitToHide);
}
};
this.hide=function(){
clearTimeout(_4d2);
clearTimeout(_4d3);
if(_4d0&&_4d0.status()=="playing"){
return;
}
_4cf.showing=false;
if(typeof _4cf.onBeforeImplode=="function"){
_4cf.onBeforeImplode(_4cd,_4ce);
}
_4d1=dojo.fx.html.implode(_4ce,_4cd,_4cf.timeToHide,function(e){
if(typeof _4cf.onAfterImplode=="function"){
_4cf.onAfterImplode(_4cd,_4ce);
}
});
if(typeof _4cf.onImploding=="function"){
dojo.event.connect(_4d1,"onAnimate",this,"onImploding");
}
};
dojo.event.connect(_4cd,"onclick",function(e){
if(_4cf.showing){
_4cf.hide();
}else{
_4cf.show();
}
});
dojo.event.connect(_4cd,"onmouseover",function(e){
if(_4cf.autoShow){
_4cf.timeShow();
}
});
dojo.event.connect(_4cd,"onmouseout",function(e){
if(_4cf.autoHide){
_4cf.timeHide();
}
});
dojo.event.connect(_4ce,"onmouseover",function(e){
clearTimeout(_4d3);
});
dojo.event.connect(_4ce,"onmouseout",function(e){
if(_4cf.autoHide){
_4cf.timeHide();
}
});
dojo.event.connect(document.documentElement||document.body,"onclick",function(e){
function isDesc(node,_4df){
while(node){
if(node==_4df){
return true;
}
node=node.parentNode;
}
return false;
}
if(_4cf.autoHide&&_4cf.showing&&!isDesc(e.target,_4ce)&&!isDesc(e.target,_4cd)){
_4cf.hide();
}
});
return this;
};
dojo.lang.mixin(dojo.fx,dojo.fx.html);

