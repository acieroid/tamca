#include <time.h>
#include <gtk/gtk.h>
#include <AL/alut.h>

static const int POMODORO_TIME = 25*60;
static const int PAUSE_TIME = 5*60;

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
 * Convert a time in second into a string (25*60 seconds -> "25:00")
 */
gchar *seconds_to_str(int seconds)
{
  /* TODO: size depend on the seconds */
  gchar *res = g_malloc(6 * sizeof res);
  g_snprintf(res, 6, "%d:%0.2d", seconds/60, seconds%60);
  return res;
}

static void destroy(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}

static void start_timer(GtkWidget *widget, gpointer data)
{
  int seconds = (int) data;
  gtk_label_set_text(GTK_LABEL(gtamca.time_label), seconds_to_str(seconds));
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

    gtk_label_set_text(GTK_LABEL(gtamca.time_label),
                       seconds_to_str(gtamca.time));
  }
  return TRUE;
}

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);
  alutInit(&argc, argv);

  /* ALUT stuff */
  gtamca.sound_buffer = alutCreateBufferHelloWorld();
  alGenSources(1, &gtamca.sound_source);
  alSourcei(gtamca.sound_source, AL_BUFFER, gtamca.sound_buffer);

  /* GTK stuff */
  gtamca.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(gtamca.window, "destroy", G_CALLBACK(destroy), NULL);

  gtamca.time_label = gtk_label_new(seconds_to_str(0));
  gtamca.start = gtk_button_new_with_label("New pomodoro");
  gtamca.pause = gtk_button_new_with_label("Pause");
  gtamca.vbox = gtk_vbox_new(TRUE, 2);
  gtamca.hbox = gtk_hbox_new(FALSE, 2);

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

  g_idle_add(update_timer, NULL);

  gtk_main();

  return 0;
}
