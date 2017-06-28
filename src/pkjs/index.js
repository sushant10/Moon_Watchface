var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};
// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);

function unitsToString(unit) {
  if (unit) {
    return "imperial";
  }
  return "metric";
}

function locationSuccess(pos) {
  // Construct URL
  var settings = {};
  var units = "metric";
  try {
    settings = JSON.parse(localStorage.getItem('clay-settings')) || {};
    units = unitsToString(settings.WeatherIsFahrenheit);
  } catch (e) {}
  var myAPIKey= '*****************';// key made private
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&units='+ units +'&appid=' + myAPIKey;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      //... no longer requires adjustment
     
      var temperature = Math.round(json.main.temp);
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log('Conditions are ' + conditions);
      // Assemble dictionary using our keys
      console.log('Unit is '+ units);
      console.log('Temperature is ' + temperature);
    var dictionary = {
      "TEMPERATURE": temperature,
      "CONDITIONS": conditions,
      "WEATHERUNIT": units
    };

    // Send to Pebble
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log("Weather info sent to Pebble successfully!");
      },
      function(e) {
        console.log("Error sending weather info to Pebble!");
      }
    );
      
    }      
  );
  
}


function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial weather
    getWeather();
  }
);
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
function(e) {
    console.log("AppMessage received!");
    getWeather();
  } 
);

