#include "monster2.hpp"
#include <allegro5/allegro_acodec.h>

#include "java.h"

bool sound_inited = false;
static int total_samples = 0;
static int curr_sample = 0;

std::string shutdownMusicName = "";
std::string shutdownAmbienceName = "";
static std::string sample_name = "";

std::map<std::string, MSAMPLE> preloaded_samples;

static MSAMPLE sample;

static DWORD music = 0;
static DWORD ambience = 0;
std::string musicName = "";
std::string ambienceName = "";
static QWORD music_loop_start = 0;
static QWORD ambience_loop_start = 0;
static float musicVolume = 1.0f;
static float ambienceVolume = 1.0f;

static std::string preloaded_names[] =
{
#ifdef LITE
	"Cure.ogg",
	"Darkness1.ogg",
	"Elixir.ogg",
	"Heal.ogg",
	"HolyWater.ogg",
	"Revive.ogg",
	"Wave.ogg",
	"Weep.ogg",
	"Whirlpool.ogg",
	"appear.ogg",
	"battle.ogg",
	"blip.ogg",
	"bolt.ogg",
	"boss.ogg",
	"chest.ogg",
	"door.ogg",
	"enemy_die.ogg",
	"error.ogg",
	"fall.ogg",
	"fire1.ogg",
	"hit.ogg",
	"ice1.ogg",
	"jump.ogg",
	"melee_woosh.ogg",
	"new_party_member.ogg",
	"nooskewl.ogg",
	"select.ogg",
	"slime.ogg",
	"spin.ogg",
	"suck.ogg",
	"swipe.ogg",
	"woosh.ogg",
#else
	"Acorns.ogg",
	"Arc.ogg",
	"Banana.ogg",
	"Beam.ogg",
	"BoF.ogg",
	"Bolt2.ogg",
	"Bolt3.ogg",
	"Charm.ogg",
	"Cure.ogg",
	"Daisy.ogg",
	"Darkness1.ogg",
	"Darkness2.ogg",
	"Darkness3.ogg",
	"Elixir.ogg",
	"Fire2.ogg",
	"Fire3.ogg",
	"Fireball.ogg",
	"Heal.ogg",
	"HolyWater.ogg",
	"Ice3.ogg",
	"Laser.ogg",
	"Machine_Gun.ogg",
	"Meow.ogg",
	"Mmm.ogg",
	"Orbit.ogg",
	"Portal.ogg",
	"Puke.ogg",
	"Punch.ogg",
	"Quick.ogg",
	"Rend.ogg",
	"Revive.ogg",
	"Slow.ogg",
	"Spray.ogg",
	"Stomp.ogg",
	"Stone.ogg",
	"Stun.ogg",
	"Swallow.ogg",
	"Thud.ogg",
	"TouchofDeath.ogg",
	"Twister.ogg",
	"UFO.ogg",
	"Vampire.ogg",
	"Wave.ogg",
	"Web.ogg",
	"Weep.ogg",
	"Whip.ogg",
	"Whirlpool.ogg",
	"appear.ogg",
	"battle.ogg",
	"blip.ogg",
	"bolt.ogg",
	"boss.ogg",
	"bow_draw.ogg",
	"bow_release_and_draw.ogg",
	"buzz.ogg",
	"cartoon_fall.ogg",
	"chest.ogg",
	"ching.ogg",
	"chomp.ogg",
	"door.ogg",
	"drain.ogg",
	"enemy_die.ogg",
	"enemy_explosion.ogg",
	"error.ogg",
	"explosion.ogg",
	"fall.ogg",
	"fire1.ogg",
	"freeze.ogg",
	"high_cackle.ogg",
	"hit.ogg",
	"ice1.ogg",
	"ignite.ogg",
	"jump.ogg",
	"low_cackle.ogg",
	"melee_woosh.ogg",
	"new_party_member.ogg",
	"nooskewl.ogg",
	"pistol.ogg",
	"push.ogg",
	"rocket_launch.ogg",
	"select.ogg",
	"sleep.ogg",
	"slice.ogg",
	"slime.ogg",
	"spin.ogg",
	"splash.ogg",
	"staff_fly.ogg",
	"staff_poof.ogg",
	"suck.ogg",
	"swipe.ogg",
	"torpedo.ogg",
	"woosh.ogg",
#endif
	""
};

static void destroyMusic(void)
{
	if (music) {
		bass_destroyMusic(music);
		music = 0;
	}
	if (ambience) {
		bass_destroyMusic(ambience);
		ambience = 0;
	}
	
	musicName = "";
	sample_name = "";
}

void initSound(void)
{
	sound_inited = true;

	bass_initSound();
	
	for (int i = 0; preloaded_names[i] != ""; i++) {
		total_samples++;
	}
}

bool loadSamples(void (*cb)(int, int))
{
	preloaded_samples[preloaded_names[curr_sample]] =
		loadSample(preloaded_names[curr_sample]);
	(*cb)(curr_sample, total_samples);
	curr_sample++;
	if (curr_sample == total_samples)
		return true;
	return false;
}

void destroySound(void)
{
	if (!sound_inited) return;

	std::map<std::string,  MSAMPLE>::iterator it;

	for (it = preloaded_samples.begin(); it != preloaded_samples.end(); it++) {
		HSAMPLE s = (HSAMPLE)it->second;
		bass_destroySample(s);
	}

	preloaded_samples.clear();

	if (sample) {
		bass_destroySample(sample);
		sample = 0;
	}

	destroyMusic();

	shutdownMusicName = musicName;
	shutdownAmbienceName = ambienceName;
	
	bass_shutdownBASS();
}


void playPreloadedSample(std::string name)
{
	if (!sound_inited) return;

	playSample(preloaded_samples[name]);
}


MSAMPLE loadSample(std::string name)
{
	MSAMPLE s = 0;

	if (!sound_inited) return s;

	s = bass_loadSample(getResource("sfx/%s", name.c_str()));

	return s;
}


void destroySample(MSAMPLE sample)
{
	if (!sound_inited) return;

	bass_destroySample(sample);
}


void playSample(MSAMPLE sample, MSAMPLE_ID *unused)
{
	(void)unused;
	if (!sound_inited) return;

	bass_playSample(sample);
}


void loadPlayDestroy(std::string name)
{
	if (!sound_inited) return;

	playPreloadedSample(name);
}

void stopAllSamples(void)
{
}

void playMusic(std::string name, float volume, bool force)
{
	if (!sound_inited) return;

	if (!force && musicName == name)
		return;

	if (name != "")
		musicName = name;

	if (music) {
		bass_destroyMusic(music);
	}

	if (name == "" || config.getMusicVolume() == 0) {
		music = 0;
		return;
	}

	music = bass_loadMusic(getResource("music/%s", name.c_str()));
	bass_playMusic(music);
	setMusicVolume(volume);
}

void setMusicVolume(float volume)
{
	if (!sound_inited) return;

	musicVolume = volume;

	volume *= config.getMusicVolume()/255.0f;

	if (music) {
		bass_setMusicVolume(music, volume);
	}
}

void playAmbience(std::string name, float vol)
{
	if (!sound_inited) return;

	ambienceName = name;

	if (ambience) {
		bass_destroyMusic(ambience);
	}

	if (name == "" || config.getMusicVolume() == 0) {
		ambience = 0;
		return;
	}

	ambience = bass_loadMusic(getResource("music/%s", name.c_str()));
	bass_playMusic(ambience);
	setAmbienceVolume(vol);
}

void setAmbienceVolume(float volume)
{
	if (!sound_inited) return;

	ambienceVolume = volume;

	volume *= config.getMusicVolume()/255.0f;

	if (ambience)
		bass_setMusicVolume(ambience, volume);
}


float getMusicVolume(void)
{
   return musicVolume;
}

float getAmbienceVolume(void)
{
   return ambienceVolume;
}

void unmuteMusic(void)
{
    playMusic(musicName, 1.0f, true);
}

void unmuteAmbience(void)
{
    playAmbience(ambienceName);
}

void restartMusic(void)
{
	playMusic(shutdownMusicName, 1.0f, true);
}


void restartAmbience(void)
{
	playAmbience(shutdownAmbienceName);
}

