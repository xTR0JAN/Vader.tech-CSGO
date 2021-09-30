#include "includes.h"

LagCompensation g_lagcomp{};;

#define DEG2RAD( x  )  ( (float)(x) * (float)(3.14159265358979323846f / 180.f) )
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / 3.14159265358979323846f) )

void LagCompensation::recalc_velocity(LagRecord* record, AimPlayer* data, LagRecord* previous) {
	static auto sv_gravity = g_csgo.m_cvar->FindVar(HASH("sv_gravity"));
	static auto sv_jump_impulse = g_csgo.m_cvar->FindVar(HASH("sv_jump_impulse"));
	static auto sv_enablebunnyhopping = g_csgo.m_cvar->FindVar(HASH("sv_enablebunnyhopping"));

	if (record->m_flags & FL_ONGROUND
		&& record->m_layers[11].m_weight > 0.0f
		&& record->m_layers[11].m_weight < 1.0f)
	{
		// float val = clamp ( ( speed - 0.55f ) / ( 0.9f - 0.55f), 0.f, 1.f );
		// layer11_weight = 1.f - val;
		auto val = (1.0f - record->m_layers[11].m_weight) * 0.35f;

		if (val > 0.0f && val < 1.0f)
			record->animation_speed = val + 0.55f;
		else
			record->animation_speed = -1.f;
	}

	if (_fdtest(&record->m_velocity.x) > 0
		|| _fdtest(&record->m_velocity.y) > 0
		|| _fdtest(&record->m_velocity.z) > 0)
		record->m_velocity.clear();

	if (!record->dormant() && previous && !previous->dormant() && previous->valid())
	{
		//
		//	calculate new velocity based on (new_origin - old_origin) / (new_time - old_time) formula.
		//
		if (record->m_lag > 1 && record->m_lag <= 20)
			record->m_velocity = (record->m_origin - previous->m_origin) / record->time_delta;

		if (abs(record->m_velocity.x) < 0.001f)
			record->m_velocity.x = 0.0f;
		if (abs(record->m_velocity.y) < 0.001f)
			record->m_velocity.y = 0.0f;
		if (abs(record->m_velocity.z) < 0.001f)
			record->m_velocity.z = 0.0f;

		if (_fdtest(&record->m_velocity.x) > 0
			|| _fdtest(&record->m_velocity.y) > 0
			|| _fdtest(&record->m_velocity.z) > 0)
			record->m_velocity.clear();

		auto curr_direction = RAD2DEG(std::atan2f(record->m_velocity.y, record->m_velocity.x));
		auto prev_direction = previous == nullptr ? FLT_MAX : RAD2DEG(std::atan2f(previous->m_velocity.y, previous->m_velocity.x));

		auto delta = math::normalize_float(curr_direction - prev_direction);

		//if (record->velocity.Length2D() > 0.1f) {
		//	if (previous->velocity.Length2D() > 0.1f && abs(delta) >= 60.f)
		//		r_log->last_time_changed_direction = g_pGlobals->realtime;
		//}
		//else
		//	r_log->last_time_changed_direction = 0;

		//
		// these requirements pass only when layer[6].weight is accurate to normalized velocity.
		//
		if (record->m_flags & FL_ONGROUND
			&& record->m_velocity.length_2d() >= 0.1f
			&& std::abs(delta) < 1.0f
			&& std::abs(record->m_duck - previous->m_duck) <= 0.0f
			&& record->m_layers[6].m_playback_rate > previous->m_layers[6].m_playback_rate
			&& record->m_layers[6].m_weight > previous->m_layers[6].m_weight)
		{
			auto weight_speed = record->m_layers[6].m_weight;

			if (weight_speed <= 0.7f && weight_speed > 0.0f)
			{
				if (record->m_layers[6].m_playback_rate == 0.0f)
					record->m_velocity.clear();
				else
				{
					const auto m_post_velocity_lenght = record->m_velocity.length_2d();

					if (m_post_velocity_lenght != 0.0f)
					{
						float mult = 1;
						if (record->m_flags & 6)
							mult = 0.34f;
						else if (record->m_fake_walk)
							mult = 0.52f;

						record->m_velocity.x = (record->m_velocity.x / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
						record->m_velocity.y = (record->m_velocity.y / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
					}
				}
			}
		}

		//
		// fix velocity with fakelag.
		//
		if (record->m_flags & FL_ONGROUND && record->m_velocity.length_2d() > 0.1f && record->m_lag > 1)
		{
			//
			// get velocity lenght from 11th layer calc.
			//
			if (record->animation_speed > 0) {
				const auto m_pre_velocity_lenght = record->m_velocity.length_2d();
				auto* weapon = record->m_player->GetActiveWeapon();

				if (weapon) {
					auto wdata = weapon->GetWpnData();
					if (wdata) {
						auto adjusted_velocity = (record->animation_speed * record->max_current_speed) / m_pre_velocity_lenght;
						record->m_velocity.x *= adjusted_velocity;
						record->m_velocity.y *= adjusted_velocity;
					}
				}
			}

			/*if (record->m_flags & FL_ONGROUND && (sv_enablebunnyhopping && !sv_enablebunnyhopping->GetBool() || previous->m_flags & FL_ONGROUND)) {
				auto max_speed = record->max_current_speed;

				if (record->m_flags & 6)
					max_speed *= 0.34f;
				else if (record->fake_walking)
					max_speed *= 0.52f;

				if (max_speed < m_pre_velocity_lenght)
					record->m_velocity *= (max_speed / m_pre_velocity_lenght);

				if (previous->m_flags & FL_ONGROUND)
					record->m_velocity.z = 0.f;
			}*/
		}

		if (data->m_records.size() > 2 && record->m_lag > 1 && !record->dormant()
			&& previous->m_velocity.length() > 0 && !(record->m_flags & FL_ONGROUND && previous->m_flags & FL_ONGROUND))
		{
			auto pre_pre_record = data->m_records[(data->m_records.size() - 2) & 63].get();

			if (!pre_pre_record->dormant() && pre_pre_record->valid()) {
				//if (record->m_velocity.Length2D() > (record->max_current_speed * 0.52f) && previous->m_velocity.Length2D() > (record->max_current_speed * 0.52f)
				//	|| record->m_velocity.Length2D() <= (record->max_current_speed * 0.52f) && previous->m_velocity.Length2D() <= (record->max_current_speed * 0.52f))
				//{
				//	auto manually_calculated = log->tick_records[(log->records_count - 2) & 63].stop_to_full_run_frac;
				//	manually_calculated += (record->m_velocity.Length2D() > (record->max_current_speed * 0.52f) ? (2.f * previous->time_delta) : -(2.f * previous->time_delta));

				//	manually_calculated = lagcomp::get().clamp(manually_calculated, 0, 1);

				//	if (abs(manually_calculated - previous->stop_to_full_run_frac) >= 0.1f)// {
				//		m_player->get_anim_state()->m_walk_run_transition = manually_calculated;
				//}

				const auto prev_direction = RAD2DEG(std::atan2f(previous->m_velocity.y, previous->m_velocity.x));

				auto real_velocity = record->m_velocity.length_2d();

				float delta = curr_direction - prev_direction;

				if (delta <= 180.0f)
				{
					if (delta < -180.0f)
						delta = delta + 360.0f;
				}
				else
				{
					delta = delta - 360.0f;
				}

				float v63 = delta * 0.5f + curr_direction;

				auto direction = (v63 + 90.f) * 0.017453292f;

				record->m_velocity.x = sinf(direction) * real_velocity;
				record->m_velocity.y = cosf(direction) * real_velocity;
			}
		}

		//bool is_jumping = record->m_flags & FL_ONGROUND && previous && previous->data_filled && !previous->dormant && !(previous->m_flags & FL_ONGROUND);

		/*if (is_jumping && record->ground_accel_last_time != record->simulation_time)
		{
			if (sv_enablebunnyhopping->GetInt() == 0) {

				// 260 x 1.1 = 286 units/s.
				float max = m_player->m_flMaxSpeed() * 1.1f;

				// get current velocity.
				float speed = record->m_velocity.Length();

				// reset velocity to 286 units/s.
				if (max > 0.f && speed > max)
					record->m_velocity *= (max / speed);
			}

			// assume the player is bunnyhopping here so set the upwards impulse.
			record->m_velocity.z = sv_jump_impulse->GetFloat();

			record->in_jump = true;
		}
		else */if (!(record->m_flags & FL_ONGROUND))
		{
			record->m_velocity.z -= sv_gravity->GetFloat() * record->time_delta * 0.5f;

			//record->in_jump = true;
		}
	}
	else if (record->valid())
	{
		auto weight_speed = record->m_layers[6].m_weight;

		if (record->m_layers[6].m_playback_rate < 0.00001f)
			record->m_velocity.clear();
		else
		{
			const auto m_post_velocity_lenght = record->m_velocity.length_2d();

			if (m_post_velocity_lenght != 0.0f && weight_speed > 0.01f && weight_speed < 0.95f)
			{
				float mult = 1;
				if (record->m_flags & 6)
					mult = 0.34f;
				else if (record->m_fake_walk)
					mult = 0.52f;

				record->m_velocity.x = (record->m_velocity.x / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
				record->m_velocity.y = (record->m_velocity.y / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
			}
		}

		if (record->m_flags & FL_ONGROUND)
			record->m_velocity.z = 0;
	}

	if (_fdtest(&record->m_velocity.x) > 0
		|| _fdtest(&record->m_velocity.y) > 0
		|| _fdtest(&record->m_velocity.z) > 0)
		record->m_velocity.clear();
	//
	//	if server had 0 velocity at animation time -> reset velocity
	//
	if (record->m_flags & FL_ONGROUND && record->m_lag > 1 && record->m_velocity.length() > 0.1f && record->m_layers[6].m_playback_rate < 0.00001f)
		record->m_velocity.clear();

	//r_log->tick_stopped = -1;
	//r_log->m_velocity_stopped = Vector::Zero;

	//m_player->invalidate_anims(4);

	/* apply proper velocity and force flags so game will not try to recalculate it. */
	//m_player->m_vecAbsVelocity() = record->m_velocity;
	record->m_player->m_vecVelocity() = record->m_velocity;
	//m_player->invalidate_anims(VELOCITY_CHANGED);

	//*(Vector*)(uintptr_t(m_player) + 0x114) = record->m_velocity;

	
}

void LagCompensation::parse_player_data(LagRecord* record, AimPlayer* data) {
	// get player ptr.
	//Player* player = record->m_player;

	//if (!player)
	//	return;

	record->m_broke_lc = false;

	const auto idx = record->m_player->index() - 1;

	auto previous = data->m_records[1].get();

	int ticks_to_simulate = 1;

	if (previous && !previous->dormant() && previous->valid() && !record->dormant())
	{
		int simulation_ticks = game::TIME_TO_TICKS(record->m_sim_time - previous->m_sim_time);

		if ((simulation_ticks - 1) > 31 || previous->m_sim_time == 0.f)
			simulation_ticks = 1;

		auto layer_cycle = record->m_layers[11].m_cycle;
		auto previous_playback = previous->m_layers[11].m_playback_rate;

		if (previous_playback > 0.f && record->m_layers[11].m_playback_rate > 0.f
			&& previous->m_layers[11].m_sequence == record->m_layers[11].m_sequence
			/*&& m_player->get_anim_state()->m_weapon == m_player->get_anim_state()->m_weapon_last*/)
		{
			auto previous_cycle = previous->m_layers[11].m_cycle;
			simulation_ticks = 0;

			if (previous_cycle > layer_cycle)
				layer_cycle = layer_cycle + 1.0f;

			while (layer_cycle > previous_cycle)
			{
				const auto ticks_backup = simulation_ticks;
				const auto playback_mult_ipt = g_csgo.m_globals->m_interval * previous_playback;

				previous_cycle = previous_cycle + (g_csgo.m_globals->m_interval * previous_playback);

				if (previous_cycle >= 1.0f)
					previous_playback = record->m_layers[11].m_playback_rate;

				++simulation_ticks;

				if (previous_cycle > layer_cycle && (previous_cycle - layer_cycle) > (playback_mult_ipt * 0.5f))
					simulation_ticks = ticks_backup;
			}
		}

		ticks_to_simulate = simulation_ticks;

		//if (record->exploit)
		//	record->simulation_time = previous->simulation_time + TICKS_TO_TIME(simulation_ticks);
	}

	ticks_to_simulate = std::clamp(ticks_to_simulate, 1, 64);

	record->m_lag = ticks_to_simulate;

	if (previous && previous->valid()) {
		if (record->m_sim_time > record->m_old_sim_time) {

			if ((record->m_origin - previous->m_origin).length_2d() > 4096.0f) {
				record->m_broke_lc = true;
			}

			// check if landed in choke cycle
			if (!record->dormant() && !previous->dormant())
			{
				if (!(record->m_flags & 1 && previous->m_flags & 1)) {
					if (record->m_layers[4].m_cycle < 0.5f) {
						bool gay = false;
					}
				}
				else
					record->m_velocity.z = 0;

				//auto at_target = math::get().CalcAngle(record->origin, ctx.m_local()->get_abs_origin()).y;

				//if (auto eye_delta = fabsf(math::get().angle_diff(record->eye_angles.y, previous->eye_angles.y)); eye_delta > 165.f && (abs(at_target) <= 65.f || abs(at_target) >= 165.f) && !record->shot_this_tick) {
				//	resolver_info->desync_swap = !resolver_info->desync_swap;
				//	//resolver_info->desync_swap_angles = record->eye_angles.y;
				//}
				//else 
				//{
				//	//auto previous = (log->records_count > 3 && !record->first_after_dormancy ? &log->tick_records[(log->records_count - 1) & 63] : nullptr);
				//	//auto previous = (log->records_count > 3 && !record->first_after_dormancy ? &log->tick_records[(log->records_count - 1) & 63] : nullptr);
				//
				//	if (log->records_count > 3) {
				//		auto prepre = (log->records_count > 3 && !record->first_after_dormancy ? &log->tick_records[(log->records_count - 2) & 63] : nullptr);
				//		auto preprepre = (log->records_count > 3 && !record->first_after_dormancy ? &log->tick_records[(log->records_count - 3) & 63] : nullptr);
				//
				//		auto eye_delta1 = abs(math::get().angle_diff(record->eye_angles.y, previous->eye_angles.y));
				//		auto eye_delta2 = abs(math::get().angle_diff(previous->eye_angles.y, preprepre->eye_angles.y));
				//		auto eye_delta3 = abs(math::get().angle_diff(record->eye_angles.y, preprepre->eye_angles.y));
				//
				//		if (fmaxf(eye_delta1, fmaxf(eye_delta2, eye_delta3)) < 35.f)
				//			resolver_info->desync_swap = false;
				//	}
				//}
			}
		}
	}

	record->max_current_speed = record->m_player->m_flMaxspeed();

	//resolver_info->tick_jumped = -1;

	//float anim_speed = FLT_MAX;
	auto weapon = record->m_player->GetActiveWeapon();

	if (weapon) {
		auto wdata = weapon->GetWpnData();

		if (wdata)
			record->max_current_speed = std::fminf(record->m_player->m_flMaxspeed(), record->m_player->m_bIsScoped() ? *(float*)(uintptr_t(wdata) + 0x134) : *(float*)(uintptr_t(wdata) + 0x130));
	}

	record->max_current_speed = fmaxf(record->max_current_speed, 0.001f);

	record->animation_speed = -1.f;

}

bool LagCompensation::StartPrediction( AimPlayer* data ) {
	// we have no data to work with.
	// this should never happen if we call this
	if( data->m_records.empty( ) )
		return false;

	// meme.
	if( data->m_player->dormant( ) )
		return false;

	// compute the true amount of updated records
	// since the last time the player entered pvs.
	size_t size{};

	// iterate records.
	for( const auto &it : data->m_records ) {
		if( it->dormant( ) )
			break;

		// increment total amount of data.
		++size;
	}

	// get first record.
	LagRecord* record = data->m_records[ 0 ].get( );

	// reset all prediction related variables.
	// this has been a recurring problem in all my hacks lmfao.
	// causes the prediction to stack on eachother.
	record->predict( );

	// check if lc broken.
	if( ( size > 1 && ( record->m_origin - data->m_records[ 1 ]->m_origin ).length_2d_sqr( ) > 4096.f )
		|| ( size > 2 && ( data->m_records[ 1 ]->m_origin - data->m_records[ 2 ]->m_origin ).length_2d_sqr( ) > 4096.f ) )
		record->m_broke_lc = true;

	// we are not breaking lagcomp at this point.
	// return false so it can aim at all the records it once
	// since server-sided lagcomp is still active and we can abuse that.
	if( !record->m_broke_lc )
		return false;

	int simulation = game::TIME_TO_TICKS( record->m_sim_time );

	// this is too much lag to fix.
	if( std::abs( g_cl.m_arrival_tick - simulation ) >= 128 )
		return true;

	// compute the amount of lag that we will predict for, if we have one set of data, use that.
	// if we have more data available, use the prevoius lag delta to counter weird fakelags that switch between 14 and 2.
	int lag = ( size <= 2 ) ? game::TIME_TO_TICKS( record->m_sim_time - data->m_records[ 1 ]->m_sim_time )
								  : game::TIME_TO_TICKS( data->m_records[ 1 ]->m_sim_time - data->m_records[ 2 ]->m_sim_time );

	// clamp this just to be sure.
	math::clamp( lag, 1, 15 );

	// get the delta in ticks between the last server net update
	// and the net update on which we created this record.
	int updatedelta = g_cl.m_server_tick - record->m_tick;

	// if the lag delta that is remaining is less than the current netlag
	// that means that we can shoot now and when our shot will get processed
	// the origin will still be valid, therefore we do not have to predict.
	if( g_cl.m_latency_ticks <= lag - updatedelta )
		return true;

	// the next update will come in, wait for it.
	int next = record->m_tick + 1;
	if( next + lag >= g_cl.m_arrival_tick )
		return true;

	float change = 0.f, dir = 0.f;

	// get the direction of the current velocity.
	if( record->m_velocity.y != 0.f || record->m_velocity.x != 0.f )
		dir = math::rad_to_deg( std::atan2( record->m_velocity.y, record->m_velocity.x ) );

	// we have more than one update
	// we can compute the direction.
	if( size > 1 ) {
		// get the delta time between the 2 most recent records.
		float dt = record->m_sim_time - data->m_records[ 1 ]->m_sim_time;

		// init to 0.
		float prevdir = 0.f;

		// get the direction of the prevoius velocity.
		if( data->m_records[ 1 ]->m_velocity.y != 0.f || data->m_records[ 1 ]->m_velocity.x != 0.f )
			prevdir = math::rad_to_deg( std::atan2( data->m_records[ 1 ]->m_velocity.y, data->m_records[ 1 ]->m_velocity.x ) );

		// compute the direction change per tick.
		change = ( math::NormalizedAngle( dir - prevdir ) / dt ) * g_csgo.m_globals->m_interval;
	}

	if( std::abs( change ) > 6.f )
		change = 0.f;

	// get the pointer to the players animation state.
	CCSGOPlayerAnimState* state = data->m_player->m_PlayerAnimState( );

	// backup the animation state.
	CCSGOPlayerAnimState backup{};
	if( state )
		std::memcpy( &backup, state, sizeof( CCSGOPlayerAnimState ) );

	// add in the shot prediction here.
	int shot = 0;

	/*Weapon* pWeapon = data->m_player->GetActiveWeapon( );
	if( pWeapon && !data->m_fire_bullet.empty( ) ) {

		static Address offset = g_netvars.get( HASH( "DT_BaseCombatWeapon" ), HASH( "m_fLastShotTime" ) );
		float last = pWeapon->get< float >( offset );

		if( game::TIME_TO_TICKS( data->m_fire_bullet.front( ).m_sim_time - last ) == 1 ) {
			WeaponInfo* wpndata = pWeapon->GetWpnData( );

			if( wpndata )
				shot = game::TIME_TO_TICKS( last + wpndata->m_cycletime ) + 1;
		}
	}*/

	int pred = 0;

	// start our predicton loop.
	while( true ) {
		// can the player shoot within his lag delta.
		/*if( shot && shot >= simulation && shot < simulation + lag ) {
		
			// if so his new lag will be the time until he shot again.
			lag = shot - simulation;
			math::clamp( lag, 3, 15 );
		
			// only predict a shot once.
			shot = 0;
		}*/

		// see if by predicting this amount of lag
		// we do not break stuff.
		next += lag;
		if( next >= g_cl.m_arrival_tick )
			break;

		// predict lag.
		for( int sim{}; sim < lag; ++sim ) {
			// predict movement direction by adding the direction change per tick to the previous direction.
			// make sure to normalize it, in case we go over the -180/180 turning point.
			dir = math::NormalizedAngle( dir + change );

			// pythagorean theorem
			// a^2 + b^2 = c^2
			// we know a and b, we square them and add them together, then root.
			float hyp = record->m_pred_velocity.length_2d( );

			// compute the base velocity for our new direction.
			// since at this point the hypotenuse is known for us and so is the angle.
			// we can compute the adjacent and opposite sides like so:
			// cos(x) = a / h -> a = cos(x) * h
			// sin(x) = o / h -> o = sin(x) * h
			record->m_pred_velocity.x = std::cos( math::deg_to_rad( dir ) ) * hyp;
			record->m_pred_velocity.y = std::sin( math::deg_to_rad( dir ) ) * hyp;

			// we hit the ground, set the upwards impulse and apply CS:GO speed restrictions.
			if( record->m_pred_flags & FL_ONGROUND ) {
				if( !g_csgo.sv_enablebunnyhopping->GetInt( ) ) {

					// 260 x 1.1 = 286 units/s.
					float max = data->m_player->m_flMaxspeed( ) * 1.1f;

					// get current velocity.
					float speed = record->m_pred_velocity.length_2d( );

					// reset velocity to 286 units/s.
					if( max > 0.f && speed > max )
						record->m_pred_velocity *= ( max / speed );
				}

				// assume the player is bunnyhopping here so set the upwards impulse.
				record->m_pred_velocity.z = g_csgo.sv_jump_impulse->GetFloat( );
			}

			// we are not on the ground
			// apply gravity and airaccel.
			else {
				// apply one tick of gravity.
				record->m_pred_velocity.z -= g_csgo.sv_gravity->GetFloat( ) * g_csgo.m_globals->m_interval;

				// compute the ideal strafe angle for this velocity.
				float speed2d = record->m_pred_velocity.length_2d( );
				float ideal   = ( speed2d > 0.f ) ? math::rad_to_deg( std::asin( 15.f / speed2d ) ) : 90.f;
				math::clamp( ideal, 0.f, 90.f );

				float smove = 0.f;
				float abschange = std::abs( change );

				if( abschange <= ideal || abschange >= 30.f ) {
					static float mod{ 1.f };

					dir  += ( ideal * mod );
					smove = 450.f * mod;
					mod  *= -1.f;
				}

				else if( change > 0.f )
					smove = -450.f;

				else
					smove = 450.f;

				// apply air accel.
				AirAccelerate( record, ang_t{ 0.f, dir, 0.f }, 0.f, smove );
			}

			// predict player.
			// convert newly computed velocity
			// to origin and flags.
			PlayerMove( record );

			// move time forward by one.
			record->m_pred_time += g_csgo.m_globals->m_interval;

			// increment total amt of predicted ticks.
			++pred;

			// the server animates every first choked command.
			// therefore we should do that too.
			if( sim == 0 && state )
				PredictAnimations( state, record );
		}
	}

	// restore state.
	if( state )
		std::memcpy( state, &backup, sizeof( CCSGOPlayerAnimState ) );

	if( pred <= 0 )
		return true;

	// lagcomp broken, invalidate bones.
	record->invalidate( );

	// re-setup bones for this record.
	g_bones.setup( data->m_player, nullptr, record );

	return true;
}

void LagCompensation::PlayerMove( LagRecord* record ) {
	vec3_t                start, end, normal;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// define trace start.
	start = record->m_pred_origin;

	// move trace end one tick into the future using predicted velocity.
	end = start + ( record->m_pred_velocity * g_csgo.m_globals->m_interval );

	// trace.
	g_csgo.m_engine_trace->TraceRay( Ray( start, end, record->m_mins, record->m_maxs ), CONTENTS_SOLID, &filter, &trace );

	// we hit shit
	// we need to fix hit.
	if( trace.m_fraction != 1.f ) {
		auto v1 = 0;

		// fix sliding on planes.
		do {
			record->m_pred_velocity -= trace.m_plane.m_normal * record->m_pred_velocity.dot( trace.m_plane.m_normal );

			float adjust = record->m_pred_velocity.dot( trace.m_plane.m_normal );
			if( adjust < 0.f )
				record->m_pred_velocity -= ( trace.m_plane.m_normal * adjust );

			start = trace.m_endpos;
			end = start + ( record->m_pred_velocity * ( g_csgo.m_globals->m_interval * ( 1.f - trace.m_fraction ) ) );

			g_csgo.m_engine_trace->TraceRay( Ray( start, end, record->m_mins, record->m_maxs ), CONTENTS_SOLID, &filter, &trace );
			if( trace.m_fraction == 1.f )
				break;
			++v1;
		} while( v1 < 2 );
	}

	// set new final origin.
	start = end = record->m_pred_origin = trace.m_endpos;

	// move endpos 2 units down.
	// this way we can check if we are in/on the ground.
	end.z -= 2.f;

	// trace.
	g_csgo.m_engine_trace->TraceRay( Ray( start, end, record->m_mins, record->m_maxs ), CONTENTS_SOLID, &filter, &trace );

	// stole from eso hack.
	if( trace.m_fraction == 1.f || trace.m_plane.m_normal.z < 0.7f )
		record->m_pred_flags &= ~FL_ONGROUND;
	else
		record->m_pred_flags |= FL_ONGROUND;

	// luaghin to the bank haha.
	if( !( record->m_pred_flags & FL_ONGROUND ) ) {
		// thanks eso.
		record->m_layers[ 4 ].m_cycle = 0.0f;
		record->m_layers[ 4 ].m_weight = 0.0f;
	}
}

void LagCompensation::AirAccelerate( LagRecord* record, ang_t angle, float fmove, float smove ) {
	vec3_t fwd, right, wishvel, wishdir;
	float  maxspeed, wishspd, wishspeed, currentspeed, addspeed, accelspeed;

	// determine movement angles.
	math::AngleVectors( angle, &fwd, &right );

	// zero out z components of movement vectors.
	fwd.z = 0.f;
	right.z = 0.f;

	// normalize remainder of vectors.
	fwd.normalize( );
	right.normalize( );

	// determine x and y parts of velocity.
	for( int i{ }; i < 2; ++i )
		wishvel[ i ] = ( fwd[ i ] * fmove ) + ( right[ i ] * smove );

	// zero out z part of velocity.
	wishvel.z = 0.f;

	// determine maginitude of speed of move.
	wishdir = wishvel;
	wishspeed = wishdir.normalize( );

	// get maxspeed.
	// TODO; maybe global this or whatever its 260 anyway always.
	maxspeed = record->m_player->m_flMaxspeed( );

	// clamp to server defined max speed.
	if( wishspeed != 0.f && wishspeed > maxspeed )
		wishspeed = maxspeed;

	// make copy to preserve original variable.
	wishspd = wishspeed;

	// cap speed.
	if( wishspd > 30.f )
		wishspd = 30.f;

	// determine veer amount.
	currentspeed = record->m_pred_velocity.dot( wishdir );

	// see how much to add.
	addspeed = wishspd - currentspeed;

	// if not adding any, done.
	if( addspeed <= 0.f )
		return;

	// Determine acceleration speed after acceleration
	accelspeed = g_csgo.sv_airaccelerate->GetFloat( ) * wishspeed * g_csgo.m_globals->m_interval;

	// cap it.
	if( accelspeed > addspeed )
		accelspeed = addspeed;

	// add accel.
	record->m_pred_velocity += ( wishdir * accelspeed );
}

void LagCompensation::PredictAnimations( CCSGOPlayerAnimState* state, LagRecord* record ) {
	struct AnimBackup_t {
		int    flags;
		int    eflags;
		vec3_t velocity;
	};

	// first off lets backup our globals.
	auto curtime = g_csgo.m_globals->m_curtime;
	auto frametime = g_csgo.m_globals->m_frametime;

	// get player ptr.
	Player* player = record->m_player;

	// backup data.
	AnimBackup_t backup;
	backup.flags = player->m_fFlags( );
	backup.eflags = player->m_iEFlags( );
	backup.velocity = player->m_vecAbsVelocity( );

	// set curtime to animtime.
	// set frametime to ipt just like on the server during simulation.
	g_csgo.m_globals->m_curtime = record->m_pred_time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	// haha.
	if( player->m_AnimOverlay( )[ 5 ].m_weight > 0.f )
		player->m_fFlags( ) |= FL_ONGROUND;

	// force to use correct abs origin and velocity ( no CalcAbsolutePosition and CalcAbsoluteVelocity calls )
	player->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

	// set predicted flags and velocity and origin.
	player->m_fFlags( ) = record->m_pred_flags;
	player->m_vecAbsVelocity( ) = record->m_pred_velocity;

	// enable re-animation in the same frame if animated already.
	if( state->m_frame >= g_csgo.m_globals->m_frame )
		state->m_frame = g_csgo.m_globals->m_frame - 1;

	bool fake = g_menu.main.aimbot.correct.get( );

	// rerun the resolver since we edited the origin.
	if( fake )
		g_resolver.ResolveAngles( player, record );

	// update animations.
	game::UpdateAnimationState( state, record->m_eye_angles );

	// rerun the pose correction cuz we are re-setupping them.
	if( fake )
		g_resolver.ResolvePoses( player, record );

	// get new rotation poses and layers.
	player->GetPoseParameters( record->m_poses );
	player->GetAnimLayers( record->m_layers );
	record->m_abs_ang = player->GetAbsAngles( );

	// restore globals.
	g_csgo.m_globals->m_curtime = curtime;
	g_csgo.m_globals->m_frametime = frametime;

	// restore player data.
	player->m_fFlags( ) = backup.flags;
	player->m_iEFlags( ) = backup.eflags;
	player->m_vecAbsVelocity( ) = backup.velocity;
}