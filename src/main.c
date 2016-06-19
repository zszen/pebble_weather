#include <pebble.h>
#include "ZApi.h"

typedef enum WEATHER_SERVICE{
	WEATHER_SERVICE_OPENWEATHER,
	WEATHER_SERVICE_BAIDUMAP,
	WEATHER_SERVICE_YAHOOWEATHER,
}WEATHER_SERVICE;

typedef enum WEATHER_TYPE{
	WEATHER_FOG = RESOURCE_ID_IMAGE_WEATHER_FOG,
	WEATHER_CLOUDY_LIGHT = RESOURCE_ID_IMAGE_WEATHER_CLOUDY_LIGHT,
	WEATHER_CLOUDY_HEAVY = RESOURCE_ID_IMAGE_WEATHER_CLOUDY_HEAVY,
	WEATHER_CLOUDY_NIGHT = RESOURCE_ID_IMAGE_WEATHER_CLOUDY_NIGHT,
	WEATHER_NIGHT = RESOURCE_ID_IMAGE_WEATHER_NIGHT,
	WEATHER_RAIN_NIGHT = RESOURCE_ID_IAMGE_WEATHER_RAIN_NIGHT,
	WEATHER_RAIN_HEAVY = RESOURCE_ID_IAMGE_WEATHER_RAIN_HEAVY,
	WEATHER_RAIN_LIGHT = RESOURCE_ID_IMAGE_WEATHER_RAIN_LIGHT,
	WEATHER_RAIN_THUNDER = RESOURCE_ID_IMAGE_WEATHER_RAIN_THUNDER,
	WEATHER_SNOW_HEAVY = RESOURCE_ID_IMAGE_WEATHER_SNOW_HEAVY,
	WEATHER_SNOW_LIGHT = RESOURCE_ID_IMAGE_WEATHER_SNOW_LIGHT,
	WEATHER_CLOUDY_SUNSHINE = RESOURCE_ID_IMAGE_WEATHER_CLOUDY_SUNSHINE,
	WEATHER_SUNSHINE = RESOURCE_ID_IMAGE_WEATHER_SUNSHINE,
	WEATHER_RAIN_SUNSHINE = RESOURCE_ID_IMAGE_WEATHER_RAIN_SUNSHINE,
	WEATHER_THUNDER = RESOURCE_ID_IMAGE_WEATHER_THUNDER,
	WEATHER_LOADING = RESOURCE_ID_IMAGE_LOADING,
	WEATHER_NONE = RESOURCE_ID_IMAGE_EMPTY,
}WEATHER_TYPE;

typedef enum SAVEKEY{
	SAVEKEY_CITYNAME,
	SAVEKEY_FREQUENCE,
	SAVEKEY_SMART,
	SAVEKEY_WEATHERTYPE,
	SAVEKEY_TIMEREQUEST,
	SAVEKEY_SERVICE,
}SAVEKEY;

typedef enum FREQUENCE{
	FREQUENCE_AUTO,
	FREQUENCE_15MIN,
	FREQUENCE_30MIN,
	FREQUENCE_1HOUR,
	FREQUENCE_2HOUR,
	FREQUENCE_4HOUR,
	FREQUENCE_6HOUR,
	FREQUENCE_12HOUR,
	FREQUENCE_1DAY,
	FREQUENCE_1WEEK,
	FREQUENCE_1MONTH,
	FREQUENCE_1YEAR,
	FREQUENCE_1CENTURY,
	FREQUENCE_NEVER,
}FREQUENCE;

static Window *mainWindow;
static GRect window_rect;
static GRect weather_rect;
static BitmapLayer *layer_weather;
static BitmapLayer *layer_loading;
static char config_cityName[256];
static FREQUENCE config_frequence;
static bool config_smartstart;
static time_t config_timeRequest;
static WEATHER_SERVICE config_weatherService;
static WEATHER_TYPE weatherTypeCurrent;
static bool configDetecting[6];
static Animation* animLoading;
static bool isParamSended;


static void motionLoadingStop(){
	if(animLoading!=NULL){
		animation_unschedule(animLoading);
		animLoading = NULL;
// 		Layer* layer = bitmap_layer_get_layer(layer_loading);
// 		GRect start = GRect(-20, 0, 20, 20);
// 		layer_set_frame(layer, start);
	}
}

static void motionLoadingStart(){
	motionLoadingStop();
	Layer* layer = bitmap_layer_get_layer(layer_loading);
	GRect start = GRect(-20, 0, 20, 20);
	GRect finish = GRect(window_rect.size.w/2-20/2, 0, 20, 20);
	PropertyAnimation* prop_anim = property_animation_create_layer_frame(layer, &start, &finish);
	animLoading = property_animation_get_animation(prop_anim);
	const int delay_ms = 500;
	const int duration_ms = 5000;
	animation_set_curve(animLoading, AnimationCurveEaseInOut);
	animation_set_delay(animLoading, delay_ms);
	animation_set_duration(animLoading, duration_ms);
	animation_schedule(animLoading);
}
static void motionLoadingComplete(){
	if(animLoading!=NULL){
		animation_unschedule(animLoading);
	}
	Layer* layer = bitmap_layer_get_layer(layer_loading);
	GRect start = layer_get_frame(layer);
	GRect finish = GRect(window_rect.size.w, 0, 20, 20);
	PropertyAnimation* prop_anim = property_animation_create_layer_frame(layer, &start, &finish);
	animLoading = property_animation_get_animation(prop_anim);
	const int delay_ms = 500;
	const int duration_ms = 3000;
	animation_set_curve(animLoading, AnimationCurveEaseInOut);
	animation_set_delay(animLoading, delay_ms);
	animation_set_duration(animLoading, duration_ms);
	animation_schedule(animLoading);
}

static void initParams(){
	layer_weather = NULL;
	animLoading = NULL;
	isParamSended = false;
	
	configDetecting[SAVEKEY_CITYNAME] = false;
	configDetecting[SAVEKEY_SMART] = false;
	configDetecting[SAVEKEY_FREQUENCE] = false;
	configDetecting[SAVEKEY_WEATHERTYPE] = false;
	configDetecting[SAVEKEY_TIMEREQUEST] = false;
	configDetecting[SAVEKEY_SERVICE] = false;
	
	if (persist_exists(SAVEKEY_WEATHERTYPE) == true) {
		weatherTypeCurrent = persist_read_int(SAVEKEY_WEATHERTYPE);
	}else{
		weatherTypeCurrent = WEATHER_NONE;
	}
	
	if (persist_exists(SAVEKEY_CITYNAME) == true) {
		persist_read_string(SAVEKEY_CITYNAME, config_cityName, sizeof(config_cityName));
	}else{
		strcpy(config_cityName, "London");
	}
	
	if (persist_exists(SAVEKEY_FREQUENCE) == true) {
		config_frequence = persist_read_int(SAVEKEY_FREQUENCE);
	}else{
		config_frequence = FREQUENCE_AUTO;
	}
	
	if (persist_exists(SAVEKEY_SMART) == true) {
		config_smartstart = persist_read_int(SAVEKEY_SMART)!=0;
	}else{
		config_smartstart = true;
	}
	
	if (persist_exists(SAVEKEY_TIMEREQUEST) == true) {
		config_timeRequest = persist_read_int(SAVEKEY_TIMEREQUEST);
	}else{
		config_timeRequest = time(NULL);
	}
	
	if (persist_exists(SAVEKEY_SERVICE) == true) {
		config_weatherService = persist_read_int(SAVEKEY_SERVICE);
	}else{
		config_weatherService = WEATHER_SERVICE_OPENWEATHER;
	}
	
	
// 	APP_LOG(APP_LOG_LEVEL_WARNING, "get %s %d %d", config_cityName,config_frequence,config_smartstart);
//     switch(weatherTypeCurrent){
// 	case WEATHER_SUNSHINE:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_SUNSHINE");
// 		break;
// 	case WEATHER_NIGHT:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_NIGHT");
// 		break;
// 	case WEATHER_CLOUDY_LIGHT:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_CLOUDY_LIGHT");
// 		break;
// 	case WEATHER_CLOUDY_SUNSHINE:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_CLOUDY_SUNSHINE");
// 		break;
// 	case WEATHER_CLOUDY_NIGHT:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_CLOUDY_NIGHT");
// 		break;
// 	case WEATHER_CLOUDY_HEAVY:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_CLOUDY_HEAVY");
// 		break;
// 	case WEATHER_FOG:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_FOG");
// 		break;
// 	case WEATHER_RAIN_LIGHT:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_RAIN_LIGHT");
// 		break;
// 	case WEATHER_RAIN_NIGHT:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_RAIN_NIGHT");
// 		break;
// 	case WEATHER_RAIN_SUNSHINE:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_RAIN_SUNSHINE");
// 		break;
// 	case WEATHER_SNOW_LIGHT:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_SNOW_LIGHT");
// 		break;
// 	case WEATHER_RAIN_HEAVY:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_RAIN_HEAVY");
// 		break;
// 	case WEATHER_RAIN_THUNDER:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_RAIN_THUNDER");
// 		break;
// 	case WEATHER_THUNDER:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_THUNDER");
// 		break;
// 	case WEATHER_SNOW_HEAVY:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_SNOW_HEAVY");
// 		break;
// 	case WEATHER_NONE:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_NONE");
// 		break;
// 	default:
// 		APP_LOG(APP_LOG_LEVEL_WARNING, "WEATHER_ERROR");
// 		break;
// 	}
	
	Layer *rootLayer = window_get_root_layer(mainWindow);
	layer_weather = ZAPI_ImageCreate(weatherTypeCurrent, weather_rect ,true);
	layer_add_child(rootLayer, bitmap_layer_get_layer(layer_weather));
	
	layer_loading = ZAPI_ImageCreate(WEATHER_LOADING, GRect(-20, 0, 20, 20), true);
	layer_add_child(rootLayer, bitmap_layer_get_layer(layer_loading));
	
}

static void saveParams(){
	if(configDetecting[SAVEKEY_SMART]){
		configDetecting[SAVEKEY_SMART] = false;
		persist_write_int(SAVEKEY_SMART, config_smartstart);
	}
	if(configDetecting[SAVEKEY_CITYNAME]){
		configDetecting[SAVEKEY_CITYNAME] = false;
		persist_write_string(SAVEKEY_CITYNAME, config_cityName);	
	}
	if(configDetecting[SAVEKEY_FREQUENCE]){
		configDetecting[SAVEKEY_FREQUENCE] = false;
		persist_write_int(SAVEKEY_FREQUENCE, config_frequence);
	}
	if(configDetecting[SAVEKEY_WEATHERTYPE]){
		configDetecting[SAVEKEY_WEATHERTYPE] = false;
		persist_write_int(SAVEKEY_WEATHERTYPE, weatherTypeCurrent);	
	}
	if(configDetecting[SAVEKEY_TIMEREQUEST]){
		configDetecting[SAVEKEY_TIMEREQUEST] = false;
		persist_write_int(SAVEKEY_TIMEREQUEST, config_timeRequest);	
	}
	if(configDetecting[SAVEKEY_SERVICE]){
		configDetecting[SAVEKEY_SERVICE] = false;
		persist_write_int(SAVEKEY_SERVICE, config_weatherService);	
	}
}

static void sendSignalForRequestWeather(){
	motionLoadingStart();
	isParamSended = true;
	APP_LOG(APP_LOG_LEVEL_WARNING, "request weather %s %d %d %d", config_cityName,config_frequence,config_smartstart, config_weatherService);
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
//     dict_write_uint8(iter, 0, 0);
	dict_write_cstring(iter, MESSAGE_KEY_CONFIG_CITYNAME, config_cityName);
	dict_write_int8(iter, MESSAGE_KEY_CONFIG_FREQUENCE, config_frequence);
	dict_write_int8(iter, MESSAGE_KEY_CONFIG_SMART, config_smartstart);
	dict_write_int8(iter, MESSAGE_KEY_CONFIG_WEATHERSERVICE, config_weatherService);
	dict_write_cstring(iter, MESSAGE_KEY_TYPE, "weather");
    // Send the message!
	app_message_outbox_send();
}

static void sendSignalForInit(){
	if(isParamSended){
		return;
	}
	isParamSended = true;
	APP_LOG(APP_LOG_LEVEL_WARNING, "request init %s %d %d %d", config_cityName,config_frequence,config_smartstart, config_weatherService);
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
//     dict_write_uint8(iter, 0, 0);
	dict_write_cstring(iter, MESSAGE_KEY_CONFIG_CITYNAME, config_cityName);
	dict_write_int8(iter, MESSAGE_KEY_CONFIG_FREQUENCE, config_frequence);
	dict_write_int8(iter, MESSAGE_KEY_CONFIG_SMART, config_smartstart);
	dict_write_int8(iter, MESSAGE_KEY_CONFIG_WEATHERSERVICE, config_weatherService);
	dict_write_cstring(iter, MESSAGE_KEY_TYPE, "init");
    // Send the message!
	app_message_outbox_send();
}

static void smartRequestWeather(){
	time_t timeNow = time(NULL);
	double timeDuring = timeNow - config_timeRequest;
	FREQUENCE frequenceTmp = config_frequence;
	if(config_frequence==FREQUENCE_AUTO){
		switch(weatherTypeCurrent){
		case WEATHER_SUNSHINE:
		case WEATHER_NIGHT:
			frequenceTmp = FREQUENCE_2HOUR;
			break;
		case WEATHER_CLOUDY_LIGHT:
		case WEATHER_CLOUDY_SUNSHINE:
		case WEATHER_CLOUDY_NIGHT:
			frequenceTmp = FREQUENCE_1HOUR;
			break;
		case WEATHER_CLOUDY_HEAVY:
		case WEATHER_FOG:
			frequenceTmp = FREQUENCE_30MIN;
			break;
		case WEATHER_RAIN_LIGHT:
		case WEATHER_RAIN_NIGHT:
		case WEATHER_RAIN_SUNSHINE:
		case WEATHER_SNOW_LIGHT:
			frequenceTmp = FREQUENCE_15MIN;
			break;
		case WEATHER_RAIN_HEAVY:
		case WEATHER_RAIN_THUNDER:
		case WEATHER_THUNDER:
		case WEATHER_SNOW_HEAVY:
			frequenceTmp = FREQUENCE_15MIN;
			break;
		default:
			break;
		}
	}
	bool isNeedRequest = weatherTypeCurrent==WEATHER_NONE;
	switch(frequenceTmp){
	case FREQUENCE_15MIN:
		if(timeDuring>600){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_30MIN:
		if(timeDuring>1500){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_1HOUR:
		if(timeDuring>3300){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_2HOUR:
		if(timeDuring>6900){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_4HOUR:
		if(timeDuring>14100){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_6HOUR:
		if(timeDuring>21300){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_12HOUR:
		if(timeDuring>42900){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_1DAY:
		if(timeDuring>86100){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_1WEEK:
		if(timeDuring>604500){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_1MONTH:
		if(timeDuring>2591700){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_1YEAR:
		if(timeDuring>31535700){
			isNeedRequest = true;
		}
		break;
	case FREQUENCE_1CENTURY:
		if(timeDuring>31535999700){
			isNeedRequest = true;
		}
		break;
	default:
		break;
	}
	if(isNeedRequest){
		sendSignalForRequestWeather();
	}else{
		APP_LOG(APP_LOG_LEVEL_WARNING, "don't need refresh wather");
		if(!isParamSended){
			sendSignalForInit();
		}
	}
}
 
static void mainWindow_load(Window *window) {
}
 
static void mainWindow_unload(Window *window) {
	bitmap_layer_destroy(layer_weather);
}
 
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if(tick_time->tm_min % 10 == 0) {
		smartRequestWeather();
	}
}
 
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	Tuple *t_type = dict_find(iterator, MESSAGE_KEY_TYPE);
	if(!t_type){
		return;
	}
	const char* strType = t_type->value->cstring;
	if(!strcmp(strType,"ready")){//first request;
		if(config_smartstart && weatherTypeCurrent!=WEATHER_NONE){
			smartRequestWeather();
		}else{
			sendSignalForRequestWeather();
		}
	}else if(!strcmp(strType,"config")){
		Tuple *t_cityName = dict_find(iterator, MESSAGE_KEY_CONFIG_CITYNAME);
		if(t_cityName){
			const char* strCityName = t_cityName->value->cstring;
			APP_LOG(APP_LOG_LEVEL_WARNING, "weather city %s",strCityName);
			strcpy(config_cityName, strCityName);
			configDetecting[SAVEKEY_CITYNAME]=true;	
		}
		Tuple *t_frequence = dict_find(iterator, MESSAGE_KEY_CONFIG_FREQUENCE);
		if(t_frequence){
			config_frequence = t_frequence->value->int8;
			APP_LOG(APP_LOG_LEVEL_WARNING, "config_frequence %d",config_frequence);
			configDetecting[SAVEKEY_FREQUENCE]=true;
		}
		Tuple *t_smart = dict_find(iterator, MESSAGE_KEY_CONFIG_SMART);
		if(t_smart){
			config_smartstart = t_smart->value->int8==1;
			APP_LOG(APP_LOG_LEVEL_WARNING, "config_smartstart %d",config_smartstart);
			configDetecting[SAVEKEY_SMART]=true;
		}
		Tuple *t_service = dict_find(iterator, MESSAGE_KEY_CONFIG_WEATHERSERVICE);
		if(t_service){
			config_weatherService = t_service->value->int8;
			APP_LOG(APP_LOG_LEVEL_WARNING, "config_weatherService %d",config_weatherService);
			configDetecting[SAVEKEY_SERVICE]=true;
		}
		{//request
			config_timeRequest = time(NULL);
			configDetecting[SAVEKEY_TIMEREQUEST]=true;
			sendSignalForRequestWeather();
		}
		{//save data
			saveParams();
		}
	}else if(!strcmp(strType,"weather")){
		{
			config_timeRequest = time(NULL);
			configDetecting[SAVEKEY_TIMEREQUEST] = true;
			saveParams();
		}
		motionLoadingComplete();
		Tuple *t_icon = dict_find(iterator, MESSAGE_KEY_DATA_ICON);
		Tuple *t_service = dict_find(iterator, MESSAGE_KEY_DATA_SERVICE);
		char iconBuffer[8];
		WEATHER_SERVICE service = t_service->value->int8;
		switch(config_weatherService){
		case WEATHER_SERVICE_BAIDUMAP:
			{
				time_t timeNow = time(NULL);
				tm* timeNowTM = localtime(&timeNow);
				const char* simbolDN;
				if(timeNowTM->tm_hour>6 && timeNowTM->tm_hour<19){
					simbolDN = "d";
				}else{
					simbolDN = "n";
				}
				const char* iconBf = t_icon->value->cstring;
				strcpy(iconBuffer, iconBf);
				strcat(iconBuffer, simbolDN);
			}
			break;
		default:
			strcpy(iconBuffer,t_icon->value->cstring);
			break;
		}
		APP_LOG(APP_LOG_LEVEL_WARNING, "iconBuffer %s t_service %d",iconBuffer,service);
		
		//http://openweathermap.org/weather-conditions
		WEATHER_TYPE weatherTypeNext = WEATHER_NONE;
		if (strcmp(iconBuffer, "01d") == 0){
			weatherTypeNext = WEATHER_SUNSHINE;
		}else if (strcmp(iconBuffer, "01n") == 0){
			weatherTypeNext = WEATHER_NIGHT;
		}else if (strcmp(iconBuffer, "02d") == 0){
			weatherTypeNext = WEATHER_CLOUDY_SUNSHINE;
		}else if (strcmp(iconBuffer, "02n") == 0){
			weatherTypeNext = WEATHER_CLOUDY_NIGHT;
		}else if (strcmp(iconBuffer, "03d") == 0 || strcmp(iconBuffer, "03n") == 0){
			weatherTypeNext = WEATHER_CLOUDY_LIGHT;
		}else if (strcmp(iconBuffer, "04d") == 0 || strcmp(iconBuffer, "04n") == 0){
			weatherTypeNext = WEATHER_CLOUDY_HEAVY;
		}else if (strcmp(iconBuffer, "09d") == 0 || strcmp(iconBuffer, "09n") == 0){
			weatherTypeNext = WEATHER_RAIN_HEAVY;
		}else if (strcmp(iconBuffer, "10d") == 0){
			weatherTypeNext = WEATHER_RAIN_SUNSHINE;
		}else if (strcmp(iconBuffer, "10n") == 0){
			weatherTypeNext = WEATHER_RAIN_NIGHT;
		}else if (strcmp(iconBuffer, "11d") == 0 || strcmp(iconBuffer, "11n") == 0){
			weatherTypeNext = WEATHER_THUNDER;
		}else if (strcmp(iconBuffer, "13d") == 0 || strcmp(iconBuffer, "13n") == 0){
			weatherTypeNext = WEATHER_SNOW_HEAVY;
		}else if (strcmp(iconBuffer, "50d") == 0 || strcmp(iconBuffer, "50n") == 0){
			weatherTypeNext = WEATHER_FOG;
		}else{
			APP_LOG(APP_LOG_LEVEL_ERROR, "unrecognized icon");
		}
		APP_LOG(APP_LOG_LEVEL_WARNING, "weatherTypeCurrent %d %d",weatherTypeCurrent, weatherTypeNext);
		if(weatherTypeCurrent!=weatherTypeNext){
			weatherTypeCurrent = weatherTypeNext;
			{//save data
				configDetecting[SAVEKEY_WEATHERTYPE]=true;
				saveParams();
			}
			ZAPI_ImageChange(layer_weather, weatherTypeCurrent);
		}
	}
}
 
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
 
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
 
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
static void init() {
  // Create main Window element and assign to pointer
  mainWindow = window_create();
	window_set_background_color(mainWindow, GColorBlack);
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(mainWindow, (WindowHandlers) {
    .load = mainWindow_load,
    .unload = mainWindow_unload
  });
	Layer* windowLayer = window_get_root_layer(mainWindow);
	window_rect = layer_get_bounds(windowLayer);
	GSize weather_size = GSize(127, 121);
	weather_rect = GRect((window_rect.size.w-weather_size.w)/2
											 , (window_rect.size.h-weather_size.h)/2
											 , weather_size.w
											 , weather_size.h);
 
  // Show the Window on the watch, with animated=true
  window_stack_push(mainWindow, true);
  
	initParams();
  // Register with TickTimerService
	switch(config_frequence){
	case FREQUENCE_AUTO:
	case FREQUENCE_15MIN:
	case FREQUENCE_30MIN:
		tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
		//tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
		break;
	case FREQUENCE_1HOUR:
	case FREQUENCE_2HOUR:
	case FREQUENCE_4HOUR:
	case FREQUENCE_6HOUR:
	case FREQUENCE_12HOUR:
		tick_timer_service_subscribe(DAY_UNIT, tick_handler);
		break;
	case FREQUENCE_1DAY:
	case FREQUENCE_1WEEK:
		tick_timer_service_subscribe(DAY_UNIT, tick_handler);
		break;
	case FREQUENCE_1MONTH:
		tick_timer_service_subscribe(MONTH_UNIT, tick_handler);
		break;
	case FREQUENCE_1YEAR:
	case FREQUENCE_1CENTURY:
		tick_timer_service_subscribe(YEAR_UNIT, tick_handler);
		break;
	case FREQUENCE_NEVER:
	default:
		//never update
		break;
	}
  
	
// 	https://raw.githubusercontent.com/zszen/PebbleFace_ZSimpleTime/master/config.html
	
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	//
}
 
static void deinit() {
  // Destroy Window
  window_destroy(mainWindow);
}
 
int main(void) {
  init();
  app_event_loop();
  deinit();
}