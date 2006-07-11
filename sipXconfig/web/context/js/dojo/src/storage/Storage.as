import DojoExternalInterface;

class Storage {
	public static var SUCCESS = "success";
	public static var FAILED = "failed";
	public static var PENDING = "pending";
	
	public var so;
	
	public function Storage(){
		DojoExternalInterface.initialize();
		DojoExternalInterface.addCallback("put", this, put);
		DojoExternalInterface.addCallback("get", this, get);
		DojoExternalInterface.addCallback("showSettings", this, showSettings);
		DojoExternalInterface.addCallback("clear", this, clear);
		DojoExternalInterface.addCallback("getKeys", this, getKeys);
		DojoExternalInterface.addCallback("remove", this, remove);
		DojoExternalInterface.loaded();
	}

	public function put(keyName, keyValue, namespace){
		// Get the SharedObject for these values and save it
		so = SharedObject.getLocal(namespace);
		
		// prepare a storage status handler
		var self = this;
		so.onStatus = function(infoObject:Object){
			//getURL("javascript:dojo.debug('FLASH: onStatus, infoObject="+infoObject.code+"')");
			
			// delete the data value if the request was denied
			if (infoObject.code == "SharedObject.Flush.Failed"){
				delete self.so.data[keyName];
			}
			
			var statusResults;
			if(infoObject.code == "SharedObject.Flush.Failed"){
				statusResults = Storage.FAILED;
			}else if(infoObject.code == "SharedObject.Flush.Pending"){
				statusResults = Storage.PENDING;
			}else if(infoObject.code == "SharedObject.Flush.Success"){
				statusResults = Storage.SUCCESS;
			}
			//getURL("javascript:dojo.debug('FLASH: onStatus, statusResults="+statusResults+"')");
			
			// give the status results to JavaScript
			DojoExternalInterface.call("dojo.storage._onStatus", null, statusResults, 
																 keyName);
		}
		
		// save the key and value
		so.data[keyName] = keyValue;
		var flushResults = so.flush();
		
		// return results of this command to JavaScript
		var statusResults;
		if(flushResults == true){
			statusResults = Storage.SUCCESS;
		}else if(flushResults == "pending"){
			statusResults = Storage.PENDING;
		}else{
			statusResults = Storage.FAILED;
		}
		
		//getURL("javascript:dojo.debug('FLASH: flat, statusResults="+statusResults+"')");
		DojoExternalInterface.call("dojo.storage._onStatus", null, statusResults, 
															 keyName);
	}

	public function get(keyName, namespace){
		// Get the SharedObject for these values and save it
		so = SharedObject.getLocal(namespace);
		var results = so.data[keyName];
		
		return results;
	}
	
	public function showSettings(){
		// Show the configuration options for the Flash player, opened to the
		// section for local storage controls
		System.showSettings(1);
		
		// there is no way we can intercept when the Close button is pressed, allowing us
		// to hide the Flash dialog. Instead, we have an image behind that tells the user
		// to press the Close button again when they are finished. We do this on a slight
		// timeout so that it appears _after_ the storage settings have appeared, otherwise
		// we will get a flicker effect as the user sees the underlying image a little before
		// the settings dialog appears
		var showCloseButton = function(){
			// TODO: Draw a simple box and button that tells the user to press
			// the button when they are finished with the settings dialog
		};
		_root.setTimeout(showCloseButton, 400);
	}
	
	public function clear(namespace){
		so = SharedObject.getLocal(namespace);
		so.clear();
		so.flush();
	}
	
	public function getKeys(namespace){
		//getURL("javascript:dojo.debug('FLASH:getKeys, namespace="+namespace+"')");
		// Returns a list of the available keys in this namespace
		
		// get the storage object
		so = SharedObject.getLocal(namespace);
		
		// get all of the keys
		var results = new Array();
		for(var i in so.data)
			results.push(i);	
		
		// join the keys together in a comma seperate string
		results = results.join(",");
		
		return results;
	}
	
	public function remove(keyName, namespace){
		// Removes a key

		// get the storage object
		so = SharedObject.getLocal(namespace);
		
		// delete this value
		delete so.data[keyName];
		
		// save the changes
		so.flush();
	}

	static function main(mc){
		/* Very rarely, on Internet Explorer, a timing issue will
		 * cause the Flash file to load before the ActiveX infrastructure
		 * has loaded, squelching our callbacks into JavaScript. Put
		 * initializing the Storage system on a very slight timeout
		 * to avoid this problem.
		 * 
		 * TODO: Move this timeout into dojo.flash somehow.
		 */
		_global.setTimeout(function(){ 
			var app = new Storage(); 
		}, 20);
	}
}

