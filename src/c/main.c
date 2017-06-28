#include <pebble.h>
#include "weather.h"

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;
static TextLayer *s_weather_layer;
static GFont s_weather_font;
static TextLayer *s_battery_layer;
static GFont s_battery_font;
static TextLayer *s_sushi_layer;
static GFont s_sushi_font;
static BitmapLayer *space_image_layer;
static GBitmap *space_image;
ClaySettings settings;

// Initialize the default settings
static void prv_default_settings() {
  settings.WeatherIsFahrenheit= false;
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "load reached");
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  prv_update_display();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "save reached");
}

// Update the display elements
static void prv_update_display(){
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_uint8(iter, 0, 0);

  // Update weather
  app_message_outbox_send();
APP_LOG(APP_LOG_LEVEL_DEBUG, "display reached");
}



static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charge";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charge", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  space_image_layer = bitmap_layer_create(bounds);
  space_image = gbitmap_create_with_resource(RESOURCE_ID_SPACE);
  
  bitmap_layer_set_compositing_mode(space_image_layer, GCompOpAssign);
  bitmap_layer_set_bitmap(space_image_layer, space_image);
  bitmap_layer_set_alignment(space_image_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(space_image_layer));
  
  
  //TIME
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 56), bounds.size.w, 50));
  //font
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BLOG_38));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, COLOR_FALLBACK(GColorWhite, GColorBlack));
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  //WEATHER
  // Create temperature Layer
  s_weather_layer = text_layer_create(
  GRect(0, PBL_IF_ROUND_ELSE(127, 124), bounds.size.w, 25));
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BLOG_18));
  // Style the text
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_font(s_weather_layer, s_weather_font);
  //add layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  
  //BATTERY
  s_battery_font= fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LOEW_13));
  s_battery_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(25, 21), bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, COLOR_FALLBACK(GColorFolly,GColorWhite));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, s_battery_font);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "100%");
  battery_state_service_subscribe(handle_battery);
  handle_battery(battery_state_service_peek()); 
  //add child layer
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
  s_sushi_font= fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LOEW_15));
  s_sushi_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(9, 4), bounds.size.w, 34));
  text_layer_set_text_color(s_sushi_layer, GColorWhite);
  text_layer_set_background_color(s_sushi_layer, GColorClear);
  text_layer_set_font(s_sushi_layer, s_sushi_font);
  text_layer_set_text_alignment(s_sushi_layer, GTextAlignmentCenter);
  text_layer_set_text(s_sushi_layer, "M O O N");
  //add child
  layer_add_child(window_layer, text_layer_get_layer(s_sushi_layer));
  
  
  }


static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_sushi_layer);
  
  //unload font
  fonts_unload_custom_font(s_battery_font);
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_weather_font);
  fonts_unload_custom_font(s_sushi_font);
  battery_state_service_unsubscribe();
  
  //Image unload
  gbitmap_destroy(space_image);
  bitmap_layer_destroy(space_image_layer);
  
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if(tick_time->tm_min % 30 == 0) {
    
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_uint8(iter, 0, 0);

  // Send the message to update 
  app_message_outbox_send();
  }
  
  update_time();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  //listen for clay config change
  Tuple *unit = dict_find(iterator, MESSAGE_KEY_WeatherIsFahrenheit);
  if (unit) {
    //update display only on weather change
    settings.WeatherIsFahrenheit = unit->value->int32 == 1;
    prv_save_settings();
  }
  // Save the new settings to persistent storage
  APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox reached");
  
  
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  static char units_buffer[32];
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
  Tuple *units_tuple = dict_find(iterator, MESSAGE_KEY_WEATHERUNIT);
  
  // If all data is available, use it
  if(temp_tuple && conditions_tuple ) {
    //Check units of weather
    char *unit_tpye =  units_tuple->value->cstring;
    //convert to readable form
    snprintf(units_buffer, sizeof(units_buffer),"%s",unit_tpye);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "unit is %s", units_buffer);
    if(strcmp(units_buffer,"imperial")==0)
    {
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)temp_tuple->value->int32); 
         APP_LOG(APP_LOG_LEVEL_DEBUG, "temp is %s", temperature_buffer);
    }
    else
    {
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "temp is %s", temperature_buffer);
      
    }
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    
  }
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);


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
  prv_load_settings();
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  update_time();
  // Register callbacks
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

}


static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
