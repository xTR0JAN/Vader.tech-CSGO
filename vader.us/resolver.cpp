#include "includes.h"
Resolver g_resolver{};;


static float NextLBYUpdate[65];
static float Add[65];
//bool is_flicking;

LagRecord* Resolver::FindIdealRecord(AimPlayer* data) {
	LagRecord* first_valid, * current;

	if (data->m_records.empty())
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for (const auto& it : data->m_records) {
		if (it->dormant() || it->immune() || !it->valid())
			continue;

		// get current record.
		current = it.get();

		// first record that was valid, store it for later.
		if (!first_valid)
			first_valid = current;

		// try to find a record with a shot, lby update, walking or no anti-aim.
		if (it->m_shot || it->m_mode == Modes::RESOLVE_BODY || it->m_mode == Modes::RESOLVE_WALK || it->m_mode == Modes::RESOLVE_NONE)
			return current;
	}

	// none found above, return the first valid record if possible.
	return (first_valid) ? first_valid : nullptr;
}

LagRecord* Resolver::FindLastRecord(AimPlayer* data) {
	LagRecord* current;

	if (data->m_records.empty())
		return nullptr;

	// iterate records in reverse.
	for (auto it = data->m_records.crbegin(); it != data->m_records.crend(); ++it) {
		current = it->get();

		// if this record is valid.
		// we are done since we iterated in reverse.
		if (current->valid() && !current->immune() && !current->dormant())
			return current;
	}

	return nullptr;
}

void Resolver::OnBodyUpdate(Player* player, float value) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// set data.
	data->m_old_body = data->m_body;
	data->m_body = value;
}

float Resolver::GetAwayAngle(LagRecord* record) {
	float  delta{ std::numeric_limits< float >::max() };
	vec3_t pos;
	ang_t  away;

	// other cheats predict you by their own latency.
	// they do this because, then they can put their away angle to exactly
	// where you are on the server at that moment in time.

	// the idea is that you would need to know where they 'saw' you when they created their user-command.
	// lets say you move on your client right now, this would take half of our latency to arrive at the server.
	// the delay between the server and the target client is compensated by themselves already, that is fortunate for us.

	// we have no historical origins.
	// no choice but to use the most recent one.
	//if( g_cl.m_net_pos.empty( ) ) {
	math::VectorAngles(g_cl.m_local->m_vecOrigin() - record->m_pred_origin, away);
	return away.y;
	//}

	// half of our rtt.
	// also known as the one-way delay.
	//float owd = ( g_cl.m_latency / 2.f );

	// since our origins are computed here on the client
	// we have to compensate for the delay between our client and the server
	// therefore the OWD should be subtracted from the target time.
	//float target = record->m_pred_time; //- owd;

	// iterate all.
	//for( const auto &net : g_cl.m_net_pos ) {
		// get the delta between this records time context
		// and the target time.
	//	float dt = std::abs( target - net.m_time );

		// the best origin.
	//	if( dt < delta ) {
	//		delta = dt;
	//		pos   = net.m_pos;
	//	}
	//}

	//math::VectorAngles( pos - record->m_pred_origin, away );
	//return away.y;
}

void Resolver::MatchShot(AimPlayer* data, LagRecord* record) {
	// do not attempt to do this in nospread mode.
	if (g_menu.main.config.mode.get() == 1)
		return;

	float shoot_time = -1.f;

	Weapon* weapon = data->m_player->GetActiveWeapon();
	if (weapon) {
		// with logging this time was always one tick behind.
		// so add one tick to the last shoot time.
		shoot_time = weapon->m_fLastShotTime() + g_csgo.m_globals->m_interval;
	}

	// this record has a shot on it.
	if (game::TIME_TO_TICKS(shoot_time) == game::TIME_TO_TICKS(record->m_sim_time)) {
		if (record->m_lag <= 2)
			record->m_shot = true;

		// more then 1 choke, cant hit pitch, apply prev pitch.
		else if (data->m_records.size() >= 2) {
			LagRecord* previous = data->m_records[1].get();

			if (previous && !previous->dormant())
				record->m_eye_angles.x = previous->m_eye_angles.x;
		}
	}
}

void Resolver::SetMode(LagRecord* record) {
	// the resolver has 3 modes to chose from.
	// these modes will vary more under the hood depending on what data we have about the player
	// and what kind of hack vs. hack we are playing (mm/nospread).

	float speed = record->m_velocity.length_2d();

	// if on ground, moving, and not fakewalking.
	if ((record->m_flags & FL_ONGROUND) && speed > 10.f && !record->m_fake_walk)
		record->m_mode = Modes::RESOLVE_WALK;

	if (g_input.GetKeyState(g_menu.main.aimbot.override.get()) && record->m_flags & FL_ONGROUND && (speed <= 10.f || record->m_fake_walk))
		record->m_mode = Modes::RESOLVE_OVERRIDE;

	// if on ground, not moving or fakewalking.
	else if ((record->m_flags & FL_ONGROUND) && (speed <= 10.f || record->m_fake_walk) && !g_input.GetKeyState(g_menu.main.aimbot.override.get()))
		record->m_mode = Modes::RESOLVE_LASTMOVE;

	// if not on ground.
	else if (!(record->m_flags & FL_ONGROUND))
		record->m_mode = Modes::RESOLVE_AIR;
}

void Resolver::ResolveAngles(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// mark this record if it contains a shot.
	MatchShot(data, record);

	// next up mark this record with a resolver mode that will be used.
	SetMode(record);

	// if we are in nospread mode, force all players pitches to down.
	// TODO; we should check thei actual pitch and up too, since those are the other 2 possible angles.
	// this should be somehow combined into some iteration that matches with the air angle iteration.
	if (g_menu.main.config.mode.get() == 1)
		record->m_eye_angles.x = 90.f;

	// we arrived here we can do the acutal resolve.
	if (record->m_mode == Modes::RESOLVE_WALK)
		ResolveWalk(data, record);

	else if (record->m_mode == Modes::RESOLVE_OVERRIDE || (g_input.GetKeyState(g_menu.main.aimbot.override.get())))
		ResolveOverride(player, record, data);

	else if (record->m_mode == Modes::RESOLVE_LASTMOVE || record->m_mode == Modes::RESOLVE_UNKNOWM && !(g_input.GetKeyState(g_menu.main.aimbot.override.get())))
		MainResolver(record, data, player);

	else if (record->m_mode == Modes::RESOLVE_AIR)
		ResolveAir(data, record, player);

	// normalize the eye angles, doesn't really matter but its clean.
	math::NormalizeAngle(record->m_eye_angles.y);
}

void Resolver::ResolveWalk(AimPlayer* data, LagRecord* record) {
	record->resolver_text[record->m_player->index()] = "MOVING";
	// apply lby to eyeangles.
	record->m_eye_angles.y = record->m_body;

	// delay body update.
	//data->m_body_update = record->m_anim_time + 0.22f;

	float speed = record->m_velocity.length_2d();

	// reset stand and body index.
	if (speed > 20.f && !record->m_fake_walk)
		data->m_upd_index = 0;

	// havent checked this yet in ResolveLastMove( ... )
	data->m_moved = false;

	data->m_stand_index = 0;
	data->m_stand_index2 = 0;
	data->m_body_index = 0;
	data->m_last_move = 0;
	data->m_unknown_move = 0;
	data->m_delta_index = 0;

	// copy the last record that this player was walking
	// we need it later on because it gives us crucial data.
	std::memcpy(&data->m_walk_record, record, sizeof(LagRecord));
}

float Resolver::GetLBYRotatedYaw(float lby, float yaw)
{
	float delta = math::NormalizedAngle(yaw - lby);
	if (fabs(delta) < 25.f)
		return lby;

	if (delta > 0.f)
		return yaw + 25.f;

	return yaw;
}

bool Resolver::IsYawSideways(Player* entity, float yaw)
{
	auto local_player = g_cl.m_local;
	if (!local_player)
		return false;

	const auto at_target_yaw = math::CalcAngle(local_player->m_vecOrigin(), entity->m_vecOrigin()).y;
	const float delta = fabs(math::NormalizedAngle(at_target_yaw - yaw));

	return delta > 20.f && delta < 160.f;
}


void Resolver::SupremAntiFreestanding(LagRecord* record) {
	record->resolver_text[record->m_player->index()] = "FREESTAND";
	// constants
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// best target.
	vec3_t enemypos = record->m_player->GetShootPosition();
	float away = GetAwayAngle(record);

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back(away - 180.f);
	angles.emplace_back(away + 90.f);
	angles.emplace_back(away - 90.f);

	// start the trace at the your shoot pos.
	vec3_t start = g_cl.m_local->GetShootPosition();

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemypos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.z };

		// draw a line for debugging purposes.
#ifdef _DEBUG
		if (g_menu.main.aimbot.debug_tools.get())
			g_csgo.m_debug_overlay->AddLineOverlay(start, end, 255, 0, 0, true, 0.1f);
#endif // _DEBUG

		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid) {
		return;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();

	record->m_eye_angles.y = best->m_yaw;
}

void Resolver::SupremAntiFreestandingReversed(LagRecord* record) {
	record->resolver_text[record->m_player->index()] = "FREESTAND";
	// constants
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// best target.
	vec3_t enemypos = record->m_player->GetShootPosition();
	float away = GetAwayAngle(record);

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back(away - 180.f);
	angles.emplace_back(away - 90.f);
	angles.emplace_back(away + 90.f);

	// start the trace at the your shoot pos.
	vec3_t start = g_cl.m_local->GetShootPosition();

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemypos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.z };

		// draw a line for debugging purposes.
#ifdef _DEBUG
		if (g_menu.main.aimbot.debug_tools.get())
			g_csgo.m_debug_overlay->AddLineOverlay(start, end, 255, 0, 0, true, 0.1f);
#endif // _DEBUG

		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid) {
		return;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();

	record->m_eye_angles.y = best->m_yaw;
}

void Resolver::collect_wall_detect(const Stage_t stage)
{
	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	if (!g_cl.m_local)
		return;

	auto g_pLocalPlayer = g_cl.m_local;

	last_eye_positions.insert(last_eye_positions.begin(), g_pLocalPlayer->m_vecOrigin() + g_pLocalPlayer->m_vecViewOffset());
	if (last_eye_positions.size() > 128)
		last_eye_positions.pop_back();

	auto nci = g_csgo.m_engine->GetNetChannelInfo();
	if (!nci)
		return;


	const int latency_ticks = game::TIME_TO_TICKS(nci->GetLatency(nci->FLOW_OUTGOING));
	auto latency_based_eye_pos = last_eye_positions.size() <= latency_ticks ? last_eye_positions.back() : last_eye_positions[latency_ticks];

	for (auto i = 1; i < g_csgo.m_globals->m_max_clients; i++)
	{
		//auto& log = player_log::get().get_log(i);
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!player || player == g_pLocalPlayer)
		{
			continue;
		}

		if (!player->enemy(g_pLocalPlayer))
		{
			continue;
		}

		if (!player->alive())
		{
			continue;
		}

		if (player->dormant())
		{
			continue;
		}

		if (player->m_vecVelocity().length_2d() > 0.1f)
		{
			continue;
		}

		//if (!log.record.empty() && player->get_simtime() - log.record[0].m_sim_time == 0)
		//	continue;

		//if (log.m_vecLastNonDormantOrig != player->get_origin() && g_pLocalPlayer->get_alive())
		//{
		//	log.m_iMode = RMODE_WALL;
		//}

		//else if (player->get_simtime() - log.m_flLastLowerBodyYawTargetUpdateTime > 1.35f && log.m_vecLastNonDormantOrig == player->get_origin() && log.m_iMode == RMODE_MOVING)
		//{
		//	if (player->get_simtime() - log.m_flLastLowerBodyYawTargetUpdateTime > 1.65f)
		//	{
		//		log.m_iMode = RMODE_WALL;
		//	}
		//}
		//else {
		//	log.m_iMode = RMODE_NORMAL;
		//}

		if (m_iMode == 1)
		{
			const auto at_target_angle = math::CalcAngle(player->m_vecOrigin(), last_eye);

			//Vector left_dir, right_dir, back_dir;
			//math::get().angle_vectors(Vector(0.f, at_target_angle.y - 90.f, 0.f), &left_dir);
			//math::get().angle_vectors(Vector(0.f, at_target_angle.y + 90.f, 0.f), &right_dir);
			//math::get().angle_vectors(Vector(0.f, at_target_angle.y + 180.f, 0.f), &back_dir);

			const auto eye_pos = player->get_eye_pos();
			//auto left_eye_pos = eye_pos + (left_dir * 16.f);
			//auto right_eye_pos = eye_pos + (right_dir * 16.f);
			//auto back_eye_pos = eye_pos + (back_dir * 16.f);

			const float height = 64;

			vec3_t direction_1, direction_2, direction_3;
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y - 90.f, 0.f), &direction_1);
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y + 90.f, 0.f), &direction_2);
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y + 180.f, 0.f), &direction_3);

			const auto left_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_1 * 16.f);
			const auto right_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_2 * 16.f);
			const auto back_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_3 * 16.f);

			//log.anti_freestanding_record.left_damage = penetration::get().get_damage(latency_based_eye_pos,
			anti_freestanding_record.left_damage = penetration::scale(player, left_damage[i], 1.f, HITGROUP_CHEST);

			//anti_freestanding_record.left_damage = penetration::scale(player, &left_damage[i],
			//	HITGROUP_CHEST);
			//data->anti_freestanding_record.right_damage = FEATURES::RAGEBOT::autowall.CalculateDamage(latency_based_eye_pos,
			//	right_eye_pos, local_player, entity, 1).damage;
			anti_freestanding_record.right_damage = penetration::scale(player, right_damage[i], 1.f, HITGROUP_CHEST);
			//penetration::get().get_damage(g_cl.m_local, player, right_eye_pos, &right_damage[i],
			//get_big_fucking_gun(), &latency_based_eye_pos);
		//BACKWARDS
			anti_freestanding_record.back_damage = penetration::scale(player, back_damage[i], 1.f, HITGROUP_CHEST);

			Ray ray;
			CGameTrace trace;
			CTraceFilterWorldOnly filter;

			Ray first_ray(left_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(first_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.left_fraction = trace.m_fraction;

			Ray second_ray(right_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(second_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.right_fraction = trace.m_fraction;

			Ray third_ray(back_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(third_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.back_fraction = trace.m_fraction;


			//penetration::get().get_damage(g_pLocalPlayer, player, left_eye_pos, &left_damage[i], get_big_fucking_gun(), &last_eye);
			//penetration::get().get_damage(g_pLocalPlayer, player, right_eye_pos, &right_damage[i], get_big_fucking_gun(), &last_eye);
			//penetration::get().get_damage(g_pLocalPlayer, player, back_eye_pos, &back_damage[i], get_big_fucking_gun(), &last_eye);
		}
	}
}

bool hitPlayer[64];

bool Resolver::AntiFreestanding(Player* entity, AimPlayer* data, float& yaw)
{

	const auto freestanding_record = anti_freestanding_record;

	//g_pEntitiyList->GetClientEntity(g_pEngine->GetLocalPlayer())

	auto local_player = g_cl.m_local;
	if (!local_player)
		return false;

	const float at_target_yaw = math::CalcAngle(local_player->m_vecOrigin(), entity->m_vecOrigin()).y;

	// not freestanding.
	if( freestanding_record.left_damage >= 20 && freestanding_record.right_damage >= 20 )
		yaw = at_target_yaw + 180.f;

	auto set = false;

	if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0)
	{
		if (freestanding_record.right_fraction < freestanding_record.left_fraction) {
			set = true;
			yaw = at_target_yaw + 90.f;
		}
		else if (freestanding_record.right_fraction > freestanding_record.left_fraction) {
			set = true;
			yaw = at_target_yaw - 90.f;
		}
		else {
			yaw = at_target_yaw + 180.f;
		}
	}
	else
	{
		if (freestanding_record.left_damage > freestanding_record.right_damage) {
			yaw = at_target_yaw + 90.f;
			set = true;
		}
		else
			yaw = at_target_yaw + 180.f;
	}

	return true;
}


void Resolver::MainResolver(LagRecord* record, AimPlayer* data, Player* player)
{
	// for no-spread call a seperate resolver.
	if (g_menu.main.config.mode.get() == 1) {
		StandNS(data, record);
		return;
	}

	// pointer for easy access.
	LagRecord* move = &data->m_walk_record;

	// get predicted away angle for the player.
	float away = GetAwayAngle(record);

	// all the resolving bs
	float diff = math::NormalizedAngle(move->m_body - record->m_body);
	float delta = record->m_anim_time - move->m_anim_time;
	auto index = player->index();

	if (move->m_sim_time > 0.f) {
		if (!data->m_moved) {
			vec3_t delta = move->m_origin - record->m_origin;
			if (delta.length() <= 128.f)
				data->m_moved = true;
		}
	}

	// predict LBY flicks.
	if (!record->dormant() && !player->dormant()) {
		// since we null velocity when they fakewalk, no need to check for it.
		if (record->m_anim_velocity.length() > 0.1f) {
			Add[player->index()] = 0.22f;
			NextLBYUpdate[player->index()] = record->m_anim_time + Add[player->index()];
			data->m_body_update = NextLBYUpdate[player->index()];
		}
		// lby wont update on this tick but after.
		else if (record->m_anim_time >= NextLBYUpdate[player->index()])
		{
			is_flicking = true;
			Add[player->index()] = 1.1f;
			NextLBYUpdate[player->index()] = record->m_anim_time + Add[player->index()];
			data->m_body_update = NextLBYUpdate[player->index()];
		}
		else
			is_flicking = false;

		// LBY updated via PROXY.
		if (data->m_body != data->m_old_body) {
			is_flicking = true;
			Add[player->index()] = g_csgo.m_globals->m_interval + 1.1f;
			NextLBYUpdate[player->index()] = record->m_anim_time + Add[player->index()];
			data->m_body_update = NextLBYUpdate[player->index()];
		}
		else
			is_flicking = false;
	}

	if (is_flicking && data->m_upd_index < 2)
	{
		//m_iMode = 0;
		record->m_eye_angles.y = record->m_body;
		iPlayers[record->m_player->index()] = false;
		record->m_mode = Modes::RESOLVE_BODY;
		record->resolver_text[record->m_player->index()] = "LBY UPDATE";
	}
	else {

		record->m_mode = Modes::RESOLVE_UNKNOWM;

		switch (data->m_missed_shots % 5) {
		case 0:
			if (fabs(diff) > 36.f)
			{
				record->resolver_text[record->m_player->index()] = "LAST MOVING LBY [1]";
				record->m_eye_angles.y = move->m_body;
			}
			else
			{
				record->resolver_text[record->m_player->index()] = "SUPREM FREESTAND [1]";
				SupremAntiFreestanding(record);
			}
			break;
		case 1:
			if (fabs(diff) > 36.f)
			{
				record->resolver_text[record->m_player->index()] = "LAST MOVING LBY [2]";
				SupremAntiFreestanding(record);
			}
			else
			{
				record->resolver_text[record->m_player->index()] = "SUPREM FREESTAND [-1]";
				SupremAntiFreestandingReversed(record);
			}
			break;
		case 2:
			record->resolver_text[record->m_player->index()] = "BRUTE BACKWARD";
			record->m_eye_angles.y = away + 180.f;
			break;
		case 3:
			record->resolver_text[record->m_player->index()] = "BRUTE LEFT";
			record->m_eye_angles.y = away + 90.f;
			break;
		case 4:
			record->resolver_text[record->m_player->index()] = "BRUTE RIGHT";
			record->m_eye_angles.y = away - 90.f;
			break;
		}
	}
}

void Resolver::StandNS(AimPlayer* data, LagRecord* record) {
	// get away angles.
	float away = GetAwayAngle(record);

	switch (data->m_shots % 8) {
	case 0:
		record->m_eye_angles.y = away + 180.f;
		break;

	case 1:
		record->m_eye_angles.y = away + 90.f;
		break;
	case 2:
		record->m_eye_angles.y = away - 90.f;
		break;

	case 3:
		record->m_eye_angles.y = away + 45.f;
		break;
	case 4:
		record->m_eye_angles.y = away - 45.f;
		break;

	case 5:
		record->m_eye_angles.y = away + 135.f;
		break;
	case 6:
		record->m_eye_angles.y = away - 135.f;
		break;

	case 7:
		record->m_eye_angles.y = away + 0.f;
		break;

	default:
		break;
	}

	// force LBY to not fuck any pose and do a true bruteforce.
	record->m_body = record->m_eye_angles.y;
}

void Resolver::ResolveAir(AimPlayer* data, LagRecord* record, Player* player) {
	// for no-spread call a seperate resolver.
	if (g_menu.main.config.mode.get() == 1) {
		AirNS(data, record);
		return;
	}

	record->resolver_text[record->m_player->index()] = "AIR";

	LagRecord* move = &data->m_walk_record;
	// else run our matchmaking air resolver.

	// we have barely any speed. 
	// either we jumped in place or we just left the ground.
	// or someone is trying to fool our resolver.
	if (record->m_velocity.length_2d() < 50.f) {
		// set this for completion.
		// so the shot parsing wont pick the hits / misses up.
		// and process them wrongly.
		record->m_eye_angles.y = move->m_body;

		// we are done.
		return;
	}

	// try to predict the direction of the player based on his velocity direction.
	// this should be a rough estimation of where he is looking.
	float velyaw = math::rad_to_deg(std::atan2(record->m_velocity.y, record->m_velocity.x));
	float away = GetAwayAngle(record);
	switch (data->m_missed_shots % 3) {
	case 0:
		record->m_eye_angles.y = away + 180.f;
		break;
	case 1:
		SupremAntiFreestanding(record);
		break;
	case 2:
		SupremAntiFreestandingReversed(record);
		break;
	}
}

void Resolver::AirNS(AimPlayer* data, LagRecord* record) {
	// get away angles.
	float away = GetAwayAngle(record);

	switch (data->m_shots % 9) {
	case 0:
		record->m_eye_angles.y = away + 180.f;
		break;

	case 1:
		record->m_eye_angles.y = away + 150.f;
		break;
	case 2:
		record->m_eye_angles.y = away - 150.f;
		break;

	case 3:
		record->m_eye_angles.y = away + 165.f;
		break;
	case 4:
		record->m_eye_angles.y = away - 165.f;
		break;

	case 5:
		record->m_eye_angles.y = away + 135.f;
		break;
	case 6:
		record->m_eye_angles.y = away - 135.f;
		break;

	case 7:
		record->m_eye_angles.y = away + 90.f;
		break;
	case 8:
		record->m_eye_angles.y = away - 90.f;
		break;

	default:
		break;
	}
}

void Resolver::ResolvePoses(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// only do this bs when in air.
	if (record->m_mode == Modes::RESOLVE_AIR) {
		// ang = pose min + pose val x ( pose range )

		// lean_yaw
		player->m_flPoseParameter()[2] = g_csgo.RandomInt(0, 4) * 0.25f;

		// body_yaw
		player->m_flPoseParameter()[11] = g_csgo.RandomInt(1, 3) * 0.25f;
	}
}


void Resolver::ResolveOverride(Player* player, LagRecord* record, AimPlayer* data) {

	// get predicted away angle for the player.
	float away = GetAwayAngle(record);

	// pointer for easy access.
	LagRecord* move = &data->m_walk_record;

	C_AnimationLayer* curr = &record->m_layers[3];
	int act = data->m_player->GetSequenceActivity(curr->m_sequence);

	if (g_input.GetKeyState(g_menu.main.aimbot.override.get())) {
		ang_t                          viewangles;
		g_csgo.m_engine->GetViewAngles(viewangles);

		//auto yaw = math::clamp (g_cl.m_local->GetAbsOrigin(), Player->origin()).y;
		const float at_target_yaw = math::CalcAngle(g_cl.m_local->m_vecOrigin(), player->m_vecOrigin()).y;

		if (fabs(math::NormalizedAngle(viewangles.y - at_target_yaw)) > 30.f)
			return g_resolver.MainResolver(record, data, record->m_player);

		record->m_eye_angles.y = (math::NormalizedAngle(viewangles.y - at_target_yaw) > 0) ? at_target_yaw + 90.f : at_target_yaw - 90.f;

		//return UTILS::GetLBYRotatedYaw(entity->m_flLowerBodyYawTarget(), (math::NormalizedAngle(viewangles.y - at_target_yaw) > 0) ? at_target_yaw + 90.f : at_target_yaw - 90.f);

		record->m_mode = Modes::RESOLVE_OVERRIDE;
	}
}
