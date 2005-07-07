	var LowBgColor="#FFCC33";
	var HighBgColor="#000080";
	var FontLowColor="black";
	var FontHighColor="white";
	var BorderColor="black";
	var BorderWidth=1;
	var BorderBtwnElmnts=1;
	var FontFamily="Arial,Helvetica,sans-serif";
	var FontSize=10;
	var MemberFontFamily="MS Sans Serif,Arial,Helvetica,sans-serif";
	var MemberFontSize=8;
	var FontBold=1;
	var FontItalic=0;
	var MenuTextCentered=0;
	var MenuCentered="left";
	var MenuVerticalCentered="top";
	var ChildOverlap=.1;
	var ChildVerticalOverlap=.1;
	var StartTop=60; //set vertical offset
	var StartLeft=250; //set horizontal offset
	var VerCorrect=0;
	var HorCorrect=0;
	var LeftPaddng=3;
	var TopPaddng=3;
	var FirstLineHorizontal=1; //set menu layout (1=horizontal, 0=vertical)
	var MenuFramesVertical=1;
	var DissapearDelay=500;
	var TakeOverBgColor=1;
	var FirstLineFrame="navig";
	var SecLineFrame="space";
	var DocTargetFrame="space";
	var WebMasterCheck=0;

	var locn = location.toString();
	var startIPLocn = locn.indexOf('/');
	var endIPLocn;
	var ip = "http://";

	var url = String(document.location);
        var fronturl = url.substr(0,5);

        if (fronturl == "https")
            ip = "https://";

	if( locn.indexOf('/', startIPLocn+1) == startIPLocn+1 )
	{
		// user entered http://10.1.1.90/...
		startIPLocn = startIPLocn+2;
		endIPLocn = locn.indexOf('/', startIPLocn);
		if(endIPLocn == -1){ 
			// user entered http://10.1.1.90
			endIPLocn = locn.length; 
		}
			
	}
	else
	{
		if(startIPLocn == -1)
		{
			//user entered 10.1.1.90
			endIPLocn = locn.length;
		}
		else
		{	// if user enters 10.1.1.90/applications.cgi
			endIPLocn = startIPLocn;
		}
		startIPLocn=0;
	}
	
	if(startIPLocn != -1 && endIPLocn != -1)
	{
		ip += locn.substring(startIPLocn, endIPLocn); 
	}

	//This random id is going to be appended to the URL. 
	//This prevents page caching as everytime the browser thinks it is a new page.
	var randomID=0;
	with (Math) {   randomID= Math.floor(Math.random() * 1000);}

//Menux=new Array("text to show","Link",No of sub elements,element height,element width);
//see accompanying "config.htm" file for more information on structure of menus

    Menu1=new Array("Home","",2,24,100);
	Menu1_1=new Array("Dial By URL",ip+"/cgi/homepage.cgi",0,22,110);	
	Menu1_2=new Array("Change Ringtone",ip+"/cgi/homepage.cgi#ringtone",0,22,110);	
	
    Menu2=new Array("Applications","",1,24,100);
	Menu2_1=new Array("Manage",ip+"/cgi/applications.cgi",0,22,100);	
	
    //if speeddial is read-only, do not show the speeddialmenu items.
    if (( document.forms["menu_controller"]) &&
        ( document.forms["menu_controller"].speeddial_enable) &&
        ( document.forms["menu_controller"].speeddial_enable.value == "false")){	
        
        var NoOffFirstLineMenus=4; //set number of main menu items
        var LastMenuPosition = 513; //Starting point of last menu 
    
        Menu3=new Array("Preferences","",2,24,100);
    	Menu3_1=new Array("Lines",ip+"/cgi/MultiLine.cgi",0,22,100);
    	Menu3_2=new Array("Call Handling",ip+"/cgi/xpressaconfiguser.cgi?ID="+randomID,0,22,100);
    
        Menu4=new Array("Administration","",5,24,100);
    	Menu4_1=new Array("User Maintenance",ip+"/cgi/login.cgi",0,22,140);
    	Menu4_2=new Array("Change Password",ip+"/cgi/password.cgi",0,22,140);
    	Menu4_3=new Array("Phone Configuration",ip+"/cgi/xpressaconfigadmin.cgi?ID="+randomID,0,22,140);
    	Menu4_4=new Array("File Uploads",ip+"/cgi/config.cgi",0,22,140);
    	Menu4_5=new Array("SIP Log",ip+"/cgi/sip.cgi",0,22,140);
	
    }else{
        var NoOffFirstLineMenus=5; //set number of main menu items
        var LastMenuPosition = 614; //Starting point of last menu 
        
        
        Menu3=new Array("Speed Dial","",2,24,100);
    	Menu3_1=new Array("Add",ip+"/cgi/addspeeddial.cgi",0,22,100);
    	Menu3_2=new Array("View",ip+"/cgi/viewspeeddial.cgi?ID="+randomID,0,22,100);
    	
    	Menu4=new Array("Preferences","",2,24,100);
    	Menu4_1=new Array("Lines",ip+"/cgi/MultiLine.cgi",0,22,100);
    	Menu4_2=new Array("Call Handling",ip+"/cgi/xpressaconfiguser.cgi?ID="+randomID,0,22,100);
    
        Menu5=new Array("Administration","",5,24,100);
    	Menu5_1=new Array("User Maintenance",ip+"/cgi/login.cgi",0,22,140);
    	Menu5_2=new Array("Change Password",ip+"/cgi/password.cgi",0,22,140);
    	Menu5_3=new Array("Phone Configuration",ip+"/cgi/xpressaconfigadmin.cgi?ID="+randomID,0,22,140);
    	Menu5_4=new Array("File Uploads",ip+"/cgi/config.cgi",0,22,140);
    	Menu5_5=new Array("SIP Log",ip+"/cgi/sip.cgi",0,22,140);

    }		
		
	var AgntUsr=navigator.userAgent.toLowerCase();
	var DomYes=(document.getElementById)?1:0;				// DOM
	var NavYes=(AgntUsr.indexOf("mozilla")!=-1&&AgntUsr.indexOf("compatible")==-1)?1:0;	// Netscape
	var ExpYes=(AgntUsr.indexOf("msie")!=-1)?1:0;				// IExplorer
	var Opr5=(AgntUsr.indexOf("opera 5")!=-1||AgntUsr.indexOf("opera/5")!=-1)?1:0;	// Opera 5
	var DomNav=(DomYes&&NavYes)?1:0;					// Netscape 6 up
 	var DomExp=(DomYes&&ExpYes)?1:0;					// IExplorer 5 up
	var Nav4=(NavYes&&!DomYes&&document.layers)?1:0;			// Netscape 4
	var Exp4=(ExpYes&&!DomYes&&document.all)?1:0;				// IExplorer 4
	var PosStrt=((NavYes||ExpYes)&&!Opr5)?1:0;				// Can start

	var FirstLocation,SecLocation,DocLocation;
	var SecWindowWidth,FirstWindowWidth,FirstWindowHeight;
	var SecLoadAgainWin;
	var FirstColPos,SecColPos,DocColPos;
	var InitRecursLevel=0,CreateRecursLevel=0,PosRecursLevel=0;
	var ShowFlag=0,FirstCreate=1,Loaded=0,Created=0,InitFlag,AcrossFrames=1;
	var FirstContainer=null,CurrentOver=null,CloseTimer=null;
	var CenterText,TxtClose,ImgStr;
	var Ztop=100;
	var M_StartTop=StartTop,M_StartLeft=StartLeft;
	var LeftExtra=(DomNav)?LeftPaddng:0;
	var TopExtra=(DomNav)?TopPaddng:0;
	var M_Hide=(Nav4)?"hide":"hidden";
	var M_Show=(Nav4)?"show":"visible";
	var Par=(parent.frames[0])?parent:window;
	var Doc=Par.document;
	var Bod=Doc.body;
	var Trigger=(parent.frames[0]&&FirstLineFrame==SecLineFrame)?(NavYes)?parent.frames[FirstLineFrame]:parent.frames[FirstLineFrame].document.body:(NavYes)?Par:Bod;
	WebMasterAlerts=["No such frame: ","Item not defined: ","Item needs height: ","Item needs width: ","Warning-Item doesn't need height: ","Warning-Item doesn't need width: ","Item Oke ","Menu tree oke"];
	if(DomNav&&!Opr5)Trigger.addEventListener("load",Go,false);
	else Trigger.onload=Go;
	Trigger.onresize=(Nav4)?ReDoWhole:RePos;

function cancelselect(){return false}

function RePos(){
	FirstWindowWidth=(ExpYes)?FirstLocation.document.body.clientWidth:FirstLocation.innerWidth;
	if(FirstWindowWidth<710){ FirstWindowWidth=710; }
	FirstWindowHeight=(ExpYes)?FirstLocation.document.body.clientHeight:FirstLocation.innerHeight;
	SecWindowWidth=(ExpYes)?SecLocation.document.body.clientWidth:SecLocation.innerWidth;
	if(SecWindowWidth<710){ SecWindowWidth=710; }
	SecWindowHeight=(ExpYes)?SecLocation.document.body.clientHeight:SecLocation.innerHeight;
	if(MenuCentered)CalcLeft();
	if(MenuVerticalCentered)CalcTop();
	PositionMenuStruct(FirstContainer,StartTop,StartLeft)}

function UnLoaded(){
	if(typeof(CloseTimer)!="undefined"&&CloseTimer)clearTimeout(CloseTimer); //typeoff fix by Martin Renschler
	Loaded=0; Created=0;
	var FCStyle=(Nav4)?FirstContainer:FirstContainer.style;
	FCStyle.visibility=M_Hide}

function ReDoWhole(){
	Doc.location.reload()}

function Check(WhichMenu,NoOf){
	var i,Hg,Wd,La,Li,Nof,array,ArrayLoc;
	ArrayLoc=(parent.frames[0])?parent.frames[FirstLineFrame]:self;
	for(i=0;i<NoOf;i++){
		array=WhichMenu+eval(i+1);
		if(!ArrayLoc[array]){WebMasterAlert(1,array); return false}
		La=ArrayLoc[array][0]; Li=ArrayLoc[array][1]; Nof=ArrayLoc[array][2];
		if(i==0){	if(!ArrayLoc[array][3]){WebMasterAlert(2,array); return false}
			if(!ArrayLoc[array][4]){WebMasterAlert(3,array); return false}
			Hg=ArrayLoc[array][3]; Wd=ArrayLoc[array][4]}
		else{	if(ArrayLoc[array][3])WebMasterAlert(4,array);
			if(ArrayLoc[array][4])WebMasterAlert(5,array)}
		if(!WebMasterAlert(6,"\n\n"+array+"\nwidth: "+Wd+"\nheight: "+Hg+"\nLabel: "+La+"\nLink: "+Li+"\nNo of sub items: "+Nof)){WebMasterCheck=0; return true}
		if(ArrayLoc[array][2])if(!Check(array+"_",ArrayLoc[array][2])) return false}
	return true}	

function WebMasterAlert(No,Xtra){
	if(WebMasterCheck)return confirm(WebMasterAlerts[No]+Xtra+"   ")}

function Go(){
	if(Loaded||!PosStrt)return;
	Created=0; Loaded=1;
	status="Building menu";
	if(FirstCreate){
		if(FirstLineFrame =="" || !parent.frames[FirstLineFrame]){WebMasterAlert(0,FirstLineFrame); FirstLineFrame=SecLineFrame}
		if(FirstLineFrame =="" || !parent.frames[FirstLineFrame]){WebMasterAlert(0,SecLineFrame); FirstLineFrame=SecLineFrame=DocTargetFrame}
		if(FirstLineFrame =="" || !parent.frames[FirstLineFrame]){WebMasterAlert(0,DocTargetFrame); FirstLineFrame=SecLineFrame=DocTargetFrame=(parent.frames[0])?parent.frames[0].name:""}
		if(SecLineFrame =="" || !parent.frames[SecLineFrame])SecLineFrame=DocTargetFrame;
		if(SecLineFrame =="" || !parent.frames[SecLineFrame])SecLineFrame=DocTargetFrame=FirstLineFrame;
		if(DocTargetFrame =="" || !parent.frames[DocTargetFrame])DocTargetFrame=SecLineFrame;
		if(WebMasterCheck){if(!Check("Menu",NoOffFirstLineMenus))return;else WebMasterAlert(7,"")}
		FirstLocation=(FirstLineFrame)?parent.frames[FirstLineFrame]:window;
		SecLocation=(SecLineFrame)?parent.frames[SecLineFrame]:window;
		DocLocation=(DocTargetFrame)?parent.frames[DocTargetFrame]:window;
		if (FirstLocation==SecLocation) AcrossFrames=0;
		if (AcrossFrames)FirstLineHorizontal=(MenuFramesVertical)?0:1;
		FirstWindowWidth=(ExpYes)?FirstLocation.document.body.clientWidth:FirstLocation.innerWidth;
		FirstWindowHeight=(ExpYes)?FirstLocation.document.body.clientHeight:FirstLocation.innerHeight;
		SecWindowWidth=(ExpYes)?SecLocation.document.body.clientWidth:SecLocation.innerWidth;
		SecWindowHeight=(ExpYes)?SecLocation.document.body.clientHeight:SecLocation.innerHeight;
		if(MenuCentered) CalcLeft();
		if(MenuVerticalCentered) CalcTop();
		if(!DomYes){CenterText=(MenuTextCentered)?(NavYes)?"<div align='center'>":"align='center'":"<spacer type='vertical' size=3>";
			if(NavYes)TxtClose="</font>"+(MenuTextCentered)?"</div>":""}}
	FirstColPos=(Nav4)?FirstLocation.document:FirstLocation.document.body;
	SecColPos=(Nav4)?SecLocation.document:SecLocation.document.body;
	DocColPos=(Nav4)?DocLocation.document:SecLocation.document.body;
	if (TakeOverBgColor)FirstColPos.bgColor=(AcrossFrames)?SecColPos.bgColor:DocColPos.bgColor;
	if(FirstCreate){FirstContainer=CreateMenuStructure("Menu",NoOffFirstLineMenus);FirstCreate=0}
	else CreateMenuStructureAgain("Menu",NoOffFirstLineMenus);
	PositionMenuStruct(FirstContainer,StartTop,StartLeft);
	InitFlag=1;	Initiate(); InitFlag=0; Created=1; 
	SecLoadAgainWin=(ExpYes)?SecLocation.document.body:SecLocation;
	SecLoadAgainWin.onunload=UnLoaded;
	status=""}


function CalcLeft(){
	var Size=FirstWindowWidth;
	if(MenuCentered!="left"){
		StartLeft=M_StartLeft;
		Size-=(FirstLineHorizontal)?(NoOffFirstLineMenus*(eval("Menu1[4]")+BorderWidth)+BorderWidth):(eval("Menu1[4]")+BorderWidth);
		if(MenuCentered=="right")StartLeft+=Size; else StartLeft+=Size/2}}

function CalcTop(){
	var Size=FirstWindowHeight;
	if(MenuVerticalCentered!="top"){	
		StartTop=M_StartTop;
		Size-=(FirstLineHorizontal)?(eval("Menu1[3]")+BorderWidth):(NoOffFirstLineMenus*(eval("Menu1[3]")+BorderWidth)+BorderWidth);
		if(MenuVerticalCentered=="bottom")StartTop+=Size; else StartTop+=Size/2}}

function PositionMenuStruct(CntnrPntr,Tp,Lt){
	var Topi,Lefti,Hori;
	var Cntnr=CntnrPntr;
	var Mmbr=Cntnr.FirstMember;
	var CntnrStyle=(!Nav4)?Cntnr.style:Cntnr;
	var MmbrStyle=(!Nav4)?Mmbr.style:Mmbr;
	var PadLeft=(Mmbr.value.indexOf("<")==-1)?LeftExtra:0;
	var PadTop=(Mmbr.value.indexOf("<")==-1)?TopExtra:0;
	var MmbrWidth=(!Nav4)?parseInt(MmbrStyle.width)+PadLeft:MmbrStyle.clip.width;
	var MmbrHeight=(!Nav4)?parseInt(MmbrStyle.height)+PadTop:MmbrStyle.clip.height;
	var CntnrWidth=(!Nav4)?parseInt(CntnrStyle.width):CntnrStyle.clip.width;
	var CntnrHeight=(!Nav4)?parseInt(CntnrStyle.height):CntnrStyle.clip.height;
	var SubTp,SubLt;
	PosRecursLevel++;
	if (PosRecursLevel==1 && AcrossFrames)(!MenuFramesVertical)?Tp=FirstWindowHeight-CntnrHeight+((Nav4)?4:0):Lt=FirstWindowWidth-CntnrWidth;
	if (PosRecursLevel==2 && AcrossFrames)(!MenuFramesVertical)?Tp=0:Lt=0;
	if (PosRecursLevel==2 && AcrossFrames){Tp+=VerCorrect;Lt+=HorCorrect}
	CntnrStyle.top=Cntnr.OrgTop=Tp;
	CntnrStyle.left=Cntnr.OrgLeft=Lt;
	if (PosRecursLevel==1 && FirstLineHorizontal){Hori=1; Lefti=CntnrWidth-MmbrWidth-2*BorderWidth;Topi=0}
	else{Hori=Lefti=0; Topi=CntnrHeight-MmbrHeight-2*BorderWidth}
	var isLastMenu=1;
	while(Mmbr!=null){
		PadLeft=(Mmbr.value.indexOf("<")==-1)?LeftExtra:0;
		PadTop=(Mmbr.value.indexOf("<")==-1)?TopExtra:0;
		MmbrStyle=(!Nav4)?Mmbr.style:Mmbr;
		MmbrWidth=(!Nav4)?parseInt(MmbrStyle.width)+PadLeft:MmbrStyle.clip.width;
		MmbrHeight=(!Nav4)?parseInt(MmbrStyle.height)+PadTop:MmbrStyle.clip.height;
		MmbrStyle=(!Nav4)?Mmbr.style:Mmbr;
		MmbrStyle.left=Lefti+BorderWidth; 
		MmbrStyle.top=Topi+BorderWidth;
		if(Nav4)Mmbr.CmdLyr.moveTo(Lefti+BorderWidth,Topi+BorderWidth);
		if(Mmbr.ChildCntnr){
			if(Hori){ 
				SubTp=Tp+Topi+MmbrHeight+BorderWidth; 
				if(isLastMenu){ SubLt=LastMenuPosition; isLastMenu=0}
				else{ SubLt=Lt+Lefti;}
			}
			else{	SubLt=Lt+Lefti+(1-ChildOverlap)*MmbrWidth+BorderWidth; SubTp=Tp+Topi+ChildVerticalOverlap*MmbrHeight}
			PositionMenuStruct(Mmbr.ChildCntnr,SubTp,SubLt)}
		(Hori)?Lefti-=(BorderBtwnElmnts)?(MmbrWidth+BorderWidth):(MmbrWidth):Topi-=(BorderBtwnElmnts)?(MmbrHeight+BorderWidth):(MmbrHeight);
		Mmbr=Mmbr.PrevMember}
	PosRecursLevel--}

function Initiate(){
	if(InitFlag) Init(FirstContainer)}

function Init(CntnrPntr){
	var MemberContainer=CntnrPntr;
	var Member=MemberContainer.FirstMember;
	var MCStyle=(Nav4)?MemberContainer:MemberContainer.style;
	InitRecursLevel++;
	MCStyle.visibility=(InitRecursLevel==1)?M_Show:M_Hide;
	while(Member!=null){
		if(Member.ChildCntnr) Init(Member.ChildCntnr);
		Member=Member.PrevMember}
	InitRecursLevel--}

function ClearAllChilds(Pntr,ChldPntr){
	var CPCCStyle;
	while (Pntr){
		if(Pntr.ChildCntnr){
			CPCCStyle=(Nav4)?Pntr.ChildCntnr:Pntr.ChildCntnr.style;
			if(Pntr.ChildCntnr!=ChldPntr)CPCCStyle.visibility=M_Hide;
			ClearAllChilds(Pntr.ChildCntnr.FirstMember,ChldPntr)}
		Pntr=Pntr.PrevMember}}	

function GoTo(){
	if(this.LinkTxt){
		status=""; 
		if(Nav4){	if(this.LowLyr.value.indexOf("<img")==-1){
				this.LowLyr.bgColor=this.LowLyr.LowBack;
				this.LowLyr.document.write(this.LowLyr.value);
				this.LowLyr.document.close()}}
		else{	this.style.backgroundColor=this.LowBack;
			this.style.color=this.LowFontColor}
		DocLocation.location.href=this.LinkTxt}}

function OpenMenu(){
	if(!Loaded||!Created) return;
	var TopScrolled=(AcrossFrames)?(ExpYes)?SecLocation.document.body.scrollTop:SecLocation.pageYOffset:0;
	var LeftScrolled=(AcrossFrames)?(ExpYes)?SecLocation.document.body.scrollLeft:SecLocation.pageXOffset:0;
	var ChildCont=(Nav4)?this.LowLyr.ChildCntnr:this.ChildCntnr;
	var ThisHeight=(Nav4)?this.clip.height:parseInt(this.style.height);
	var ThisWidth=(Nav4)?this.clip.width:parseInt(this.style.width);
	CurrentOver=this; InitFlag=0;
	if(ShowFlag){
		ClearAllChilds(this.Container.FirstMember,ChildCont); 
		if(this.Level==1) ShowFlag=0}
	if(Nav4){	if(this.LowLyr.value.indexOf("<img")==-1){
			this.LowLyr.bgColor=this.LowLyr.HighBack;
			this.LowLyr.document.write(this.LowLyr.Overvalue);
			this.LowLyr.document.close()}}
	else{	this.style.backgroundColor=this.HighBack;
		this.style.color=this.HighFontColor}
	if(ChildCont!=null)	{
		var ChildContWidth=(Nav4)?this.LowLyr.ChildCntnr.clip.width:parseInt(this.ChildCntnr.style.width);
		var ChildContHeight=(Nav4)?this.LowLyr.ChildCntnr.clip.height:parseInt(this.ChildCntnr.style.height);
		var ChCntTL=(Nav4)?this.LowLyr.ChildCntnr:this.ChildCntnr.style;
		var SubLt=ChildCont.OrgLeft+LeftScrolled;
		var SubTp=ChildCont.OrgTop+TopScrolled;
		while(SubLt+ChildContWidth>SecWindowWidth+LeftScrolled){
			if(this.Level==1)SubLt=SecWindowWidth+LeftScrolled-ChildContWidth;
			else SubLt-=10}
		while(SubTp+ChildContHeight>TopScrolled+SecWindowHeight){
			if(this.Level==1)SubTp=TopScrolled+SecWindowHeight-ChildContHeight;
			else SubTp-=10}
		ChCntTL.top=SubTp;
		ChCntTL.left=SubLt;
		ChCntTL.visibility=M_Show;
		ShowFlag=1}
	status=this.LinkTxt}	

function CloseMenu(){
	if(!Loaded||!Created) return;
	if(Nav4){if(this.LowLyr.value.indexOf("<img")==-1){
		this.LowLyr.bgColor=this.LowLyr.LowBack;
		this.LowLyr.document.write(this.LowLyr.value);
		this.LowLyr.document.close()}}
	else{	this.style.backgroundColor=this.LowBack;
		this.style.color=this.LowFontColor}
	status="";
	if(this==CurrentOver){
		InitFlag=1;
		if (CloseTimer) clearTimeout(CloseTimer);
		CloseTimer=setTimeout("Initiate(CurrentOver)",DissapearDelay)}}

function ContainerSetUp(Wdth,Hght,NoOff){
	this.FirstMember=null;
	this.OrgLeft=0;
	this.OrgTop=0;
	if (CreateRecursLevel==1&&FirstLineHorizontal){
		Wdth=(BorderBtwnElmnts)?NoOff*(Wdth+BorderWidth)+BorderWidth:NoOff*Wdth+2*BorderWidth;
		Hght+=2*BorderWidth}
	else{ 	Hght=(BorderBtwnElmnts)?NoOff*(Hght+BorderWidth)+BorderWidth:NoOff*Hght+2*BorderWidth;
		Wdth+=2*BorderWidth}
	if(Nav4){	this.visibility="hide";
		this.bgColor=BorderColor;
		this.resizeTo(Wdth,Hght)}
	else{	this.Level=CreateRecursLevel;
		this.style.backgroundColor=BorderColor;
		this.style.width=Wdth;
		this.style.height=Hght;
		this.style.fontFamily=FontFamily;
		this.style.fontWeight=(FontBold)?"bold":"normal";
		this.style.fontStyle=(FontItalic)?"italic":"normal";
		this.style.fontSize=FontSize+"pt";
		this.style.zIndex=CreateRecursLevel+Ztop}}

function MemberSetUp(MmbrCntnr,PrMmbr,WhatMenu,Wdth,Hght){
	var Location=(CreateRecursLevel==1)?FirstLocation:SecLocation;
	var MemVal=eval(WhatMenu+"[0]");
	var t,T,L,W,H,S;
	this.value=MemVal;
	this.ChildCntnr=null;
	this.PrevMember=PrMmbr;
	this.Level=CreateRecursLevel;
	var isTopMenu=eval(WhatMenu+"[1]");
	if(isTopMenu==null || isTopMenu==""){ this.style.cursor="default"; }
	else { this.style.cursor="hand"; this.style.fontSize=MemberFontSize+"pt"; this.style.fontFamily=MemberFontFamily;}
	this.LinkTxt=eval(WhatMenu+"[1]");
	this.Container=MmbrCntnr;
	if(MemVal.indexOf("<")==-1){this.style.width=Wdth-LeftExtra; this.style.height=Hght-TopExtra; this.style.paddingLeft=LeftPaddng; this.style.paddingTop=TopPaddng}
	else{this.style.width=Wdth; this.style.height=Hght}
	this.style.overflow="hidden"
	this.LowBack=LowBgColor;
	this.LowFontColor=FontLowColor;
	this.HighBack=(MemVal.indexOf("<img")==-1)?HighBgColor:LowBgColor;
	this.HighFontColor=(MemVal.indexOf("<img")==-1)?FontHighColor:FontLowColor; 
	this.style.color=this.LowFontColor;
	this.style.backgroundColor=this.LowBack;
	if(MenuTextCentered)this.style.textAlign="center";
	if(MemVal.indexOf("<")==-1&&DomYes){t=Location.document.createTextNode(MemVal);this.appendChild(t)}
	else this.innerHTML=MemVal;
	if(MemVal.indexOf("<img")==-1&&eval(WhatMenu+"[2]")){
		S=(CreateRecursLevel==1&&FirstLineHorizontal)?"tridown.gif":"tri.gif";
		W=(CreateRecursLevel==1&&FirstLineHorizontal)?10:5;
		H=(CreateRecursLevel==1&&FirstLineHorizontal)?5:10;
		T=(CreateRecursLevel==1&&FirstLineHorizontal)?Hght-7:Hght/2-5;
		L=(CreateRecursLevel==1&&FirstLineHorizontal)?Wdth-12:Wdth-7;
		//if(DomYes){t=Location.document.createElement("img"); this.appendChild(t); t.style.position="absolute"; t.src=S; t.style.width=W; t.style.height=H; t.style.top=T; t.style.left=L}
		//else{MemVal+="<div style="position:absolute; top:"+T+"; left:"+L+"; width:"+W+"; height:"+H+";visibility:inherit"><img src=""+S+""></div>"; this.innerHTML=MemVal}
		}
	if(ExpYes){this.onmouseover=OpenMenu; this.onmouseout=CloseMenu; this.onclick=GoTo}
	else{this.addEventListener("mouseover",OpenMenu,false); this.addEventListener("mouseout",CloseMenu,false); this.addEventListener("click",GoTo,false)}}

function NavMemberSetUp(MmbrCntnr,PrMmbr,WhatMenu,Wdth,Hght){
	this.value=eval(WhatMenu+"[0]");
	if(LeftPaddng&&this.value.indexOf("<")==-1&&!MenuTextCentered)this.value="&nbsp\;"+this.value;
	if(FontBold)this.value=this.value.bold();
	if(FontItalic)this.value=this.value.italics();
	this.Overvalue=this.value;
	this.value=this.value.fontcolor(FontLowColor);
	this.Overvalue=this.Overvalue.fontcolor(FontHighColor);
	var isTopMenu=eval(WhatMenu+"[1]");
	if(isTopMenu==null || isTopMenu=="")
	{
		this.cursor="default"; 
		this.value=CenterText+"<font face='"+FontFamily+"' point-size='"+FontSize+"' color='"+FontLowColor+"'>"+this.value+TxtClose;
		this.Overvalue=CenterText+"<font face='"+FontFamily+"' point-size='"+FontSize+"' color='"+FontHighColor+"'>"+this.Overvalue+TxtClose;
	}
	else
	{
		this.cursor="hand"; 
		this.value=CenterText+"<font face='"+MemberFontFamily+"' point-size='"+MemberFontSize+"' color='"+FontLowColor+"'>"+this.value+TxtClose;
		this.Overvalue=CenterText+"<font face='"+MemberFontFamily+"' point-size='"+MemberFontSize+"' color='"+FontHighColor+"'>"+this.Overvalue+TxtClose;
	}	
	this.LowBack=LowBgColor;
	this.HighBack=HighBgColor;
	this.visibility="inherit";
	this.ChildCntnr=null;
	this.PrevMember=PrMmbr;
	this.bgColor=LowBgColor;
	this.resizeTo(Wdth,Hght);
	this.document.write(this.value);
	this.document.close();
	this.CmdLyr=new Layer(Wdth,MmbrCntnr);
	this.CmdLyr.visibility="inherit";
	this.CmdLyr.Level=CreateRecursLevel;
	this.CmdLyr.LinkTxt=eval(WhatMenu+"[1]");
	this.CmdLyr.onmouseover=OpenMenu;
	this.CmdLyr.onmouseout=CloseMenu;
	this.CmdLyr.captureEvents(Event.MOUSEUP);
	this.CmdLyr.onmouseup=GoTo;
	this.CmdLyr.LowLyr=this;
	this.CmdLyr.Container=MmbrCntnr;
	this.CmdLyr.resizeTo(Wdth,Hght);
	if(this.value.indexOf("<img")==-1 &&eval(WhatMenu+"[2]")){
		this.CmdLyr.ImgLyr=new Layer(10,this.CmdLyr);
		this.CmdLyr.ImgLyr.visibility="inherit";
		this.CmdLyr.ImgLyr.top=(CreateRecursLevel==1&&FirstLineHorizontal)?Hght-7:Hght/2-5;
		this.CmdLyr.ImgLyr.left=(CreateRecursLevel==1&&FirstLineHorizontal)?Wdth-12:Wdth-7;
		this.CmdLyr.ImgLyr.width=(CreateRecursLevel==1&&FirstLineHorizontal)?10:5;
		this.CmdLyr.ImgLyr.height=(CreateRecursLevel==1&&FirstLineHorizontal)?5:10;
		ImgStr=(CreateRecursLevel==1&&FirstLineHorizontal)?"<img src='tridown.gif'>":"<img src='tri.gif'>";
		//this.CmdLyr.ImgLyr.document.write(ImgStr);
		this.CmdLyr.ImgLyr.document.close()}}

function CreateMenuStructure(MenuName,NumberOf){
	CreateRecursLevel++;
	var i,NoOffSubs,Member;
	var PreviousMember=null;
	var WhichMenu=MenuName+"1";
	var MenuWidth=eval(WhichMenu+"[4]");
	var MenuHeight=eval(WhichMenu+"[3]");
	var Location=(CreateRecursLevel==1)?FirstLocation:SecLocation;
	if(DomYes){
		var MemberContainer=Location.document.createElement("div");
		MemberContainer.style.position="absolute";
		MemberContainer.style.visibility="hidden";
		Location.document.body.appendChild(MemberContainer)}
	else	if(Nav4) var MemberContainer=new Layer(MenuWidth,Location)
		else{	WhichMenu+="c";
			Location.document.body.insertAdjacentHTML("AfterBegin","<div id='"+WhichMenu+"' style='visibility:hidden; position:absolute;'><\/div>"); 
			var MemberContainer=Location.document.all[WhichMenu]}
	MemberContainer.SetUp=ContainerSetUp;
	MemberContainer.SetUp(MenuWidth,MenuHeight,NumberOf);
	if(Exp4){	MemberContainer.InnerString="";
		for(i=1;i<NumberOf+1;i++){
			WhichMenu=MenuName+eval(i);
			NoOffSubs=eval(WhichMenu+"[2]");
			MemberContainer.InnerString+="<div id='"+WhichMenu+"' style='position:absolute;'><\/div>"}
		MemberContainer.innerHTML=MemberContainer.InnerString}
	for(i=1;i<NumberOf+1;i++){
		WhichMenu=MenuName+eval(i);
		NoOffSubs=eval(WhichMenu+"[2]");
		if(DomYes){
			Member=Location.document.createElement("div");
			Member.style.position="absolute";
			Member.style.visibility="inherit";
			MemberContainer.appendChild(Member);
			Member.SetUp=MemberSetUp}
		else	if(Nav4){Member=new Layer(MenuWidth,MemberContainer);
			Member.SetUp=NavMemberSetUp}
		else{	Member=Location.document.all[WhichMenu];
			Member.SetUp=MemberSetUp}
		Member.SetUp(MemberContainer,PreviousMember,WhichMenu,MenuWidth,MenuHeight);
		if(NoOffSubs) Member.ChildCntnr=CreateMenuStructure(WhichMenu+"_",NoOffSubs);
		PreviousMember=Member}
	MemberContainer.FirstMember=Member;
	CreateRecursLevel--;
	return(MemberContainer)}

function CreateMenuStructureAgain(MenuName,NumberOf){
	var i,WhichMenu,NoOffSubs;
	var PreviousMember,Member=FirstContainer.FirstMember;
	CreateRecursLevel++;
	for(i=NumberOf;i>0;i--){
		WhichMenu=MenuName+eval(i);
		NoOffSubs=eval(WhichMenu+"[2]");
		PreviousMember=Member;
		if(NoOffSubs)Member.ChildCntnr=CreateMenuStructure(WhichMenu+"_",NoOffSubs);
		Member=Member.PrevMember}
	CreateRecursLevel--}
