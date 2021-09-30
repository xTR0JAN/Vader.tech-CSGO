#include "includes.h"

#include "minhook/minhook.h"

Hooks                g_hooks{ };;
CustomEntityListener g_custom_entity_listener{ };;

void Pitch_proxy( CRecvProxyData *data, Address ptr, Address out ) {
	// normalize this fucker.
	math::NormalizeAngle( data->m_Value.m_Float );

	// clamp to remove retardedness.
	math::clamp( data->m_Value.m_Float, -90.f, 90.f );

	// call original netvar proxy.
	if ( g_hooks.m_Pitch_original )
		g_hooks.m_Pitch_original( data, ptr, out );
}

void Body_proxy( CRecvProxyData *data, Address ptr, Address out ) {
	Stack stack;

	static Address RecvTable_Decode{ pattern::find( g_csgo.m_engine_dll, XOR( "EB 0D FF 77 10" ) ) };

	// call from entity going into pvs.
	if ( stack.next( ).next( ).ReturnAddress( ) != RecvTable_Decode ) {
		// convert to player.
		Player *player = ptr.as< Player * >( );

		// store data about the update.
		g_resolver.OnBodyUpdate( player, data->m_Value.m_Float );
	}

	// call original proxy.
	if ( g_hooks.m_Body_original )
		g_hooks.m_Body_original( data, ptr, out );
}

void AbsYaw_proxy( CRecvProxyData *data, Address ptr, Address out ) {
	// convert to ragdoll.
	//Ragdoll* ragdoll = ptr.as< Ragdoll* >( );

	// get ragdoll owner.
	//Player* player = ragdoll->GetPlayer( );

	// get data for this player.
	/*AimPlayer* aim = &g_aimbot.m_players[ player->index( ) - 1 ];

	if( player && aim ) {
	if( !aim->m_records.empty( ) ) {
	LagRecord* match{ nullptr };

	// iterate records.
	for( const auto &it : aim->m_records ) {
	// find record that matches with simulation time.
	if( it->m_sim_time == player->m_flSimulationTime( ) ) {
	match = it.get( );
	break;
	}
	}

	// we have a match.
	// and it is standing
	// TODO; add air?
	if( match /*&& match->m_mode == Resolver::Modes::RESOLVE_STAND*/// ) {
	/*	RagdollRecord record;
	record.m_record   = match;
	record.m_rotation = math::NormalizedAngle( data->m_Value.m_Float );
	record.m_delta    = math::NormalizedAngle( record.m_rotation - match->m_lbyt );

	float death = math::NormalizedAngle( ragdoll->m_flDeathYaw( ) );

	// store.
	//aim->m_ragdoll.push_front( record );

	//g_cl.print( tfm::format( XOR( "rot %f death %f delta %f\n" ), record.m_rotation, death, record.m_delta ).data( ) );
	}
	}*/
	//}

	// call original netvar proxy.
	if ( g_hooks.m_AbsYaw_original )
		g_hooks.m_AbsYaw_original( data, ptr, out );
}

void Force_proxy( CRecvProxyData *data, Address ptr, Address out ) {
	// convert to ragdoll.
	Ragdoll *ragdoll = ptr.as< Ragdoll * >( );

	// get ragdoll owner.
	Player *player = ragdoll->GetPlayer( );

	// we only want this happening to noobs we kill.
	if ( g_menu.main.misc.ragdoll_force.get( ) && g_cl.m_local && player && player->enemy( g_cl.m_local ) ) {
		// get m_vecForce.
		vec3_t vel = { data->m_Value.m_Vector[ 0 ], data->m_Value.m_Vector[ 1 ], data->m_Value.m_Vector[ 2 ] };

		// give some speed to all directions.
		vel *= 1000.f;

		// boost z up a bit.
		if ( vel.z <= 1.f )
			vel.z = 2.f;

		vel.z *= 2.f;

		// don't want crazy values for this... probably unlikely though?
		math::clamp( vel.x, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );
		math::clamp( vel.y, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );
		math::clamp( vel.z, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );

		// set new velocity.
		data->m_Value.m_Vector[ 0 ] = vel.x;
		data->m_Value.m_Vector[ 1 ] = vel.y;
		data->m_Value.m_Vector[ 2 ] = vel.z;
	}

	if ( g_hooks.m_Force_original )
		g_hooks.m_Force_original( data, ptr, out );
}

void CL_Move(float accumulated_extra_samples, bool bFinalTick) {
	auto original = o_CLMove;


	// Recharge every other tick
// By not sending a command this tick, our commands allowed for simulation goes up so we can shift again
// Task for the reader: Automate this properly without preventing the second DT shot...
	if (g_csgo.m_globals->m_tick_count % 2 && g_cl.doubletapCharge < g_menu.main.aimbot.doubletap_ticks.get()) {
		g_cl.doubletapCharge++;
		// Task for the reader: Tickbase still increases when recharging, but the client doesn't know that...
		return;
	}

	original(accumulated_extra_samples, bFinalTick);

	//////// Shift if needed
	// Every time we call original again, we create another command to send to the server
	// All of these extra commands will be simulated by the server as long as we have a tick allowed for simulation
	// Recharging "earns" us extra ticks allowed for simulation, which we can "spend" via sending multiple commands to shift

	// Task for the reader: Fix tickbase (not required for shifting but will prevent pred errors and occasional failure to predict the third shot)
	// Hint #1: Look where the game modifies m_nTickbase and do stuff there
	// Hint #2: Print out your serverside tickbase and see how it changes when you shift
	g_cl.isShifting = true;
	{
		for (g_cl.ticksToShift = std::min(g_cl.doubletapCharge, g_cl.ticksToShift); g_cl.ticksToShift > 0; g_cl.ticksToShift--) {
			original(accumulated_extra_samples, bFinalTick); // Create an extra movement command (will call CreateMove)
			if (g_cl.ticksToShift <= 1) {
				g_cl.lastShiftedCmdNr = g_csgo.m_cl->m_last_outgoing_command;
			}
			//if (!g_menu.main.aimbot.slow_teleport.get()) {
			//	g_cl.m_cmd->m_side_move = 0;
			//	g_cl.m_cmd->m_forward_move = 0;
			//}
		}
	}
	g_cl.isShifting = false;
	g_cl.ignoreallcmds = false;
	//return (void)original(accumulated_extra_samples, bFinalTick);
}

typedef long(__stdcall* EndScene_t)(IDirect3DDevice9*);
inline EndScene_t o_EndScene;
DWORD dwOld_D3DRS_COLORWRITEENABLE;
IDirect3DVertexDeclaration9* vertDec;
IDirect3DVertexShader9* vertShader;

#include "ImGUIMenu/menu.hpp"
long __stdcall EndScene(IDirect3DDevice9* pDevice)
{
	g_IMGUIMenu.Initialized = g_IMGUIMenu.Initialize(pDevice); if (!g_IMGUIMenu.Initialized) return o_EndScene(pDevice);
	if (GetForegroundWindow() == g_csgo.m_window && (GetAsyncKeyState(VK_INSERT) & 1)) g_IMGUIMenu.Opened = !g_IMGUIMenu.Opened;

	pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
	pDevice->GetVertexDeclaration(&vertDec);
	pDevice->GetVertexShader(&vertShader);
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	/*render stuff*/
	{
		g_IMGUIMenu.Render();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
	pDevice->SetVertexDeclaration(vertDec);
	pDevice->SetVertexShader(vertShader);

	return o_EndScene(pDevice);

}

void* GetF(void* class_, int index) { return (void*)((unsigned int)(*(int**)class_)[index]); }

void Hooks::init() {
	MH_Initialize();
	MH_CreateHook(GetF(g_csgo.m_d3ddevice, 42), &EndScene, (void**)&o_EndScene);
	MH_CreateHook(pattern::find(PE::GetModule(HASH("engine.dll")), XOR("55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8A")), &CL_Move, reinterpret_cast<void**>(&o_CLMove));
	MH_EnableHook(MH_ALL_HOOKS);

	// hook wndproc.
	auto m_hWindow = FindWindowA(XOR("Valve001"), NULL);
	g_csgo.m_window = m_hWindow;
	m_old_wndproc = (WNDPROC)g_winapi.SetWindowLongA(g_csgo.m_game->m_hWindow, GWL_WNDPROC, util::force_cast<LONG>(Hooks::WndProc));
	//m_old_wndproc = (WNDPROC)g_winapi.SetWindowLongA(m_hWindow, GWL_WNDPROC, util::force_cast<LONG>(Hooks::WndProc));

	// setup normal VMT hooks.
	m_panel.init(g_csgo.m_panel);
	m_panel.add(IPanel::PAINTTRAVERSE, util::force_cast(&Hooks::PaintTraverse));

	m_client.init(g_csgo.m_client);
	m_client.add(CHLClient::LEVELINITPREENTITY, util::force_cast(&Hooks::LevelInitPreEntity));
	m_client.add(CHLClient::LEVELINITPOSTENTITY, util::force_cast(&Hooks::LevelInitPostEntity));
	m_client.add(CHLClient::LEVELSHUTDOWN, util::force_cast(&Hooks::LevelShutdown));
	m_client.add(CHLClient::FRAMESTAGENOTIFY, util::force_cast(&Hooks::FrameStageNotify));
	m_client.add(CHLClient::USRCMDTODELTABUFFER, util::force_cast(&Hooks::WriteUsercmdDeltaToBuffer));


	m_engine.init(g_csgo.m_engine);
	m_engine.add(IVEngineClient::ISCONNECTED, util::force_cast(&Hooks::IsConnected));
	m_engine.add(IVEngineClient::ISHLTV, util::force_cast(&Hooks::IsHLTV));

	m_prediction.init(g_csgo.m_prediction);
	m_prediction.add(CPrediction::INPREDICTION, util::force_cast(&Hooks::InPrediction));
	m_prediction.add(CPrediction::RUNCOMMAND, util::force_cast(&Hooks::RunCommand));

	m_client_mode.init(g_csgo.m_client_mode);
	m_client_mode.add(IClientMode::SHOULDDRAWPARTICLES, util::force_cast(&Hooks::ShouldDrawParticles));
	m_client_mode.add(IClientMode::SHOULDDRAWFOG, util::force_cast(&Hooks::ShouldDrawFog));
	m_client_mode.add(IClientMode::OVERRIDEVIEW, util::force_cast(&Hooks::OverrideView));
	m_client_mode.add(IClientMode::CREATEMOVE, util::force_cast(&Hooks::CreateMove));
	m_client_mode.add(IClientMode::DOPOSTSPACESCREENEFFECTS, util::force_cast(&Hooks::DoPostScreenSpaceEffects));

	m_surface.init(g_csgo.m_surface);
	m_surface.add(ISurface::LOCKCURSOR, util::force_cast(&Hooks::LockCursor));
	m_surface.add(ISurface::PLAYSOUND, util::force_cast(&Hooks::PlaySound));
	m_surface.add(ISurface::ONSCREENSIZECHANGED, util::force_cast(&Hooks::OnScreenSizeChanged));

	m_model_render.init(g_csgo.m_model_render);
	m_model_render.add(IVModelRender::DRAWMODELEXECUTE, util::force_cast(&Hooks::DrawModelExecute));

	m_render_view.init(g_csgo.m_render_view);
	m_render_view.add(IVRenderView::SCENEEND, util::force_cast(&Hooks::SceneEnd));

	m_shadow_mgr.init(g_csgo.m_shadow_mgr);
	m_shadow_mgr.add(IClientShadowMgr::COMPUTESHADOWDEPTHTEXTURES, util::force_cast(&Hooks::ComputeShadowDepthTextures));

	m_view_render.init(g_csgo.m_view_render);
	m_view_render.add(CViewRender::ONRENDERSTART, util::force_cast(&Hooks::OnRenderStart));
	m_view_render.add(CViewRender::RENDERVIEW, util::force_cast(&Hooks::RenderView));
	m_view_render.add(CViewRender::RENDER2DEFFECTSPOSTHUD, util::force_cast(&Hooks::Render2DEffectsPostHUD));
	m_view_render.add(CViewRender::RENDERSMOKEOVERLAY, util::force_cast(&Hooks::RenderSmokeOverlay));

	m_match_framework.init(g_csgo.m_match_framework);
	m_match_framework.add(CMatchFramework::GETMATCHSESSION, util::force_cast(&Hooks::GetMatchSession));

	m_material_system.init(g_csgo.m_material_system);
	m_material_system.add(IMaterialSystem::OVERRIDECONFIG, util::force_cast(&Hooks::OverrideConfig));

	m_fire_bullets.init(g_csgo.TEFireBullets);
	m_fire_bullets.add(7, util::force_cast(&Hooks::PostDataUpdate));

	g_custom_entity_listener.init();

	// cvar hooks.
	m_debug_spread.init(g_csgo.weapon_debug_spread_show);
	m_debug_spread.add(ConVar::GETINT, util::force_cast(&Hooks::DebugSpreadGetInt));

	m_net_show_fragments.init(g_csgo.net_showfragments);
	m_net_show_fragments.add(ConVar::GETBOOL, util::force_cast(&Hooks::NetShowFragmentsGetBool));

	// set netvar proxies.
	g_netvars.SetProxy(HASH("DT_CSPlayer"), HASH("m_angEyeAngles[0]"), Pitch_proxy, m_Pitch_original);
	g_netvars.SetProxy(HASH("DT_CSPlayer"), HASH("m_flLowerBodyYawTarget"), Body_proxy, m_Body_original);
	g_netvars.SetProxy(HASH("DT_CSRagdoll"), HASH("m_vecForce"), Force_proxy, m_Force_original);
	g_netvars.SetProxy(HASH("DT_CSRagdoll"), HASH("m_flAbsYaw"), AbsYaw_proxy, m_AbsYaw_original);
}