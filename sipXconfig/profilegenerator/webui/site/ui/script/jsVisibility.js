function checkVis()
{
    // get values from select boxes that will determine visibility rules
    var callsForward = document.getElementById( "PHONESET_FORWARD_UNCONDITIONAL" ).value;
    var callsNoAnswer = document.getElementById( "PHONESET_AVAILABLE_BEHAVIOR" ).value;
    var callsBusy = document.getElementById( "PHONESET_BUSY_BEHAVIOR" ).value;

    // get div elements that to hide / show
    var xp2022 = document.getElementById( "xp_2022" );
    var xp2021 = document.getElementById( "xp_2021" );
    var xp2007 = document.getElementById( "xp_2007" );
    var xp2020 = document.getElementById( "xp_2020" );

    // show if true, else hide
    if ( callsForward == "ENABLE" )
    {
        xp2022.style.display = '';
    }
    else
    {
        xp2022.style.display = "none";
    }

    if ( callsNoAnswer == "FORWARD_ON_NO_ANSWER" )
    {
        xp2021.style.display = '';
        xp2007.style.display = '';
    }
    else
    {
        xp2021.style.display = "none";
        xp2007.style.display = "none";
    }

    if ( callsBusy == "FORWARD" )
    {
        xp2020.style.display = '';
    }
    else
    {
        xp2020.style.display = "none";
    }
}
