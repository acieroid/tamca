/**
 * This program is licensed under the 2-clause BSD License, see the
 * LICENSE file
 */
#include <time.h>
#include <gtk/gtk.h>
#include <AL/alut.h>

static int POMODORO_TIME = 25*60;
static int PAUSE_TIME = 5*60;
static char *FILENAME = "ding.wav";
static char *FORMAT = "<span font=\"35\">%d:%.2d</span>";

static GOptionEntry options[] = {
  { "time", 't', 0, G_OPTION_ARG_INT, &POMODORO_TIME, "Time of a pomodoro (in seconds)", "N" },
  { "pause", 'p', 0, G_OPTION_ARG_INT, &PAUSE_TIME, "Time of a pause (in seconds)", "N" },
  { "sound", 's', 0, G_OPTION_ARG_STRING, &FILENAME, "File to use as sound", "PATH" },
  { "format", 'f', 0, G_OPTION_ARG_STRING, &FORMAT, "Format of the time", "FMT" }
};

struct {
  GtkWidget *window; /* window */
  GtkWidget *time_label; /* label */
  GtkWidget *start, *pause; /* buttons */
  GtkWidget *vbox, *hbox; /* boxes */
  ALuint sound_buffer, sound_source;
  gboolean started;
  int time;
  time_t last_update;
} gtamca;

/**
 * Convert a time in second into a markup string using FORMAT
 */
gchar *seconds_to_markup(int seconds)
{
  return g_markup_printf_escaped(FORMAT, seconds/60, seconds%60);
}

static void destroy(GtkWidget *widget, gpointer data)
{
  if (!alutExit()) {
    g_error("ALUT error: %s\n", alutGetErrorString(alutGetError()));
  }
  gtk_main_quit();
}

static void start_timer(GtkWidget *widget, gpointer data)
{
  int seconds = (int) data;
  gtk_label_set_markup(GTK_LABEL(gtamca.time_label),
                       seconds_to_markup(seconds));
  gtamca.started = TRUE;
  gtamca.time = seconds;
  gtamca.last_update = time(NULL);
}

static gboolean update_timer(gpointer data)
{
  time_t current_time;

  if (gtamca.time == 0)
    return TRUE;

  if (gtamca.started && time(NULL) - gtamca.last_update >= 1) {
    current_time = time(NULL);
    gtamca.time -= current_time - gtamca.last_update;
    gtamca.last_update = current_time;

    if (gtamca.time <= 0) {
      gtamca.time = 0;
      alSourcePlay(gtamca.sound_source);
    }

    gtk_label_set_markup(GTK_LABEL(gtamca.time_label),
                         seconds_to_markup(gtamca.time));
  }
  return TRUE;
}

int main(int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context;

  gtk_init(&argc, &argv);

  /* Parse options */
  context = g_option_context_new("- A pomodoro technique timer");
  g_option_context_add_main_entries(context, options, NULL);
  g_option_context_add_group(context, gtk_get_option_group(TRUE));
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_error("Can't parse options: %s", error->message);
    return 1;
  }

  if (!alutInit(&argc, argv)) {
    g_error("ALUT error: %s", alutGetErrorString(alutGetError()));
    return 1;
  }

  /* ALUT stuff */
  gtamca.sound_buffer = alutCreateBufferFromFile(FILENAME);
  alGenSources(1, &gtamca.sound_source);
  alSourcei(gtamca.sound_source, AL_BUFFER, gtamca.sound_buffer);

  /* GTK stuff */
  gtamca.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(gtamca.window, "destroy", G_CALLBACK(destroy), NULL);

  gtamca.time_label = gtk_label_new(NULL);
  gtamca.start = gtk_button_new_with_label("New pomodoro");
  gtamca.pause = gtk_button_new_with_label("Pause");
  gtamca.vbox = gtk_vbox_new(TRUE, 2);
  gtamca.hbox = gtk_hbox_new(FALSE, 2);

  gtk_label_set_markup(GTK_LABEL(gtamca.time_label), seconds_to_markup(0));

  g_signal_connect(gtamca.start, "clicked",
                   G_CALLBACK(start_timer), (gpointer) POMODORO_TIME);
  g_signal_connect(gtamca.pause, "clicked",
                   G_CALLBACK(start_timer), (gpointer) PAUSE_TIME);

  gtk_container_add(GTK_CONTAINER(gtamca.hbox), gtamca.start);
  gtk_container_add(GTK_CONTAINER(gtamca.hbox), gtamca.pause);
  gtk_container_add(GTK_CONTAINER(gtamca.vbox), gtamca.time_label);
  gtk_container_add(GTK_CONTAINER(gtamca.vbox), gtamca.hbox);
  gtk_container_add(GTK_CONTAINER(gtamca.window), gtamca.vbox);

  gtk_widget_show_all(gtamca.hbox);
  gtk_widget_show_all(gtamca.vbox);
  gtk_widget_show_all(gtamca.window);

  g_timeout_add(500, update_timer, NULL);

  gtk_main();

  return 0;
}
