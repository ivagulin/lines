#ifndef SOUND_H
#define SOUND_H

#include "glib-object.h"

#include "misc.h"

#define TYPE_SOUND (sound_get_type ())
#define SOUND(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_SOUND, Sound))

typedef struct _Sound
{
	GObject parent;
	struct Mix_Chunk *files[EventMax];
}Sound;

typedef struct _SoundClass
{
	GObjectClass parent;
}SoundClass;

GType sound_get_type();
void sound_play(Sound*, EventType);

#endif
