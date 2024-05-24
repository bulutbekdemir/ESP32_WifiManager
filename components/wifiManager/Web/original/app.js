/**
 * Add gobals here
 */
var seconds 	= null;
var wifiConnectInterval = null;

/**
 * Initialize functions here.
 */
$(document).ready(function(){
    getWifiNetworks();
    /*
    $("#connect_wifi").on("click", function(){
        checkCredentials();
    });*/ 
});  

/*!
* Gets the wifi networks.
*/
function getWifiNetworks()
{
    var xhr = new XMLHttpRequest();
    var requestURL = "/scannedWifiNetworks";
    xhr.open('POST', requestURL, false);
    xhr.send('scannedWifiNetworks');

    if(xhr.readyState == 4 && xhr.status == 200)
    {
        var response = JSON.parse(xhr.responseText);
       
        document.getElementById("wifi_connect_status").innerHTML = "<h4 class='rd'>Status:" + response.ap_count + "</h4>";
        document.getElementById("wifi_connect_status").innerHTML = "<h4 class='rd'>Scanned:" + response.ap_count + "</h4>";
          
    }
}

/*!
* Gets wifi connection status.
*/
function getWifiConnectStatus()
{
    var xhr = new XMLHttpRequest();
    var requestURL = "/wifiConnectStatus";
    xhr.open('POST', requestURL, false);
    xhr.send('wifi_connect_status');

    if(xhr.readyState == 4 && xhr.status == 200)
    {
        var response = JSON.parse(xhr.responseText);

        document.getElementById("wifi_connect_status").innerHTML = "Connecting...";
        if(response.wifi_connect_status == 1)
        {
            document.getElementById("wifi_connect_status").innerHTML = "<h4 class='rd'>Connection Failed!</h4>";
            stopWifiStatusInterval();
        }
        else if(response.wifi_connect_status == 2)
        {
            document.getElementById("wifi_connect_status").innerHTML = "<h4 class='gr'>Connected!</h4>";
            stopWifiStatusInterval();
        }
    }
}

/*!
* Stops the wifi connection status.
*/
function stopWifiStatusInterval()
{
    if(wifiConnectInterval != null)
    {
        clearInterval(wifiConnectInterval);
        wifiConnectInterval = null;
    }   
}

/*!
* Starts interval check for wifi connection status.
*/
function startWifiStatusInterval()
{   
    wifiConnectInterval = setInterval(getWifiConnectStatus, 2000);
}

/*!
* Starts the wifi connection status.
*/
function connectWifi()
{
    ///> Get the selected SSID and password
    var selectedSSID = $("#connect_ssid").val();
    var writedPassword = $("#connect_password").val();

    $.ajax({
        url: "/connectWifi.json",
        dataType: "json",
        type: "POST",
        cache: false,
        headers: {
            "ConnectSSID": selectedSSID, 
            "ConnectPassword": writedPassword
        },
        data: {
            "time": Date.now()
        },
    });

    ///> Start the interval for checking the wifi connection status
    startWifiStatusInterval();
}

/*!
 * Checks the wifi credentials.
 */
function checkCredentials()
{
    errorList = "";
    credsOK = true;

    selectedSSID = $("#connect_ssid").val();
    writedPassword = $("#connect_password").val();

    if (selectedSSID == "")
    {
        errorList += "<h4 class='rd'> SSID is required and can NOT be empty! </h4>";
        credsOK = false;
    }
    if(writedPassword == "")
    {
        errorList += "<h4 class='rd'> Password is required and can NOT be \
                        empty for Auth req networks </h4>"; //! TODO: Check if the network is open and make conecting without password
        credsOK = false;
    }
    if(!credsOK)
    {
        $("#Connect_Credentials_Errors").html(errorList);
    }
    else
    {
        $("#Connect_Credentials_Errors").html("");
        connectWifi();
    }
}

function showPassword()
{
    var x = document.getElementById("connect_password");
    if (x.type === "password") 
    {
        x.type = "text";
    } 
    else 
    {
        x.type = "password";
    }
}
