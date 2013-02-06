#import <GameKit/GameKit.h>

// Game Center stuff
#define NOSOUND
#include "monster2.hpp"
#undef NOSOUND

#ifdef ALLEGRO_MACOSX
#include <allegro5/allegro_osx.h>
#endif

#include "mygamecentervc.h"

int is_authenticated = NOTYET;
NSMutableDictionary *achievementsDictionary;

BOOL isGameCenterAPIAvailable()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	// Check for presence of GKLocalPlayer class.
	BOOL localPlayerClassAvailable = (NSClassFromString(@"GKLocalPlayer")) != nil;

	BOOL osVersionSupported = FALSE;

#ifdef ALLEGRO_IPHONE
	// The device must be running iOS 4.1 or later.
	NSString *reqSysVer = @"4.1";
	NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
	osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
#else
	OSErr err;
	SInt32 systemVersion, versionMajor, versionMinor, versionBugFix;
	if ((err = Gestalt(gestaltSystemVersion, &systemVersion)) == noErr) {
		if (systemVersion >= 0x1080) {
			osVersionSupported = TRUE;
		}
	}
#endif

	[pool drain];
	
	return (localPlayerClassAvailable && osVersionSupported);
}

void loadAchievements(void)
{
	if (!isGameCenterAPIAvailable() || !is_authenticated)
		return;

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	achievementsDictionary = [[NSMutableDictionary alloc] init];
	[GKAchievement loadAchievementsWithCompletionHandler:^(NSArray *achievements, NSError *error)
	 {
		 if (error == nil)
		 {
			 for (GKAchievement* achievement in achievements) {
				 [achievementsDictionary setObject: achievement forKey: achievement.identifier];
			 }
		 }
	 }];
	 
	 [pool drain];
}

#define NUM_ACHIEVEMENTS 30

NSString *achievements_backlog[NUM_ACHIEVEMENTS];
int num_backlog_achievements = 0;

void reportAchievementIdentifier(NSString* identifier, bool notification);

void authenticatePlayer(void)
{
	if (!isGameCenterAPIAvailable()) {
		is_authenticated = 0;
		return;
	}
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	GKLocalPlayer *localPlayer = [GKLocalPlayer localPlayer];

	[localPlayer authenticateWithCompletionHandler:^(NSError *error) {
		if (localPlayer.isAuthenticated)
		{
			// Perform additional tasks for the authenticated player.
			is_authenticated = 1;
			loadAchievements();
			int i;
			int n = num_backlog_achievements;
			for (i = 0; i < n; i++) {
				NSString *s = achievements_backlog[0];
				int j;
				for (j = 1; j < num_backlog_achievements; j++) {
					achievements_backlog[j-1] = achievements_backlog[j];
				}
				num_backlog_achievements--;
				reportAchievementIdentifier(s, false);
				[s release];
			}
		}
		else {
			is_authenticated = 0;
		}
	}];
	
	[pool drain];
}

bool reset_complete = false;

void resetAchievements(void)
{
	if (!isGameCenterAPIAvailable() || !is_authenticated)
		return;

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	// Clear all locally saved achievement objects.
	achievementsDictionary = [[NSMutableDictionary alloc] init];

	// Clear all progress saved on Game Center
	[GKAchievement resetAchievementsWithCompletionHandler:^(NSError *error)
	 {
		 if (error != nil) {
			 // handle errors
		 }
		 reset_complete = true;
	 }];
	 
	 [pool drain];
}

void reportAchievementIdentifier(NSString* identifier, bool notification)
{
	if (!isGameCenterAPIAvailable() || !is_authenticated)
		return;

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if ([achievementsDictionary objectForKey:identifier] != nil) {
		[pool drain];
		return;
	}
	// check the backlog!
	int i;
	for (i = 0; i < num_backlog_achievements; i++) {
		if (NSOrderedSame == [achievements_backlog[i] compare:identifier]) {
			// already there
			[pool drain];
			return;
		}
	}
	
	float percent = 100;
	
	GKAchievement *achievement = [[[GKAchievement alloc] initWithIdentifier: identifier] autorelease];
	if (achievement)
	{
		[achievementsDictionary setObject:achievement forKey:identifier];
		achievement.percentComplete = percent;
		[achievement reportAchievementWithCompletionHandler:^(NSError *error)
		 {
			 if (error != nil)
			 {
				 // Retain the achievement object and try again later (not shown).
				 if (num_backlog_achievements < NUM_ACHIEVEMENTS) {
					 achievements_backlog[num_backlog_achievements] = [[NSString alloc] initWithString:identifier];
					 num_backlog_achievements++;
				 }
			 }
		 }];
	}
	
	[pool drain];
}

struct Holder
{
	int num;
	NSString *ident;
};

void do_milestone(int num)
{
	num++;

	Holder holders[NUM_ACHIEVEMENTS] = {
		{ 3, MS_REMOVED_SQUEEKY_BOARDS_3 },
		{ 15, MS_RIDER_JOINED_15 },
		{ 20, MS_DEFEATED_MONSTER_20 },
		{ 26, MS_GOT_BADGE_26 },
		{ 30, MS_RIOS_JOINED_30 },
		{ 40, MS_GUNNAR_JOINED_40 },
		{ 43, MS_GOT_RING_43 },
		{ 48, MS_BEAT_WITCH_48 },
		{ 56, MS_KILLED_GOLEMS_56 },
		{ 59, MS_GOT_KEY_59 },
		{ 65, MS_GOT_MEDALLION_65 },
		{ 67, MS_BEACH_BATTLE_DONE_67 },
		{ 74, MS_GOT_MILK_74 },
		{ 76, MS_SUB_SCENE_DONE_76 },
		{ 89, MS_GOT_LOOKING_SCOPE_89 },
		{ 87, MS_DRAINED_POOL_87 },
		{ 96, MS_BEAT_TIGGY_96 },
		{ 102, MS_FREED_PRISONER_102 },
		{ 98, MS_BEAT_ARCHERY_98 },
		{ 123, MS_GOT_STAFF_123 },
		{ 180, MS_FOREST_GOLD_180 },
		{ 135, MS_BEAT_TREE_135 },
		{ 149, MS_BEAT_GIRL_DRAGON_149 },
		{ 153, MS_ON_MOON_153 },
		{ 154, MS_TIPPER_JOINED_154 },
		{ 167, MS_GOT_ORB_167 },
		{ 168, MS_MRBIG_CHEST_168 },
		{ 171, MS_BEAT_TODE_171 },
		{ 176, MS_SUN_SCENE_176 },
		{ 177, MS_DONE_CREDITS_177 }
	};

	for (int i = 0; i < NUM_ACHIEVEMENTS; i++)
	{
		if (holders[i].num == num) {
			reportAchievementIdentifier(holders[i].ident, true);
			achievement_time = al_get_time();
			achievement_show = true;
			return;
		}
	}
}

volatile bool modalViewShowing = false;

void showAchievements(void)
{
// FIXME: Do OS X
#ifdef ALLEGRO_IPHONE
	MyGameCenterVC *uv = [[MyGameCenterVC alloc] initWithNibName:nil bundle:nil];
	[uv performSelectorOnMainThread: @selector(showAchievements) withObject:nil waitUntilDone:YES];
	while (modalViewShowing) {
		al_rest(0.001);
	}
	[uv release];
	
	ALLEGRO_DISPLAY *d = airplay_connected ? controller_display : display;
	
	UIWindow *window = al_iphone_get_window(d);
	UIView *view = al_iphone_get_view(d);
	[window bringSubviewToFront:view];
#else

	modalViewShowing = true;
	al_show_mouse_cursor(display);
	MyGameCenterVC *vc = [[MyGameCenterVC alloc] init];
	[vc performSelectorOnMainThread: @selector(showAchievements) withObject:nil waitUntilDone:FALSE];
	while (modalViewShowing) {
		al_rest(0.001);
	}
	[vc release];
	al_hide_mouse_cursor(display);

#endif
}
