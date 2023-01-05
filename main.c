#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "sound.h"
#include "scoremgr.h"

static GtkWidget *rightbar;
static GtkWidget *leftbar;
static gboolean sounds = TRUE;
static ScoreMgr *smgr;
static Sound *sound;
static Prompt* prompt;
static Table* table;
static GtkWidget* window;

void on_ball_moved(GObject*, gpointer);

static void rand_balls()
{
	GList *empty = find_empty(table->model);
	int n_empty = g_list_length(empty);
	_list_free(empty);

	int to_rand = MIN(n_step_balls, n_empty);

	Color new[n_step_balls];
	ZEROAT(&new);
	for(int i = 0; i<to_rand; i++)
		new[i] = rand()%(ColorMax-1) + 1;
	prompt_set(prompt, new);
}

static void add_to_score(int n_balls)
{
	static const int length2score [] = {10,12,18,28,42};
	int index = CLAMP(n_balls - 5, 0, G_N_ELEMENTS(length2score)-1);
	int highest = smgr->scores[0].score;
	int newval = gtk_progress_get_value(GTK_PROGRESS(rightbar)) + length2score[index];
	if(newval < highest)
		gtk_progress_set_value(GTK_PROGRESS(rightbar), newval);
	else{
		gtk_progress_configure(GTK_PROGRESS(leftbar), highest, 0, newval);
		gtk_progress_configure(GTK_PROGRESS(rightbar), newval, 0, newval);
	}
}

static gboolean check_lines()
{
	GList *lines = find_lines(table->model);
	if(lines){
		add_to_score(g_list_length(lines));
		table_disappear(table, lines);
	}
	
	return lines != NULL;
}

void on_init_game()
{
	//clear table
	memset(table->model, 0, sizeof(table->model));	

	//init ball prompt
	rand_balls();

	//configure table
	on_ball_moved(G_OBJECT(table), NULL);

	gtk_progress_configure(GTK_PROGRESS(leftbar), smgr->scores[0].score, 0, smgr->scores[0].score);
	gtk_progress_configure(GTK_PROGRESS(rightbar), 0, 0, smgr->scores[0].score);

	sound_play(sound, EventStart);
}

void on_end_game()
{
	int score = gtk_progress_get_value(GTK_PROGRESS(rightbar));
	if(score > smgr->scores[n_scores-1].score){
		sound_play(sound, EventTada);
		scoremgr_add(smgr, score);
	}
	else{
		gtk_dialog_run(GTK_DIALOG(smgr));
		gtk_widget_hide(GTK_WIDGET(smgr));
	}
	on_init_game();
}

void on_balls_appear(GObject *obj, gpointer data)
{
	check_lines();

	GList *empty = find_empty(table->model);
	int n_empty = g_list_length(empty);
	if(n_empty == 0){
		on_end_game();
		return;
	}
	_list_free(empty);

	rand_balls();
}

void on_ball_moved(GObject *obj, gpointer data)
{
	if(check_lines())
		return;

	GList *empty = find_empty(table->model);
	int n_empty = g_list_length(empty);

	Color balls[n_step_balls];
	prompt_get(prompt, balls);

	GList *to_appear = NULL;
	for(int i = 0; i<MIN(n_step_balls, n_empty); i++){
		Position *p = g_list_nth_data(empty, rand()%g_list_length(empty));
		empty = g_list_remove(empty, p);
		Position new = {p->x, p->y, balls[i]};
		to_appear = g_list_append(to_appear, g_memdup(&new, sizeof(new)));		
	}
	_list_free(empty);
	table_appear(table, to_appear);
}

void on_show_scores()
{
	gtk_dialog_run(GTK_DIALOG(smgr));
	gtk_widget_hide(GTK_WIDGET(smgr));
}

void on_table_event(Table *table, EventType event, gpointer data)
{
	if(sounds)
		sound_play(sound, event);
}

void on_toggle_sounds(GtkToggleToolButton *button)
{
	sounds = gtk_toggle_tool_button_get_active(button);
}

void on_about(GtkToolButton *button, gpointer data)
{
	char *authors[] = {"Игорь Вагулин", NULL};
	gtk_show_about_dialog(GTK_WINDOW(window), "authors", authors,
						   "program-name", "Линии",
						   NULL);
}

static GtkWidget* create_toolbar()
{
	GtkWidget *tb = gtk_toolbar_new();
	struct {
		char *name;
		char *tip;
		char *fname;
		GtkSignalFunc handler;
	}items[]={
		{"Заново", "Начать новую игру", GTK_STOCK_NEW, on_init_game},
		{"Счёт", "Показать список лучших", GTK_STOCK_EDIT, on_show_scores},
	};
	for(int i=0; i<G_N_ELEMENTS(items); i++){
		GtkStockItem sitem;
		GtkWidget *icon = NULL;
		if(gtk_stock_lookup(items[i].fname, &sitem))
			icon = gtk_image_new_from_stock(items[i].fname, GTK_ICON_SIZE_SMALL_TOOLBAR);
		else
			icon = gtk_image_new_from_file(items[i].fname);
		GtkToolItem *item = gtk_tool_button_new(icon, items[i].name);
		gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), items[i].tip);
		g_signal_connect(item, "clicked", (GCallback)items[i].handler, NULL);
		gtk_toolbar_insert(GTK_TOOLBAR(tb), item, gtk_toolbar_get_n_items(GTK_TOOLBAR(tb)));
	}

	GtkToolItem *item = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(item), "Звуки");
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), "Включить/выключить звуки");
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), TRUE);
	g_signal_connect(item, "toggled", (GCallback)on_toggle_sounds, NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(tb), item, gtk_toolbar_get_n_items(GTK_TOOLBAR(tb)));
	
	item = gtk_separator_tool_item_new();
	g_object_set(item, "draw", FALSE, NULL);
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(item), TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(tb), item, gtk_toolbar_get_n_items(GTK_TOOLBAR(tb)));

	item = gtk_tool_button_new_from_stock(GTK_STOCK_ABOUT);
	g_signal_connect(item, "clicked", (GCallback)on_about, NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(tb), item, gtk_toolbar_get_n_items(GTK_TOOLBAR(tb)));

	gtk_toolbar_set_style(GTK_TOOLBAR(tb), GTK_TOOLBAR_BOTH);

	return tb;
}

static GtkWidget* create_main()
{
	GtkWidget* window = NULL;
	GtkWidget* hbox = gtk_hbox_new(FALSE, 0);
	GtkWidget* vbox = gtk_vbox_new(FALSE, 2);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", gtk_main_quit, NULL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window), "Линии");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_container_add(GTK_CONTAINER(window), hbox);

	sound = g_object_new(TYPE_SOUND, 0);
	
	smgr = g_object_new(TYPE_SCORE_MGR, 0);
	gtk_window_set_transient_for(GTK_WINDOW(smgr), GTK_WINDOW(window));

	GtkWidget *tb = create_toolbar();
	gtk_box_pack_start(GTK_BOX(vbox), tb, FALSE, TRUE, 0);

	leftbar = gtk_progress_bar_new();
	rightbar = gtk_progress_bar_new();
	GtkSizeGroup *sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sg, leftbar);
	gtk_size_group_add_widget(sg, rightbar);
	gtk_progress_set_show_text(GTK_PROGRESS(rightbar), TRUE);
	gtk_progress_set_format_string(GTK_PROGRESS(rightbar), "%v");
	gtk_progress_set_show_text(GTK_PROGRESS(leftbar), TRUE);
	gtk_progress_set_format_string(GTK_PROGRESS(leftbar), "%v");
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(rightbar), GTK_PROGRESS_BOTTOM_TO_TOP);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(leftbar), GTK_PROGRESS_BOTTOM_TO_TOP);
	gtk_box_pack_start(GTK_BOX(hbox), leftbar, FALSE, FALSE, 0);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), vbox);
	gtk_box_pack_start(GTK_BOX(hbox), rightbar, FALSE, FALSE, 0);
	
	table = g_object_new(TYPE_TABLE, 0);
	g_signal_connect(table, "ball-moved", (GCallback)on_ball_moved, NULL);
	g_signal_connect(table, "balls-appear", (GCallback)on_balls_appear, NULL);
	g_signal_connect_swapped(table, "table-event", (GCallback)on_table_event, sound);
	gtk_box_pack_end(GTK_BOX(vbox), GTK_WIDGET(table), TRUE, TRUE, 0);
	
	prompt = g_object_new(TYPE_PROMPT, 0);
	gtk_box_pack_end(GTK_BOX(vbox), GTK_WIDGET(prompt), FALSE, TRUE, 0);
	
	on_init_game();
	
	return window;
}

int main(int argc, char* argv[])
{
	srand(time(NULL));
	gtk_init(&argc, &argv);

	window = create_main();
	gtk_widget_show_all(window);
	gtk_main();

	gtk_widget_destroy(GTK_WIDGET(smgr));
	g_object_unref(sound);
}
