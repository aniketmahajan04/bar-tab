#include "gdk/gdk.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtkcssprovider.h"
#include "pango/pango-layout.h"
#include <asm-generic/errno.h>
#include <gtk-layer-shell.h>
#include <gtk/gtk.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define TODO_FILE_NAME ".wayland_todo_list"
#define MAX_TODO_FILE_BYTES (1024 * 1024)
#define MAX_TODO_TASKS 1000

typedef struct {
  GtkWidget *main_vbox;
  GtkWidget *entry;
  GtkWidget *popover;
  GtkWidget *placeholder_label;
} TodoAppData;

typedef struct {
  GtkWidget *box;
  GtkWidget *label;
  TodoAppData *app_data;
} TaskRowData;

static gchar *get_todo_file_path() {
  return g_build_filename(g_get_home_dir(), TODO_FILE_NAME, NULL);
}

// forward declaration so functions can trigger saves interchangeabley
static void save_task_to_disk(TodoAppData *app_data);

static void save_task_to_disk(TodoAppData *app_data) {
  gchar *file_path = get_todo_file_path();
  FILE *file = fopen(file_path, "w");
  g_free(file_path);

  if (!file) {
    g_printerr("Error: Could not open save file for writing.\n");
    return;
  }

  GList *children =
      gtk_container_get_children(GTK_CONTAINER(app_data->main_vbox));

  for (GList *iter = children; iter != NULL; iter = iter->next) {
    GtkWidget *row_box = GTK_WIDGET(iter->data);

    const gchar *widget_name = gtk_widget_get_name(row_box);

    if (g_strcmp0(widget_name, "header-box") == 0 ||
        row_box == app_data->placeholder_label) {
      continue;
    }

    // Inside each row_box, find the label (which holds the text)
    GList *row_elements = gtk_container_get_children(GTK_CONTAINER(row_box));
    if (row_elements) {
      GtkWidget *label = GTK_WIDGET(row_elements->data);
      if (GTK_IS_LABEL(label)) {
        const gchar *text = gtk_label_get_text(GTK_LABEL(label));

        // Only save active text (ignores markup formatting tags)
        if (text && text[0] != '<') {
          fprintf(file, "%s\n", text);
        }
      }
      g_list_free(row_elements);
    }
  }

  g_list_free(children);
  fclose(file);
}

void custom_css(void) {
  GtkCssProvider *provider = gtk_css_provider_new();

  // later add the styling to match my waybar
  const gchar *css_style = "* {\n"
                           "  font-family: \"JetBrainsMono Nerd Font\", \"Font "
                           "Awesome 6 Free\", sans-serif;\n"
                           "  font-size: 14px;\n"
                           "}\n"
                           "\n"
                           "window {\n"
                           "  background-color: transparent;\n"
                           "  border: none;\n"
                           "  box-shadow: none;\n"
                           "}\n"
                           "\n"
                           "decoration, decoration:backdrop {\n"
                           "  background-color: transparent;\n"
                           "  box-shadow: none;\n"
                           "  border-radius: 0;\n"
                           "}\n"
                           "\n"
                           ".app-container {\n"
                           "  background-color: #1e2024;\n"
                           "  border: 1px solid #2a2d36;\n"
                           "  border-radius: 18px;\n"
                           "  min-width: 380px;\n"
                           "  padding: 10px 20px;\n"
                           "}\n"
                           "\n"
                           "#header-box {\n"
                           "  background-color: #1e2024;\n"
                           "  border-radius: 12px;\n"
                           "  padding: 8px 12px;\n"
                           "  margin-bottom: 6px;\n"
                           "}\n"
                           "\n"
                           ".header-label {\n"
                           "  color: #565f89;\n"
                           "  font-weight: bold;\n"
                           "  font-size: 15px;\n"
                           "}\n"
                           "\n"
                           ".header-button {\n"
                           "  background-color: #2a2d36;\n"
                           "  color: #2a2d36;\n"
                           "  border-radius: 8px;\n"
                           "  border: none;\n"
                           "  padding: 4px;\n"
                           "  min-width: 0;\n"
                           "  min-height: 0;\n"
                           "  transition: all 0.2s ease;\n"
                           "}\n"
                           "\n"
                           ".header-button:hover {\n"
                           "  background-color: rgba(202, 197, 198, 0.09);\n"
                           "  color: #000000;\n"
                           "}\n"
                           "\n"
                           ".close-button {\n"
                           "  background-color: rgba(243, 139, 168, 0.12);\n"
                           "  border-radius: 8px;\n"
                           "  border: none;\n"
                           "  padding: 4px;\n"
                           "  min-width: 0;\n"
                           "  min-height: 0;\n"
                           "  transition: all 0.2s ease;\n"
                           "}\n"
                           "\n"
                           ".close-button:hover {\n"
                           "  color: #ff6467;"
                           "}\n"
                           "\n"
                           "label.placeholder {\n"
                           "  color: #565f89;\n"
                           "  font-style: italic;\n"
                           "  font-size: 13px;\n"
                           "  border-radius: 10px;\n"
                           "  padding: 10px;\n"
                           "}\n"
                           "\n"
                           ".task-row {\n"
                           "  background-color: #1e2024;\n"
                           "  border: 1px solid rgba(42, 45, 54, 0.6);\n"
                           "  border-radius: 10px;\n"
                           "  padding: 8px 12px;\n"
                           "  margin-bottom: 6px;\n"
                           "  transition: all 0.2s ease;\n"
                           "}\n"
                           "\n"
                           ".task-row:hover {\n"
                           "  background-color: #25272c;\n"
                           "}\n"
                           "\n"
                           ".task-label {\n"
                           "  color: #cdd6f4;\n"
                           "}\n"
                           "\n"
                           ".task-comp-btn {\n"
                           "  margin-left: 20px;\n"
                           "}\n"
                           "\n"
                           ".task-comp-btn, .task-edit-btn, .task-del-btn {\n"
                           "  background-color: #2a2d36;\n"
                           "  color: #0F171F;\n"
                           "  border-radius: 8px;\n"
                           "  border: none;\n"
                           "  padding: 4px;\n"
                           "  min-width: 0;\n"
                           "  min-height: 0;\n"
                           "  transition: all 0.15s ease;\n"
                           "}\n"
                           "\n"
                           ".task-comp-btn:hover {\n"
                           "  background-color: #a6e3a1;\n"
                           "  color: #5FAD56;\n"
                           "}\n"
                           "\n"
                           ".task-edit-btn:hover {\n"
                           "  background-color: #86b4fa;\n"
                           "  color: #89b4fa;\n"
                           "}\n"
                           "\n"
                           ".task-del-btn:hover {\n"
                           "  background-color: #f38ba8;\n"
                           "  color: #ff6467;\n"
                           "}\n"
                           "\n"
                           "entry {\n"
                           "  background-color: #1e2024;\n"
                           "  color: #cdd6f4;\n"
                           "  border-radius: 10px;\n"
                           "  border: 1px solid #2a2d36;\n"
                           "  padding: 2px 6px;\n"
                           "  caret-color: #f5c2e7;\n"
                           "}\n"
                           "\n"
                           "entry:focus {\n"
                           "  border-color: #565f89;\n"
                           "}\n"
                           "\n"
                           "entry placeholder {\n"
                           "  color: #585b70;\n"
                           "}\n"
                           "\n"
                           ".task-row entry.inline-edit {\n"
                           "  background-color: transparent;\n"
                           "  border: none;\n"
                           "  padding: 0;\n"
                           "  margin: 0;\n"
                           "}\n"
                           "\n"
                           "popover {\n"
                           "  background-color: transparent;\n"
                           "}\n"
                           "\n"
                           "popover > contents {\n"
                           "  background-color: #111215;\n"
                           "  border: 1px solid #2a2d36;\n"
                           "  border-radius: 6px;\n"
                           "  padding: 6px;\n"
                           "}\n";

  gtk_css_provider_load_from_data(provider, css_style, -1, NULL);

  gtk_style_context_add_provider_for_screen(
      gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void on_add_button_clicked(GtkWidget *button, gpointer user_data) {
  GtkWidget *popover = GTK_WIDGET(user_data);
  gtk_popover_popup(GTK_POPOVER(popover));
}

static void on_edit_done(GtkEntry *entry, gpointer user_data) {
  TaskRowData *row = (TaskRowData *)user_data;
  const gchar *new_text = gtk_entry_get_text(entry);

  if (g_strcmp0(new_text, "") != 0) {
    gtk_label_set_text(GTK_LABEL(row->label), new_text);
  }

  // Bring the label back into view
  gtk_widget_show(row->label);

  // Destroy the temporary enty box entirely
  gtk_widget_destroy(GTK_WIDGET(entry));

  // Trigger sync to system storage file directly here
  save_task_to_disk(row->app_data);
}

static void on_edit_clicked(GtkWidget *button, gpointer user_data) {
  TaskRowData *row = (TaskRowData *)user_data;

  if (!gtk_widget_get_visible(row->label))
    return;

  const gchar *current_text = gtk_label_get_text(GTK_LABEL(row->label));

  // create a temporary inline text from entry box
  GtkWidget *edit_entry = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(edit_entry),
                              "inline-edit");
  gtk_entry_set_text(GTK_ENTRY(edit_entry), current_text);

  // Hide the static text label
  gtk_widget_hide(row->label);

  // Pack the entry box at the very beginning of the horizontal task row
  gtk_box_pack_start(GTK_BOX(row->box), edit_entry, TRUE, TRUE, 0);

  // Move the entry box to index 0 so it stays on the far left!
  gtk_box_reorder_child(GTK_BOX(row->box), edit_entry, 0);

  gtk_widget_show(edit_entry);

  gtk_widget_grab_focus(edit_entry);

  g_signal_connect(edit_entry, "activate", G_CALLBACK(on_edit_done), row);
}

static void on_delete_clicked(GtkWidget *button, gpointer user_data) {
  TaskRowData *row_data = (TaskRowData *)user_data;
  TodoAppData *app_data = row_data->app_data;

  // 1. Destroy the horizontal layout row container
  gtk_container_remove(GTK_CONTAINER(app_data->main_vbox), row_data->box);

  save_task_to_disk(app_data);

  // 2. Safely free up the heap allocated struct data
  gtk_widget_destroy(row_data->box);
  g_free(row_data);

  // 3. Get all current children inside the vertical container
  GList *children =
      gtk_container_get_children(GTK_CONTAINER(app_data->main_vbox));

  // Note: header_box + placeholder_lable = 2 non-todo child elements
  // If count is 2, it means there are absolutely NO activate task left!
  if (g_list_length(children) <= 2) {
    gtk_widget_show(app_data->placeholder_label);
  }
  g_list_free(children);
}

static void on_complete_clicked(GtkWidget *button, gpointer user_data) {

  TaskRowData *row_data = (TaskRowData *)user_data;

  const gchar *current_text = gtk_label_get_text(GTK_LABEL(row_data->label));

  if (current_text[0] == '<') {
    return;
  }

  gchar *marked_up_text = g_strdup_printf(
      "<s><span foreground='#505a5c'>%s</span></s>", current_text);

  gtk_label_set_markup(GTK_LABEL(row_data->label), marked_up_text);

  g_free(marked_up_text);
}

static void on_task_entry_activated(GtkEntry *entry, gpointer user_data) {
  TodoAppData *data = (TodoAppData *)user_data;

  const gchar *text = gtk_entry_get_text(entry);

  if (g_strcmp0(text, "") != 0) {
    gtk_widget_hide(data->placeholder_label);

    GtkWidget *new_task_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_style_context_add_class(gtk_widget_get_style_context(new_task_hbox),
                                "task-row");
    GtkWidget *new_label = gtk_label_new(text);
    gtk_style_context_add_class(gtk_widget_get_style_context(new_label),
                                "task-label");

    gtk_label_set_ellipsize(GTK_LABEL(new_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(new_label), 24);
    GtkWidget *comp_btn = gtk_button_new_from_icon_name(
        "object-select-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *edit_btn = gtk_button_new_from_icon_name(
        "document-edit-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *delete_btn = gtk_button_new_from_icon_name(
        "edit-delete-symbolic", GTK_ICON_SIZE_BUTTON);

    gtk_style_context_add_class(gtk_widget_get_style_context(comp_btn),
                                "task-comp-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(edit_btn),
                                "task-edit-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(delete_btn),
                                "task-del-btn");

    TaskRowData *row_data = g_malloc(sizeof(TaskRowData));
    row_data->box = new_task_hbox;
    row_data->label = new_label;
    row_data->app_data = data; // Give it a link to check the master app state

    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_clicked),
                     row_data);

    g_signal_connect(edit_btn, "clicked", G_CALLBACK(on_edit_clicked),
                     row_data);
    g_signal_connect(comp_btn, "clicked", G_CALLBACK(on_complete_clicked),
                     row_data);

    gtk_box_pack_start(GTK_BOX(new_task_hbox), new_label, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(new_task_hbox), delete_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(new_task_hbox), edit_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(new_task_hbox), comp_btn, FALSE, FALSE, 0);

    // Packing the new row into our main vertical layout container
    gtk_box_pack_start(GTK_BOX(data->main_vbox), new_task_hbox, FALSE, FALSE,
                       0);

    // This will force the GTK to instantly draw the new elements on screen
    gtk_widget_show_all(new_task_hbox);

    // Clean up: empty the text box and hide the popover bubble
    gtk_entry_set_text(entry, "");
    gtk_popover_popdown(GTK_POPOVER(data->popover));

    // save to file by handing over the data
    save_task_to_disk(data);
  }
}

static void append_task_row_from_text(TodoAppData *data, const gchar *text) {
  gtk_widget_hide(data->placeholder_label);

  GtkWidget *new_task_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

  gtk_style_context_add_class(gtk_widget_get_style_context(new_task_hbox),
                              "task-row");

  GtkWidget *new_label = gtk_label_new(text);
  gtk_style_context_add_class(gtk_widget_get_style_context(new_label),
                              "task-label");
  gtk_label_set_xalign(GTK_LABEL(new_label), 0.0);
  gtk_label_set_ellipsize(GTK_LABEL(new_label), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars(GTK_LABEL(new_label), 24);

  GtkWidget *comp_btn = gtk_button_new_from_icon_name("object-select-symbolic",
                                                      GTK_ICON_SIZE_BUTTON);
  gtk_style_context_add_class(gtk_widget_get_style_context(comp_btn),
                              "task-comp-btn");

  GtkWidget *edit_btn = gtk_button_new_from_icon_name("document-edit-symbolic",
                                                      GTK_ICON_SIZE_BUTTON);
  gtk_style_context_add_class(gtk_widget_get_style_context(edit_btn),
                              "task-edit-btn");

  GtkWidget *delete_btn = gtk_button_new_from_icon_name("edit-delete-symbolic",
                                                        GTK_ICON_SIZE_BUTTON);
  gtk_style_context_add_class(gtk_widget_get_style_context(delete_btn),
                              "task-del-btn");

  TaskRowData *row_data = g_malloc(sizeof(TaskRowData));
  row_data->box = new_task_hbox;
  row_data->label = new_label;
  row_data->app_data = data;

  g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_clicked),
                   row_data);
  g_signal_connect(edit_btn, "clicked", G_CALLBACK(on_edit_clicked), row_data);
  g_signal_connect(comp_btn, "clicked", G_CALLBACK(on_complete_clicked),
                   row_data);

  gtk_box_pack_start(GTK_BOX(new_task_hbox), new_label, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(new_task_hbox), delete_btn, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(new_task_hbox), edit_btn, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(new_task_hbox), comp_btn, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(data->main_vbox), new_task_hbox, FALSE, FALSE, 0);

  gtk_widget_show_all(new_task_hbox);
}

static void load_tasks_from_disk(TodoAppData *app_data) {

  gchar *file_path = get_todo_file_path();

  if (g_file_test(file_path, G_FILE_TEST_EXISTS)) {
    GFile *todo_file = g_file_new_for_path(file_path);
    GError *error = NULL;
    GFileInfo *info =
        g_file_query_info(todo_file, G_FILE_ATTRIBUTE_STANDARD_SIZE,
                          G_FILE_QUERY_INFO_NONE, NULL, &error);
    g_object_unref(todo_file);

    if (!info) {
      g_printerr("Error: Could not read todo file metadata: %s\n",
                 error ? error->message : "unknown error");
      g_clear_error(&error);
      g_free(file_path);
      return;
    }

    guint64 file_size =
        g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_STANDARD_SIZE);
    g_object_unref(info);

    if (file_size > MAX_TODO_FILE_BYTES) {
      g_printerr("Error: Todo file is too large (%" G_GUINT64_FORMAT
                 " bytes). Skipping load; move or delete %s and restart.\n",
                 file_size, file_path);
      g_free(file_path);
      return;
    }
  }

  FILE *file = fopen(file_path, "r");
  g_free(file_path);

  if (!file)
    return;

  char line[256];
  guint loaded_tasks = 0;
  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = 0;

    if (g_strcmp0(line, "") != 0) {
      append_task_row_from_text(app_data, line);
      loaded_tasks++;

      if (loaded_tasks >= MAX_TODO_TASKS) {
        g_printerr("Warning: Loaded %u tasks (limit reached). Extra lines "
                   "ignored.\n",
                   MAX_TODO_TASKS);
        break;
      }
    }
  }

  fclose(file);
}

void build_ui(GtkWidget *window) {
  // Allocate memory for our custom data bundle
  // We make it 'static' so the data stays alive in system memory while the app
  // runs
  static TodoAppData data;

  GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_name(header_box, "header-box");

  GtkWidget *title_label = gtk_label_new("Todo");
  gtk_style_context_add_class(gtk_widget_get_style_context(title_label),
                              "header-label");
  gtk_widget_set_margin_end(title_label, 16);

  GtkWidget *add_button =
      gtk_button_new_from_icon_name("list-add-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_margin_start(add_button, 180);

  GtkWidget *closing_button = gtk_button_new_from_icon_name(
      "window-close-symbolic", GTK_ICON_SIZE_BUTTON);

  gtk_style_context_add_class(gtk_widget_get_style_context(add_button),
                              "header-button");
  gtk_style_context_add_class(gtk_widget_get_style_context(closing_button),
                              "close-button");

  g_signal_connect(closing_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

  gtk_box_pack_start(GTK_BOX(header_box), title_label, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(header_box), closing_button, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(header_box), add_button, FALSE, FALSE, 0);

  GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_style_context_add_class(gtk_widget_get_style_context(main_vbox),
                              "app-container");
  gtk_widget_set_margin_top(main_vbox, 16);
  gtk_widget_set_margin_bottom(main_vbox, 16);
  gtk_widget_set_margin_start(main_vbox, 16);
  gtk_widget_set_margin_end(main_vbox, 16);

  gtk_box_pack_start(GTK_BOX(main_vbox), header_box, FALSE, FALSE, 0);

  data.placeholder_label =
      gtk_label_new("✨ Clean slate! Create your first todo.");
  gtk_widget_set_margin_top(data.placeholder_label, 0);
  gtk_widget_set_margin_bottom(data.placeholder_label, 0);

  gtk_style_context_add_class(
      gtk_widget_get_style_context(data.placeholder_label), "placeholder");
  gtk_box_pack_start(GTK_BOX(main_vbox), data.placeholder_label, FALSE, FALSE,
                     0);

  GtkWidget *popover = gtk_popover_new(add_button);
  gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_BOTTOM);

  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Add a new task...");
  gtk_container_add(GTK_CONTAINER(popover), entry);
  gtk_widget_show(entry);

  data.main_vbox = main_vbox;
  data.entry = entry;
  data.popover = popover;

  // Connect the signals
  // Signal A: When '+' is clicked, call our popover display helper
  g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_button_clicked),
                   popover);

  // Signal B: When user hits 'Enter' inide text box, trigger row creation
  g_signal_connect(entry, "activate", G_CALLBACK(on_task_entry_activated),
                   &data);

  gtk_container_add(GTK_CONTAINER(window), main_vbox);

  load_tasks_from_disk(&data);

  gtk_widget_show_all(window);

  GList *children = gtk_container_get_children(GTK_CONTAINER(data.main_vbox));

  if (g_list_length(children) <= 2) {
    gtk_widget_show(data.placeholder_label);
  } else {
    gtk_widget_hide(data.placeholder_label);
  }

  g_list_free(children);
}

GtkWidget *create_window() {
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  GdkScreen *screen = gtk_widget_get_screen(window);
  GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
  if (visual != NULL) {
    gtk_widget_set_visual(window, visual);
    gtk_widget_set_app_paintable(window, TRUE);
  }

  gtk_layer_init_for_window(GTK_WINDOW(window));

  if (!gtk_layer_is_supported()) {
    g_printerr("layer-shell protocol not supported by compositor!\n");
  }

  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

  gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
  gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

  gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);

  gtk_layer_set_keyboard_interactivity(GTK_WINDOW(window), TRUE);

  gtk_widget_set_size_request(GTK_WIDGET(window), 400, 200);
  gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, 0);
  gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, 10);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit),
                   NULL);

  return window;
}

int main(int argc, char **argv) {

  // Initialize GTK engine
  gtk_init(&argc, &argv);

  // Inject the css stylesheet here
  custom_css();

  // Calling structural window renderer
  GtkWidget *window = create_window();

  // UI content rendere
  build_ui(window);

  // Instruct GTK to draw everything to the screen
  // gtk_widget_show_all(window);

  // Fire off the main event loop block
  gtk_main();

  return 0;
}
