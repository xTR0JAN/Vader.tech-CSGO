#include "includes.h"

// execution callbacks..
void callbacks::SkinUpdate() {
	g_skins.m_update = true;
}

void callbacks::ForceFullUpdate() {
	//static DWORD tick{};
	//
	//if (tick != g_winapi.GetTickCount()) {
	//	g_csgo.cl_fullupdate->m_callback();
	//	tick = g_winapi.GetTickCount();
	//}
	//

	g_csgo.m_cl->m_delta_tick = -1;
}

void callbacks::ToggleThirdPerson() {
	g_visuals.m_thirdperson = !g_visuals.m_thirdperson;
}

void callbacks::ToggleLeft() {
	g_hvh.m_left = !g_hvh.m_left;
	g_hvh.m_right = false;
	g_hvh.m_back = false;
}

void callbacks::ToggleRight() {
	g_hvh.m_right = !g_hvh.m_right;
	g_hvh.m_left = false;
	g_hvh.m_back = false;
}

void callbacks::ToggleBack() {
	g_hvh.m_back = !g_hvh.m_back;
	g_hvh.m_left = false;
	g_hvh.m_right = false;
}

void callbacks::ToggleFakeLatency() {
	g_aimbot.m_fake_latency = !g_aimbot.m_fake_latency;
}

void callbacks::ToggleKillfeed() {
	KillFeed_t* feed = (KillFeed_t*)g_csgo.m_hud->FindElement(HASH("SFHudDeathNoticeAndBotStatus"));
	if (feed)
		g_csgo.ClearNotices(feed);
}

void callbacks::SaveHotkeys() {
	g_config.SaveHotkeys();
}

void callbacks::ConfigLoad1() {
	g_aimbot.m_fake_latency = false;
	g_config.load(&g_menu.main, XOR("1.tech"));
	g_menu.main.config.config.select(1 - 1);

	g_cl.print(tfm::format(XOR("loaded config 1\n")));
}

void callbacks::ConfigLoad2() {
	g_aimbot.m_fake_latency = false;
	g_config.load(&g_menu.main, XOR("2.tech"));
	g_menu.main.config.config.select(2 - 1);
	g_cl.print(tfm::format(XOR("loaded config 2\n")));
}

void callbacks::ConfigLoad3() {
	g_aimbot.m_fake_latency = false;
	g_config.load(&g_menu.main, XOR("3.tech"));
	g_menu.main.config.config.select(3 - 1);
	g_cl.print(tfm::format(XOR("loaded config 3\n")));
}

void callbacks::ConfigLoad4() {
	g_aimbot.m_fake_latency = false;
	g_config.load(&g_menu.main, XOR("4.tech"));
	g_menu.main.config.config.select(4 - 1);
	g_cl.print(tfm::format(XOR("loaded config 4\n")));
}

void callbacks::ConfigLoad5() {
	g_aimbot.m_fake_latency = false;
	g_config.load(&g_menu.main, XOR("5.tech"));
	g_menu.main.config.config.select(5 - 1);
	g_cl.print(tfm::format(XOR("loaded config 5\n")));
}

void callbacks::ConfigLoad6() {
	g_aimbot.m_fake_latency = false;
	g_config.load(&g_menu.main, XOR("6.tech"));
	g_menu.main.config.config.select(6 - 1);
	g_cl.print(tfm::format(XOR("loaded config 6\n")));
}

void callbacks::ConfigLoad() {
	g_aimbot.m_fake_latency = false;
	std::string config = g_menu.main.config.config.GetActiveItem();
	std::string file = tfm::format(XOR("%s.tech"), config.data());

	g_config.load(&g_menu.main, file);
	g_cl.print(tfm::format(XOR("loaded config %s\n"), config.data()));
}

void callbacks::ConfigSave() {
	g_aimbot.m_fake_latency = false;
	std::string config = g_menu.main.config.config.GetActiveItem();
	std::string file = tfm::format(XOR("%s.tech"), config.data());

	g_config.save(&g_menu.main, file);
	g_cl.print(tfm::format(XOR("saved config %s\n"), config.data()));
}

void callbacks::HiddenCvar() {
	g_cl.UnlockHiddenConvars();
}

void callbacks::JoinServer()
{
	g_cl.ServerSelection();
}

bool callbacks::IsBaimHealth() {
	return g_menu.main.aimbot.baim2.get(1);
}

bool callbacks::IsAfterMisses() {
	return (g_menu.main.aimbot.baim2.get(4) || g_menu.main.aimbot.baim1.get(5));
}

void callbacks::ToggleDT() {
	g_aimbot.m_double_tap = !g_aimbot.m_double_tap;
}

void callbacks::ToggleAATest() {
	g_aimbot.aaTest = !g_aimbot.aaTest;
}

bool callbacks::IsFovOn() {
	return g_menu.main.aimbot.fov.get();
}

bool callbacks::IsHitchanceOn() {
	return g_menu.main.aimbot.hitchance.get();
}

bool callbacks::IsHitchanceNoscopeOn() {
	return g_menu.main.aimbot.hitchance_noscope.get();
}

bool callbacks::IsPenetrationOn() {
	return g_menu.main.aimbot.penetrate.get();
}

// menu elements
bool callbacks::IsZeusBot() {
	return g_menu.main.aimbot.zeusbot.get();
}

bool callbacks::IsMultipointOn() {
	return !g_menu.main.aimbot.multipoint.GetActiveIndices().empty();
}

bool callbacks::IsMultipointBodyOn() {
	return g_menu.main.aimbot.multipoint.get(2);
}

bool callbacks::GeneralCategory() {
	return g_menu.main.aimbot.weaponselect.get() == 0;
}

bool callbacks::GeneralHeadScale() {
	return g_menu.main.aimbot.general_multipoint.get(0);
}

bool callbacks::GeneralBodyScale() {
	return g_menu.main.aimbot.general_multipoint.get(1) ||
		g_menu.main.aimbot.general_multipoint.get(2) ||
		g_menu.main.aimbot.general_multipoint.get(3);
}

bool callbacks::GeneralAutowall() {
	return g_menu.main.aimbot.general_autowall.get();
}

bool callbacks::GeneralHitchance() {
	return g_menu.main.aimbot.general_hitchance.get();
}

bool callbacks::GeneralForceHealth() {
	return g_menu.main.aimbot.general_forcebaim.get(1);
}

// pistol
bool callbacks::PistolCategory() {
	return g_menu.main.aimbot.weaponselect.get() == 1;
}

bool callbacks::PistolHeadScale() {
	return g_menu.main.aimbot.pistol_multipoint.get(0);
}

bool callbacks::PistolBodyScale() {
	return g_menu.main.aimbot.pistol_multipoint.get(1) ||
		g_menu.main.aimbot.pistol_multipoint.get(2) ||
		g_menu.main.aimbot.pistol_multipoint.get(3);
}

bool callbacks::PistolAutowall() {
	return g_menu.main.aimbot.pistol_autowall.get();
}

bool callbacks::PistolHitchance() {
	return g_menu.main.aimbot.pistol_hitchance.get();
}

bool callbacks::PistolForceHealth() {
	return g_menu.main.aimbot.pistol_forcebaim.get(1);
}

// heavy pistol
bool callbacks::HeavyCategory() {
	return g_menu.main.aimbot.weaponselect.get() == 2;
}

bool callbacks::HeavyHeadScale() {
	return g_menu.main.aimbot.heavypistol_multipoint.get(0);
}

bool callbacks::HeavyBodyScale() {
	return g_menu.main.aimbot.heavypistol_multipoint.get(1) ||
		g_menu.main.aimbot.heavypistol_multipoint.get(2) ||
		g_menu.main.aimbot.heavypistol_multipoint.get(3);
}

bool callbacks::HeavyAutowall() {
	return g_menu.main.aimbot.heavypistol_autowall.get();
}

bool callbacks::HeavyHitchance() {
	return g_menu.main.aimbot.heavypistol_hitchance.get();
}

bool callbacks::HeavyForceHealth() {
	return g_menu.main.aimbot.heavypistol_forcebaim.get(1);
}

// scout
bool callbacks::ScoutCategory() {
	return g_menu.main.aimbot.weaponselect.get() == 3;
}

bool callbacks::ScoutHeadScale() {
	return g_menu.main.aimbot.scout_multipoint.get(0);
}

bool callbacks::ScoutBodyScale() {
	return g_menu.main.aimbot.scout_multipoint.get(1) ||
		g_menu.main.aimbot.scout_multipoint.get(2) ||
		g_menu.main.aimbot.scout_multipoint.get(3);
}

bool callbacks::ScoutAutowall() {
	return g_menu.main.aimbot.scout_autowall.get();
}

bool callbacks::ScoutHitchance() {
	return g_menu.main.aimbot.scout_hitchance.get();
}

bool callbacks::ScoutForceHealth() {
	return g_menu.main.aimbot.scout_forcebaim.get(1);
}

// auto
bool callbacks::AutoCategory() {
	return g_menu.main.aimbot.weaponselect.get() == 4;
}

bool callbacks::AutoHeadScale() {
	return g_menu.main.aimbot.auto_multipoint.get(0);
}

bool callbacks::AutoBodyScale() {
	return g_menu.main.aimbot.auto_multipoint.get(1) ||
		g_menu.main.aimbot.auto_multipoint.get(2) ||
		g_menu.main.aimbot.auto_multipoint.get(3);
}

bool callbacks::AutoAutowall() {
	return g_menu.main.aimbot.auto_autowall.get();
}

bool callbacks::AutoHitchance() {
	return g_menu.main.aimbot.auto_hitchance.get();
}

bool callbacks::AutoForceHealth() {
	return g_menu.main.aimbot.auto_forcebaim.get(1);
}

// awp
bool callbacks::AwpCategory() {
	return g_menu.main.aimbot.weaponselect.get() == 5;
}

bool callbacks::AwpHeadScale() {
	return g_menu.main.aimbot.awp_multipoint.get(0);
}

bool callbacks::AwpBodyScale() {
	return g_menu.main.aimbot.awp_multipoint.get(1) ||
		g_menu.main.aimbot.awp_multipoint.get(2) ||
		g_menu.main.aimbot.awp_multipoint.get(3);
}

bool callbacks::AwpAutowall() {
	return g_menu.main.aimbot.awp_autowall.get();
}

bool callbacks::AwpHitchance() {
	return g_menu.main.aimbot.awp_hitchance.get();
}

bool callbacks::AwpForceHealth() {
	return g_menu.main.aimbot.awp_forcebaim.get(1);
}

bool callbacks::IsAntiAimModeStand() {
	return g_menu.main.antiaim.mode.get() == 0;
}

bool callbacks::HasStandYaw() {
	return g_menu.main.antiaim.yaw_stand.get() > 0;
}

bool callbacks::IsStandYawJitter() {
	return g_menu.main.antiaim.yaw_stand.get() == 2;
}

bool callbacks::IsStandYawRotate() {
	return g_menu.main.antiaim.yaw_stand.get() == 3;
}

bool callbacks::IsStandYawRnadom() {
	return g_menu.main.antiaim.yaw_stand.get() == 4;
}

bool callbacks::IsStandDirAuto() {
	return g_menu.main.antiaim.dir_stand.get() == 0;
}

bool callbacks::IsStandDirCustom() {
	return g_menu.main.antiaim.dir_stand.get() == 4;
}

bool callbacks::IsAntiAimModeWalk() {
	return g_menu.main.antiaim.mode.get() == 1;
}

bool callbacks::WalkHasYaw() {
	return g_menu.main.antiaim.yaw_walk.get() > 0;
}

bool callbacks::IsWalkYawJitter() {
	return g_menu.main.antiaim.yaw_walk.get() == 2;
}

bool callbacks::IsWalkYawRotate() {
	return g_menu.main.antiaim.yaw_walk.get() == 3;
}

bool callbacks::IsWalkYawRandom() {
	return g_menu.main.antiaim.yaw_walk.get() == 4;
}

bool callbacks::IsWalkDirAuto() {
	return g_menu.main.antiaim.dir_walk.get() == 0;
}

bool callbacks::IsWalkDirCustom() {
	return g_menu.main.antiaim.dir_walk.get() == 4;
}

bool callbacks::IsAntiAimModeAir() {
	return g_menu.main.antiaim.mode.get() == 2;
}

bool callbacks::AirHasYaw() {
	return g_menu.main.antiaim.yaw_air.get() > 0;
}

bool callbacks::IsAirYawJitter() {
	return g_menu.main.antiaim.yaw_air.get() == 2;
}

bool callbacks::IsAirYawRotate() {
	return g_menu.main.antiaim.yaw_air.get() == 3;
}

bool callbacks::IsAirYawRandom() {
	return g_menu.main.antiaim.yaw_air.get() == 4;
}

bool callbacks::IsAirDirAuto() {
	return g_menu.main.antiaim.dir_air.get() == 0;
}

bool callbacks::IsAirDirCustom() {
	return g_menu.main.antiaim.dir_air.get() == 4;
}

bool callbacks::IsFakeAntiAimRelative() {
	return g_menu.main.antiaim.fake_yaw.get() == 2;
}

bool callbacks::IsTransparentProps() {
	return g_menu.main.visuals.transparent_props.get();
}

bool callbacks::IsNightMode() {
	return g_menu.main.visuals.world.get() == 1;
}

bool callbacks::IsCustomWorld() {
	return g_menu.main.visuals.world.get() == 3;
}

bool callbacks::IsGradientESP()
{
	return g_menu.main.players.gradient_esp.get();
}

bool callbacks::IsSkyBoxChange() {
	return g_menu.main.misc.skyboxchange.get();
}

bool callbacks::IsDamageVisible() {
	return g_menu.main.aimbot.minimal_damage.get() == 1;
}

bool callbacks::IsCustomLby() {
	return g_menu.main.antiaim.body_fake_stand.get() == 5;
}

bool callbacks::IsDamageInVisible() {
	return g_menu.main.aimbot.penetrate_minimal_damage.get() == 1;
}

bool callbacks::IsFakeAntiAimJitter() {
	return g_menu.main.antiaim.fake_yaw.get() == 3;
}

bool callbacks::IsConfigMM() {
	return g_menu.main.config.mode.get() == 0;
}

bool callbacks::IsStaticMPOn() {
	return g_menu.main.aimbot.static_point_scale.get();
}

bool callbacks::IsConfigNS() {
	return g_menu.main.config.mode.get() == 1;
}

bool callbacks::IsConfig1() {
	return g_menu.main.config.config.get() == 0;
}

bool callbacks::IsConfig2() {
	return g_menu.main.config.config.get() == 1;
}

bool callbacks::IsConfig3() {
	return g_menu.main.config.config.get() == 2;
}

bool callbacks::IsConfig4() {
	return g_menu.main.config.config.get() == 3;
}

bool callbacks::IsConfig5() {
	return g_menu.main.config.config.get() == 4;
}

bool callbacks::IsConfig6() {
	return g_menu.main.config.config.get() == 5;
}

bool callbacks::IsChamsSelection0() {
	return g_menu.main.players.chams_entity_selection.get() == 0;
}

bool callbacks::LocalGlowChamsSelection()
{
	return g_menu.main.players.chams_entity_selection.get() == 0 && g_menu.main.players.chams_local_mat.get() == 4;
}

bool callbacks::IsChamsSelection1() {
	return g_menu.main.players.chams_entity_selection.get() == 1;
}

bool callbacks::EnemyGlowChamsSelection()
{
	return g_menu.main.players.chams_entity_selection.get() == 1 && g_menu.main.players.chams_enemy_mat.get() == 4;
}

bool callbacks::IsChamsSelection2() {
	return g_menu.main.players.chams_entity_selection.get() == 2;
}

bool callbacks::IsChamsSelection3() {
	return g_menu.main.players.chams_entity_selection.get() == 3;
}

// weaponcfgs callbacks.
bool callbacks::DEAGLE() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::DEAGLE;
}

bool callbacks::ELITE() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::ELITE;
}

bool callbacks::FIVESEVEN() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::FIVESEVEN;
}

bool callbacks::GLOCK() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::GLOCK;
}

bool callbacks::AK47() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::AK47;
}

bool callbacks::AUG() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::AUG;
}

bool callbacks::AWP() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::AWP;
}

bool callbacks::FAMAS() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::FAMAS;
}

bool callbacks::G3SG1() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::G3SG1;
}

bool callbacks::GALIL() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::GALIL;
}

bool callbacks::M249() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::M249;
}

bool callbacks::M4A4() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::M4A4;
}

bool callbacks::MAC10() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::MAC10;
}

bool callbacks::P90() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::P90;
}

bool callbacks::UMP45() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::UMP45;
}

bool callbacks::XM1014() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::XM1014;
}

bool callbacks::BIZON() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::BIZON;
}

bool callbacks::MAG7() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::MAG7;
}

bool callbacks::NEGEV() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::NEGEV;
}

bool callbacks::SAWEDOFF() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::SAWEDOFF;
}

bool callbacks::TEC9() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::TEC9;
}

bool callbacks::P2000() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::P2000;
}

bool callbacks::MP7() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::MP7;
}

bool callbacks::MP9() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::MP9;
}

bool callbacks::NOVA() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::NOVA;
}

bool callbacks::P250() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::P250;
}

bool callbacks::SCAR20() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::SCAR20;
}

bool callbacks::SG553() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::SG553;
}

bool callbacks::SSG08() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::SSG08;
}

bool callbacks::M4A1S() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::M4A1S;
}

bool callbacks::USPS() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::USPS;
}

bool callbacks::CZ75A() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::CZ75A;
}

bool callbacks::REVOLVER() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::REVOLVER;
}

bool callbacks::KNIFE_BAYONET() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_BAYONET;
}

bool callbacks::KNIFE_FLIP() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_FLIP;
}

bool callbacks::KNIFE_GUT() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_GUT;
}

bool callbacks::KNIFE_KARAMBIT() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_KARAMBIT;
}

bool callbacks::KNIFE_M9_BAYONET() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_M9_BAYONET;
}

bool callbacks::KNIFE_HUNTSMAN() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_HUNTSMAN;
}

bool callbacks::KNIFE_FALCHION() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_FALCHION;
}

bool callbacks::KNIFE_BOWIE() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_BOWIE;
}

bool callbacks::KNIFE_BUTTERFLY() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_BUTTERFLY;
}

bool callbacks::KNIFE_SHADOW_DAGGERS() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_SHADOW_DAGGERS;
}

bool callbacks::AUTO_STOP() {
	return !g_menu.main.movement.autostop_always_on.get();
}