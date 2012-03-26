#ifndef M2_CONFIG_H
#define M2_CONFIG_H

#include "monster2.hpp"

enum DPAD_TYPE 
{
	DPAD_NONE = 0,
	DPAD_HYBRID_1,
	DPAD_HYBRID_2,
	DPAD_TOTAL_1,
	DPAD_TOTAL_2
};

enum CFG_DIFFICULTY
{
	CFG_DIFFICULTY_EASY = 0,
	CFG_DIFFICULTY_NORMAL = 1,
	CFG_DIFFICULTY_HARD = 2
};

enum CFG_TUNING
{
	CFG_TUNING_BATTERY = 0,
	CFG_TUNING_BALANCED = 1,
	CFG_TUNING_PERFORMANCE = 2
};

enum CFG_SHAKE_ACTION
{
	CFG_SHAKE_CANCEL = 0,
	CFG_SHAKE_CHANGE_SONG = 1
};

enum CFG_FILTER_TYPE
{
	FILTER_NONE = 0,
	FILTER_LINEAR,
	FILTER_SCALE2X,
	FILTER_SCALE3X,
	FILTER_SCALE4X,
	FILTER_SCALE2X_LINEAR,
	FILTER_SCALE3X_LINEAR,
	FILTER_SCALE4X_LINEAR,
	FILTER_SCALEDOWN,
	FILTER_HALFSCALE
};

class Configuration {
public:
	ScreenDescriptor* getWantedGraphicsMode();
	void setWantedGraphicsMode(ScreenDescriptor& sd);
	bool showDebugMessages();
	void setShowDebugMessages(bool show);
	bool useCompiledScripts();
	bool getWaitForVsync();
	void setWaitForVsync(bool vs);
	bool getGamepadAvailable();
	void setGamepadAvailable(bool available);
	bool useSmoothScaling();
	void setUseSmoothScaling(bool smooth);
	int getSFXVolume();
	void setSFXVolume(int volume);
	int getMusicVolume();
	void setMusicVolume(int volume);
	int getJoyButton1();
	void setJoyButton1(int b1);
	int getJoyButton2();
	void setJoyButton2(int b2);
	int getJoyButton3();
	void setJoyButton3(int b3);
	int getKey1(void);
	void setKey1(int k1);
	int getKey2(void);
	void setKey2(int k2);
	int getKey3(void);
	void setKey3(int k2);
	int getKeyLeft(void);
	void setKeyLeft(int kl);
	int getKeyRight(void);
	void setKeyRight(int kr);
	int getKeyUp(void);
	void setKeyUp(int ku);
	int getKeyDown(void);
	void setKeyDown(int kd);
	int getTargetFPS(void);
	bool getUseOnlyMemoryBitmaps(void);
	void setUseOnlyMemoryBitmaps(bool onlyMem);
	bool getLowCpuUsage(void);
	void setLowCpuUsage(bool low);
	int getStick(void);
	void setStick(int s);
	int getAxis(void);
	void setAxis(int a);
	bool getXbox360(void);
	void setXbox360(bool x);
	int getAdapter(void);
	void setAdapter(int a);
#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
	int getDpadType(void) { return cfg_dpad_type; }
	void setDpadType(int b) { cfg_dpad_type = b; }
	bool getTellUserToUseDpad(void) { return cfg_tellusertousedpad; }
	void setTellUserToUseDpad(bool b) { cfg_tellusertousedpad = b; }
	bool getSwapButtons(void) { return cfg_swap_buttons; }
	void setSwapButtons(bool b) { cfg_swap_buttons = b; }
#endif
	int getDifficulty(void) { return cfg_difficulty; }
	void setDifficulty(int d) { cfg_difficulty = d; }
	int getTuning(void) { return cfg_tuning; }
	void setTuning(int t) { cfg_tuning = t; }
#ifdef ALLEGRO_IPHONE
	int getShakeAction(void) { return cfg_shake_action; }
	void setShakeAction(int t) { cfg_shake_action = t; }
#endif
	int getFilterType(void) {
		return cfg_filter_type;
	}
	void setFilterType(int t) {
		cfg_filter_type = t;
		if (!display)
			return;
		set_linear_mag_filter(buffer, t == FILTER_LINEAR);
		if (scaleXX_buffer) {
			set_linear_mag_filter(scaleXX_buffer, t == FILTER_SCALE2X_LINEAR || t == FILTER_SCALE3X_LINEAR);
		}
	}
#ifdef ALLEGRO_IPHONE
	int getAutoRotation(void) {
		return cfg_auto_rotation;
	}
	void setAutoRotation(int auto_rot) {
		cfg_auto_rotation = auto_rot;
		if (display) {
			if (auto_rot == 0)
			{
				al_change_display_option(display, ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_270_DEGREES);
			}
			else if (auto_rot == 1)
			{
				al_change_display_option(display, ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_90_DEGREES);
			}
			else
			{
				al_change_display_option(display, ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE);
			}
		}
		else
		{
			if (auto_rot == 0)
			{
				al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_270_DEGREES, ALLEGRO_REQUIRE);
			}
			else if (auto_rot == 1)
			{
				al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_90_DEGREES, ALLEGRO_REQUIRE);
			}
			else
			{
				al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_REQUIRE);
			}
		}
	}
#endif
	bool getMaintainAspectRatio(void) {
		return cfg_maintain_aspect_ratio;
	}
	void setMaintainAspectRatio(bool m) {
		cfg_maintain_aspect_ratio = m;
#if !defined ALLEGRO_IPHONE && !defined ALLEGRO_ANDROID
		set_screen_params();
#elif defined ALLEGRO_ANDROID
		if (tguiIsInitialized()) {
#else
		if (tguiIsInitialized() && is_ipad()) {
#endif
			if (m) {
				tguiSetScreenParameters(screen_offset_x, screen_offset_y, 1.0f, 1.0f);
			}
			else {
				tguiSetScreenParameters(0, 0, screen_ratio_x, screen_ratio_y);
			}
#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
		}
#endif
	}
	void setLanguage(int l);
	int getLanguage(void);
	void read();
	void write();
	Configuration();
private:
	ScreenDescriptor wantedMode;	// wanted screen resolution & depth
	bool debugMessages;		// show debugging messages?
	bool compiledScripts;		// use compiled scripts?
	bool waitForVsync;
	bool loaded;	// loaded from file or default?
	bool smoothScaling;
	int sfxVolume;
	int musicVolume;
	int joyB1;
	int joyB2;
	int joyB3;
	int key1;
	int key2;
	int key3;
	int keyLeft;
	int keyRight;
	int keyUp;
	int keyDown;
	bool gamepadAvailable;
	bool onlyMemoryBitmaps;
	bool lowCpuUsage;
	int stick;
	int adapter;
	int axis;
	bool xbox360;
	int cfg_tuning;
	int cfg_difficulty;
	int cfg_filter_type;
	bool cfg_maintain_aspect_ratio;
	int language;
	bool fixed_language;
#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
	int cfg_dpad_type;
	bool cfg_tellusertousedpad;
	int cfg_swap_buttons;
#endif
#ifdef ALLEGRO_IPHONE
	int cfg_shake_action;
	int cfg_auto_rotation;
#endif
};

extern Configuration config;

bool do_config(void);

#endif
