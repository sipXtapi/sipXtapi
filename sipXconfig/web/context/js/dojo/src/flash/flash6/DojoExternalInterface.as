/** 
		An implementation of Flash 8's ExternalInterface that works with Flash 6
		and which is source-compatible with Flash 8. 
		
		@author Brad Neuberg, bkn3@columbia.edu 
*/

class DojoExternalInterface{
	public static var available:Boolean;
	private static var callbacks = new Object();
	private static var _movieLoaded = false, _gatewayLoaded = false,
								 _loadedFired = false;
	
	public static function initialize(){ 
		//getURL("javascript:alert('FLASH:DojoExternalInterface initialize')");
		// FIXME: Set available variable by testing for capabilities
		DojoExternalInterface.available = true;
		// FIXME: do a test run to see if we can communicate from Flash to JavaScript
		// and back again to make sure we can actually communicate (set 'available'
		// variable)
		initializeFlashRunner();
	}
	
	public static function addCallback(methodName:String, instance:Object, 
										 								 method:Function) : Boolean{
		// A variable that indicates whether the call below succeeded
		_root._succeeded = null;
		
		// Callbacks are registered with the JavaScript side as follows.
		// On the Flash side, we maintain a lookup table that associates
		// the methodName with the actual instance and method that are
		// associated with this method.
		// Using fscommand, we send over the action "addCallback", with the
		// argument being the methodName to add, such as "foobar".
		// The JavaScript takes these values and registers the existence of
		// this callback point.
		
		// precede the method name with a _ character in case it starts
		// with a number
		callbacks["_" + methodName] = {_instance: instance, _method: method};
		fscommand("addCallback", methodName);
		
		// The API for ExternalInterface says we have to make sure the call
		// succeeded; check to see if there is a value 
		// for _succeeded, which is set by the JavaScript side
		if(_root._succeeded == null){
			return false;
		}else{
			return true;
		}
	}
	
	public static function call(methodName:String, 
															resultsCallback:Function) : Void{
		// FIXME: support full JSON serialization
		
		// First, we pack up all of the arguments to this call and set them
		// as Flash variables, which the JavaScript side will unpack using
		// plugin.GetVariable(). We set the number of arguments as "_numArgs",
		// and add each argument as a variable, such as "_1", "_2", etc., starting
		// from 0.
		// We then execute an fscommand with the action "call" and the
		// argument being the method name. JavaScript takes the method name,
		// retrieves the arguments using GetVariable, executes the method,
		// and then places the return result in a Flash variable
		// named "_returnResult".
		_root._numArgs = arguments.length - 2;
		for(var i = 2; i < arguments.length; i++){
			var argIndex = i - 2;
			_root["_" + argIndex] = arguments[i];
		}
		
		_root._returnResult = undefined;
		fscommand("call", methodName);
		
		// immediately return if the caller is not waiting for return results
		if(resultsCallback == undefined || resultsCallback == null){
			return;
		}
		
		// check at regular intervals for return results	
		var resultsChecker = function(){
			if(_root._returnResult != undefined){
				clearInterval(_root._callbackID);
				resultsCallback.call(null, _root._returnResult);
			}
		};	
		_root._callbackID = setInterval(resultsChecker, 100);
	}
	
	/** 
			Called by Flash to indicate to JavaScript that we are ready to have
			our Flash functions called. Calling loaded()
			will fire the dojo.flash.loaded() event, so that JavaScript can know that
			Flash has finished loading and adding its callbacks, and can begin to
			interact with the Flash file.
	*/
	public static function loaded(){
		_movieLoaded = true;
		if(_movieLoaded == true && _gatewayLoaded == true && _loadedFired == false){
			_loadedFired = true;
			call("dojo.flash.loaded");
		}
	}
	
	/** 
			Handles and executes a JavaScript to Flash method call. Used by
			initializeFlashRunner. 
	*/
	public static function _handleJSCall(){
		// get our parameters
		var numArgs = parseInt(_root._numArgs);
		var jsArgs = new Array();
		for(var i = 0; i < numArgs; i++){
			var currentValue = _root["_" + i];
			jsArgs.push(currentValue);
		}
		
		// get our function name
		var functionName = _root._functionName;
		
		// now get the actual instance and method object to execute on,
		// using our lookup table that was constructed by calls to
		// addCallback on initialization
		var instance = callbacks["_" + functionName]._instance;
		var method = callbacks["_" + functionName]._method;
		
		// execute it
		var results = method.apply(instance, jsArgs);
		
		// return the results
		_root._returnResult = results;
	}
	
	/** Called by the flash6_gateway.swf to indicate that it is loaded. */
	public static function _gatewayReady(){
		_gatewayLoaded = true;
		if(_movieLoaded == true && _gatewayLoaded == true && _loadedFired == false){
			_loadedFired = true;
			call("dojo.flash.loaded");
		}
	}
	
	/** 
			When JavaScript wants to communicate with Flash it simply sets
			the Flash variable "_execute" to true; this method creates the
			internal Movie Clip, called the Flash Runner, that makes this
			magic happen.
	*/
	private static function initializeFlashRunner(){
		// figure out where our Flash movie is
		var swfLoc = "../..";
		if(swfLoc.charAt(swfLoc.length - 1) != '/'){
			swfLoc = swfLoc + "/";
		}
		swfLoc = swfLoc + "flash6_gateway.swf";
		
		// load our gateway helper file
		_root.createEmptyMovieClip("_flashRunner", 5000);
		_root._flashRunner._lockroot = true;
		_root._flashRunner.loadMovie(swfLoc);
	}
}

// vim:ts=4:noet:tw=0:
