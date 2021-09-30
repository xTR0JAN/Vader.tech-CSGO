#include "includes.h"

Aimbot g_aimbot{ };;


bool CanFireWithExploit(int m_iShiftedTick)
{
	// curtime before shift
	float curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase() - m_iShiftedTick);
	return g_cl.CanFireWeapon(curtime);
}

bool Aimbot::CanDT() {
	return g_cl.m_local->alive() && m_double_tap && !g_input.GetKeyState(g_menu.main.movement.fakewalk.get());
}

void Aimbot::DoubleTap()
{
	static bool did_shift_before = false;
	static bool reset = true;
	static int clock = 0;
	auto ticks_to_shift = g_menu.main.aimbot.doubletap_ticks.get();
	g_cl.can_dt_shoot = false;

	if (CanDT() && !g_csgo.m_gamerules->m_bFreezePeriod())
	{
		if (m_double_tap)
		{
			if (reset)
			{
				if (g_cl.m_cmd->m_buttons & IN_ATTACK)
					clock = 0;

				clock++;

				// if this sucks for you when peeking then limit shift ticks to 11. we dont need to compensate lag comp issues. this can also be turned into a checkbox if you want to.

				// if ( menu bool )
				// ticks_to_shift = std::clamp(ticks_to_shift, 0, 11);

				if (clock >= 60) // make this thing higher if you want to delay the recharge more.
				{
					g_cl.m_tick_to_recharge = ticks_to_shift;
					g_cl.can_recharge = true;
					g_cl.m_tick_to_shift = 0;
					clock = 0;
					reset = false;
				}
			}
			else
			{
				if (g_cl.m_charged)
				{
					g_cl.can_dt_shoot = true;

					if (g_cl.m_cmd->m_buttons & IN_ATTACK)
					{
						*g_cl.m_packet = true;
						g_cl.m_tick_to_shift = ticks_to_shift;
						reset = true;
						g_cl.m_charged = false;
						if (!g_menu.main.aimbot.slow_teleport.get()) {
							g_cl.m_cmd->m_side_move = 0;
							g_cl.m_cmd->m_forward_move = 0;
						}
					}
				}
			}
		}
	}
}

bool test;

void AimPlayer::UpdateAnimations(LagRecord* record) {
	CCSGOPlayerAnimState* state = m_player->m_PlayerAnimState();
	if (!state)
		return;

	// player respawned.
	if (m_player->m_flSpawnTime() != m_spawn) {
		// reset animation state.
		game::ResetAnimationState(state);

		// note new spawn time.
		m_spawn = m_player->m_flSpawnTime();
	}		

	// first off lets backup our globals.
	auto curtime = g_csgo.m_globals->m_curtime;
	auto realtime = g_csgo.m_globals->m_realtime;
	auto frametime = g_csgo.m_globals->m_frametime;
	auto absframetime = g_csgo.m_globals->m_abs_frametime;
	auto framecount = g_csgo.m_globals->m_frame;
	auto tickcount = g_csgo.m_globals->m_tick_count;
	auto interpolation = g_csgo.m_globals->m_interp_amt;

	// backup stuff that we do not want to fuck with.
	AnimationBackup_t backup;

	backup.m_origin = m_player->m_vecOrigin( );
	backup.m_abs_origin = m_player->GetAbsOrigin( );
	backup.m_velocity = m_player->m_vecVelocity( );
	backup.m_abs_velocity = m_player->m_vecAbsVelocity( );
	backup.m_flags = m_player->m_fFlags( );
	backup.m_eflags = m_player->m_iEFlags( );
	backup.m_duck = m_player->m_flDuckAmount( );
	backup.m_body = m_player->m_flLowerBodyYawTarget( );
	m_player->GetAnimLayers( backup.m_layers );

	// set globals.
	g_csgo.m_globals->m_curtime = record->m_anim_time;
	g_csgo.m_globals->m_realtime = record->m_anim_time;
	g_csgo.m_globals->m_frame = game::TIME_TO_TICKS( record->m_anim_time );
	g_csgo.m_globals->m_tick_count = game::TIME_TO_TICKS( record->m_anim_time );
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_abs_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_interp_amt = 0.f;

	// is player a bot?
	bool bot = game::IsFakePlayer( m_player->index( ) );

	// reset resolver/fakewalk/fakeflick state.
	record->m_mode = Resolver::Modes::RESOLVE_NONE;
	record->m_fake_walk = false;
	record->m_fake_flick = false;

	// thanks llama.
	if( record->m_flags & FL_ONGROUND ) {
		// they are on ground.
		state->m_ground = true;
		// no they didnt land.
		state->m_land = false;
	}

	// fix velocity.
	if( record->m_lag > 0 && record->m_lag < 16 && m_records.size( ) >= 2 ) {
		// get pointer to previous record.
		LagRecord* previous = m_records[ 1 ].get( );

		// valid previous record.
		if( previous && !previous->dormant( ) ) {
			// get the sim delta between both records.
			record->m_choke_time = record->m_sim_time - previous->m_sim_time;

			// how many ticks did he choke?
			const int choked = game::TIME_TO_TICKS( record->m_choke_time );

			// set this.
			record->m_lag = choked;

			// this is record is out of lagcompensation.
			if( ( choked - 1 ) > 19 || previous->m_sim_time == 0.f ) {
				record->m_choke_time = g_csgo.m_globals->m_interval;
				record->m_lag = 1;
			}

			// he isnt choking any packets.
			if( record->m_lag < 1 ) {
				record->m_choke_time = g_csgo.m_globals->m_interval;
				record->m_lag = 1;
			}

			// "default" velocity we will be using if all other calculations seem to fail.
			record->m_velocity = ( record->m_origin - previous->m_origin ) * ( 1.f / record->m_choke_time );
		}
	}

	// fix CGameMovement::FinishGravity
	if( !( m_player->m_fFlags( ) & FL_ONGROUND ) )
		record->m_velocity.z -= game::TICKS_TO_TIME( g_csgo.sv_gravity->GetFloat( ) );
	else
		record->m_velocity.z = 0.0f;

	// set this fucker, it will get overriden.
	record->m_anim_velocity = record->m_velocity;

	// fix various issues with the game.
	// these issues can only occur when a player is choking data.
	if( record->m_lag > 1 && !bot ) {
		auto speed = record->m_velocity.length( );

		// detect fakewalking players
		if( speed > 1.f
			&& record->m_lag >= 12
			&& record->m_layers[ 12 ].m_weight == 0.0f
			&& record->m_layers[ 6 ].m_weight == 0.0f
			&& record->m_layers[ 6 ].m_playback_rate < 0.0001f
			&& ( record->m_flags & FL_ONGROUND ) )
			record->m_fake_walk = true;

		// detect fake flick players.
		if( m_records.size( ) >= 2 ) {
			auto previous = m_records[ 1 ].get( );
			if( previous && !previous->dormant( ) ) {
				// they are fake flicking, we DUMP HERE!
				if( record->m_velocity.length( ) < 18.f
					&& record->m_layers[ 6 ].m_weight != 1.0f
					&& record->m_layers[ 6 ].m_weight != 0.0f
					&& record->m_layers[ 6 ].m_weight != previous->m_layers[ 6 ].m_weight
					&& ( record->m_flags & FL_ONGROUND ) )
					record->m_fake_flick = true;
			}
		}

		// if they fakewalk scratch this shit.
		if( record->m_fake_walk )
			record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };

		// we need atleast 2 updates/records
		// to fix these issues.
		if( m_records.size( ) >= 2 ) {
			// get pointer to previous record.
			LagRecord* previous = m_records[ 1 ].get( );

			// valid previous record.
			if( previous && !previous->dormant( ) ) {
				// LOL.
				if( ( record->m_origin - previous->m_origin ).is_zero( ) )
					record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };

				// jumpfall.
				bool bOnGround = record->m_flags & FL_ONGROUND;
				bool bJumped = false;
				bool bLandedOnServer = false;
				float flLandTime = 0.f;

				// magic llama code.
				if( record->m_layers[ 4 ].m_cycle < 0.5f && ( !( record->m_flags & FL_ONGROUND ) || !( previous->m_flags & FL_ONGROUND ) ) ) {
					flLandTime = record->m_sim_time - float( record->m_layers[ 4 ].m_playback_rate / record->m_layers[ 4 ].m_cycle );
					bLandedOnServer = flLandTime >= previous->m_sim_time;
				}

				// jump_fall fix
				if( bLandedOnServer && !bJumped ) {
					if( flLandTime <= record->m_anim_time ) {
						bJumped = true;
						bOnGround = true;
					}
					else {
						bOnGround = previous->m_flags & FL_ONGROUND;
					}
				}

				// do the fix. hahaha
				if( bOnGround ) {
					m_player->m_fFlags( ) |= FL_ONGROUND;
				}
				else {
					m_player->m_fFlags( ) &= ~FL_ONGROUND;
				}

				// delta in duckamt and delta in time..
				float duck = record->m_duck - previous->m_duck;
				float time = record->m_sim_time - previous->m_sim_time;

				// get the duckamt change per tick.
				float change = ( duck / time ) * g_csgo.m_globals->m_interval;

				// fix crouching players.
				m_player->m_flDuckAmount( ) = previous->m_duck + change;

				if( !record->m_fake_walk ) {
					// fix the velocity till the moment of animation.
					vec3_t velo = record->m_velocity - previous->m_velocity;

					// accel per tick.
					vec3_t accel = ( velo / time ) * g_csgo.m_globals->m_interval;

					// set the anim velocity to the previous velocity.
					// and predict one tick ahead.
					record->m_anim_velocity = previous->m_velocity + accel;
				}
			}
		}
	}

	// lol?
	for( int i = 0; i < 13; i++ )
		m_player->m_AnimOverlay( )[ i ].m_owner = m_player;

	bool fake = g_menu.main.aimbot.correct.get( ) && !bot;

	// if using fake angles, correct angles.
	if( fake ) 
		g_resolver.ResolveAngles( m_player, record );

	// force to use correct abs origin and velocity ( no CalcAbsolutePosition and CalcAbsoluteVelocity calls )
	m_player->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

	// set stuff before animating.
	m_player->m_vecOrigin( ) = record->m_origin;
	m_player->m_vecVelocity( ) = m_player->m_vecAbsVelocity( ) = record->m_anim_velocity;
	m_player->m_flLowerBodyYawTarget( ) = record->m_body;

	// write potentially resolved angles.
	m_player->m_angEyeAngles( ) = record->m_eye_angles;

	// get invalidated bone cache.
	static auto& invalidatebonecache = pattern::find( g_csgo.m_client_dll, XOR( "C6 05 ? ? ? ? ? 89 47 70" ) ).add( 0x2 );

	// make sure we keep track of the original invalidation state
	const auto oldbonecache = invalidatebonecache;

	// update animtions now.
	m_player->m_bClientSideAnimation( ) = true;
	m_player->UpdateClientSideAnimation( );
	m_player->m_bClientSideAnimation( ) = false;

	// we don't want to enable cache invalidation by accident
	invalidatebonecache = oldbonecache;

	// player animations have updated.
	m_player->InvalidatePhysicsRecursive(InvalidatePhysicsBits_t::ANGLES_CHANGED);
	m_player->InvalidatePhysicsRecursive(InvalidatePhysicsBits_t::ANIMATION_CHANGED);
	m_player->InvalidatePhysicsRecursive(InvalidatePhysicsBits_t::BOUNDS_CHANGED);
	m_player->InvalidatePhysicsRecursive(InvalidatePhysicsBits_t::SEQUENCE_CHANGED);

	// if fake angles.
	if( fake ) {
		// correct poses.
		g_resolver.ResolvePoses( m_player, record );
	}

	// store updated/animated poses and rotation in lagrecord.
	m_player->GetPoseParameters( record->m_poses );
	record->m_abs_ang = m_player->GetAbsAngles( );

	// restore backup data.
	m_player->m_vecOrigin( ) = backup.m_origin;
	m_player->m_vecVelocity( ) = backup.m_velocity;
	m_player->m_vecAbsVelocity( ) = backup.m_abs_velocity;
	m_player->m_fFlags( ) = backup.m_flags;
	m_player->m_iEFlags( ) = backup.m_eflags;
	m_player->m_flDuckAmount( ) = backup.m_duck;
	m_player->m_flLowerBodyYawTarget( ) = backup.m_body;
	m_player->SetAbsOrigin( backup.m_abs_origin );
	m_player->SetAnimLayers( backup.m_layers );

	// restore globals.
	g_csgo.m_globals->m_curtime = curtime;
	g_csgo.m_globals->m_realtime = realtime;
	g_csgo.m_globals->m_frametime = frametime;
	g_csgo.m_globals->m_abs_frametime = absframetime;
	g_csgo.m_globals->m_frame = framecount;
	g_csgo.m_globals->m_tick_count = tickcount;
	g_csgo.m_globals->m_interp_amt = interpolation;

	// IMPORTANT: do not restore poses here, since we want to preserve them for rendering.
	// also dont restore the render angles which indicate the model rotation.
}

void AimPlayer::OnNetUpdate(Player* player) {
	bool reset = (!g_menu2.main.aimbot.enable || player->m_lifeState() == LIFE_DEAD || !player->enemy(g_cl.m_local));
	bool disable = (!reset && !g_cl.m_processing);

	// if this happens, delete all the lagrecords.
	if (reset) {
		player->m_bClientSideAnimation() = true;
		m_records.clear();
		return;
	}

	// just disable anim if this is the case.
	if (disable) {
		player->m_bClientSideAnimation() = true;
		return;
	}

	// update player ptr if required.
	// reset player if changed.
	if (m_player != player)
		m_records.clear();

	// update player ptr.
	m_player = player;

	// indicate that this player has been out of pvs.
	// insert dummy record to separate records
	// to fix stuff like animation and prediction.
	if (player->dormant()) {
		bool insert = true;

		// we have any records already?
		if (!m_records.empty()) {

			LagRecord* front = m_records.front().get();

			// we already have a dormancy separator.
			if (front->dormant())
				insert = false;
		}

		if (insert) {
			m_moved = false;

			// add new record.
			m_records.emplace_front(std::make_shared< LagRecord >(player));

			// get reference to newly added record.
			LagRecord* current = m_records.front().get();

			// mark as dormant.
			current->m_dormant = true;
		}
	}

	bool update = ( m_records.empty( ) || player->m_flSimulationTime( ) > m_records.front( ).get( )->m_sim_time );// is this players simulation invalid?
	bool m_bUpdatePlayerAnims = false; // should we update this players data this frame?

	// ensure this entity isn't dormant or null simtime before attempting to update him.
	if( !update && !player->dormant( ) && player->m_flSimulationTime( ) != 0.f ) {
		//update = true; pretty sure this was the problem of double updating in supremacy lol.

		if ( player->m_AnimOverlay( )[ 11 ].m_cycle != m_records.front( ).get( )->m_sim_cycle || player->m_AnimOverlay( )[ 11 ].m_playback_rate != m_records.front( ).get( )->m_sim_rate )
			// all is well, update this player's data.
			m_bUpdatePlayerAnims = true;

		// fix data.
		player->m_flSimulationTime() = game::TICKS_TO_TIME(g_csgo.m_cl->m_server_tick);
	}

	bool m_bFinalUpdate = false; // our final decision to update my mans.

	// determine wether to actually update dawg.
	if( m_bUpdatePlayerAnims || update )
		m_bFinalUpdate = true;

	// don't update this player's animations..
	if( !m_bFinalUpdate )
		return;

	// this is the first data update we are receving
	// OR we received data with a newer simulation context.
	if( m_bFinalUpdate ) {
		// add new record.
		m_records.emplace_front(std::make_shared< LagRecord >(player));

		// get reference to newly added record.
		LagRecord* current = m_records.front().get();

		// mark as non dormant.
		current->m_dormant = false;

		// update animations on current record.
		// call resolver.
		UpdateAnimations(current);

		// create bone matrix for this record.
		g_bones.setup(m_player, nullptr, current);
	}

	// no need to store insane amt of data.
	while (m_records.size() > 256)
		m_records.pop_back();
}

void AimPlayer::OnRoundStart(Player* player) {
	m_player = player;
	m_walk_record = LagRecord{ };
	m_shots = 0;
	m_missed_shots = 0;

	// reset stand and body index.
	m_stand_index = 0;
	m_stand_index2 = 0;
	m_body_index = 0;
	m_upd_index = 0;

	m_records.clear();
	m_hitboxes.clear();

	// IMPORTANT: DO NOT CLEAR LAST HIT SHIT.
}

void AimPlayer::SetupHitboxes(LagRecord* record, bool history) {
	// reset hitboxes.
	m_hitboxes.clear();


	bool prefer_head = record->m_velocity.length_2d() > 71.f;

	// prefer

	if (g_menu.main.aimbot.head1.get(0))
		m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::PREFER });

	if (g_menu.main.aimbot.head1.get(1) && prefer_head)
		m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::PREFER });

	if (g_menu.main.aimbot.head1.get(2) && !(record->m_mode != Resolver::Modes::RESOLVE_NONE && record->m_mode != Resolver::Modes::RESOLVE_WALK && record->m_mode != Resolver::Modes::RESOLVE_BODY))
		m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::PREFER });

	if (g_menu.main.aimbot.head1.get(3) && !(record->m_pred_flags & FL_ONGROUND))
		m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::PREFER });

	if (g_cl.m_weapon_id == ZEUS) {
		// hitboxes for the zeus.
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });
		return;
	}

	// prefer, always.
	if (g_menu.main.aimbot.baim1.get(0))
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });

	// prefer, lethal.
	if (g_menu.main.aimbot.baim1.get(1))
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::LETHAL });

	// prefer, lethal x2.
	if (g_menu.main.aimbot.baim1.get(2))
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::LETHAL2 });

	// prefer, fake.
	if (g_menu.main.aimbot.baim1.get(3) && record->m_mode != Resolver::Modes::RESOLVE_NONE && record->m_mode != Resolver::Modes::RESOLVE_WALK && record->m_mode != Resolver::Modes::RESOLVE_BODY)
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });

	// prefer, in air.
	if (g_menu.main.aimbot.baim1.get(4) && !(record->m_pred_flags & FL_ONGROUND))
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });

	// prefer, in air.
	if (g_menu.main.aimbot.baim1.get(5) && (m_last_move >= g_menu.main.aimbot.misses.get() || m_unknown_move >= g_menu.main.aimbot.misses.get()))
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });

	bool only{ false };

	// only, always.
	if (g_menu.main.aimbot.baim2.get(0)) {
		only = true;
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });
	}

	// only, health.
	if (g_menu.main.aimbot.baim2.get(1) && m_player->m_iHealth() <= (int)g_menu.main.aimbot.baim_hp.get()) {
		only = true;
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });
	}

	// only, fake.
	if (g_menu.main.aimbot.baim2.get(2) && record->m_mode != Resolver::Modes::RESOLVE_NONE && record->m_mode != Resolver::Modes::RESOLVE_WALK && record->m_mode != Resolver::Modes::RESOLVE_BODY) {
		only = true;
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });
	}

	// only, in air.
	if (g_menu.main.aimbot.baim2.get(3) && !(record->m_pred_flags & FL_ONGROUND)) {
		only = true;
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });
	}

	// only, in air.
	if (g_menu.main.aimbot.baim2.get(4) && (m_last_move >= g_menu.main.aimbot.misses.get() || m_unknown_move >= g_menu.main.aimbot.misses.get())) {
		only = true;
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });
	}

	// only, on key.
	if (g_input.GetKeyState(g_menu.main.aimbot.baim_key.get())) {
		only = true;
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });
	}

	// only baim conditions have been met.
	// do not insert more hitboxes.
	if (only)
		return;

	std::vector< size_t > hitbox{ history ? g_menu.main.aimbot.hitbox_history.GetActiveIndices() : g_menu.main.aimbot.hitbox.GetActiveIndices() };
	if (hitbox.empty())
		return;

	bool ignore_limbs = record->m_velocity.length_2d() > 71.f && g_menu.main.aimbot.ignor_limbs.get();

	for (const auto& h : hitbox) {
		// head.
		if (h == 0)
			m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::NORMAL });

		// chest.
		if (h == 1) {
			m_hitboxes.push_back({ HITBOX_THORAX, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_CHEST, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_UPPER_CHEST, HitscanMode::NORMAL });
		}

		// stomach.
		if (h == 2) {
			m_hitboxes.push_back({ HITBOX_PELVIS, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::NORMAL });
		}

		// arms.
		if (h == 3 && !ignore_limbs) {
			m_hitboxes.push_back({ HITBOX_L_UPPER_ARM, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_R_UPPER_ARM, HitscanMode::NORMAL });
		}

		// legs.
		if (h == 4) {
			m_hitboxes.push_back({ HITBOX_L_THIGH, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_R_THIGH, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_L_CALF, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_R_CALF, HitscanMode::NORMAL });
		}

		// foot.
		if (h == 5 && !ignore_limbs) {
			m_hitboxes.push_back({ HITBOX_L_FOOT, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_R_FOOT, HitscanMode::NORMAL });
		}
	}
}

void Aimbot::init() {
	// clear old targets.
	m_targets.clear();

	m_target = nullptr;
	m_aim = vec3_t{ };
	m_angle = ang_t{ };
	m_damage = 0.f;
	m_record = nullptr;
	m_stop = false;

	m_best_dist = std::numeric_limits< float >::max();
	m_best_fov = 180.f + 1.f;
	m_best_damage = 0.f;
	m_best_hp = 100 + 1;
	m_best_lag = std::numeric_limits< float >::max();
	m_best_height = std::numeric_limits< float >::max();

	m_shoot_next_tick = false;
}

void Aimbot::StripAttack() {
	if (g_cl.m_weapon_id == REVOLVER)
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK2;

	else
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK;
}

void Aimbot::think() {
	// do all startup routines.
	init();

	// sanity.
	if (!g_cl.m_weapon)
		return;

	// no grenades or bomb.
	if (g_cl.m_weapon_type == WEAPONTYPE_GRENADE || g_cl.m_weapon_type == WEAPONTYPE_C4)
		return;

	if (!g_cl.m_weapon_fire)
		StripAttack();

	// we have no aimbot enabled.
	if (!g_menu.main.aimbot.enable.get())
		return;

	// animation silent aim, prevent the ticks with the shot in it to become the tick that gets processed.
	// we can do this by always choking the tick before we are able to shoot.
	bool revolver = g_cl.m_weapon_id == REVOLVER && g_cl.m_revolver_cock != 0;

	// one tick before being able to shoot.
	if (revolver && g_cl.m_revolver_cock > 0 && g_cl.m_revolver_cock == g_cl.m_revolver_query) {
		*g_cl.m_packet = false;
		return;
	}

	// we have a normal weapon or a non cocking revolver
	// choke if its the processing tick.
	if (g_cl.m_weapon_fire && !g_cl.m_lag && !revolver) {
		*g_cl.m_packet = false;
		StripAttack();
		return;
	}

	// no point in aimbotting if we cannot fire this tick.
	if (!g_cl.m_weapon_fire)
		return;

	// setup bones for all valid targets.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!IsValidTarget(player))
			continue;

		AimPlayer* data = &m_players[i - 1];
		if (!data)
			continue;

		// store player as potential target this tick.
		m_targets.emplace_back(data);
	}

	// run knifebot.
	if (g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS) {

		if (g_menu.main.aimbot.knifebot.get())
			knife();

		return;
	}

	// scan available targets... if we even have any.
	find();

	// finally set data when shooting.
	apply();
}

void Aimbot::find() {
	struct BestTarget_t { Player* player;  AimPlayer* target; vec3_t pos; float damage; LagRecord* record; };

	vec3_t       tmp_pos;
	float        tmp_damage;
	BestTarget_t best;
	best.player = nullptr;
	best.target = nullptr;
	best.damage = -1.f;
	best.pos = vec3_t{ };
	best.record = nullptr;

	if (m_targets.empty())
		return;

	if (g_cl.m_weapon_id == ZEUS && !g_menu.main.aimbot.zeusbot.get())
		return;
	// iterate all targets.
	for (const auto& t : m_targets) {
		if (t->m_records.empty())
			continue;

		// this player broke lagcomp.
		// his bones have been resetup by our lagcomp.
		// therfore now only the front record is valid.
		if (g_menu.main.aimbot.lagfix.get() && g_lagcomp.StartPrediction(t)) {
			LagRecord* front = t->m_records.front().get();

			if (g_csgo.m_globals->m_tick_count % 5 == 1) {
				t->SetupHitboxes(front, false);
				if (t->m_hitboxes.empty())
					continue;

				// rip something went wrong..
				if (t->GetBestAimPosition(tmp_pos, tmp_damage, front) && SelectTarget(front, tmp_pos, tmp_damage)) {

					// if we made it so far, set shit.
					best.player = t->m_player;
					best.pos = tmp_pos;
					best.damage = tmp_damage;
					best.record = front;
					best.target = t;
				}
			}
		}

		// player did not break lagcomp.
		// history aim is possible at this point.
		else {
			LagRecord* ideal = g_resolver.FindIdealRecord(t);
			if (!ideal)
				continue;
			if (g_csgo.m_globals->m_tick_count % 5 == 1) {
				t->SetupHitboxes(ideal, false);
				if (t->m_hitboxes.empty())
					continue;

				// try to select best record as target.
				if (t->GetBestAimPosition(tmp_pos, tmp_damage, ideal) && SelectTarget(ideal, tmp_pos, tmp_damage)) {
					// if we made it so far, set shit.
					best.player = t->m_player;
					best.pos = tmp_pos;
					best.damage = tmp_damage;
					best.record = ideal;
					best.target = t;
				}
			}

			LagRecord* last = g_resolver.FindLastRecord(t);
			if (!last || last == ideal)
				continue;

			t->SetupHitboxes(last, true);
			if (t->m_hitboxes.empty())
				continue;

			// rip something went wrong..
			if (t->GetBestAimPosition(tmp_pos, tmp_damage, last) && SelectTarget(last, tmp_pos, tmp_damage)) {
				// if we made it so far, set shit.
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.record = last;
				best.target = t;
			}
		}
	}
	// verify our target and set needed data.
	if (best.player && best.record) {
		// calculate aim angle.
		math::VectorAngles(best.pos - g_cl.m_shoot_pos, m_angle);

		// set member vars.
		m_target = best.player;
		m_aim = best.pos;
		m_damage = best.damage;
		m_record = best.record;

		if (best.target != m_old_target) {
			m_shoot_next_tick = false;
		}
		// write data, needed for traces / etc.
		m_record->cache();

		const bool bOnLand = !( g_cl.m_flags & FL_ONGROUND ) && g_cl.m_local->m_fFlags( ) & FL_ONGROUND;

		// set autostop shit.
		if( g_cl.m_local->m_fFlags( ) & FL_ONGROUND && !bOnLand ) {
			// we have autostop on and not in walk exploit.
			if( g_menu.main.movement.autostop_always_on.get( ) && !g_input.GetKeyState( g_menu.main.movement.nefariouswalk.get( ) ) ) {
				// since we autostop when fakewalking when choke limit is reached.
				if( !g_input.GetKeyState( g_menu.main.movement.fakewalk.get( ) ) ) {
					// set this, if we arent jumping.
					m_stop = !( g_cl.m_buttons & IN_JUMP );
				}
			}
		}

		float hitchance = g_menu.main.aimbot.hitchance_amount.get();


		if (!g_cl.m_local->m_bIsScoped() && g_menu.main.aimbot.hitchance_noscope.get())
			hitchance = g_menu.main.aimbot.hitchance_noscope_amount.get();

		bool on = g_menu.main.config.mode.get() == 0;
		bool hit = on && CheckHitchance(m_target, m_angle, hitchance);

		// if we can scope.
		bool can_scope = !g_cl.m_local->m_bIsScoped() && (g_cl.m_weapon_id == AUG || g_cl.m_weapon_id == SG553 || g_cl.m_weapon_type == WEAPONTYPE_SNIPER_RIFLE);

		if (can_scope) {
			// always.
			if (g_menu.main.aimbot.zoom.get() == 1) {
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;
				return;
			}

			// hitchance fail.
			else if (g_menu.main.aimbot.zoom.get() == 2 && on && !hit) {
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;
				return;
			}
		}

		if (hit || !on || m_shoot_next_tick && m_double_tap) {
			// right click attack.
			if (g_menu.main.config.mode.get() == 1 && g_cl.m_weapon_id == REVOLVER)
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;

			// left click attack.
			else
				g_cl.m_cmd->m_buttons |= IN_ATTACK;

			m_old_target = best.target;
		}
	}
}

bool Aimbot::CanHit(vec3_t start, vec3_t end, LagRecord* record, int box, bool in_shot, BoneArray* bones)
{
	if (!record || !record->m_player)
		return false;

	// backup player
	const auto backup_origin = record->m_player->m_vecOrigin();
	const auto backup_abs_origin = record->m_player->GetAbsOrigin();
	const auto backup_abs_angles = record->m_player->GetAbsAngles();
	const auto backup_obb_mins = record->m_player->m_vecMins();
	const auto backup_obb_maxs = record->m_player->m_vecMaxs();
	const auto backup_cache = record->m_player->m_iBoneCache();

	// always try to use our aimbot matrix first.
	auto matrix = record->m_bones;

	// this is basically for using a custom matrix.
	if (in_shot)
		matrix = bones;

	if (!matrix)
		return false;

	const model_t* model = record->m_player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(record->m_player->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(box);
	if (!bbox)
		return false;

	vec3_t min, max;
	const auto IsCapsule = bbox->m_radius != -1.f;

	if (IsCapsule) {
		math::VectorTransform(bbox->m_mins, matrix[bbox->m_bone], min);
		math::VectorTransform(bbox->m_maxs, matrix[bbox->m_bone], max);
		const auto dist = math::SegmentToSegment(start, end, min, max);

		if (dist < bbox->m_radius) {
			return true;
		}
	}
	else {
		CGameTrace tr;

		// setup trace data
		record->m_player->m_vecOrigin() = record->m_origin;
		record->m_player->SetAbsOrigin(record->m_origin);
		record->m_player->SetAbsAngles(record->m_abs_ang);
		record->m_player->m_vecMins() = record->m_mins;
		record->m_player->m_vecMaxs() = record->m_maxs;
		record->m_player->m_iBoneCache() = reinterpret_cast<matrix3x4_t**>(matrix);

		// setup ray and trace.
		g_csgo.m_engine_trace->ClipRayToEntity(Ray(start, end), MASK_SHOT, record->m_player, &tr);

		record->m_player->m_vecOrigin() = backup_origin;
		record->m_player->SetAbsOrigin(backup_abs_origin);
		record->m_player->SetAbsAngles(backup_abs_angles);
		record->m_player->m_vecMins() = backup_obb_mins;
		record->m_player->m_vecMaxs() = backup_obb_maxs;
		record->m_player->m_iBoneCache() = backup_cache;

		// check if we hit a valid player / hitgroup on the player and increment total hits.
		if (tr.m_entity == record->m_player && game::IsValidHitgroup(tr.m_hitgroup))
			return true;
	}

	return false;
}

bool Aimbot::CheckHitchance(Player* player, const ang_t& angle, float chance) {
	if (chance < 1)
		return true;

	vec3_t forward, right, up;
	vec3_t src = g_cl.m_shoot_pos;
	math::AngleVectors(angle, &forward, &right, &up);
	CGameTrace tr;

	int cHits = 0;
	int cNeededHits = static_cast<int>(255 * (chance / 78));

	g_cl.m_weapon->UpdateAccuracyPenalty();
	float weap_spread = g_cl.m_weapon->GetSpread();
	float weap_inaccuracy = g_cl.m_weapon->GetInaccuracy();
	float weapon_range = g_cl.m_weapon->GetWpnData()->m_range;

	for (int i = 0; i < 255; i++)
	{
		math::random_seed(i);

		float a = math::random_float(0.f, 1.f);
		float b = math::random_float(0.f, 2.f * M_PI);
		float c = math::random_float(0.f, 1.f);
		float d = math::random_float(0.f, 2.f * M_PI);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		vec3_t spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.normalized();

		ang_t viewAnglesSpread;
		math::VectorAngles3(direction, up, viewAnglesSpread);
		math::Normalize(viewAnglesSpread);

		vec3_t viewForward;
		math::AngleVectors69(viewAnglesSpread, viewForward);
		viewForward.normalize();
		viewForward = src + (viewForward * weapon_range);
		g_csgo.m_engine_trace->ClipRayToEntity(Ray(src, viewForward), MASK_SHOT, player, &tr);

		if (tr.m_entity == player && game::IsValidHitgroup(tr.m_hitgroup))
			cHits++;

		if (static_cast<int>((static_cast<float>(cHits) / 255.f) * 78.f) >= chance)
			return true;

		if ((255 - i + cHits) < cNeededHits)
			return false;
	}
	return false;
}


bool AimPlayer::SetupHitboxPoints(LagRecord* record, BoneArray* bones, int index, std::vector< vec3_t >& points) {
	// reset points.
	points.clear();

	const model_t* model = m_player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(m_player->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(index);
	if (!bbox)
		return false;

	// get hitbox scales.
	float scale = g_menu.main.aimbot.scale.get() / 100.f;

	// big inair fix.
	if (!(record->m_pred_flags & FL_ONGROUND))
		scale = 0.7f;

	float bscale = g_menu.main.aimbot.body_scale.get() / 100.f;

	// these indexes represent boxes.
	if (bbox->m_radius <= 0.f) {
		// references: 
		//      https://developer.valvesoftware.com/wiki/Rotation_Tutorial
		//      CBaseAnimating::GetHitboxBonePosition
		//      CBaseAnimating::DrawServerHitboxes

		// convert rotation angle to a matrix.
		matrix3x4_t rot_matrix;
		g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

		// apply the rotation to the entity input space (local).
		matrix3x4_t matrix;
		math::ConcatTransforms(bones[bbox->m_bone], rot_matrix, matrix);

		// extract origin from matrix.
		vec3_t origin = matrix.GetOrigin();

		// compute raw center point.
		vec3_t center = (bbox->m_mins + bbox->m_maxs) / 2.f;

		// the feet hiboxes have a side, heel and the toe.
		if (index == HITBOX_R_FOOT || index == HITBOX_L_FOOT) {
			float d1 = (bbox->m_mins.z - center.z) * 0.875f;

			// invert.
			if (index == HITBOX_L_FOOT)
				d1 *= -1.f;

			// side is more optimal then center.
			points.push_back({ center.x, center.y, center.z + d1 });

			if (g_menu.main.aimbot.multipoint.get(3)) {
				// get point offset relative to center point
				// and factor in hitbox scale.
				float d2 = (bbox->m_mins.x - center.x) * scale;
				float d3 = (bbox->m_maxs.x - center.x) * scale;

				// heel.
				points.push_back({ center.x + d2, center.y, center.z });

				// toe.
				points.push_back({ center.x + d3, center.y, center.z });
			}
		}

		// nothing to do here we are done.
		if (points.empty())
			return false;

		// rotate our bbox points by their correct angle
		// and convert our points to world space.
		for (auto& p : points) {
			// VectorRotate.
			// rotate point by angle stored in matrix.
			p = { p.dot(matrix[0]), p.dot(matrix[1]), p.dot(matrix[2]) };

			// transform point to world space.
			p += origin;
		}
	}

	// these hitboxes are capsules.
	else {
		// factor in the pointscale.
		float r = bbox->m_radius * scale;
		float br = bbox->m_radius * bscale;

		// compute raw center point.
		vec3_t center = (bbox->m_mins + bbox->m_maxs) / 2.f;

		// head has 5 points.
		if (index == HITBOX_HEAD) {
			// add center.
			points.push_back(center);

			if (g_menu.main.aimbot.multipoint.get(0)) {
				// rotation matrix 45 degrees.
				// https://math.stackexchange.com/questions/383321/rotating-x-y-points-45-degrees
				// std::cos( deg_to_rad( 45.f ) )
				constexpr float rotation = 0.70710678f;

				// top/back 45 deg.
				// this is the best spot to shoot at.
				points.push_back({ bbox->m_maxs.x + (rotation * r), bbox->m_maxs.y + (-rotation * r), bbox->m_maxs.z });

				// right.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z + r });

				// left.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z - r });

				// back.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y - r, bbox->m_maxs.z });

				// get animstate ptr.
				CCSGOPlayerAnimState* state = record->m_player->m_PlayerAnimState();

				// add this point only under really specific circumstances.
				// if we are standing still and have the lowest possible pitch pose.
				if (state && record->m_anim_velocity.length() <= 0.1f && record->m_eye_angles.x <= state->m_min_pitch) {

					// bottom point.
					points.push_back({ bbox->m_maxs.x - r, bbox->m_maxs.y, bbox->m_maxs.z });
				}
			}
		}

		// body has 5 points.
		else if (index == HITBOX_BODY) {
			// center.
			points.push_back(center);

			// back.
			if (g_menu.main.aimbot.multipoint.get(2))
				points.push_back({ center.x, bbox->m_maxs.y - br, center.z });
		}

		else if (index == HITBOX_PELVIS || index == HITBOX_UPPER_CHEST) {
			// back.
			points.push_back({ center.x, bbox->m_maxs.y - r, center.z });
		}

		// other stomach/chest hitboxes have 2 points.
		else if (index == HITBOX_THORAX || index == HITBOX_CHEST) {
			// add center.
			points.push_back(center);

			// add extra point on back.
			if (g_menu.main.aimbot.multipoint.get(1))
				points.push_back({ center.x, bbox->m_maxs.y - r, center.z });
		}

		else if (index == HITBOX_R_CALF || index == HITBOX_L_CALF) {
			// add center.
			points.push_back(center);

			// half bottom.
			if (g_menu.main.aimbot.multipoint.get(3))
				points.push_back({ bbox->m_maxs.x - (bbox->m_radius / 2.f), bbox->m_maxs.y, bbox->m_maxs.z });
		}

		else if (index == HITBOX_R_THIGH || index == HITBOX_L_THIGH) {
			// add center.
			points.push_back(center);
		}

		// arms get only one point.
		else if (index == HITBOX_R_UPPER_ARM || index == HITBOX_L_UPPER_ARM) {
			// elbow.
			points.push_back({ bbox->m_maxs.x + bbox->m_radius, center.y, center.z });
		}

		// nothing left to do here.
		if (points.empty())
			return false;

		// transform capsule points.
		for (auto& p : points)
			math::VectorTransform(p, bones[bbox->m_bone], p);
	}

	return true;
}

bool AimPlayer::GetBestAimPosition(vec3_t& aim, float& damage, LagRecord* record) {
	bool                  done, pen;
	float                 dmg, pendmg;
	HitscanData_t         scan;
	std::vector< vec3_t > points;

	// get player hp.
	int hp = std::min(100, m_player->m_iHealth());

	if (g_cl.m_weapon_id == ZEUS) {
		dmg = pendmg = hp;
		pen = true;
	}

	else {
		dmg = g_menu.main.aimbot.minimal_damage.get();
		if (g_menu.main.aimbot.minimal_damage_hp.get())
			dmg = std::ceil((dmg / 100.f) * hp);

		pendmg = g_menu.main.aimbot.penetrate_minimal_damage.get();
		if (g_menu.main.aimbot.penetrate_minimal_damage_hp.get())
			pendmg = std::ceil((pendmg / 100.f) * hp);

		pen = g_menu.main.aimbot.penetrate.get();
	}

	// write all data of this record l0l.
	record->cache();

	// iterate hitboxes.
	for (const auto& it : m_hitboxes) {
		done = false;

		// setup points on hitbox.
		if (!SetupHitboxPoints(record, record->m_bones, it.m_index, points))
			continue;

		// iterate points on hitbox.
		for (const auto& point : points) {
			penetration::PenetrationInput_t in;

			in.m_damage = dmg;
			in.m_damage_pen = pendmg;
			in.m_can_pen = pen;
			in.m_target = m_player;
			in.m_from = g_cl.m_local;
			in.m_pos = point;

			// ignore mindmg.
			if (it.m_mode == HitscanMode::LETHAL || it.m_mode == HitscanMode::LETHAL2)
				in.m_damage = in.m_damage_pen = 1.f;

			penetration::PenetrationOutput_t out;

			// we can hit p!
			if (penetration::run(&in, &out)) {

				// nope we did not hit head..
				if (it.m_index == HITBOX_HEAD && out.m_hitgroup != HITGROUP_HEAD)
					continue;

				// prefered hitbox, just stop now.
				if (it.m_mode == HitscanMode::PREFER)
					done = true;

				// this hitbox requires lethality to get selected, if that is the case.
				// we are done, stop now.
				else if (it.m_mode == HitscanMode::LETHAL && out.m_damage >= m_player->m_iHealth())
					done = true;

				// 2 shots will be sufficient to kill.
				else if (it.m_mode == HitscanMode::LETHAL2 && (out.m_damage * 2.f) >= m_player->m_iHealth())
					done = true;

				// this hitbox has normal selection, it needs to have more damage.
				else if (it.m_mode == HitscanMode::NORMAL) {
					// we did more damage.
					if (out.m_damage >= scan.m_damage) {
						// save new best data.
						scan.m_damage = out.m_damage;
						scan.m_pos = point;

						// if the first point is lethal
						// screw the other ones.
						if (point == points.front() && out.m_damage >= m_player->m_iHealth())
							break;
					}
				}

				// we found a preferred / lethal hitbox.
				if (done) {
					// save new best data.
					scan.m_damage = out.m_damage;
					scan.m_pos = point;
					break;
				}
			}
		}

		// ghetto break out of outer loop.
		if (done)
			break;
	}

	// we found something that we can damage.
	// set out vars.
	if (scan.m_damage > 0.f) {
		aim = scan.m_pos;
		damage = scan.m_damage;
		return true;
	}

	return false;
}

bool Aimbot::SelectTarget(LagRecord* record, const vec3_t& aim, float damage) {
	float dist, fov, height;
	int   hp;

	// fov check.
	if (g_menu.main.aimbot.fov.get()) {
		// if out of fov, retn false.
		if (math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, aim) > g_menu.main.aimbot.fov_amount.get())
			return false;
	}

	switch (g_menu.main.aimbot.selection.get()) {

		// distance.
	case 0:
		dist = (record->m_pred_origin - g_cl.m_shoot_pos).length();

		if (dist < m_best_dist) {
			m_best_dist = dist;
			return true;
		}

		break;

		// crosshair.
	case 1:
		fov = math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, aim);

		if (fov < m_best_fov) {
			m_best_fov = fov;
			return true;
		}

		break;

		// damage.
	case 2:
		if (damage > m_best_damage) {
			m_best_damage = damage;
			return true;
		}

		break;

		// lowest hp.
	case 3:
		// fix for retarded servers?
		hp = std::min(100, record->m_player->m_iHealth());

		if (hp < m_best_hp) {
			m_best_hp = hp;
			return true;
		}

		break;

		// least lag.
	case 4:
		if (record->m_lag < m_best_lag) {
			m_best_lag = record->m_lag;
			return true;
		}

		break;

		// height.
	case 5:
		height = record->m_pred_origin.z - g_cl.m_local->m_vecOrigin().z;

		if (height < m_best_height) {
			m_best_height = height;
			return true;
		}

		break;

	default:
		return false;
	}

	return false;
}

void Aimbot::apply() {
	bool attack, attack2;

	// attack states.
	attack = (g_cl.m_cmd->m_buttons & IN_ATTACK);
	attack2 = (g_cl.m_weapon_id == REVOLVER && g_cl.m_cmd->m_buttons & IN_ATTACK2);

	// ensure we're attacking.
	if (attack || attack2) {
		// choke every shot.
		*g_cl.m_packet = false;

		if (m_shoot_next_tick)
			m_shoot_next_tick = false;

		if (m_target) {
			// make sure to aim at un-interpolated data.
			// do this so BacktrackEntity selects the exact record.
			if (m_record && !m_record->m_broke_lc)
				g_cl.m_cmd->m_tick = game::TIME_TO_TICKS(m_record->m_sim_time + g_cl.m_lerp);

			// set angles to target.
			g_cl.m_cmd->m_view_angles = m_angle;

			// if not silent aim, apply the viewangles.
			if (!g_menu.main.aimbot.silent.get())
				g_csgo.m_engine->SetViewAngles(m_angle);

			if (g_menu.main.aimbot.debugaim.get()) {
				Color the_color = g_menu.main.aimbot.debugaim_color.get();
				g_visuals.DrawHitboxMatrix(m_record, the_color, 10.f);
			}
		}

		// nospread.
		if (g_menu.main.aimbot.nospread.get() && g_menu.main.config.mode.get() == 1)
			NoSpread();

		// norecoil.
		if (g_menu.main.aimbot.norecoil.get())
			g_cl.m_cmd->m_view_angles -= g_cl.m_local->m_aimPunchAngle() * g_csgo.weapon_recoil_scale->GetFloat();

		// store fired shot.
		g_shots.OnShotFire(m_target ? m_target : nullptr, m_target ? m_damage : -1.f, g_cl.m_weapon_info->m_bullets, m_target ? m_record : nullptr);

		// set that we fired.
		g_cl.m_shot = true;

		if (!m_shoot_next_tick && g_cl.m_tick_to_shift > 8 && g_cl.m_charged) {
			m_shoot_next_tick = true;
		}
	}
}

void Aimbot::NoSpread() {
	bool    attack2;
	vec3_t  spread, forward, right, up, dir;

	// revolver state.
	attack2 = (g_cl.m_weapon_id == REVOLVER && (g_cl.m_cmd->m_buttons & IN_ATTACK2));

	// get spread.
	spread = g_cl.m_weapon->CalculateSpread(g_cl.m_cmd->m_random_seed, attack2);

	// compensate.
	g_cl.m_cmd->m_view_angles -= { -math::rad_to_deg(std::atan(spread.length_2d())), 0.f, math::rad_to_deg(std::atan2(spread.x, spread.y)) };
}

bool Aimbot::can_hit_hitbox(const vec3_t start, const vec3_t end, LagRecord* animation, int box, bool in_shot, BoneArray* bones)
{
	if (!animation || !animation->m_player)
		return false;

	// always try to use our aimbot matrix first.
	auto matrix = m_current_matrix;

	// this is basically for using a custom matrix.
	if (in_shot)
		matrix = bones;

	if (!matrix)
		return false;

	const model_t* model = animation->m_player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(animation->m_player->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(box);
	if (!bbox)
		return false;


	vec3_t min, max;

	const auto is_capsule = bbox->m_radius != -1.f;

	if (is_capsule)
	{
		math::VectorTransform(bbox->m_mins, animation->m_bones[bbox->m_bone], min);
		math::VectorTransform(bbox->m_maxs, animation->m_bones[bbox->m_bone], max);
		const auto dist = math::SegmentToSegment(start, end, min, max);

		if (dist < bbox->m_radius)
			return true;
	}

	if (!is_capsule)
	{
		math::VectorTransform(math::vector_rotate(bbox->m_mins, bbox->m_angle), animation->m_bones[bbox->m_bone], min);
		math::VectorTransform(math::vector_rotate(bbox->m_maxs, bbox->m_angle), animation->m_bones[bbox->m_bone], max);

		math::VectorITransform(start, animation->m_bones[bbox->m_bone], min);
		math::vector_i_rotate(end, animation->m_bones[bbox->m_bone], max);

		if (math::IntersectLineWithBB(min, max, bbox->m_mins, bbox->m_maxs))
			return true;
	}

	return false;
}