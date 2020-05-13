// Copyright 2015-16 Stefan Heule
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "obsidian.h"


////////////////////////////////////////////
//// Default values for the configuration
////////////////////////////////////////////

// defaults are also in src/obsidian.c, src/js/pebble-js-app.js and config/js/preview.js
uint8_t config_color_outer_background = COLOR_FALLBACK(GColorDarkGrayARGB8, GColorBlackARGB8);
uint8_t config_color_inner_background = COLOR_FALLBACK(GColorWhiteARGB8, GColorWhiteARGB8);
uint8_t config_color_minute_hand = COLOR_FALLBACK(GColorBlackARGB8, GColorBlackARGB8);
uint8_t config_color_inner_minute_hand = COLOR_FALLBACK(GColorLightGrayARGB8, GColorBlackARGB8);
uint8_t config_color_hour_hand = COLOR_FALLBACK(GColorJaegerGreenARGB8, GColorBlackARGB8);
uint8_t config_color_inner_hour_hand = COLOR_FALLBACK(GColorLightGrayARGB8, GColorBlackARGB8);
uint8_t config_color_circle = COLOR_FALLBACK(GColorBlackARGB8, GColorBlackARGB8);
uint8_t config_color_ticks = COLOR_FALLBACK(GColorBlackARGB8, GColorBlackARGB8);
uint8_t config_color_day_of_week = COLOR_FALLBACK(GColorJaegerGreenARGB8, GColorBlackARGB8);
uint8_t config_color_date = COLOR_FALLBACK(GColorBlackARGB8, GColorBlackARGB8);
uint8_t config_battery_logo = 1;
uint8_t config_color_battery_logo = COLOR_FALLBACK(PBL_IF_ROUND_ELSE(GColorDarkGrayARGB8, GColorBlackARGB8),
                                                   GColorWhiteARGB8);
uint8_t config_color_battery_30 = COLOR_FALLBACK(PBL_IF_ROUND_ELSE(GColorYellowARGB8, GColorBlackARGB8),
                                                 GColorWhiteARGB8);
uint8_t config_color_battery_20 = COLOR_FALLBACK(PBL_IF_ROUND_ELSE(GColorOrangeARGB8, GColorBlackARGB8),
                                                 GColorWhiteARGB8);
uint8_t config_color_battery_10 = COLOR_FALLBACK(PBL_IF_ROUND_ELSE(GColorRedARGB8, GColorBlackARGB8),
                                                 GColorWhiteARGB8);
uint8_t config_color_battery_bg_30 = COLOR_FALLBACK(PBL_IF_ROUND_ELSE(GColorWhiteARGB8, GColorYellowARGB8),
                                                    GColorBlackARGB8);
uint8_t config_color_battery_bg_20 = COLOR_FALLBACK(PBL_IF_ROUND_ELSE(GColorWhiteARGB8, GColorOrangeARGB8),
                                                    GColorBlackARGB8);
uint8_t config_color_battery_bg_10 = COLOR_FALLBACK(PBL_IF_ROUND_ELSE(GColorWhiteARGB8, GColorRedARGB8),
                                                    GColorBlackARGB8);
uint8_t config_color_bluetooth_logo = COLOR_FALLBACK(GColorWhiteARGB8, GColorBlackARGB8);
uint8_t config_color_bluetooth_logo_2 = COLOR_FALLBACK(GColorBlackARGB8, GColorWhiteARGB8);
uint8_t config_bluetooth_logo = true;
uint8_t config_vibrate_disconnect = true;
uint8_t config_vibrate_reconnect = true;
uint8_t config_message_disconnect = true;
uint8_t config_message_reconnect = true;
uint8_t config_minute_ticks = 1;
uint8_t config_hour_ticks = 1;
uint8_t config_color_weather = COLOR_FALLBACK(GColorBlackARGB8, GColorBlackARGB8);
uint16_t config_weather_refresh = 30;
uint16_t config_weather_expiration = 3*60;
uint8_t config_square = true;
uint8_t config_seconds = 0;
uint8_t config_color_seconds = COLOR_FALLBACK(GColorJaegerGreenARGB8, GColorBlackARGB8);
uint8_t config_date_format = 0;


////////////////////////////////////////////
//// Global variables
////////////////////////////////////////////

/** A pointer to our window, for later deallocation. */
Window *window;

/** All layers */
Layer *layer_background;

/** Buffers for strings */
char buffer_1[30];
char buffer_2[30];

/** The center of the watch */
GPoint center;

/** The height and width of the watch */
int16_t height;
int16_t width;

#ifdef OBSIDIAN_SHOW_NUMBERS
/** Open Sans font. */
GFont font_open_sans;
#endif

/** Fonts. */
#ifdef PBL_COLOR
FFont* font_main;
FFont* font_weather;
#else
GFont font_main;
GFont font_main_big;
GFont font_weather;
#endif

/** Is the bluetooth popup current supposed to be shown? */
bool show_bluetooth_popup;

/** The timer for the bluetooth popup */
AppTimer *timer_bluetooth_popup;

/** The current weather information. */
Weather weather;

/** Is the JS runtime ready? */
bool js_ready;

/** A timer used to schedule weather updates. */
AppTimer * weather_request_timer;



////////////////////////////////////////////
//// Implementation
////////////////////////////////////////////

/*
#define LOG(fmt, args...) \
  do { \
    char buffer[80]; \
    snprintf(buffer, ARRAY_LENGTH(buffer), fmt, ## args); \
    graphics_context_set_text_color(ctx, COLOR_ACCENT); \
    graphics_context_set_fill_color(ctx, GColorWhite); \
    graphics_fill_rect(ctx, GRect(0, 0, 144, 60), 0, GCornerNone); \
    graphics_draw_text(ctx, buffer, font_system_18px_bold, GRect(5, 0, 144-2*5, 50), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); \
  } while (0)
#define LOG2(fmt, args...) \
  do { \
    char buffer[80]; \
    snprintf(buffer, ARRAY_LENGTH(buffer), fmt, ## args); \
    graphics_context_set_text_color(ctx, COLOR_ACCENT); \
    graphics_draw_text(ctx, buffer, font_system_18px_bold, GRect(5, 21, 144-2*5, 50), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); \
  } while (0)
#define LOG3(fmt, args...) \
  do { \
    char buffer[80]; \
    snprintf(buffer, ARRAY_LENGTH(buffer), fmt, ## args); \
    graphics_context_set_text_color(ctx, COLOR_ACCENT); \
    graphics_draw_text(ctx, buffer, font_system_18px_bold, GRect(5, 21+21, 144-2*5, 50), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); \
  } while (0)
*/

/**
 * Handler for time ticks.
 */
void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
    if (config_seconds == 0 || (tick_time->tm_sec == 0) || (tick_time->tm_sec % config_seconds == 0)) {
      layer_mark_dirty(layer_background);
    }
#ifdef DEBUG_ITER_COUNTER
    debug_iter += 1;
#endif
}

void timer_callback_bluetooth_popup(void *data) {
    show_bluetooth_popup = false;
    timer_bluetooth_popup = NULL;
    layer_mark_dirty(layer_background);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth change callback");
}

void handle_bluetooth(bool connected) {
    // redraw background (to turn on/off the logo)
    layer_mark_dirty(layer_background);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth change");

    bool show_popup = false;
    bool vibrate = false;
    if ((config_message_reconnect && connected) || (config_message_disconnect && !connected)) {
        show_popup = true;
    }
    if ((config_vibrate_reconnect && connected) || (config_vibrate_disconnect && !connected)) {
        vibrate = true;
    }

    // vibrate
    if (vibrate && !quiet_time_is_active()) {
        vibes_double_pulse();
    }

    // turn light on
    if (show_popup && !quiet_time_is_active()) {
        light_enable_interaction();
    }

    // show popup
    if (show_popup) {
        show_bluetooth_popup = true;
        if (timer_bluetooth_popup) {
            app_timer_reschedule(timer_bluetooth_popup, OBSIDIAN_BLUETOOTH_POPUP_MS);
        } else {
            timer_bluetooth_popup = app_timer_register(OBSIDIAN_BLUETOOTH_POPUP_MS, timer_callback_bluetooth_popup,
                                                       NULL);
        }
    }
}

// Add heart bitmap layer
// static uint32_t imageBackground, imageSteps, imageHrm, imageWatchBattery, imagePhoneBattery;
static uint32_t heart_image_id;
static GBitmap *heart_bitmap;
static BitmapLayer *heart_image_layer;
static TextLayer *heart_bpm_layer;

// static bool more_heart_rate_samples;
// more_heart_rate_samples = health_service_set_heart_rate_sample_period(1);

// static uint32_t heart_rate_sample;
// heart_rate_sample = 1;
// health_service_set_heart_rate_sample_period(heart_rate_sample);
// health_service_set_heart_rate_sample_period(heart_rate_sample);


static void draw_heart(Window *window) {
    heart_image_id = RESOURCE_ID_IMAGE_HEART_BLACK;
   	heart_bitmap = gbitmap_create_with_resource(heart_image_id);
    heart_image_layer = bitmap_layer_create(GRect(127, 142, 10, 10));
    bitmap_layer_set_bitmap(heart_image_layer, heart_bitmap);
    
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(heart_image_layer));
    
    // fetch heart rate bpm and write on layer
    static char heart_bpm_buffer[8];
    //snprintf(heart_bpm_buffer, sizeof(heart_bpm_buffer), "%d", (int)health_service_peek_current_value(HealthMetricRestingHeartRateBPM));
    // snprintf(heart_bpm_buffer, sizeof(heart_bpm_buffer), "%d", (int)health_service_peek_current_value(HealthMetricHeartRateRawBPM));
    snprintf(heart_bpm_buffer, sizeof(heart_bpm_buffer), "%d", (int)health_service_peek_current_value(HealthMetricHeartRateBPM));
    heart_bpm_layer = text_layer_create(GRect(70, 135, 53, 200));
    text_layer_set_background_color(heart_bpm_layer, GColorClear);
    text_layer_set_text_color(heart_bpm_layer, GColorBlack);
    text_layer_set_text(heart_bpm_layer, heart_bpm_buffer);
    text_layer_set_font(heart_bpm_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(heart_bpm_layer, GTextAlignmentRight);

  	layer_add_child(window_get_root_layer(window), text_layer_get_layer(heart_bpm_layer));
}

static uint32_t step_image_id;
static GBitmap *step_bitmap;
static BitmapLayer *step_image_layer;
static TextLayer *step_num_layer;

static void draw_step(Window *window) {
    // draw shoe
    step_image_id = RESOURCE_ID_IMAGE_SHOE_BLACK;
   	step_bitmap = gbitmap_create_with_resource(step_image_id);
    step_image_layer = bitmap_layer_create(GRect(7, 140, 15, 14));
    bitmap_layer_set_bitmap(step_image_layer, step_bitmap);
    
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(step_image_layer));
    
    // fetch steps number and write on layer
    static char step_num_buffer[8];
    snprintf(step_num_buffer, sizeof(step_num_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    step_num_layer = text_layer_create(GRect(23, 135, 62, 20));
    text_layer_set_background_color(step_num_layer, GColorClear);
    text_layer_set_text_color(step_num_layer, GColorBlack);
    text_layer_set_text(step_num_layer, step_num_buffer);
    text_layer_set_font(step_num_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(step_num_layer, GTextAlignmentLeft);
  	
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(step_num_layer));
}

static uint32_t phone_image_id;
static GBitmap *phone_bitmap;
static BitmapLayer *phone_image_layer;
// static TextLayer *phone_num_layer;

static void draw_phone(Window *window) {
    // draw phone
    phone_image_id = RESOURCE_ID_IMAGE_PHONE_BLACK;
   	phone_bitmap = gbitmap_create_with_resource(phone_image_id);
    phone_image_layer = bitmap_layer_create(GRect(7, 11, 10, 20));
    bitmap_layer_set_bitmap(phone_image_layer, phone_bitmap);
    
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(phone_image_layer));
    
//    // fetch steps number and write on layer
//    static char step_num_buffer[8];
//    snprintf(step_num_buffer, sizeof(step_num_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
//    step_num_layer = text_layer_create(GRect(23, 135, 62, 20));
//    text_layer_set_background_color(step_num_layer, GColorClear);
//    text_layer_set_text_color(step_num_layer, GColorBlack);
//    text_layer_set_text(step_num_layer, step_num_buffer);
//    text_layer_set_font(step_num_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
//    text_layer_set_text_alignment(step_num_layer, GTextAlignmentLeft);
//  	
//    layer_add_child(window_get_root_layer(window), text_layer_get_layer(step_num_layer));
}

static void health_handler(HealthEventType event, void *context) {
    if (event == HealthEventMovementUpdate) {
        draw_step(window);
        draw_heart(window);
    }
}

static void battery_handler(HealthEventType event, void *context) {
    if (event == HealthEventMovementUpdate) {
        draw_phone(window);
    }
}
/**
 * Window load callback.
 */
void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // create layer
    layer_background = layer_create(bounds);
    layer_set_update_proc(layer_background, background_update_proc);
    layer_add_child(window_layer, layer_background);
    
    // in window_load subscribe to health events
    if(health_service_events_subscribe(health_handler, NULL)) {
        // force initial steps display
        health_handler(HealthEventMovementUpdate, NULL);
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    }
    draw_phone(window);
    
    // load fonts
#ifdef PBL_COLOR
    font_main = ffont_create_from_resource(RESOURCE_ID_MAIN_FFONT);
    font_weather = ffont_create_from_resource(RESOURCE_ID_WEATHER_FFONT);
#else
    font_main = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    font_main_big = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    font_weather = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_NUPE_23));
#endif

    // initialize
    show_bluetooth_popup = false;
}

/**
 * Window unload callback.
 */
void window_unload(Window *window) {
    layer_destroy(layer_background);
#ifdef OBSIDIAN_SHOW_NUMBERS
    fonts_unload_custom_font(font_open_sans);
#endif
#ifdef PBL_COLOR
    ffont_destroy(font_main);
    ffont_destroy(font_weather);
#else
    fonts_unload_custom_font(font_weather);
#endif
}

void subscribe_tick(bool also_unsubscribe) {
    if (also_unsubscribe) {
        tick_timer_service_unsubscribe();
    }
    TimeUnits unit = MINUTE_UNIT;
    if (config_seconds != 0) {
      unit = SECOND_UNIT;
    }
#ifdef DEBUG_ITER_COUNTER
    unit = SECOND_UNIT;
#endif
    tick_timer_service_subscribe(unit, handle_second_tick);
}


//  /////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////// Health  from AnalogFit //////////////////////
//  /////////////////////////////////////////////////////////////////////////////
//  static void health_handler(HealthEventType event, void *context) {
//    static char s_value_buffer[8];
//    if (event == HealthEventMovementUpdate) {
//      // display the step count
//      snprintf(s_value_buffer, sizeof(s_value_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
//      text_layer_set_text(s_num_label, s_value_buffer);
//  						
//  		//Vibrate only once on steps goal reach
//  		if( atoi(settings.StepsGoal) > 0 &&
//  			 ((int)health_service_sum_today(HealthMetricStepCount) >= atoi(settings.StepsGoal)) && 
//  			 stepsGoalReached == false) {
//  			// Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
//  			static const uint32_t segments[] = { 200, 100, 100, 50, 100, 50, 700};
//  			VibePattern pat = {
//    			.durations = segments,
//    			.num_segments = ARRAY_LENGTH(segments),
//  			};
//  			vibes_enqueue_custom_pattern(pat);
//  			stepsGoalReached = true;
//  		}
//    }
//  	
//  	#if PBL_API_EXISTS(health_service_peek_current_value)
//      /** Display the Heart Rate **/
//      HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
//      static char s_hrm_buffer[8];
//      snprintf(s_hrm_buffer, sizeof(s_hrm_buffer), "%lu", (uint32_t) value);
//      text_layer_set_text(s_bpm_label, s_hrm_buffer);
//      layer_set_hidden(text_layer_get_layer(s_bpm_label), false);
//    #else
//      layer_set_hidden(text_layer_get_layer(s_bpm_label), true);
//    #endif
//  }


//  
// void subscribe_health(bool also_unsubscribe) {
//     if (also_unsubscribe) {
//         tick_timer_service_unsubscribe();
//     }
//     TimeUnits unit = MINUTE_UNIT;
//     if (config_seconds != 0) {
//       unit = SECOND_UNIT;
//     }
// #ifdef DEBUG_ITER_COUNTER
//     unit = SECOND_UNIT;
// #endif
//     tick_timer_service_subscribe(unit, handle_second_tick);
// }
// 
// // in window_load subscribe to health events
// if(health_service_events_subscribe(health_handler, NULL)) {
//     // force initial steps display
//     health_handler(HealthEventMovementUpdate, NULL);
// } else {
//     APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
// }
// 
/**
 * Initialization.
 */
void init() {
    read_config_all();

// some alternative themes (for screenshots)
#if defined(SCREENSHOT_ALT_THEME_1) && defined(PBL_COLOR)
    uint8_t accent_col = GColorRedARGB8;
#endif
#if defined(SCREENSHOT_ALT_THEME_2) && defined(PBL_COLOR)
    uint8_t accent_col = GColorBlueARGB8;
#endif
#if (defined(SCREENSHOT_ALT_THEME_1) || defined(SCREENSHOT_ALT_THEME_2)) && defined(PBL_COLOR)
    config_color_day_of_week = accent_col;
    config_color_hour_hand = accent_col;
#endif
#if defined(SCREENSHOT_ALT_THEME_3) && defined(PBL_COLOR)
    config_hour_ticks = 3;
    config_minute_ticks = 3;
    config_color_outer_background = GColorPurpleARGB8;
    config_color_inner_background = GColorPurpleARGB8;
    config_color_minute_hand = GColorBlackARGB8;
    config_color_hour_hand = GColorBlackARGB8;
    config_color_circle = GColorBlackARGB8;
    config_color_ticks = GColorBlackARGB8;
    config_color_day_of_week = GColorPurpleARGB8;
    config_color_date = GColorPurpleARGB8;
    config_battery_logo = 3;
    config_color_inner_minute_hand = GColorBlackARGB8;
    config_color_inner_hour_hand = GColorBlackARGB8;
#endif
#if defined(SCREENSHOT_ALT_THEME_4) && defined(PBL_COLOR)
    config_color_outer_background = GColorBlackARGB8;
    config_color_inner_background = GColorBlackARGB8;
    config_color_minute_hand = GColorWhiteARGB8;
    config_color_hour_hand = GColorBabyBlueEyesARGB8;
    config_color_circle = GColorBlackARGB8;
    config_color_ticks = GColorBlackARGB8;
    config_color_day_of_week = GColorBabyBlueEyesARGB8;
    config_color_date = GColorWhiteARGB8;
    config_battery_logo = 3;
    config_color_inner_minute_hand = GColorLightGrayARGB8;
    config_color_inner_hour_hand = config_color_inner_minute_hand;
#endif
#if defined(SCREENSHOT_ALT_THEME_5) && defined(PBL_COLOR)
    uint8_t col1 = GColorChromeYellowARGB8;
    uint8_t col2 = GColorVividCeruleanARGB8;
    config_hour_ticks = 3;
    config_minute_ticks = 3;
    config_color_outer_background = col2;
    config_color_inner_background = col2;
    config_color_minute_hand = col1;
    config_color_hour_hand = col1;
    config_color_circle = col2;
    config_color_ticks = col1;
    config_color_day_of_week = col2;
    config_color_date = col2;
    config_battery_logo = 3;
    config_color_inner_minute_hand = col1;
    config_color_inner_hour_hand = config_color_inner_minute_hand;
#endif
#if defined(SCREENSHOT_ALT_THEME_6) && defined(PBL_COLOR)
    config_hour_ticks = 1;
    config_minute_ticks = 2;
    config_color_outer_background = GColorWhiteARGB8;
    config_color_inner_background = GColorWhiteARGB8;
    config_color_minute_hand = GColorRedARGB8;
    config_color_hour_hand = GColorBlackARGB8;
    config_color_circle = GColorBlackARGB8;
    config_color_ticks = GColorBlackARGB8;
    config_color_day_of_week = GColorWhiteARGB8;
    config_color_date = GColorWhiteARGB8;
    config_battery_logo = 3;
    config_color_inner_minute_hand = GColorRedARGB8;
    config_color_inner_hour_hand = GColorBlackARGB8;
#endif
#if defined(SCREENSHOT_ALT_THEME_7) && defined(PBL_COLOR)
    config_hour_ticks = 1;
    config_minute_ticks = 2;
    config_color_outer_background = GColorWhiteARGB8;
    config_color_inner_background = GColorWhiteARGB8;
    config_color_minute_hand = GColorBlackARGB8;
    config_color_hour_hand = GColorBlackARGB8;
    config_color_circle = GColorBlackARGB8;
    config_color_ticks = GColorBlackARGB8;
    config_color_day_of_week = GColorWhiteARGB8;
    config_color_date = GColorBlackARGB8;
    config_battery_logo = 3;
    config_color_inner_minute_hand = GColorBlackARGB8;
    config_color_inner_hour_hand = GColorBlackARGB8;
#endif
#if defined(SCREENSHOT_ALT_THEME_8) && defined(PBL_COLOR)
    config_hour_ticks = 1;
    config_minute_ticks = 2;
    config_color_outer_background = GColorWhiteARGB8;
    config_color_inner_background = GColorIcterineARGB8;
    config_color_minute_hand = GColorBlueARGB8;
    config_color_hour_hand = GColorBlackARGB8;
    config_color_circle = GColorBlackARGB8;
    config_color_ticks = GColorBlackARGB8;
    config_color_day_of_week = GColorIcterineARGB8;
    config_color_date = GColorBlueARGB8;
    config_battery_logo = 3;
    config_color_inner_minute_hand = GColorBlueARGB8;
    config_color_inner_hour_hand = GColorBlackARGB8;
#endif
#if (defined(SCREENSHOT_ALT_THEME_1) || \
    defined(SCREENSHOT_ALT_THEME_2) || \
    defined(SCREENSHOT_ALT_THEME_3) || \
    defined(SCREENSHOT_ALT_THEME_4) || \
    defined(SCREENSHOT_ALT_THEME_5) || \
    defined(SCREENSHOT_ALT_THEME_6) || \
    defined(SCREENSHOT_ALT_THEME_7) || \
    defined(SCREENSHOT_ALT_THEME_8)) && defined(PBL_COLOR)
    config_color_weather = config_color_date;
#endif
#ifdef DEBUG_NO_BATTERY_ICON
    config_battery_logo = 3;
#endif
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
    });
    draw_heart(window);
    draw_step(window);
    window_stack_push(window, true);
    
    
    subscribe_tick(false);
    bluetooth_connection_service_subscribe(handle_bluetooth);

    app_message_open(OBSIDIAN_INBOX_SIZE, OBSIDIAN_OUTBOX_SIZE);
    app_message_register_inbox_received(inbox_received_handler);    
}

/**
 * De-initialisation.
 */
void deinit() {
    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();

    window_destroy(window);

    app_message_deregister_callbacks();
}

/**
 * Main entry point.
 */
int main() {
    init();
    app_event_loop();
    deinit();
}
