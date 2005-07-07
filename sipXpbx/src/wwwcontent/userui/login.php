<?php
    // This form is called from the index page with the login credentials
    // lineid and password
    //require("../phplib/mailbox.inc");
	session_start();
    session_register("SESSION");
    // Create a Mailbox Object and add it to the session
    if ( !isset($SESSION) ) {
		//$mailbox = new Mailbox($lineid, "/main/product/sw/sipxchange/mediasrv/voicemail/webui/vmailstore/");
        // is this a new mailbox?  If so create the folders (hack code)
        //if ( count ($mailbox->getFolderInfo()) == 0 ) {
            //$mailbox->createFolder ("inbox");
            //$mailbox->createFolder ("saved");
            //$mailbox->createFolder ("deleted");
        //}
        // and register the session variable
    	$SESSION["mailbox"] = $mailbox;
		header("Location: frameset.php");
    } 
?>
