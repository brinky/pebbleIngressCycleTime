#include <pebble.h>

Window *my_window;
TextLayer *tl_cycle;
TextLayer *tl_cp;
TextLayer *tl_countdown;
TextLayer *tl_list;
GBitmap *img_res;
BitmapLayer *bl_res;

#define START_TIME_SEC 1388523600
#define CYCLE_SEC 630000
#define CP_SEC 18000
#define BUF_SIZE 30
#define SHOW_CP_NUM 6

char buffer[4][BUF_SIZE];

static void update_time(bool fullupdate) {
	time_t rt = time(NULL);
	uint32_t t = rt - START_TIME_SEC;
	uint32_t countdown = CP_SEC - t % CP_SEC;
	struct tm *tms;
	
	if(!fullupdate){
		fullupdate = (rt % 3600 == 0);
	}
	
	if(fullupdate){
		uint32_t cycle = t / CYCLE_SEC;
		//TODO calc year - decrement cycle
		uint32_t year = 2014;
		uint32_t cp = (t % CYCLE_SEC) / CP_SEC + 1;
		snprintf(buffer[0], BUF_SIZE, "%lu.%02lu", year, cycle);
		snprintf(buffer[1], BUF_SIZE, "%02lu/35", cp);
		text_layer_set_text(tl_cycle, buffer[0]);
		text_layer_set_text(tl_cp, buffer[1]);
	
		uint32_t next = rt + countdown;
		tms = localtime((time_t*) &next);
		int nc = tms->tm_hour;
		for(int i=0; i<SHOW_CP_NUM; ++i, nc = (nc+5) % 24){
			snprintf(buffer[3] + 4*i, BUF_SIZE, "%02d: ", nc);
		}
		buffer[3][SHOW_CP_NUM * 4 - 1] = '\0';
		text_layer_set_text(tl_list, buffer[3]);
	}
	tms = localtime((time_t*)&countdown);
	strftime(buffer[2], BUF_SIZE, "%H:%M:%S", tms);
	text_layer_set_text(tl_countdown, buffer[2]);
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
	

	tl_countdown = text_layer_create(GRect(0, 28, frame.size.w, 30));
	text_layer_set_font(tl_countdown, font_b);
	text_layer_set_text_alignment(tl_countdown, GTextAlignmentCenter);
	text_layer_set_background_color(tl_countdown, GColorBlack);
	text_layer_set_text_color(tl_countdown, GColorWhite);
	
	tl_list = text_layer_create(GRect(0, frame.size.h-20, frame.size.w, 20));
	text_layer_set_font(tl_list, font_s);
	text_layer_set_text_alignment(tl_list, GTextAlignmentCenter);
	text_layer_set_background_color(tl_list, GColorBlack);
	text_layer_set_text_color(tl_list, GColorWhite);
	
	img_res = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RES);
	bl_res = bitmap_layer_create(GRect(0, 64, frame.size.w, frame.size.h-88));
	bitmap_layer_set_bitmap(bl_res, img_res);
	bitmap_layer_set_alignment(bl_res, GAlignCenter);
	
	layer_add_child(root_layer, bitmap_layer_get_layer(bl_res));
	layer_add_child(root_layer, text_layer_get_layer(tl_cycle));	
	layer_add_child(root_layer, text_layer_get_layer(tl_cp));	
	layer_add_child(root_layer, text_layer_get_layer(tl_countdown));	
	layer_add_child(root_layer, text_layer_get_layer(tl_list));
	
	tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
	update_time(true);
}

void handle_deinit(void) {
	text_layer_destroy(tl_cycle);
	text_layer_destroy(tl_cp);
	text_layer_destroy(tl_countdown);
	text_layer_destroy(tl_list);
	bitmap_layer_destroy(bl_res);
	gbitmap_destroy(img_res);
	window_destroy(my_window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}