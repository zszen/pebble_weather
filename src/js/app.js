// var Settings = null;
// var playerInfo=null;
// localStorage.setItem("username", "John");
// console.log( "username = " + localStorage.getItem("username"));

var config_cityName = "London";
var config_frequence = 0;
var config_smart = 1;
var config_weatherService = 0;
var config_baidu_weather={
	"晴":"01",
	"多云":"02",
	"阴":"03",
	"阵雨":"10",
	"雷阵雨":"09",
	"雷阵雨伴有冰雹":"09",
	"雨夹雪":"13",
	"小雨":"04",
	"中雨":"10",
	"大雨":"09",
	"暴雨":"09",
	"大暴雨":"09",
	"特大暴雨":"09",
	"阵雪":"13",
	"小雪":"04",
	"中雪":"13",
	"大雪":"13",
	"暴雪":"13",
	"雾":"50",
	"冻雨":"09",
	"沙尘暴":"50",
	"小到中雨":"09",
	"中到大雨":"09",
	"大到暴雨":"09",
	"暴雨到大暴雨":"09",
	"大暴雨到特大暴雨":"09",
	"小到中雪":"13",
	"中到大雪":"13",
	"大到暴雪":"13",
	"浮尘":"50",
	"扬沙":"50",
	"强沙尘暴":"50",
	"霾":"50",
	"无":"01"
};

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () 
  {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function weatherRequest(cityName) {
	console.log("cityName " + cityName);
	switch(config_weatherService){
		case 1://baidu
			//http://apistore.baidu.com/apiworks/servicedetail/112.html
			//data {"errNum":0,"errMsg":"success","retData":{"city":"\u5317\u4eac","pinyin":"beijing","citycode":"101010100","date":"16-06-19","time":"08:00","postCode":"100000","longitude":116.391,"latitude":39.904,"altitude":"33","weather":"\u591a\u4e91","temp":"35","l_tmp":"24","h_tmp":"35","WD":"\u65e0\u6301\u7eed\u98ce\u5411","WS":"\u5fae\u98ce(<10km\/h)","sunrise":"04:45","sunset":"19:46"}}
			var url = "http://apis.baidu.com/apistore/weatherservice/weather?citypinyin="+cityName+"&t=" + Math.random();
			var req = new XMLHttpRequest();
			req.open("GET", url, true);
			req.setRequestHeader("apikey", "83be998f52ac36084de254720df90064");
			req.send();
			req.onreadystatechange = function() {
				console.log("req.readyState " + req.readyState+" req.status "+req.status);
				if (req.readyState==4 && req.status==200){
					console.log(req.responseText);
					var json = JSON.parse(req.responseText);
					if(json===null)return;
					if(json.errNum!="0")return;
					if(json.retData===null)return;
					console.log(json.retData.weather);
					console.log(json.retData.temp);
					
					// Temperature in Kelvin requires adjustment
					var temperature = json.retData.temp;
					console.log("Temperature is " + temperature);

					// Conditions
					var conditions = "none";      
					console.log("Conditions are " + conditions);

					var city = json.retData.pinyin;
					console.log("City are " + city);

					var icon = config_baidu_weather[json.retData.weather];
					console.log("icon are " + icon);

					var weatherID = "none";
					console.log("weatherID " + weatherID);
					
					// Assemble dictionary using our keys
					var dataSend = {"TYPE":"weather",
									"DATA_SERVICE":config_weatherService,
									"DATA_TEMPERATURE":temperature,
									"DATA_CONDITIONS":conditions,
									"DATA_CITY":city,
									"DATA_ICON":icon,
									"DATA_WEAHTERID":weatherID};

					// Send to Pebble
					Pebble.sendAppMessage(dataSend, function(e) {
						console.log("Weather info sent to Pebble successfully!");
						},
						function(e) {
							console.log("Error sending weather info to Pebble!");
					});
				}
			};
			break;
		case 2://yahoo
		case 0://openweather
		default:
			//back data
			//{"coord":{"lon":116.22,"lat":40.22},"weather":[{"id":800,"main":"Clear","description":"clear sky","icon":"01n"}],"base":"cmc stations","main":{"temp":302.04,"pressure":997,"humidity":42,"temp_min":302.04,"temp_max":302.04},"wind":{"speed":0.76,"deg":10.5077},"rain":{},"clouds":{"all":0},"dt":1465998776,"sys":{"type":3,"id":121460,"message":0.0022,"country":"CN","sunrise":1465937100,"sunset":1465991197},"id":2038154,"name":"Changping","cod":200}
			//Construct URL
			//var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
			//pos.coords.latitude + "&lon=" + pos.coords.longitude;
			var url = "http://api.openweathermap.org/data/2.5/weather?q="+cityName+"&APPID=d69504445b4e11ea34900d3717b2267c";
			console.log("weather url is " + url);
			// Send request to OpenWeatherMap
			xhrRequest(url, 'GET', function(responseText) {
				//responseText contains a JSON object with weather info
				console.log("json is " + responseText);
				var json = JSON.parse(responseText);
				if(json===null)return;
				if(json.weather===null)return;

				// Temperature in Kelvin requires adjustment
				var temperature = Math.round(json.main.temp - 273.15);
				console.log("Temperature is " + temperature);

				// Conditions
				var conditions = json.weather[0].main;      
				console.log("Conditions are " + conditions);

				var city = json.name;
				console.log("City are " + city);

				var icon = json.weather[0].icon;
				console.log("icon are " + icon);

				var weatherID = json.weather[0].id;
				console.log("weatherID " + weatherID);

				// Assemble dictionary using our keys
				var dataSend = {"TYPE":"weather",
								"DATA_SERVICE":config_weatherService,
								"DATA_TEMPERATURE":temperature,
								"DATA_CONDITIONS":conditions,
								"DATA_CITY":city,
								"DATA_ICON":icon,
								"DATA_WEAHTERID":weatherID};

				// Send to Pebble
				Pebble.sendAppMessage(dataSend, function(e) {
					console.log("Weather info sent to Pebble successfully!");
					},
					function(e) {
						console.log("Error sending weather info to Pebble!");
				});
			});
			break;
	}
}

// function locationError(err) 
// {
//   console.log("Error requesting location!");
// }

// function getWeather() 
// {
//   navigator.geolocation.getCurrentPosition(
//     locationSuccess,
//     locationError,
//     {timeout: 15000, maximumAge: 60000}
//   );
// }
 
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
	function(e) {
		console.log("PebbleKit JS ready!");
// 		Settings = require('settings');
// 		playerInfo = Settings.data('playerInfo');
// 		if(playerInfo.weatherCityName===null){
// 			Settings.data('playerInfo', { weatherCityName: "london" });
// 		}
// 		// Get the initial weather
// // 	    getWeather();
// 		playerInfo = Settings.data('playerInfo');
// 		console.log("weather city : " + playerInfo.weatherCityName);
// 		weatherRequest(localCityName);
		
		var dataSend = {"TYPE":"ready"};
      // Send to Pebble
      Pebble.sendAppMessage(dataSend,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
// 					Pebble.setTimeOut("locationSuccess",1000);
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
// 					Pebble.setTimeOut("locationSuccess",1000);
        });
  }
);
 
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e)   {
    console.log("AppMessage received!");
//     for(var name in e.payload){
// 		console.log("obj: "+name+" val: "+e.payload[name]);
// 	}
	config_cityName = e.payload.CONFIG_CITYNAME;
	config_frequence = e.payload.CONFIG_FREQUENCE;
	config_smart = e.payload.CONFIG_SMART;
	config_weatherService = e.payload.CONFIG_WEATHERSERVICE;
	var config_type = e.payload.TYPE;
	console.log("js init:"+config_type+config_cityName+config_frequence+config_smart+config_weatherService);
	if(config_type=="weather"){
		weatherRequest(config_cityName);
	}
  }                     
);

Pebble.addEventListener('showConfiguration', function() {
	var url = 'http://23.105.195.65/pebble/weather_config.html?cityname='+config_cityName+"&frequence="+config_frequence+"&smart="+config_smart+"&weatherservice="+config_weatherService;
	console.log("config url: "+url);
	Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
	// Decode the user's preferences
	if(!e.response)return;
	var configData = JSON.parse(decodeURIComponent(e.response));
	
	//save
// 	Settings.data('playerInfo', { weatherCityName: configData.weather_location_name });
	
	// Send to the watchapp via AppMessage
	config_cityName = configData.CONFIG_CITYNAME;
	config_frequence = parseInt(configData.CONFIG_FREQUENCE);
	config_smart = parseInt(configData.CONFIG_SMART);
	config_weatherService = parseInt(configData.CONFIG_WEATHERSERVICE);
	console.log("config change : "+config_cityName+config_frequence+config_smart+config_weatherService);
	var dict = {"TYPE":"config",
				"CONFIG_CITYNAME": config_cityName,
				"CONFIG_FREQUENCE":config_frequence,
				"CONFIG_SMART":config_smart,
				"CONFIG_WEATHERSERVICE":config_weatherService};
	// Send to the watchapp
	Pebble.sendAppMessage(dict, function() {
		console.log('Config data sent successfully!');
	}, function(e) {
		console.log('Error sending config data!');
	});
});
												
												
												