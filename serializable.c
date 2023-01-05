#include "serializable.h"

GType serializable_get_type()
{
	static GType type = 0;
	
	if(type == 0){
		const GTypeInfo serializable_type_info = {
			sizeof (SerializableIface), /* class_size */
			NULL, /* base_init */
			NULL, /* base_finalize */
			NULL,
			NULL, /* class_finalize */
			NULL, /* class_data */
			0,
			0,
			NULL
		};

		type = g_type_register_static (G_TYPE_INTERFACE, "Serializable",
									   &serializable_type_info, 0);
		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}
	
	return type;
}
