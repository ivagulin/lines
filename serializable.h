#ifndef _SERIALIZABLE_H_
#define _SERIALIZABLE_H_

#include <glib-object.h>

#define TYPE_SERIALIZABLE  (serializable_get_type ())
#define SERIALIZABLE(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_SERIALIZABLE, Serializable))

typedef struct _Serializable Serializable;

typedef struct
{
	void (*serialize)(Serializable *me, void** data, int* len);
	void (*deserialize)(Serializable *me, void* data, int len);
}SerializableIface;

GType serializable_get_type();

#endif
