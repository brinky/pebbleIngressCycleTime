#include <pebble.h>

Window *my_window;
TextLayer *tl_cycle;
TextLayer *tl_cp;
TextLayer *tl_countdown;
TextLayer *tl_list;

#define START_TIME_SEC 1388523598
#define CYCLE_SEC 630000
#define CP_SEC 18000
#define BUF_SIZE 100

char buffer[10][BUF_SIZE];

static void update_time(struct tm *tick_time) {
	
	//time_t t = mktime(tick_time);
	
	time_t t = time(NULL);
	t = t - START_TIME_SEC;
	uint32_t cycle = t / CYCLE_SEC;
	//TODO calc year - decrement cycle
	uint32_t year = 2014;
	uint32_t cp = (t % CYCLE_SEC) / CP_SEC + 1;
	uint32_t countdown = CP_SEC - t % CP_SEC;
	uint32_t tmp = (year%100)*60 + cycle;

	strftime(buffer[0], BUF_SIZE, "20%M.%S ", localtime((time_t*)&tmp));
	strftime(buffer[1], BUF_SIZE, "%S/35", localtime((time_t*)&cp));
	text_layer_set_text(tl_cycle, buffer[0]);
	text_layer_set_text(tl_cp, buffer[1]);
	
	struct tm *tms = localtime((time_t*)&countdown);
	strftime(buffer[2], BUF_SIZE, "%H:%M:%S", tms);
	text_layer_set_text(tl_countdown, buffer[2]);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
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
	
	tl_cycle = text_layer_create(GRect(10, 10, 70, 26));
	text_layer_set_font(tl_cycle, font_m);	
	text_layer_set_text_alignment(tl_cycle, GTextAlignmentLeft);
	text_layer_set_background_color(tl_cycle, GColorBlack);
	text_layer_set_text_color(tl_cycle, GColorWhite);
	
	tl_cp = text_layer_create(GRect(10+70, 10, frame.size.w-20-70, 26));
	text_layer_set_font(tl_cp, font_m);
	text_layer_set_text_alignment(tl_cp, GTextAlignmentRight);
	text_layer_set_background_color(tl_cp, GColorBlack);
	text_layer_set_text_color(tl_cp, GColorWhite);
	

	tl_countdown = text_layer_create(GRect(10, 38, frame.size.w-20, 30));
	text_layer_set_font(tl_countdown, font_b);
	text_layer_set_text_alignment(tl_countdown, GTextAlignmentCenter);
	text_layer_set_background_color(tl_countdown, GColorBlack);
	text_layer_set_text_color(tl_countdown, GColorWhite);
	
	tl_list = text_layer_create(GRect(10, 70, frame.size.w-20, frame.size.h-64));
	text_layer_set_font(tl_list, font_s);
	text_layer_set_background_color(tl_list, GColorBlack);
	text_layer_set_text_color(tl_list, GColorWhite);
	
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