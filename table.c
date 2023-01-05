#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <stdlib.h>

#include "table.h"

static const Position invalid = {-1,-1, None};
static const double cell_size = 50.0;
static const double ball_radius = 18.0;
static const int jump_range = 6;
static const int jump_interval = 100;
static const int anim_interval = 50;
static const int last_frame = 5;
#define t_length (n_columns*cell_size)

inline guint32 _get_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec/1000;
}

static cairo_pattern_t* c_c2p(Color color)
{
	static struct {int r,g,b;} c_to_c[ColorMax] = {
		{0,0,0},
		{238,0,0},
		{139,26,26},
		{0,191,255},
		{0,0,255},
		{255,62,150},
		{0,205,0},
		{255,215,0},
	};

	cairo_pattern_t *retval = 
		cairo_pattern_create_radial(cell_size/6, cell_size/6, 0, 
									0, 0, ball_radius);
	cairo_pattern_add_color_stop_rgb(retval, 0, 1,1,1);
	cairo_pattern_add_color_stop_rgb(retval, ball_radius, 
									 c_to_c[color].r/255.0, c_to_c[color].g/255.0, c_to_c[color].b/255.0);

	return retval;
}

static void c_to_position(cairo_t *cr, Position pos)
{
	cairo_save(cr);
	double xc = pos.x*cell_size + cell_size / 2;
	double yc = pos.y*cell_size + cell_size / 2;
	cairo_translate(cr, xc, yc);
}

static void c_draw_ball(cairo_t *cr, Color color, guint32 js)
{
	if(js!=0){
		guint c_step = ((_get_ms() - js) / jump_interval) % jump_range;
		double to_c = ABS(c_step - jump_range/2.0);
		double c_transl = jump_range/2.0 - to_c;
		if(to_c<2){
			double c_scale = (2-to_c)/20.0;
			cairo_scale(cr, 1+c_scale, 1-c_scale);
		}
		cairo_translate(cr, 0, c_transl*2);
	}

	//draw border
	cairo_arc(cr, 0, 0, ball_radius, 0, 2*G_PI);
	cairo_set_source_rgb(cr, 0,0,0);
	cairo_stroke_preserve(cr);

	//fill with gradient color
	cairo_pattern_t *pattern = c_c2p(color);
	cairo_set_source(cr, pattern);
	cairo_fill(cr);
	cairo_pattern_destroy(pattern);
}

static void c_draw_cell(cairo_t *cr)
{
	cairo_save(cr);
	cairo_set_line_width(cr, 2);
	
	cairo_rectangle(cr, -cell_size/2, -cell_size/2, cell_size, cell_size);
	cairo_stroke(cr);	

	cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
	cairo_rectangle(cr, -cell_size/2 + 2, -cell_size/2 + 2, cell_size - 4, cell_size - 4);
	cairo_stroke(cr);	

	cairo_restore(cr);
}

static gboolean on_expose(GtkWidget *widget, GdkEventExpose *event)
{
	/*
	static int draw_c = 0;
	printf("draw %06d\n", draw_c++);
	*/

	Table *table = TABLE(widget);
	cairo_t *cr = gdk_cairo_create(GDK_DRAWABLE(widget->window));
	
	//some initialization
	cairo_set_source_rgb(cr, 0,0,0);

	//draw hover
	if(table->over.x >= 0 && table->over.y >= 0){
		c_to_position(cr, table->over);
		cairo_set_source_rgba(cr, 0, 0.2, 1, 0.1);
		cairo_rectangle(cr, -cell_size/2, -cell_size/2, cell_size, cell_size);
		cairo_fill(cr);
		cairo_restore(cr);
	}

	//draw balls
	for(int i = 0; i<n_columns; i++)
		for(int j = 0; j<n_columns; j++){
			Color color = table->model[i][j];
			Position curr = {i,j};
			int js = _pos_equal(curr, table->sel)?table->jump_start:0;
			c_to_position(cr, curr);
			c_draw_cell(cr);
			cairo_set_line_width(cr, 1);
			if(color != None)
				c_draw_ball(cr, color, js);
			cairo_restore(cr);
		}

	for(GList *it = table->to_disappear; it; it = it->next){
		double scale = 1.0 + 1.0/last_frame - 1.0/last_frame * table->anim_frame;
		Position *p = it->data;
		c_to_position(cr, *p);
		cairo_scale(cr, scale, scale);
		c_draw_ball(cr, p->c, 0);
		cairo_restore(cr);
	}

	for(GList *it = table->to_appear; it; it = it->next){
		double scale = 1.0/last_frame * table->anim_frame;
		Position *p = it->data;
		c_to_position(cr, *p);
		cairo_scale(cr, scale, scale);
		c_draw_ball(cr, p->c, 0);
		cairo_restore(cr);
	}

	cairo_destroy(cr);

	return TRUE;
}

static gboolean on_motion(GtkWidget *widget, GdkEventMotion *event)
{
	Table *table = TABLE(widget);
	Position over;
	
	if(!_between(event->x, 0, 0+t_length) || 
	   !_between(event->y, 0, 0+t_length))
		over = invalid;
	else{
		int x = (event->x)/cell_size;
		int y = (event->y)/cell_size;
		over = (Position){x,y};
	}

	if(!_pos_equal(table->over, over))
		gtk_widget_queue_draw(GTK_WIDGET(table));

	table->over = over;
	
	return TRUE;
}

static void mark_ball(Table *table)
{
	g_signal_emit_by_name(table, "table-event", EventChoose);
	table->sel = table->over;
	table->jump_start = _get_ms();
	if(table->jump_source)
		g_source_remove(table->jump_source);
	table->jump_source = g_timeout_add(jump_interval, 
									   (GSourceFunc)(gtk_widget_queue_draw), 
									   GTK_WIDGET(table));
}

static void attempt_move(Table *table)
{
	GList *way = find_way(table->model, table->sel, table->over);

	if(way){
		//stop jump animation
		if(table->jump_source)
			g_source_remove(table->jump_source);
		
		//change colors in model
		table->model[table->over.x][table->over.y] = table->model[table->sel.x][table->sel.y];
		table->model[table->sel.x][table->sel.y] = None;
		table->sel = invalid;
		//emit signals
		g_signal_emit_by_name(table, "ball-moved");
		g_signal_emit_by_name(table, "table-event", EventMove);
	}
	else
		g_signal_emit_by_name(table, "table-event", EventFail);

	way = _list_free(way);
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event)
{
	Table *table = TABLE(widget);
	int ox = table->over.x;
	int oy = table->over.y;

	if(table->anim_source != 0)
		return FALSE;
	
	if(table->model[ox][oy] != None && _pos_valid(table->over))
		mark_ball(table);
	else if(table->model[ox][oy] == None && _pos_valid(table->sel))
		attempt_move(table);

	return TRUE;
}

static void table_finalize(GObject *obj)
{
	Table *table = TABLE(obj);
	if(table->jump_source)
		g_source_remove(table->jump_source);
	if(table->anim_source)
		g_source_remove(table->anim_source);
}

static void table_init(Table *table)
{
	table->over = invalid;
	table->sel = invalid;
	
	gtk_widget_set_size_request(GTK_WIDGET(table), t_length, t_length);
	gtk_widget_add_events(GTK_WIDGET(table), GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK);
}

static void table_class_init(TableClass *class)
{
	G_OBJECT_CLASS(class)->finalize = table_finalize;
	GTK_WIDGET_CLASS(class)->expose_event = on_expose;
	GTK_WIDGET_CLASS(class)->button_press_event = on_button_press;
	GTK_WIDGET_CLASS(class)->motion_notify_event = on_motion;

	g_signal_new("ball-moved", TYPE_TABLE, G_SIGNAL_RUN_LAST, 0, 
				 NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	g_signal_new("table-event", TYPE_TABLE, G_SIGNAL_RUN_LAST, 0, 
				 NULL, NULL, g_cclosure_marshal_VOID__ENUM, G_TYPE_NONE, 1, G_TYPE_INT);
	g_signal_new("balls-appear", TYPE_TABLE, G_SIGNAL_RUN_LAST, 0, 
				 NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}
G_DEFINE_TYPE (Table, table, GTK_TYPE_DRAWING_AREA);

void anim_end(Table *table)
{
	for(GList *it = table->to_appear; it; it=it->next){
		Position *p = it->data;
		table->model[p->x][p->y] = p->c;
	}

	if(table->anim_source)
		g_source_remove(table->anim_source);
	table->anim_frame = 0;
	table->anim_source = 0;

	if(table->to_appear){
		table->to_appear = _list_free(table->to_appear);
		g_signal_emit_by_name(table, "balls-appear");
	}
	if(table->to_disappear){
		table->to_disappear = _list_free(table->to_disappear);
		g_signal_emit_by_name(table, "table-event", EventDestroy);
	}

	gtk_widget_queue_draw(GTK_WIDGET(table));
}

gboolean anim_step(Table *table)
{	
	if(table->anim_frame >= last_frame){
		anim_end(table);
		return FALSE;
	}	
	table->anim_frame++;
	gtk_widget_queue_draw(GTK_WIDGET(table));
	return TRUE;
}

void anim_start(Table *table)
{
	if(table->anim_source)
		g_source_remove(table->anim_source);
	table->anim_source = g_timeout_add(anim_interval, (GSourceFunc)anim_step, table);
	table->anim_frame = 1;
	gtk_widget_queue_draw(GTK_WIDGET(table));
}

void table_appear(Table *table, GList *list)
{
	table->to_appear = list;
	anim_start(table);
}

void table_disappear(Table *table, GList *list)
{
	for(GList *it = list; it; it=it->next){
		Position *p = it->data;
		table->model[p->x][p->y] = None;
	}
	
	table->to_disappear = list;
	anim_start(table);
}

static gboolean prompt_expose(GtkWidget *widget, GdkEventExpose *event)
{
	Prompt *prompt = PROMPT(widget);
	cairo_t *cr = gdk_cairo_create(GDK_DRAWABLE(GTK_WIDGET(prompt)->window));
	
	double trans = GTK_WIDGET(prompt)->allocation.width / 2 -  cell_size * n_step_balls/2 ;
	cairo_translate(cr, trans, 0);

	//draw balls
	for(int i = 0; i<n_step_balls; i++){
		c_to_position(cr, (Position){i,0});
		c_draw_cell(cr);
		cairo_set_line_width(cr, 1);
		if(prompt->balls[i] != None){
			cairo_scale(cr, 0.5, 0.5);
			c_draw_ball(cr, prompt->balls[i], 0);
		}
		cairo_restore(cr);
	}

	cairo_destroy(cr);
	return TRUE;
}

static void prompt_init(Prompt *prompt)
{
	gtk_widget_set_size_request(GTK_WIDGET(prompt), 0, cell_size);
}

static void prompt_class_init(PromptClass *class)
{
	GTK_WIDGET_CLASS(class)->expose_event = prompt_expose;
}

void prompt_set(Prompt *prompt, Color src[n_step_balls])
{
	for(int i=0; i<n_step_balls; i++)
		prompt->balls[i] = src[i];
	gtk_widget_queue_draw(GTK_WIDGET(prompt));
}

void prompt_get(Prompt *prompt, Color dst[n_step_balls])
{
	for(int i=0; i<n_step_balls; i++)
		dst[i] = prompt->balls[i];
}

G_DEFINE_TYPE (Prompt, prompt, GTK_TYPE_DRAWING_AREA);
