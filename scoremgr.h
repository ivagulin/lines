#ifndef SCORE_H
#define SCORE_H

#include "gtk/gtk.h"
#include "misc.h"

#define TYPE_SCORE_MGR (scoremgr_get_type ())
#define SCORE_MGR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_SCORE_MGR, ScoreMgr))

typedef struct Score {
	char *name;
	int score;
}Score;

typedef struct _ScoreMgr
{
	GtkDialog parent;
	Score scores[n_scores];
	GtkWidget* labels[n_scores+1][2];
	gchar *name;
}ScoreMgr;

typedef struct _ScoreMgrClass
{
	GtkDialogClass parent;
}ScoreMgrClass;

GType scoremgr_get_type();
void scoremgr_add(ScoreMgr*, int result);

#endif
