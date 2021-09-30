#pragma once

#include "color.h"



class AimbotTab2 {
public:
	bool enable;
	bool silent;
	bool hitscan;
	bool hsonly;
	bool correct;
	bool lagfix;
	bool hitchance;
	bool ignor_limbs;
	bool lethal;
	bool running;
	bool inair;
	bool spointscale;
	float		  scale;
	float		  body_scale;
	int minimal_damage;
	int hitchance_amount;
	int hitchance_noscope_amount;
	bool hitchance_noscope;
	bool penetration;
	int penetrate_minimal_damage;
	int zoom;
	bool knifebot;
	bool zeusbot;
	bool norecoil;
	int inverter_key;
};

class VisualsTab2 {
public:
	// players col 1
	bool enable;
	bool box;
	bool dormant;
	bool gradientesp;
	bool offscreen_esp;
	bool name;
	bool skeleton;
	bool health;
	bool flags;
	bool weapon;
	int weapon_type;
	bool ammo;
	bool lby_update;

	Color health_color{ 0, 0, 0, 255 };
	Color name_color{ 0, 0, 0, 255 };
	Color skeleton_color{ 255, 255, 255, 255 };
	Color offscreen_esp_color{ 0, 128, 255, 255 };
	Color gradient_esp_color{ 0, 128, 255, 255 };
	Color color_esp_enemy{ 255, 255, 255, 255 };
	Color ammo_color{ 0, 166, 255, 255 };
	Color lby_color{ 208, 0, 255, 255 };

	// players col 2

	bool glow;
	int chams_selection;
	int chams_material_local;
	int chams_material_enemy;
	int chams_material_friendly;
	int chams_material_history;
	bool chams_local;
	bool chams_enemy;
	bool chams_friendly;
	bool chams_history;
	bool blend_on_scope;
	float blend_value;

	Color glow_color{ 0, 255, 0 };
	Color chams_local_color{ 255, 255, 255, 255 };
	Color chams_enemy_color{ 255, 0, 0, 255 };
	Color chams_friendly_color{ 0, 255, 0, 255 };
	Color chams_history_color{ 0, 0, 0, 100 };

	// visuals col 1

	bool dropped_weapons;
	bool distance_weapons;
	bool dropped_weapons_ammo;
	bool projectiles;
	bool planted_c4;
	bool dont_render_teammates;
	int world_modulation;
	float world_darkness;
	bool transparent_props;
	float transparency_scale;
	bool force_enemies_radar;
	bool override_fog;
	float fog_start;
	float fog_end;
	float fog_density;

	// visuals col 2

	bool remove_vis_recoi;
	bool remove_smoke;
	bool remove_fog;
	bool remove_flash;
	bool remove_scope;
	bool override_fov;
	float fov;
	bool override_fov_scope;
	bool override_viewmodel_fov;
	float view_fov;
	bool specatators;
	bool force_crosshair;
	bool vis_spread;
	bool pen_crosshair;
	bool pen_crosshair_damage;
	bool indicators;
	bool keybind_status;
	bool grenade_prediction;
	bool impact_beams;
	float impact_time;
	float thirdperson_distance;

	Color distance_weapons_color {255,255,255,255};
	Color dropped_weapons_ammo_color{ 0,255,255,255 };
	Color projectiles_color{ 255,255,255,255 };
	Color world_color{ 100,100,100,255 };
	Color spread_color{ 255,255,255,255 };
	Color impacts_color{ 0,100,255,255 };
	Color impacts_color_hurt{ 255,0,0,255 };
	//OPTION(Color, color_esp_enemy, Color(0, 128, 255));
};

class MiscTab2 {
public:
	bool hitmarker;
	bool unlockinv;
	bool skyboxchange;
	bool killsound;
	bool ragdoll;
	bool revealrank;
	bool preservekillfeed;
	bool clantagchanger;
	bool slidewalk;
	bool watermark;
	bool ratiochanger;
	bool logmisses;
	int skyvalue;
	int hitsound;
	float ratio;
};

class MainForm2 {
public:
	// aimbot.
	AimbotTab2    aimbot;
	VisualsTab2	  visuals;
	MiscTab2	  misc;
};

class Menu2 {
public:
	MainForm2 main;
};

extern Menu2 g_menu2;