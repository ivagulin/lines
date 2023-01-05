#include <gtk/gtklabel.h>
#include <string.h>
#include "scoremgr.h"

static void scoremgr_update(ScoreMgr *smgr)
{
	for(int i=0; i<n_scores; i++){
		gchar *name = NULL;
		if(smgr->scores[i].name && strlen(smgr->scores[i].name))
			name = smgr->scores[i].name;
		else
			name = "Аноним";
		gchar *score = g_strdup_printf("%d", smgr->scores[i].score);

		gtk_label_set_text(GTK_LABEL(smgr->labels[i+1][0]), name);
		gtk_label_set_text(GTK_LABEL(smgr->labels[i+1][1]), score);
		
		g_free(score);
	}
}

static void scoremgr_save(ScoreMgr *smgr)
{	
	GKeyFile *kf = g_key_file_new();
	for(int i=0; i<n_scores; i++){
		static char key[100];
		if(!smgr->scores[i].name || !strlen(smgr->scores[i].name))
			continue;
		sprintf(key, "name%d", i);
		g_key_file_set_string(kf, "main", key, smgr->scores[i].name);
		sprintf(key, "score%d", i);
		g_key_file_set_integer(kf, "main", key, smgr->scores[i].score);
	}

	gchar *data = g_key_file_to_data(kf, 0, 0);
	g_key_file_free(kf);

	g_file_set_contents("score.dat", data, -1, 0);

	g_free(data);
}

static void scoremgr_load(ScoreMgr *smgr)
{
	GKeyFile *kf = g_key_file_new();
	g_key_file_load_from_file(kf, "score.dat", 0, NULL);

	for(int i=0; i<n_scores; i++){
		static char key[100];
		sprintf(key, "name%d", i);
		smgr->scores[i].name = g_key_file_get_string(kf, "main", key, NULL);
		sprintf(key, "score%d", i);
		smgr->scores[i].score = g_key_file_get_integer(kf, "main", key, NULL);
	}

	if(smgr->scores[0].score < 10){
		smgr->scores[0].score = 1678;
		smgr->scores[0].name = g_strdup("Автор");
	}

	scoremgr_update(smgr);
}

static void scoremgr_finalize(GObject *obj)
{
	ScoreMgr *smgr = SCORE_MGR(obj);
	scoremgr_save(smgr);
	g_free(smgr->name);
}

static void scoremgr_init(ScoreMgr *smgr)
{
	GtkBox *vbox = GTK_BOX(GTK_DIALOG(smgr)->vbox);
	GtkSizeGroup *sgr = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	GtkSizeGroup *sgl = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	g_signal_connect(smgr, "delete-event", (GCallback)gtk_widget_hide_on_delete, NULL);

	gtk_box_set_spacing(vbox, 5);
	for(int i=0; i<n_scores+1; i++){
		GtkWidget *box = gtk_hbox_new(FALSE, 5);
		smgr->labels[i][0] = gtk_label_new("");
		smgr->labels[i][1] = gtk_label_new("");
		if(i==0){
			gtk_label_set_markup(GTK_LABEL(smgr->labels[i][0]), "<span weight=\"bold\">Имя</span>");
			gtk_label_set_markup(GTK_LABEL(smgr->labels[i][1]), "<span weight=\"bold\">Счёт</span>");
			gtk_label_set_width_chars(GTK_LABEL(smgr->labels[i][0]), 20);
			gtk_label_set_width_chars(GTK_LABEL(smgr->labels[i][1]), 10);
		}
		gtk_size_group_add_widget(GTK_SIZE_GROUP(sgr), smgr->labels[i][1]);
		gtk_size_group_add_widget(GTK_SIZE_GROUP(sgl), smgr->labels[i][0]);
		gtk_box_pack_start(GTK_BOX(box), smgr->labels[i][0], TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(box), smgr->labels[i][1], FALSE, TRUE, 0);
		gtk_box_pack_start_defaults(vbox, box);
		if(i==0)
			gtk_box_pack_start_defaults(vbox, gtk_hseparator_new());
	}
	scoremgr_load(smgr);

	gtk_dialog_add_button(GTK_DIALOG(smgr), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	gtk_dialog_set_default_response(GTK_DIALOG(smgr), GTK_RESPONSE_CLOSE);
	gtk_window_set_title(GTK_WINDOW(smgr), "Лучшие результаты");
	
	gtk_widget_show_all(GTK_WIDGET(smgr));
	gtk_widget_hide(GTK_WIDGET(smgr));

	smgr->name = g_strdup("");
}

static void scoremgr_class_init(ScoreMgrClass *class)
{
	G_OBJECT_CLASS(class)->finalize = scoremgr_finalize;
}
G_DEFINE_TYPE (ScoreMgr, scoremgr, GTK_TYPE_DIALOG);

void scoremgr_add(ScoreMgr *smgr, int result)
{
	int position = -1;

	for(int i=0; i<n_scores; i++)
		if(smgr->scores[i].score < result){
			position = i;
			break;
		}

	if(position != -1){
		GtkWidget *congr = gtk_label_new("");
		gtk_misc_set_padding(GTK_MISC(congr), 15, 15);
		char *str = g_strdup_printf("<b>Поздравляю!\nНовый рекорд-%d</b>", result);
		gtk_label_set_markup(GTK_LABEL(congr), str);
		g_free(str);
		GtkWidget *query = gtk_label_new("Введите ваше имя");
		GtkWidget *entry = gtk_entry_new();
		gtk_entry_set_text(GTK_ENTRY(entry), smgr->name);
		gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
		GtkWidget *dialog = gtk_dialog_new_with_buttons("", gtk_window_get_transient_for(GTK_WINDOW(smgr)),
														GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, 0);
		GtkWidget *vbox = GTK_DIALOG(dialog)->vbox;
		gtk_box_pack_start_defaults(GTK_BOX(vbox), congr);
		gtk_box_pack_start_defaults(GTK_BOX(vbox), query);
		gtk_box_pack_start_defaults(GTK_BOX(vbox), entry);
		gtk_widget_show_all(dialog);
		gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
		int response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_hide(GTK_WIDGET(dialog));
		if(response == GTK_RESPONSE_OK){
			g_free(smgr->name);
			smgr->name = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
			g_free(smgr->scores[n_scores-1].name);
			for(int i=n_scores-1; i>position; i--){
				smgr->scores[i].name = smgr->scores[i-1].name;
				smgr->scores[i].score = smgr->scores[i-1].score;
			}
			smgr->scores[position].name = g_strdup(smgr->name);
			smgr->scores[position].score = result;
			scoremgr_update(smgr);
			gtk_dialog_run(GTK_DIALOG(smgr));
			gtk_widget_hide(GTK_WIDGET(smgr));
		}
		gtk_widget_destroy(dialog);
	}
}
