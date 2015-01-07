#include <pebble.h>
#include <PDUtils.h>

Window *my_window;
TextLayer *tl_cycle;
TextLayer *tl_cp;
TextLayer *tl_countdown;
TextLayer *tl_list;
TextLayer *tl_conn_layer;
TextLayer *tl_batt_layer;
TextLayer *tl_realtime;
GBitmap *img_res;
BitmapLayer *bl_res;

#define START_TIME_SEC 0  //checkpoints actually can be done without special modulus - unix epoch is okay
#define CYCLE_SEC 630000
#define CP_SEC 18000
#define BUF_SIZE 30
#define SHOW_CP_NUM 3 // number of checkpoints displayed
#define CP_DATA_SIZE 6 // number of characters per checkpoint

char buffer[6][BUF_SIZE];

void handle_conn(bool connected) {
	if (connected) {
			text_layer_set_text(tl_conn_layer, "BT: OK");
      vibes_short_pulse();
	} else {
			text_layer_set_text(tl_conn_layer, "BT: LOST");
      vibes_long_pulse();
	}
}

void handle_batt(BatteryChargeState charge) {
  static char battstate[BUF_SIZE];
  snprintf(battstate, BUF_SIZE, "W: %d%% %s%s", charge.charge_percent, charge.is_charging?"C":"", charge.is_plugged?"P":"");
  text_layer_set_text(tl_batt_layer, battstate);
}

static void update_time(bool fullupdate) {

	time_t rt = time(NULL);
  uint32_t t = rt;
	uint32_t countdown = CP_SEC - t % CP_SEC;
	struct tm *tms;
  struct tm *acttime;

	
	if(!fullupdate){
		fullupdate = (rt % 3600 == 0);
	}
	
	if(fullupdate){
    uint32_t next = rt + countdown;
		tms = localtime((time_t*) &next);
		uint32_t year = tms->tm_year+1900; //we have our year
    uint32_t offset = ymd_to_scalar(year,1,1); //this is the beginning of the current year
		uint32_t cycle = t - offset / CYCLE_SEC;
    uint32_t cp = (t % CYCLE_SEC) / CP_SEC + 1;
        APP_LOG( APP_LOG_LEVEL_ERROR , "%lu: offset, %lu: cycle, %lu: checkpoint, %lu: year", offset, cycle, cp, year);

		snprintf(buffer[0], BUF_SIZE, "%lu.%02lu", year, cycle);
		snprintf(buffer[1], BUF_SIZE, "%02lu/35", cp);
		text_layer_set_text(tl_cycle, buffer[0]);
		text_layer_set_text(tl_cp, buffer[1]);
	

		int nc = tms->tm_hour;
		for(int i=0; i<SHOW_CP_NUM; ++i, nc = (nc+5) % 24){
			snprintf(buffer[3] + CP_DATA_SIZE*i, BUF_SIZE, "%02d:00,", nc); 
		}
		buffer[3][SHOW_CP_NUM * CP_DATA_SIZE - 1] = '\0';
		text_layer_set_text(tl_list, buffer[3]);
	}
	tms = localtime((time_t*)&countdown);
	strftime(buffer[2], BUF_SIZE, "%H:%M:%S", tms);
	text_layer_set_text(tl_countdown, buffer[2]);
  
  acttime = localtime((time_t*)&rt);
  strftime(buffer[5], BUF_SIZE, "%H:%M:%S", acttime);
  text_layer_set_text(tl_realtime, buffer[5]);
  
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_time(false);
}

void handle_init(void) {
	my_window = window_create();
	window_stack_push(my_window, true);
	
	GFont font_s = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
	GFont font_m = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
	GFont font_b = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
	Layer *root_layer = window_get_root_layer(my_window);	
	GRect frame = layer_get_frame(root_layer);
	
	window_set_background_color(my_window, GColorBlack);
	
	tl_cycle = text_layer_create(GRect(0, 0, 70, 26));
	text_layer_set_font(tl_cycle, font_m);	
	text_layer_set_text_alignment(tl_cycle, GTextAlignmentLeft);
	text_layer_set_background_color(tl_cycle, GColorBlack);
	text_layer_set_text_color(tl_cycle, GColorWhite);
	
	tl_cp = text_layer_create(GRect(70, 0, frame.size.w-70, 26));
	text_layer_set_font(tl_cp, font_m);
	text_layer_set_text_alignment(tl_cp, GTextAlignmentRight);
	text_layer_set_background_color(tl_cp, GColorBlack);
	text_layer_set_text_color(tl_cp, GColorWhite);
	

	tl_countdown = text_layer_create(GRect(0, 25, frame.size.w, 30));
	text_layer_set_font(tl_countdown, font_b);
	text_layer_set_text_alignment(tl_countdown, GTextAlignmentCenter);
	text_layer_set_background_color(tl_countdown, GColorBlack);
	text_layer_set_text_color(tl_countdown, GColorWhite);
	
	tl_list = text_layer_create(GRect(0, frame.size.h-25, frame.size.w, 20));
	text_layer_set_font(tl_list, font_s);
	text_layer_set_text_alignment(tl_list, GTextAlignmentCenter);
	text_layer_set_background_color(tl_list, GColorBlack);
	text_layer_set_text_color(tl_list, GColorWhite);
	
	img_res = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RES); //48x67
	bl_res = bitmap_layer_create(GRect(0, 60, 50, 70));
	bitmap_layer_set_bitmap(bl_res, img_res);
	bitmap_layer_set_alignment(bl_res, GAlignLeft);

  tl_realtime = text_layer_create(GRect(55, 60, frame.size.w-60, 20));
	text_layer_set_font(tl_realtime, font_s);
	text_layer_set_text_alignment(tl_realtime, GTextAlignmentRight);
	text_layer_set_background_color(tl_realtime, GColorBlack);
	text_layer_set_text_color(tl_realtime, GColorWhite);
  
	tl_conn_layer = text_layer_create(GRect(55, 80, frame.size.w-60, 20));
  text_layer_set_font(tl_conn_layer, font_s);
	text_layer_set_text_alignment(tl_conn_layer, GTextAlignmentRight);
	text_layer_set_background_color(tl_conn_layer, GColorBlack);
	text_layer_set_text_color(tl_conn_layer, GColorWhite);
	text_layer_set_text(tl_conn_layer, "BT: --");
	
  tl_batt_layer = text_layer_create(GRect(55, 100, frame.size.w-60, 20));
  text_layer_set_font(tl_batt_layer, font_s);
	text_layer_set_text_alignment(tl_batt_layer, GTextAlignmentRight);
	text_layer_set_background_color(tl_batt_layer, GColorBlack);
	text_layer_set_text_color(tl_batt_layer, GColorWhite);
	text_layer_set_text(tl_batt_layer, "BATT: --");
 	
  
	layer_add_child(root_layer, bitmap_layer_get_layer(bl_res));
	layer_add_child(root_layer, text_layer_get_layer(tl_cycle));	
	layer_add_child(root_layer, text_layer_get_layer(tl_cp));	
	layer_add_child(root_layer, text_layer_get_layer(tl_countdown));	
	layer_add_child(root_layer, text_layer_get_layer(tl_list));
	layer_add_child(root_layer, text_layer_get_layer(tl_conn_layer));
  layer_add_child(root_layer, text_layer_get_layer(tl_batt_layer));
  layer_add_child(root_layer, text_layer_get_layer(tl_realtime));
  
	tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
	bluetooth_connection_service_subscribe(handle_conn);
	handle_conn(bluetooth_connection_service_peek());
  battery_state_service_subscribe(handle_batt);
  handle_batt(battery_state_service_peek());
	update_time(true);
}


void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	text_layer_destroy(tl_cycle);
	text_layer_destroy(tl_cp);
	text_layer_destroy(tl_countdown);
	text_layer_destroy(tl_list);
	text_layer_destroy(tl_conn_layer);
  text_layer_destroy(tl_batt_layer);
	bitmap_layer_destroy(bl_res);
	gbitmap_destroy(img_res);
	window_destroy(my_window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
