#ifndef MISC_H
#define MISC_H

typedef enum {
	None = 0,
	Red,
	Vinous,
	Sky,
	Blue,
	Lilac,
	Green,
	Yellow,
	ColorMax,
}Color;

typedef enum{
	EventChoose = 0,
	EventDestroy,
	EventFail,
	EventMove,
	EventStart,
	EventTada,
	EventMax,
}EventType;

typedef struct _Position
{
	int x;
	int y;
	Color c;
}Position;

#define n_scores 10
#define n_step_balls 3
#define n_columns 9

#define fortable(x)						\
	for(int i = 0; i<x; i++)			\
		for(int j = 0; j<x; j++)		\
			
#define ZEROAT(x)								\
	do{											\
		memset(x, 0, sizeof(*x));				\
	}while(0)									\

inline static gboolean _between(int val, int min, int max)
{
	return val>min && val<max;
}

inline static gboolean _pos_equal(Position val1, Position val2)
{
	return val1.x == val2.x && val1.y == val2.y;
}

inline static gboolean _pos_valid(Position p)
{
	return p.x >= 0 && p.x < n_columns && p.y >= 0 && p.y < n_columns;
}

inline static gpointer _list_free(GList *list)
{
	g_list_foreach(list, (GFunc)g_free, NULL);
	g_list_free(list);	
	return NULL;
}

GList* find_way(Color map[n_columns][n_columns], Position from, Position to);
GList* find_lines(Color map[n_columns][n_columns]);
GList* find_empty(Color map[n_columns][n_columns]);

#endif
