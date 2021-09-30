#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include <d3d9.h>
#include <d3dx9.h>

#include "../options.h"

#include "Fonts.h"
#include <chrono>
#include "retard.h"
#include <imgui.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include "../color.h"
namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip)) {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
            return true;
        }
        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

#define InsertSliderInt(x1,x2,x3,x4,x5) ImGui::NewLine(); ImGui::PushItemWidth(159.f); ImGui::SliderInt(x1, x2, x3, x4, x5); ImGui::PopItemWidth();
#define InsertSliderFloat(x1,x2,x3,x4,x5) ImGui::NewLine(); ImGui::PushItemWidth(159.f); ImGui::SliderFloat(x1, x2, x3, x4, x5); ImGui::PopItemWidth();
#define InsertCombo(x1,x2,x3,x4) ImGui::NewLine(); ImGui::PushItemWidth(159.f); style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0); ImGui::Combo(x1, x2, x3, x4); style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0); ImGui::PopItemWidth();
#define InsertCheckbox(x1, x2, x3) static Checkbox x1; x1.Draw(x2,x3);

 
static int tab = 0;
static int aimbotTab = 1;
static int rageTab = 0;
static int legitTab = 0;
static int visualsTab = 0;

static Checkbox emptyc;

bool PlaceHolderEmptyFUckingPieceOFsSHIT = true;
#define InsertEmpty() emptyc.Draw("##", &PlaceHolderEmptyFUckingPieceOFsSHIT);

bool testbox = false, testbox1 = false;
void Legitbot()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	//ImGui::BeginGroupBox("Legitbot", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();


}
void Ragebot()
{
	//autosniper, sniper, pistol, heavy pistol, other

	ImGuiStyle* style = &ImGui::GetStyle();
	//ImGui::BeginGroupBox("Ragebot", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();

	InsertCheckbox(Ragebot, "Enable", &g_menu2.main.aimbot.enable);
	if (g_menu2.main.aimbot.enable) {
		const char* auto_scope[]{ "Off","Always","Hitchance Fail" };

		InsertCheckbox(Silent, "Silent Aim", &g_menu2.main.aimbot.silent);

		InsertCheckbox(Hitscan, "Hitscan", &g_menu2.main.aimbot.hitscan);

		InsertCheckbox(HSOnly, "Head Shot Only", &g_menu2.main.aimbot.hsonly);

		InsertCheckbox(Resolver, "Anti-Aim Correction", &g_menu2.main.aimbot.correct);

		InsertCheckbox(FakelagPred, "Fake-Lag Prediction", &g_menu2.main.aimbot.lagfix);

		InsertCheckbox(IgnoreLimbs, "Ignore Limbs When Moving", &g_menu2.main.aimbot.ignor_limbs);

		InsertCheckbox(KnifeBot, "Knifebot", &g_menu2.main.aimbot.knifebot);

		InsertCheckbox(ZeusBot, "Zeusbot", &g_menu2.main.aimbot.zeusbot);

		InsertCheckbox(CompRecoil, "Compensate Recoil", &g_menu2.main.aimbot.norecoil);

		ImGui::NextColumn();
		//InsertEmpty();
		ImGui::NewLine();
		switch (rageTab)
		{
		case 0:
			//ImGui::Text("Config - Autosniper");
			InsertCheckbox(Hitchance, "Hitchance", &g_menu2.main.aimbot.hitchance);
			if (g_menu2.main.aimbot.hitchance) {
				InsertSliderInt("Hitchance", &g_menu2.main.aimbot.hitchance_amount, 1, 100, "%1.f%");
			}

			InsertCheckbox(Noscope_Hitchance, "No-Scope Hitchance", &g_menu2.main.aimbot.hitchance_noscope);
			if (g_menu2.main.aimbot.hitchance_noscope) {
				InsertSliderInt("Noscope_Hitchance", &g_menu2.main.aimbot.hitchance_noscope_amount, 1, 100, "%1.f%");
			}

			InsertSliderInt("Minimal Damage", &g_menu2.main.aimbot.minimal_damage, 1, 100, "%1.f%");

			InsertCheckbox(WallPen, "Autowall", &g_menu2.main.aimbot.penetration);

			if (g_menu2.main.aimbot.penetration) {
				InsertSliderInt("Autowall Minimal Damage", &g_menu2.main.aimbot.penetrate_minimal_damage, 1, 100, "%1.f%");
			}

			InsertCombo("Auto Scope", &g_menu2.main.aimbot.zoom, auto_scope, IM_ARRAYSIZE(auto_scope));

			ImGui::NextColumn();
			//InsertEmpty();
			ImGui::NewLine();
			InsertCheckbox(SPointScale, "Static Point Scale", &g_menu2.main.aimbot.spointscale);

			if (g_menu2.main.aimbot.spointscale) {
				InsertSliderFloat("Head Scale", &g_menu2.main.aimbot.scale, 0, 100, "%1.f%");
				InsertSliderFloat("Body Scale", &g_menu2.main.aimbot.body_scale, 0, 100, "%1.f%");
			}

			InsertCheckbox(PreferLethal, "Prefer Body Aim if Lethal", &g_menu2.main.aimbot.lethal);
			InsertCheckbox(PreferInAir, "Prefer Body Aim in Air", &g_menu2.main.aimbot.inair);
			InsertCheckbox(PreferWhenRun, "Prefer Head Aim if Running", &g_menu2.main.aimbot.running);

			//ImGui::Text("Hitboxes - Autosniper");
			//InsertSliderFloat("Head Scale", g_Options.ragebot_auto_head_scale, 1, 100, "%1.f%");
			//InsertSliderFloat("Body Scale", g_Options.ragebot_auto_body_scale, 1, 100, "%1.f%");
			//InsertCombo("Hitscan", g_Options.ragebot_auto_hitscan, hitscan_array, IM_ARRAYSIZE(hitscan_array));
			break;
		case 1:
			InsertCheckbox(Hitchance1, "Hitchance", &g_menu2.main.aimbot.hitchance);
			if (g_menu2.main.aimbot.hitchance) {
				InsertSliderInt("Hitchance", &g_menu2.main.aimbot.hitchance_amount, 1, 100, "%1.f%");
			}

			InsertCheckbox(Noscope_Hitchance1, "No-Scope Hitchance", &g_menu2.main.aimbot.hitchance_noscope);
			if (g_menu2.main.aimbot.hitchance_noscope) {
				InsertSliderInt("Noscope_Hitchance", &g_menu2.main.aimbot.hitchance_noscope_amount, 1, 100, "%1.f%");
			}

			InsertSliderInt("Minimal Damage", &g_menu2.main.aimbot.minimal_damage, 1, 100, "%1.f%");

			InsertCheckbox(WallPen1, "Autowall", &g_menu2.main.aimbot.penetration);

			if (g_menu2.main.aimbot.penetration) {
				InsertSliderInt("Autowall Minimal Damage", &g_menu2.main.aimbot.penetrate_minimal_damage, 1, 100, "%1.f%");
			}

			InsertCombo("Auto Scope", &g_menu2.main.aimbot.zoom, auto_scope, IM_ARRAYSIZE(auto_scope));

			ImGui::NextColumn();
			//InsertEmpty();
			ImGui::NewLine();
			InsertCheckbox(SPointScale1, "Static Point Scale", &g_menu2.main.aimbot.spointscale);

			if (g_menu2.main.aimbot.spointscale) {
				InsertSliderFloat("Head Scale", &g_menu2.main.aimbot.scale, 0, 100, "%1.f%");
				InsertSliderFloat("Body Scale", &g_menu2.main.aimbot.body_scale, 0, 100, "%1.f%");
			}

			InsertCheckbox(PreferLethal1, "Prefer Body Aim if Lethal", &g_menu2.main.aimbot.lethal);
			InsertCheckbox(PreferInAir1, "Prefer Body Aim in Air", &g_menu2.main.aimbot.inair);
			InsertCheckbox(PreferWhenRun1, "Prefer Head Aim if Running", &g_menu2.main.aimbot.running);
			break;
		case 2:
			InsertCheckbox(Hitchance2, "Hitchance", &g_menu2.main.aimbot.hitchance);
			if (g_menu2.main.aimbot.hitchance) {
				InsertSliderInt("Hitchance", &g_menu2.main.aimbot.hitchance_amount, 1, 100, "%1.f%");
			}

			InsertCheckbox(Noscope_Hitchance2, "No-Scope Hitchance", &g_menu2.main.aimbot.hitchance_noscope);
			if (g_menu2.main.aimbot.hitchance_noscope) {
				InsertSliderInt("Noscope_Hitchance", &g_menu2.main.aimbot.hitchance_noscope_amount, 1, 100, "%1.f%");
			}

			InsertSliderInt("Minimal Damage", &g_menu2.main.aimbot.minimal_damage, 1, 100, "%1.f%");

			InsertCheckbox(WallPen2, "Autowall", &g_menu2.main.aimbot.penetration);

			if (g_menu2.main.aimbot.penetration) {
				InsertSliderInt("Autowall Minimal Damage", &g_menu2.main.aimbot.penetrate_minimal_damage, 1, 100, "%1.f%");
			}

			InsertCombo("Auto Scope", &g_menu2.main.aimbot.zoom, auto_scope, IM_ARRAYSIZE(auto_scope));

			ImGui::NextColumn();
			//InsertEmpty();
			ImGui::NewLine();
			InsertCheckbox(SPointScale2, "Static Point Scale", &g_menu2.main.aimbot.spointscale);

			if (g_menu2.main.aimbot.spointscale) {
				InsertSliderFloat("Head Scale", &g_menu2.main.aimbot.scale, 0, 100, "%1.f%");
				InsertSliderFloat("Body Scale", &g_menu2.main.aimbot.body_scale, 0, 100, "%1.f%");
			}

			InsertCheckbox(PreferLethal2, "Prefer Body Aim if Lethal", &g_menu2.main.aimbot.lethal);
			InsertCheckbox(PreferInAi2r, "Prefer Body Aim in Air", &g_menu2.main.aimbot.inair);
			InsertCheckbox(PreferWhenRun2, "Prefer Head Aim if Running", &g_menu2.main.aimbot.running);
			break;
		case 3:
			InsertCheckbox(Hitchance3, "Hitchance", &g_menu2.main.aimbot.hitchance);
			if (g_menu2.main.aimbot.hitchance) {
				InsertSliderInt("Hitchance", &g_menu2.main.aimbot.hitchance_amount, 1, 100, "%1.f%");
			}

			InsertCheckbox(Noscope_Hitchance3, "No-Scope Hitchance", &g_menu2.main.aimbot.hitchance_noscope);
			if (g_menu2.main.aimbot.hitchance_noscope) {
				InsertSliderInt("Noscope_Hitchance", &g_menu2.main.aimbot.hitchance_noscope_amount, 1, 100, "%1.f%");
			}

			InsertSliderInt("Minimal Damage", &g_menu2.main.aimbot.minimal_damage, 1, 100, "%1.f%");

			InsertCheckbox(WallPen3, "Autowall", &g_menu2.main.aimbot.penetration);

			if (g_menu2.main.aimbot.penetration) {
				InsertSliderInt("Autowall Minimal Damage", &g_menu2.main.aimbot.penetrate_minimal_damage, 1, 100, "%1.f%");
			}

			InsertCombo("Auto Scope", &g_menu2.main.aimbot.zoom, auto_scope, IM_ARRAYSIZE(auto_scope));

			ImGui::NextColumn();
			//InsertEmpty();
			ImGui::NewLine();
			InsertCheckbox(SPointScale3, "Static Point Scale", &g_menu2.main.aimbot.spointscale);

			if (g_menu2.main.aimbot.spointscale) {
				InsertSliderFloat("Head Scale", &g_menu2.main.aimbot.scale, 0, 100, "%1.f%");
				InsertSliderFloat("Body Scale", &g_menu2.main.aimbot.body_scale, 0, 100, "%1.f%");
			}

			InsertCheckbox(PreferLethal3, "Prefer Body Aim if Lethal", &g_menu2.main.aimbot.lethal);
			InsertCheckbox(PreferInAir3, "Prefer Body Aim in Air", &g_menu2.main.aimbot.inair);
			InsertCheckbox(PreferWhenRun3, "Prefer Head Aim if Running", &g_menu2.main.aimbot.running);
			break;

		}
		ImGui::EndColumns();
	}
	
}

void biggestMeme()
{
	ImGui::NewLine();
	ImGui::NewLine();
}

void biggestMeme2()
{
	ImGui::Text("    ");
	ImGui::SameLine();
}

void Antiaim()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	const char* styles[]{ "Static","Jitter","Spin" };

	//ImGui::BeginGroupBox("Antiaim", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);

	ImGui::NewLine();

	ImGui::Text("Key");
	biggestMeme2();
	ImGui::Hotkey("##Invert", &g_menu2.main.aimbot.inverter_key, ImVec2{ 60,20 });

	//ImGui::Text("Inverter Key");
	//ImGui::Hotkey("##Invert", g_Options.inverter_key, ImVec2{ 159,20 });

	ImGui::EndColumns();
	//	ImGui::EndGroupBox();
}

void Visuals()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	const char* chams_selection[]{ "Local","Enemy","Friendly","History" };
	const char* chams_material[]{ "Material","Flat","Metallic","Shaded", "Glow" };
	const char* chams_material_local[]{ "Material","Flat","Metallic","Shaded", "Glow", "Stick Man" };

	//ImGui::BeginGroupBox("Visuals", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);

	switch (visualsTab ) {
	case 0:
		InsertCheckbox(Visuals, "Enable", &g_menu2.main.visuals.enable);
		ImGui::NewLine();
		if (g_menu2.main.visuals.enable)
		{
			const char* type_shit[]{ "Icon","Text" };
			ImGui::Text("Render");

			InsertCheckbox(Boxes, "Boxes", &g_menu2.main.visuals.box);
			if (g_menu2.main.visuals.box) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##Boxes", &g_menu2.main.visuals.color_esp_enemy, true);
			}
			InsertCheckbox(Name, "Name ESP", &g_menu2.main.visuals.name);
			if (g_menu2.main.visuals.name) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##Name ESP", &g_menu2.main.visuals.name_color, true);
			}
			InsertCheckbox(Skeleton, "Skeleton ESP", &g_menu2.main.visuals.skeleton);
			if (g_menu2.main.visuals.skeleton) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##Skeleton ESP", &g_menu2.main.visuals.skeleton_color, true);
			}
			InsertCheckbox(Dormant, "Dormant ESP", &g_menu2.main.visuals.dormant);
			InsertCheckbox(GradESP, "Gradient ESP", &g_menu2.main.visuals.gradientesp);
			if (g_menu2.main.visuals.gradientesp) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##Gradient ESP", &g_menu2.main.visuals.gradient_esp_color, true);
			}
			InsertCheckbox(OffScreenESP, "Offscreen Arrows", &g_menu2.main.visuals.offscreen_esp);
			if (g_menu2.main.visuals.offscreen_esp) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##Offscreen Arrows", &g_menu2.main.visuals.offscreen_esp_color, true);
			}
			InsertCheckbox(Health, "Health Bar", &g_menu2.main.visuals.health);
			if (g_menu2.main.visuals.health) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##Health Bar", &g_menu2.main.visuals.health_color, true);
			}
			InsertCheckbox(Flags, "Important Flags", &g_menu2.main.visuals.flags);
			InsertCheckbox(Weapon, "Weapon ESP", &g_menu2.main.visuals.weapon);
			if (g_menu2.main.visuals.weapon) {
				InsertCombo("Weapon ESP Type", &g_menu2.main.visuals.weapon_type, type_shit, IM_ARRAYSIZE(type_shit));
			}

			InsertCheckbox(Ammo, "Ammo ESP", &g_menu2.main.visuals.ammo);
			if (g_menu2.main.visuals.ammo) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##Ammo ESP", &g_menu2.main.visuals.ammo_color, true);
			}

			InsertCheckbox(LBYTimer, "LBY Timer", &g_menu2.main.visuals.lby_update);
			if (g_menu2.main.visuals.lby_update) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##LBY Timer", &g_menu2.main.visuals.lby_color, true);
			}

			ImGui::NextColumn();
			biggestMeme();

			ImGui::Text("Players");

			InsertCheckbox(Glow , "Glow ESP", &g_menu2.main.visuals.glow);
			if (g_menu2.main.visuals.glow) {
				ImGui::SameLine(150.f);
				ImGuiEx::ColorEdit4("##Glow ESP", &g_menu2.main.visuals.glow_color, true);
			}

			InsertCombo("Chams selection", &g_menu2.main.visuals.chams_selection, chams_selection, IM_ARRAYSIZE(chams_selection));
			switch (g_menu2.main.visuals.chams_selection) {
			case 0:
				InsertCheckbox(LocalChams, "Chams Local", &g_menu2.main.visuals.chams_local);
				if (g_menu2.main.visuals.chams_local) {
					ImGui::SameLine(150.f);
					ImGuiEx::ColorEdit4("##Chams Local", &g_menu2.main.visuals.chams_local_color, true);
					InsertCombo("Chams material", &g_menu2.main.visuals.chams_material_local, chams_material_local, IM_ARRAYSIZE(chams_material_local));
					InsertCheckbox(BlendOnScope, "Blend On Scope", &g_menu2.main.visuals.blend_on_scope);
					InsertSliderFloat("Blend Scale", &g_menu2.main.aimbot.scale, 0, 100, "%1.f%");
				}
				break;
			case 1:
				InsertCheckbox(EnemyChams, "Chams Enemy", &g_menu2.main.visuals.chams_enemy);
				if (g_menu2.main.visuals.chams_enemy) {
					ImGui::SameLine(150.f);
					ImGuiEx::ColorEdit4("##Chams Enemy", &g_menu2.main.visuals.chams_enemy_color, true);
					InsertCombo("Chams material", &g_menu2.main.visuals.chams_material_enemy, chams_material, IM_ARRAYSIZE(chams_material));
				}
				break;
			case 2:
				InsertCheckbox(FriendlyChams, "Chams Friendly", &g_menu2.main.visuals.chams_friendly);
				if (g_menu2.main.visuals.chams_friendly) {
					ImGui::SameLine(150.f);
					ImGuiEx::ColorEdit4("##Chams Friendly", &g_menu2.main.visuals.chams_friendly_color, true);
					InsertCombo("Chams material", &g_menu2.main.visuals.chams_material_friendly, chams_material, IM_ARRAYSIZE(chams_material));
				}
				break;
			case 3:
				InsertCheckbox(HistoryChams, "Chams History", &g_menu2.main.visuals.chams_history);
				if (g_menu2.main.visuals.chams_history) {
					ImGui::SameLine(150.f);
					ImGuiEx::ColorEdit4("##Chams History", &g_menu2.main.visuals.chams_history_color, true);
					InsertCombo("Chams material", &g_menu2.main.visuals.chams_material_history, chams_material, IM_ARRAYSIZE(chams_material));
				}
				break;
			}
		} break;
	case 1:
		ImGui::NewLine();
		const char* world_modulation[]{ "Off","Night","Fullbright","Custom" };

		InsertCheckbox(DoppedWeapons, "Weapons", &g_menu2.main.visuals.dropped_weapons);
		InsertCheckbox(DistanceWeapons, "Weapons Distance", &g_menu2.main.visuals.distance_weapons);
		if (&g_menu2.main.visuals.distance_weapons) {
			ImGui::SameLine(150.f);
			ImGuiEx::ColorEdit4("##Weapons Distance", &g_menu2.main.visuals.distance_weapons_color, true);
		}
		InsertCheckbox(DropepdWeaponsAmmo, "Weapons Ammo", &g_menu2.main.visuals.dropped_weapons_ammo);
		if (&g_menu2.main.visuals.dropped_weapons_ammo) {
			ImGui::SameLine(150.f);
			ImGuiEx::ColorEdit4("##Weapons Ammo", &g_menu2.main.visuals.dropped_weapons_ammo_color, true);
		}
		InsertCheckbox(Projectiles, "Projectiles", &g_menu2.main.visuals.projectiles);
		if (&g_menu2.main.visuals.projectiles) {
			ImGui::SameLine(150.f);
			ImGuiEx::ColorEdit4("##Projectiles", &g_menu2.main.visuals.projectiles_color, true);
		}
		InsertCheckbox(PlantedC4, "Planted C4", &g_menu2.main.visuals.planted_c4);
		InsertCheckbox(DontRenderTeammates, "Dont Render Teammates", &g_menu2.main.visuals.dont_render_teammates);
		InsertCombo("World Modulation", &g_menu2.main.visuals.world_modulation, world_modulation, IM_ARRAYSIZE(world_modulation));
		if (g_menu2.main.visuals.world_modulation == 1)
		{
			InsertSliderFloat("World Darkness", &g_menu2.main.visuals.world_darkness, 0, 100, "%1.f%");
		}
		else if (g_menu2.main.visuals.world_modulation == 3)
		{
			ImGui::Text("Color");
			ImGui::SameLine();
			ImGuiEx::ColorEdit4("##World Modulation", &g_menu2.main.visuals.world_color, true);
		}
		InsertCheckbox(TransparentProps, "Transparent Props", &g_menu2.main.visuals.transparent_props);
		InsertSliderFloat("Transparency Scale", &g_menu2.main.visuals.transparency_scale, 0, 100, "%1.f%");
		InsertCheckbox(ForceEnemiesOnRadar, "Force Enemies On Radar", &g_menu2.main.visuals.force_enemies_radar);
		InsertCheckbox(OverrideFog, "Override Fog", &g_menu2.main.visuals.override_fog);
		if (g_menu2.main.visuals.override_fog)
		{
			InsertSliderFloat("Fog Start", &g_menu2.main.visuals.fog_start, 0, 100, "%1.f%");
			InsertSliderFloat("Fog End", &g_menu2.main.visuals.fog_end, 0, 100, "%1.f%");
			InsertSliderFloat("Fog Density", &g_menu2.main.visuals.fog_density, 0, 100, "%1.f%");
		}

		ImGui::NextColumn();
		ImGui::NewLine();

		InsertCheckbox(RemoveVisRecoil, "Remove Vis Recoil", &g_menu2.main.visuals.remove_vis_recoi);
		InsertCheckbox(RemoveSmoke, "Remove Smoke", &g_menu2.main.visuals.remove_smoke);
		InsertCheckbox(RemoveFog, "Remove Fog", &g_menu2.main.visuals.remove_fog);
		InsertCheckbox(RemoveFlash, "Remove Flashbang", &g_menu2.main.visuals.remove_flash);
		InsertCheckbox(RemoveScope, "Remove Scope", &g_menu2.main.visuals.remove_scope);
		InsertCheckbox(OverrideFOV, "Override FOV", &g_menu2.main.visuals.override_fov);
		InsertSliderFloat("FOV", &g_menu2.main.visuals.fov, 60, 140, "%1.f%");
		InsertCheckbox(OverrideFOVScope, "Override FOV in Scope", &g_menu2.main.visuals.override_fov_scope);
		InsertCheckbox(OverrideViewFOV, "Override Viewmodel FOV", &g_menu2.main.visuals.override_viewmodel_fov);
		InsertSliderFloat("Viewmodel FOV", &g_menu2.main.visuals.view_fov, 60, 140, "%1.f%");
		InsertCheckbox(Spectators, "Spectators List", &g_menu2.main.visuals.specatators);
		InsertCheckbox(ForceCrosshair, "Force Crosshair", &g_menu2.main.visuals.force_crosshair);
		InsertCheckbox(VisSpread, "Visualize Spread", &g_menu2.main.visuals.vis_spread);
		if (g_menu2.main.visuals.vis_spread)
		{
			ImGui::SameLine();
			ImGuiEx::ColorEdit4("##Visualize Spread", &g_menu2.main.visuals.spread_color, true);
		}

		ImGui::NextColumn();
		ImGui::NewLine();

		InsertCheckbox(PenCrosshair, "Penetration Crosshair", &g_menu2.main.visuals.pen_crosshair);
		InsertCheckbox(PenCrossDamage, "Penetration Crosshair Damage", &g_menu2.main.visuals.pen_crosshair_damage);
		InsertCheckbox(Indicators, "Indicators", &g_menu2.main.visuals.indicators);
		InsertCheckbox(KeybindStatus, "Keybind Status", &g_menu2.main.visuals.keybind_status);
		InsertCheckbox(GrenadePrediction, "Grenade Prediction", &g_menu2.main.visuals.grenade_prediction);
		InsertCheckbox(ImpactBeams, "Impact Beams", &g_menu2.main.visuals.impact_beams);
		ImGui::Text("Impact Color");
		ImGui::SameLine();
		ImGuiEx::ColorEdit4("##Impact Color", &g_menu2.main.visuals.impacts_color, true);
		ImGui::Text("Impact Color Hurt");
		ImGui::SameLine();
		ImGuiEx::ColorEdit4("##Impact Color Hurt", &g_menu2.main.visuals.impacts_color_hurt, true);
		InsertSliderFloat("Impact Time", &g_menu2.main.visuals.impact_time, 1, 10, "%1.f%");
		InsertSliderFloat("Thirdperson Distance", &g_menu2.main.visuals.thirdperson_distance, 50, 300, "%1.f%");

		break;
	}

	ImGui::EndColumns();
	//ImGui::EndGroupBox();
}

void Misc()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	const char* skychanger[]{ "Tibet","Embassy","Italy","Daylight","Cloudy","Night 1","Night 2","Night Flat","Day HD","Day","Rural","Vertigo HD","Vertigo Blue HD","Vertigo","Vietnam","Dusty Sky","Jungle","Nuke","Office"  };
	const char* hitsounds[]{ "Off","Arena Switch","Opera","Bameware","Dopium","Bubble","COD","Fatality","Pop"  };

//	ImGui::BeginGroupBox("Misc", ImVec2(572.f, 300.f));
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	//InsertEmpty();
		ImGui::NewLine();
		//ImGui::Text("Misc");
		InsertCheckbox(Hitmarker, "Hitmarker", &g_menu2.main.misc.hitmarker);
		InsertCombo("Hitsound", &g_menu2.main.misc.hitsound, hitsounds, IM_ARRAYSIZE(hitsounds));
		InsertCheckbox(Killsound, "Killsound", &g_menu2.main.misc.killsound);
		InsertCheckbox(UnlockInventory, "Unlock Inventory", &g_menu2.main.misc.unlockinv);
		InsertCheckbox(SkyboxChanger, "Skybox Changer", &g_menu2.main.misc.skyboxchange);
		InsertCombo("Sky Value", &g_menu2.main.misc.skyvalue, skychanger, IM_ARRAYSIZE(skychanger));
		InsertCheckbox(Ragdoll, "Ragdoll Force", &g_menu2.main.misc.ragdoll);
		InsertCheckbox(RevealRank, "Reveal Ranks", &g_menu2.main.misc.revealrank);
		InsertCheckbox(PreserveKillfeed, "Preserve Killfeed", &g_menu2.main.misc.preservekillfeed);
		InsertCheckbox(ClanTag, "Clan-Tag Changer", &g_menu2.main.misc.clantagchanger);
		InsertCheckbox(SlideWalk, "Slide Walk", &g_menu2.main.misc.slidewalk);
		InsertCheckbox(Watermark, "Enable Watermark", &g_menu2.main.misc.watermark);
		InsertCheckbox(AspectRatio, "Aspect Ratio", &g_menu2.main.misc.ratiochanger);
		InsertSliderFloat("Aspect Ratio", &g_menu2.main.misc.ratio, 0, 2, "%1.f%");
		InsertCheckbox(LogMisses, "Log Misses", &g_menu2.main.misc.logmisses);

	ImGui::EndColumns();
//	ImGui::EndGroupBox();
}

bool IMGUIMenu::Initialize(IDirect3DDevice9* pDevice)
{
	static bool initialized = false;
	if (!initialized)
	{
		ImGui::CreateContext();
		CreateStyle();
		IDirect3DSwapChain9* pChain = nullptr;
		D3DPRESENT_PARAMETERS pp = {};
		D3DDEVICE_CREATION_PARAMETERS param = {};
		pDevice->GetCreationParameters(&param);
		pDevice->GetSwapChain(0, &pChain);

		if (pChain)
			pChain->GetPresentParameters(&pp);


		ImGui_ImplWin32_Init(param.hFocusWindow);
		ImGui_ImplDX9_Init(pDevice);
		ImGui_ImplDX9_CreateDeviceObjects();
		initialized = true;
	}

	return initialized;
}

void IMGUIMenu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void IMGUIMenu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void IMGUIMenu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

auto windowFlags = (ImGuiWindowFlags_NoCollapse
    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding);

ImFont* gravity;
ImFont* gravityBold;
ImFont* Streamster;
ImFont* watermark;


void IMGUIMenu::Render()
{
	if (!Opened) return;

	//ImGui::GetIO().MouseDrawCursor = _visible;

	ImGuiStyle* style = &ImGui::GetStyle();

	ImGui::PushFont(gravityBold);

	ImGui::SetNextWindowSize(ImVec2(600.f, 550.f));
	ImGui::Begin("##menu", &_visible, windowFlags);

	style->WindowPadding = ImVec2(7.f, 7.f);

	style->Colors[ImGuiCol_MenuAccent] = ImColor(255, 215, 0);
	style->Colors[ImGuiCol_Logo] = ImColor(0, 87, 255);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		{
			ImGui::BeginTitleBar("Title Bar", ImVec2(586.f, 55.f), false);

			ImGui::PopFont();
			ImGui::PushFont(Streamster);
			ImGui::SameLine(5.f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGuiCol_Logo);
			//ImGui::Text("");
			ImGui::PopStyleColor();
			ImGui::PopFont();
			ImGui::PushFont(gravityBold);
			ImGui::Text("jimmy's mom has got it going on");
		//	ImGui::Image(obam::globals::menuLogo, ImVec2(600.f, 55.f));
			//ImGui::SameLine(400.f);
			//ImGui::Text(loader_info.username);

			ImGui::EndTitleBar();
		}
		ImGui::PopStyleVar();
		style->Colors[ImGuiCol_ChildBg] = ImColor(41, 32, 59);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		{
			ImGui::BeginChild("Tabs", ImVec2(586.f, 35.f), false);

			ImGui::SameLine(8.f);

			switch (tab) {

			case 0:
				if (ImGui::SelectedTab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  NOT SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 1:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  NOT SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 2:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  NOT SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 3:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  NOT SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 4:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::Tab("  NOT SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			case 5:
				if (ImGui::Tab("  AIMBOT  ", ImVec2(0.f, 35.f))) tab = 0;
				ImGui::SameLine();
				if (ImGui::Tab("  ANTI-AIM  ", ImVec2(0.f, 35.f))) tab = 1;
				ImGui::SameLine();
				if (ImGui::Tab("  VISUALS  ", ImVec2(0.f, 35.f))) tab = 2;
				ImGui::SameLine();
				if (ImGui::Tab("  MISC  ", ImVec2(0.f, 35.f))) tab = 3;
				ImGui::SameLine();
				if (ImGui::Tab("  SKINS  ", ImVec2(0.f, 35.f))) tab = 4;
				ImGui::SameLine();
				if (ImGui::SelectedTab("  NOT SCRIPTS  ", ImVec2(0.f, 35.f))) tab = 5;
				break;
			default:
				break;
			}

			ImGui::EndChild();
		}
		ImGui::PopStyleVar();

		style->Colors[ImGuiCol_ChildBg] = ImColor(31, 24, 46);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		{
			ImGui::BeginChild("Sub Tabs", ImVec2(586.f, 25.f), false);

			ImGui::SameLine(8.f);


			if (tab == 0) {

				switch (aimbotTab) {

				case 0:
					if (ImGui::SubTab("  Rage  ", ImVec2(0.f, 25.f))) aimbotTab = 1;
					ImGui::SameLine();
					if (ImGui::SelectedSubTab("  Legit  ", ImVec2(0.f, 25.f))) aimbotTab = 0;

					ImGui::SameLine(195.f);
					switch (legitTab)
					{
					case 0:
						if (ImGui::SelectedSubTab("  Rifle  ", ImVec2(0.f, 25.f))) legitTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) legitTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) legitTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) legitTab = 3;
						break;
					case 1:
						if (ImGui::SubTab("  Rifle  ", ImVec2(0.f, 25.f))) legitTab = 0;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Sniper  ", ImVec2(0.f, 25.f))) legitTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) legitTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) legitTab = 3;
						break;
					case 2:
						if (ImGui::SubTab("  Rifle  ", ImVec2(0.f, 25.f))) legitTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) legitTab = 1;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Pistol  ", ImVec2(0.f, 25.f))) legitTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) legitTab = 3;
						break;
					case 3:
						if (ImGui::SubTab("  Rifle  ", ImVec2(0.f, 25.f))) legitTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) legitTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) legitTab = 2;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Other  ", ImVec2(0.f, 25.f))) legitTab = 3;
						break;
					default:
						break;
					}
					break;
				case 1:
					if (ImGui::SelectedSubTab("  Rage  ", ImVec2(0.f, 25.f))) aimbotTab = 1;
					ImGui::SameLine();
					if (ImGui::SubTab("  Legit  ", ImVec2(0.f, 25.f))) aimbotTab = 0;

					ImGui::SameLine(195.f);
					switch (rageTab)
					{
					case 0:
						if (ImGui::SelectedSubTab("  Auto  ", ImVec2(0.f, 25.f))) rageTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) rageTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) rageTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) rageTab = 3;
						break;
					case 1:
						if (ImGui::SubTab("  Auto  ", ImVec2(0.f, 25.f))) rageTab = 0;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Sniper  ", ImVec2(0.f, 25.f))) rageTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) rageTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) rageTab = 3;
						break;
					case 2:
						if (ImGui::SubTab("  Auto  ", ImVec2(0.f, 25.f))) rageTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) rageTab = 1;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Pistol  ", ImVec2(0.f, 25.f))) rageTab = 2;
						ImGui::SameLine();
						if (ImGui::SubTab("  Other  ", ImVec2(0.f, 25.f))) rageTab = 3;
						break;
					case 3:
						if (ImGui::SubTab("  Auto  ", ImVec2(0.f, 25.f))) rageTab = 0;
						ImGui::SameLine();
						if (ImGui::SubTab("  Sniper  ", ImVec2(0.f, 25.f))) rageTab = 1;
						ImGui::SameLine();
						if (ImGui::SubTab("  Pistol  ", ImVec2(0.f, 25.f))) rageTab = 2;
						ImGui::SameLine();
						if (ImGui::SelectedSubTab("  Other  ", ImVec2(0.f, 25.f))) rageTab = 3;
						break;
					default:
						break;
					}
				default:
					break;
				}
			}
			else if (tab == 1)
		    {
				ImGui::SelectedSubTab("  Basic  ", ImVec2(0.f, 25.f));
			}
			else if (tab == 2)
			{
				switch (visualsTab) {
				case 0:
					if (ImGui::SelectedSubTab("  Player  ", ImVec2(0.f, 25.f))) visualsTab = 0;
					ImGui::SameLine();
					if (ImGui::SubTab("  Visuals  ", ImVec2(0.f, 25.f))) visualsTab = 1; 
					break;
				case 1:
					if (ImGui::SubTab("  Player  ", ImVec2(0.f, 25.f))) visualsTab = 0;
					ImGui::SameLine();
					if (ImGui::SelectedSubTab("  Visuals  ", ImVec2(0.f, 25.f))) visualsTab = 1; 
					break;
				}
			
			}
			else if (tab == 3)
			{
			ImGui::SelectedSubTab("  Misc  ", ImVec2(0.f, 25.f));
			}

			ImGui::EndChild();
		}
		ImGui::PopStyleVar();
		//ImGui::BeginColumns
		style->Colors[ImGuiCol_ChildBg] = ImColor(25, 20, 27);
		ImGui::BeginChild("Main", ImVec2(586.f, 420.f), false);
	ImGui::SameLine(7.f);
	style->Colors[ImGuiCol_ChildBg] = ImColor(21, 17, 29);
//	ImGui::BeginGroupBox("Controls", ImVec2(572, 407.f));
	switch (tab)
	{
	case 0:
		if (aimbotTab == 1)
			Ragebot();
		else
			Legitbot();
		break;
	case 1:
		Antiaim();
		break;
	case 2:
		Visuals();
		break;
	case 3:
		Misc();
	default:
		break;
	}
	style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
	//ImGui::EndGroupBox();
	ImGui::EndChild();
	ImGui::End();
	ImGui::PopFont();
  
}

void IMGUIMenu::Toggle()
{
    _visible = !_visible;
}

void IMGUIMenu::CreateStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::StyleColorsDark(&style);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.GrabMinSize = 5.0f;
	style.ScrollbarSize = 12.f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.FrameBorderSize = 0.4f;
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	//io.MouseDrawCursor = true;
	char buffer[MAX_PATH];
	//GetWindowsDirectoryA(buffer, MAX_PATH);

	gravity = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravity_compressed_data, gravity_compressed_size, 13.f);
	Streamster = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Streamster_compressed_data, Streamster_compressed_size, 50.f);
	watermark = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 16.f);
	gravityBold = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 13.f);


}

