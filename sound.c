#include "SDL.h"
#include "SDL_mixer.h"

#include "sound.h"

static char* Event2SName[]={
	"sound/choose.wav",
	"sound/destroy.wav",
	"sound/fail.wav",
	"sound/move.wav",
	"sound/start.wav",
	"sound/tada.wav",
};
static int sound_count = 0;

static void sound_init(Sound *sound)
{
	if(sound_count == 0){
		SDL_Init(SDL_INIT_AUDIO);
		Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
		sound_count++;
	}
	
	for(int i=0; i<EventMax; i++)
		sound->files[i] = Mix_LoadWAV(Event2SName[i]);
}

static void sound_finalize(GObject *obj)
{
	Sound *sound = SOUND(obj);	
	for(int i=0; i<EventMax; i++)
		Mix_FreeChunk(sound->files[i]);

	sound_count--;
	if(sound_count == 0){
		Mix_CloseAudio();
		SDL_Quit();
	}
}

static void sound_class_init(SoundClass *class)
{
	G_OBJECT_CLASS(class)->finalize = sound_finalize;
}

void sound_play(Sound* sound, EventType event)
{
	if(event>=0 && event<EventMax)
		Mix_PlayChannel(-1, sound->files[event], 0);
}
G_DEFINE_TYPE (Sound, sound, G_TYPE_OBJECT);
