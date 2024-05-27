const BtnAdd = document.getElementById("set_btn");
const DivContainer = document.getElementById("network_select_buttons");

const wifi_auth_mode = Object.freeze({
  0: "OPEN",
  1: "WEP",
  2: "WPA_PSK",
  3: "WPA2_PSK",
  4: "WPA_WPA2_PSK",
  5: "Wifi EAP Security",
  6: "Wifi EAP Security",
  7: "WPA3_PSK",
  8: "WPA2_WPA3_PSK",
  9: "WAPI_PSK",
  10: "OWE",
  11: "WPA3_ENT_SUITE_B_192_BIT",
  12: "WPA3_PSK_EXT_KEY",
  13: "WPA3_PSK + WPA3_PSK_EXT_KEY",
  14: "MAX"
});

BtnAdd.addEventListener("click", AddNew);

/*!
 * Initialize functions here.
 */
$(document).ready(function () {
  getWifiNetworks();
});

/*!
* Add new button.
*/
function AddNew(ssid, authmode, rssi) {
  const newbtn = document.createElement("button");
  newbtn.classList.add("btn");
  newbtn.innerHTML = "SSID: " + ssid + " Authmode: " + authmode + " RSSI: " + rssi;
  newbtn.id = ssid;
  newbtn.onclick = function () {
    //console.log("Button Clicked");
    window.location.href = "password.html?ssid=" + encodeURIComponent(ssid);
  }
  console.log("add");
  DivContainer.appendChild(newbtn);
}

/*!
* Gets the wifi networks.
*/
function getWifiNetworks() {
  var xhr = new XMLHttpRequest();
  var requestURL = "/listofScannedWifiNetworks";
  xhr.open('POST', requestURL, false);
  xhr.send('listofScannedWifiNetworks');

  if (xhr.readyState == 4 && xhr.status == 200) {
    var response = JSON.parse(xhr.responseText);

    for (var i = 0; i < response.ap_count; i++) {
      AddNew(response.ap_records[i].ssid, getWifiSecurityType(response.ap_records[i].authmode), response.ap_records[i].rssi);
    }

    //AddNew(response.status, response.ap_count);
    //console.log(response);        
  }
}

/*!
* Gets the wifi networks list
*/
function getWifiNetworksList(list_size, responseTEXT) {

  var response = JSON.parse(responseText);

  console.log(response);



}

/*!
* Wifi Securtiy Type
*/
function getWifiSecurityType(authmode) {
  return wifi_auth_mode[authmode];
}

