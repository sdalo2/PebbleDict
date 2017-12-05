#include <pebble.h>
#define KEY_DEFINITION 0

static Window *s_main_window, *s_def_window;
static TextLayer *s_question_layer, *s_prompt_layer, *s_def_layer, *s_word_layer;
int i = 0;
static DictationSession *s_dictation_session;
// This is a scroll layer
static ScrollLayer *s_scroll_layer;

static TextLayer *s_defn;
//static TextLayer *s_last_text;
static char s_last_text_buffer[256];
static char s_defn_buffer[256];

static bool s_speaking_enabled;


static void sendMessage(char *last_text){
  DictionaryIterator* dictionaryIterator = NULL;
  app_message_outbox_begin(&dictionaryIterator);
  dict_write_cstring(dictionaryIterator,0,last_text);
  dict_write_end(dictionaryIterator);
  AppMessageResult result = app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_INFO, "Send resut: %d", (int)result);
}

static void next_def_handler(void *word) {
  //text_layer_set_text(s_question_layer, word);
  //text_layer_set_text(s_prompt_layer, "Press Select to search your word!");
  window_set_background_color(s_main_window, GColorVividCerulean );
  s_speaking_enabled = true;
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *tuple = dict_find(iterator, 0);
  if (!tuple) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "The tuple was missing!");
    return;
  }
  s_speaking_enabled = true;
  APP_LOG(APP_LOG_LEVEL_INFO, "Got message: %s", tuple->value->cstring);
  snprintf(s_defn_buffer, sizeof(s_defn_buffer), "%s", tuple->value->cstring);
  //where the word and def show up
  
  GSize size = graphics_text_layout_get_content_size(s_defn_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                                        GRect(0, 0, 134, 1000), GTextOverflowModeWordWrap, GTextAlignmentCenter);
  layer_set_frame((Layer *)s_def_layer, GRect(5, 40, 134, size.h));
  scroll_layer_set_content_size(s_scroll_layer, GSize(144, 50 + size.h));
  text_layer_set_text(s_word_layer, s_last_text_buffer); 
  text_layer_set_text(s_def_layer, s_defn_buffer);
  window_stack_push(s_def_window, true);
}


static void check_def(char *word) {
  //DictionaryIterator* dictionaryIterator = NULL;
  //get the def from JS
  snprintf(s_last_text_buffer, sizeof(s_last_text_buffer), "%s", word);
  sendMessage(s_last_text_buffer);
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    // Check for the def 
    check_def(transcription);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Transcription failed.\n\nError ID:\n%d", (int)status);
  }
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //s_speaking_enabled = true;
  if(s_speaking_enabled) {
    // Start voice dictation UI
    dictation_session_start(s_dictation_session);
    s_speaking_enabled = false;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
   GRect bounds = layer_get_bounds(window_layer);
  
  s_question_layer = text_layer_create(GRect(5, 5, bounds.size.w - 10, bounds.size.h));
  text_layer_set_font(s_question_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(s_question_layer, GColorWhite);
  text_layer_set_text_alignment(s_question_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_question_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_question_layer));
  
  s_prompt_layer = text_layer_create(GRect(5, 120, bounds.size.w - 10, bounds.size.h));
  text_layer_set_text(s_prompt_layer, "Press Select to search your word!");
  text_layer_set_font(s_prompt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_prompt_layer, GColorWhite);
  text_layer_set_text_alignment(s_prompt_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_prompt_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_prompt_layer));
 
}

static void window_load2(Window *window) {
  Layer *window_layer2 = window_get_root_layer(window);
  GRect bounds2 = layer_get_bounds(window_layer2);
  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(s_scroll_layer, s_def_window);
  layer_add_child(window_layer2, (Layer *)s_scroll_layer);
  
  //s_word_layer = text_layer_create(GRect(5, 5, bounds2.size.w - 10, bounds2.size.h));
  text_layer_set_font(s_word_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(s_word_layer, GColorWhite);
  text_layer_set_text_alignment(s_word_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_word_layer, GColorClear);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_word_layer));
  
  //s_def_layer = text_layer_create(GRect(5, 30, bounds2.size.w - 10, bounds2.size.h));
  text_layer_set_font(s_def_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_def_layer, GColorWhite);
  text_layer_set_text_alignment(s_def_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_def_layer, GColorClear);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_def_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_prompt_layer);
  text_layer_destroy(s_question_layer);
;}
static void window_unload2(Window *window) {
//   text_layer_destroy(s_def_layer);
//   text_layer_destroy(s_word_layer);
;}

static void init() {
  app_message_open(512, 512);
  app_message_register_inbox_received(inbox_received_callback);
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
  
  s_def_window = window_create();
  Layer *window_layer2 = window_get_root_layer(s_def_window);
  GRect bounds2 = layer_get_bounds(window_layer2);
  window_set_window_handlers(s_def_window, (WindowHandlers) {
    .load = window_load2,
    .unload = window_unload2,
  });
  
  window_set_click_config_provider(s_main_window, click_config_provider);

  // Initialize the scroll layer
  s_scroll_layer = scroll_layer_create(bounds2);
  s_word_layer = text_layer_create(GRect(5, 5, bounds2.size.w - 10, bounds2.size.h));
  s_def_layer = text_layer_create(GRect(5, 40, bounds2.size.w - 10, bounds2.size.h));
  
  

  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_last_text_buffer), 
                                                 dictation_session_callback, NULL);

  window_set_background_color(s_main_window, GColorVividCerulean);
  window_set_background_color(s_def_window, GColorVividCerulean);

  text_layer_set_text(s_question_layer, "PebbleDict");
  s_speaking_enabled = true;
  
  //init2();
  
}

static void deinit() {
  // Free the last session data
  dictation_session_destroy(s_dictation_session);

  window_destroy(s_main_window);
  window_destroy(s_def_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
