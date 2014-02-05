#include <pebble.h>

Window *my_window;
TextLayer *tl_cycle;
TextLayer *tl_cp;
TextLayer *tl_countdown;
TextLayer *tl_list;

#define START_TIME_SEC 1388530800
#define CYCLE_SEC 630000
#define CP_SEC 18000
#define BUF_SIZE 100

char buffer[BUF_SIZE];

static void update_time(struct tm *tick_time) {
	time_t t = mktime(tick_time);
	
	t = t - START_TIME_SEC;
	uint32_t cycle = t / CYCLE_SEC;
	//TODO calc year - decrement cycle
	uint32_t year = 2014;
	uint32_t cp = (t % START_TIME_SEC) / CP_SEC + 1;
	uint32_t countdown = CP_SEC - t % CP_SEC;
	uint32_t tmp = (year%100)*60 + cycle;

	//sprintf(buffer, "%lu.%2lu", year, cycle);
	strftime(buffer, BUF_SIZE, "20%M.%S", localtime((time_t*)&tmp));
	text_layer_set_text(tl_cycle, buffer);
	strftime(buffer, BUF_SIZE, "cp%S", localtime((time_t*)&cp));
	text_layer_set_text(tl_cp, buffer);
	struct tm *tms = localtime((time_t*)&countdown);
	strftime(buffer, BUF_SIZE, "T -%H:%M:%S", tms);
	text_layer_set_text(tl_countdown, buffer);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

void handle_init(void) {
	my_window = window_create();
	window_stack_push(my_window, true);
	
	GFont font_s = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
	GFont font_m = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
	GFont font_b = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
	Layer *root_layer = window_get_root_layer(my_window);	
	GRect frame = layer_get_frame(root_layer);
	tl_cycle = text_layer_create(GRect(0, 20, frame.size.w, 22));
	text_layer_set_font(tl_cycle, font_m);
	tl_cp = text_layer_create(GRect(0, 42, frame.size.w, 22));
	text_layer_set_font(tl_cp, font_m);
	tl_countdown = text_layer_create(GRect(0, 64, frame.size.w, 26));
	text_layer_set_font(tl_countdown, font_b);
	tl_list = text_layer_create(GRect(0, 90, frame.size.w, frame.size.h - 110));
	text_layer_set_font(tl_list, font_s);
	
	layer_add_child(root_layer, text_layer_get_layer(tl_cycle));	
	layer_add_child(root_layer, text_layer_get_layer(tl_cp));	
	layer_add_child(root_layer, text_layer_get_layer(tl_countdown));	
	layer_add_child(root_layer, text_layer_get_layer(tl_list));	
	tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
}

void handle_deinit(void) {
	  text_layer_destroy(tl_cycle);
	  text_layer_destroy(tl_cp);
	  text_layer_destroy(tl_countdown);
	  text_layer_destroy(tl_list);
	  window_destroy(my_window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}