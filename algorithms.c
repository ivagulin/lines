#include <gtk/gtk.h>
#include <string.h>

#include "table.h"

typedef struct
{
	Position pos;
	GList *childs;
} Node ;

const static double cell_size = 50.0;
#define t_length (n_columns*cell_size)
static const int n_line_balls = 5;

static GList* _get_childs(Position pos)
{
	int neigh[] = {-1, -1, 1, 1};
	GList *ret = NULL;

	for(int i = 0; i<G_N_ELEMENTS(neigh); i++){
		Position *new = g_new0(Position, 1);
		*new = pos;
		
		if(i%2 == 0)
			new->y += neigh[i];
		else
			new->x += neigh[i];

		if(_pos_valid(*new)){
			ret = g_list_append(ret, new);
		}
		else
			g_free(new);
	}

	return ret;
}

static void draw_tree(cairo_t *cr, Node* n)
{
	for(GList *it=n->childs; it; it = g_list_next(it)){
		Node *child = it->data;

		//line
		cairo_move_to(cr, n->pos.x*cell_size+cell_size/2, n->pos.y*cell_size+cell_size/2);
		cairo_line_to(cr, child->pos.x*cell_size+cell_size/2, child->pos.y*cell_size+cell_size/2);
		cairo_stroke(cr);

		//arc
		double angle = 0;
		if(n->pos.y != child->pos.y)
			angle = (n->pos.y < child->pos.y) ? G_PI/2 : G_PI + G_PI_2;
		else
			angle = (n->pos.x < child->pos.x) ? 0 : G_PI;				
		cairo_arc(cr, child->pos.x*cell_size+cell_size/2, child->pos.y*cell_size+cell_size/2, 5, 
				  angle - G_PI_2, angle+G_PI_2);
		cairo_stroke(cr);

		//proceed
		draw_tree(cr, child);
	}
}


static __attribute__((unused)) void draw_branch(cairo_t *cr, GList* l)
{
	for(GList *it=l; g_list_next(it); it = g_list_next(it)){
		Position *curr = it->data;
		Position *next = g_list_next(it)->data;

		//line
		cairo_move_to(cr, curr->x*cell_size+cell_size/2, curr->y*cell_size+cell_size/2);
		cairo_line_to(cr, next->x*cell_size+cell_size/2, next->y*cell_size+cell_size/2);
		cairo_stroke(cr);

		//arc
		double angle = 0;
		if(curr->y != next->y)
			angle = (curr->y < next->y) ? G_PI/2 : G_PI + G_PI_2;
		else
			angle = (curr->x < next->x) ? 0 : G_PI;				
		cairo_arc(cr, next->x*cell_size+cell_size/2, next->y*cell_size+cell_size/2, 5, 
				  angle - G_PI_2, angle+G_PI_2);
		cairo_stroke(cr);
	}
}

#include <gtk/gtk.h>
static __attribute__((unused)) GdkPixbuf* visualize(void (*f)(cairo_t*, gpointer), gpointer data)
{
	int width = cell_size * n_columns;
	GdkColormap *cmap = gdk_rgb_get_colormap();
	GdkPixmap *pixmap = gdk_pixmap_new(NULL, width, width, 24);
	gdk_drawable_set_colormap(GDK_DRAWABLE(pixmap), cmap);
	cairo_t *cr = gdk_cairo_create(GDK_DRAWABLE(pixmap));
	
	//clean area
	cairo_rectangle(cr, 0,0, width, width);
	cairo_set_source_rgb(cr, 1,1,1);
	cairo_fill(cr);
	
	//draw horz and vert lines
	cairo_set_source_rgb(cr, 0,0,0);
	for(int i = 0; i<=t_length; i+=cell_size){
		//horz
		cairo_move_to(cr, 0, i);
		cairo_line_to(cr, t_length, i);
		cairo_stroke(cr);

		//vert
		cairo_move_to(cr, i, 0);
		cairo_line_to(cr, i, t_length);
		cairo_stroke(cr);
	}	

	//draw arrows
	f(cr, data);
	
	cairo_destroy(cr);
	GdkPixbuf *pb = gdk_pixbuf_get_from_drawable(NULL, GDK_DRAWABLE(pixmap), NULL,
												 0,0,0,0,
												 width,width);

	//show in dialog
	GtkWidget *image = gtk_image_new_from_pixbuf(pb);
	GtkDialog *dialog = (GtkDialog*)gtk_dialog_new();
	gtk_container_add(GTK_CONTAINER(dialog->vbox), image);
	gtk_widget_show_all(GTK_WIDGET(dialog));
	gtk_dialog_run(dialog);
	gtk_widget_destroy(GTK_WIDGET(dialog));

	return pb;
}

static GList* find_branch(Node *node, Position where)
{
	if(_pos_equal(node->pos, where)){
		return g_list_prepend(NULL, g_memdup(&node->pos, sizeof(Position)));
	}

	for(GList *it=node->childs; it; it = g_list_next(it)){
		GList *ret = find_branch(it->data, where);
		if(ret){
			return g_list_prepend(ret, g_memdup(&node->pos, sizeof(Position)));
		}
	}
	
	return NULL;
}

static void free_tree(Node *node)
{
	for(GList *it = node->childs; it; it=it->next)
		free_tree(it->data);

	g_list_free(node->childs);
	g_free(node);
}

GList* find_way(Color map[n_columns][n_columns], Position from, Position where)
{
	Node *root = NULL;
	root = g_new0(Node, 1);
	root->pos = from;

	gboolean added[n_columns][n_columns];
	memset(added, 0, sizeof(added));

	GList *left_nodes = NULL;
	left_nodes = g_list_append(left_nodes, root);

	while(left_nodes != NULL){
		gboolean where_added = FALSE;
		Node *node = g_list_first(left_nodes)->data;
		left_nodes = g_list_remove(left_nodes, g_list_first(left_nodes)->data);
		
		GList *childs = _get_childs(node->pos);
		for(GList *it=childs; it; it = g_list_next(it)){
			Position *p = it->data;
			
			if( map[p->x][p->y] == None && added[p->x][p->y] == FALSE){
				Node *new = g_new0(Node, 1);
				new->pos = *p;
				
				node->childs = g_list_append(node->childs, new);
				left_nodes = g_list_append(left_nodes, new);
				added[p->x][p->y] = TRUE;
			}


			if(_pos_equal(*p, where))
				where_added = TRUE;
		}
		childs = _list_free(childs);

		if(where_added)
			break;
	};

	GList *path = find_branch(root, where);
	free_tree(root);
	
	return path;
}


static GList* check_line(GList *line)
{
	GList *ret = NULL;
	Color color = None;
	int len = 0;
	
	for(GList *it = line; it; it=it->next){
		Position *p = it->data;
		if(p->c == color && color != None){
			len++;
			ret = g_list_append(ret, g_memdup(p, sizeof(*p)));
		}
		else{
			if(len >= n_line_balls)
				break;
			_list_free(ret);
			ret = g_list_append(NULL, g_memdup(p, sizeof(*p)));
			len = 1;
			color = p->c;
		}
	}

	if(len >= n_line_balls)
		return ret;
	else
		return _list_free(ret);
	
}

GList* find_lines(Color map[n_columns][n_columns])
{
	GList *ret = NULL;

	//find horizontal
	for(int i = 0; i<n_columns; i++){
		GList *line = NULL;
		for(int j = 0; j<n_columns; j++){
			Position p = {i, j, map[i][j]};
			line = g_list_append(line, g_memdup(&p, sizeof(p)));
		}
		ret = g_list_concat(ret, check_line(line));
		line = _list_free(line);
	}

	//find vertical
	for(int i = 0; i<n_columns; i++){
		GList *line = NULL;
		for(int j = 0; j<n_columns; j++){
			Position p = {j, i, map[j][i]};
			line = g_list_append(line, g_memdup(&p, sizeof(p)));
		}
		ret = g_list_concat(ret, check_line(line));
		line = _list_free(line);
	}

	//find diagonal from nw to se
	for(int i = -n_columns+1; i<n_columns; i++){
		GList *line = NULL;
		for(int j = 0; j<n_columns; j++){
			int x = i+j;
			int y = j;
			Position p = {x, y, map[x][y]};
			if(_pos_valid(p))
				line = g_list_append(line, g_memdup(&p, sizeof(p)));
		}
		ret = g_list_concat(ret, check_line(line));
		line = _list_free(line);
	}

	//find diagonal from ne to sw
	for(int i = 0; i<n_columns*2-1; i++){
		GList *line = NULL;
		for(int j = 0; j<n_columns; j++){
			int x = i-j;
			int y = j;
			Position p = {x, y, map[x][y]};
			if(_pos_valid(p))
				line = g_list_append(line, g_memdup(&p, sizeof(p)));
		}
		ret = g_list_concat(ret, check_line(line));
		line = _list_free(line);
	}

	return ret;
}

GList *find_empty(Color map[n_columns][n_columns])
{
	GList *ret = NULL;
	
	fortable(n_columns){
		if(map[i][j] == None){
			Position p = {i,j};
			ret = g_list_append(ret, g_memdup(&p, sizeof(p)));
		}
	}

	return ret;
}
