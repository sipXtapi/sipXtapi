<?php
    // This form is called from the index page with the login credentials
    // lineid and password
    session_start();
    session_register("SESSION");
    // Create a Mailbox Object and add it to the session
    if ( !isset($SESSION) ) {
        header("Location: frameset.php");
    } 
?>
