#ifndef TABLE_H
#define TABLE_H

#include "gtk/gtkdrawingarea.h"
#include "misc.h"

#define TYPE_TABLE (table_get_type ())
#define TABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_TABLE, Table))
#define TYPE_PROMPT (prompt_get_type ())
#define PROMPT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_PROMPT, Prompt))

typedef struct _Table
{
	GtkDrawingArea parent;
	Color model[n_columns][n_columns];

	Position sel;
	guint32 jump_start;
	int jump_source;

	Position over;

	GList *to_appear;
	GList *to_disappear;
	int anim_source;
	int anim_frame;
	
}Table;

typedef struct _TableClass
{
	GtkDrawingAreaClass parent_class;
}TableClass;

GType table_get_type();
void table_clear(Table *);
void table_appear(Table *, GList *balls);
void table_disappear(Table *, GList *balls);

typedef struct _Prompt
{
	GtkDrawingArea parent;
	Color balls[n_step_balls];
}Prompt;

typedef struct _PromptClass
{
	GtkDrawingAreaClass parent;
}PromptClass;

GType prompt_get_type();
void prompt_set(Prompt *p, Color src[n_step_balls]);
void prompt_get(Prompt *p, Color dst[n_step_balls]);

#endif //TABLE_H
