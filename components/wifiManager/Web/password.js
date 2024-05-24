// Get the current URL
const uri = window.location.href;

// Create a new URL object with the URI
const url = new URL(uri);

// Get the "ssid" and "authmode" component values from the URL
const searchParams = url.searchParams;
const ssid = searchParams.get("ssid");

// Print the values to the console
console.log("SSID:", ssid);

$("#connect_wifi").on("click", function(){
	connectWifi();
});

/*!
* Starts the wifi connection status.
*/
function connectWifi()
{
    ///> Get the selected SSID and password
    var selectedSSID = ssid;
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

/*!
* Starts interval check for wifi connection status.
*/
function startWifiStatusInterval()
{   
    wifiConnectInterval = setInterval(getWifiConnectStatus, 2000);
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
