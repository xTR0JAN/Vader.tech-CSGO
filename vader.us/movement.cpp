#include "includes.h"

#define M_PI 3.1415927f

#define DEG2RAD( x  )  ( (float)(x) * (float)(3.14159265358979323846f / 180.f) )
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / 3.14159265358979323846f) )

Movement g_movement{ };;

void Movement::JumpRelated() {
	if (g_cl.m_local->m_MoveType() == MOVETYPE_NOCLIP)
		return;

	if ((g_cl.m_cmd->m_buttons & IN_JUMP) && !(g_cl.m_flags & FL_ONGROUND)) {
		// bhop.
		if (g_menu.main.movement.bhop.get())
			g_cl.m_cmd->m_buttons &= ~IN_JUMP;

		// duck jump ( crate jump ).
		if (g_menu.main.movement.airduck.get())
			g_cl.m_cmd->m_buttons |= IN_DUCK;
	}
}

void Movement::Strafe() {
	vec3_t velocity;
	float  delta /*abs_delta, velocity_angle, velocity_delta, correct*/;

	// don't strafe while noclipping or on ladders..
	if (g_cl.m_local->m_MoveType() == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;

	// disable strafing while pressing shift.
	// don't strafe if not holding primary jump key.
	if ((g_cl.m_buttons & IN_SPEED) || !(g_cl.m_buttons & IN_JUMP) || (g_cl.m_flags & FL_ONGROUND))
		return;

	// get networked velocity ( maybe absvelocity better here? ).
	// meh, should be predicted anyway? ill see.
	velocity = g_cl.m_local->m_vecVelocity();

	// get the velocity len2d ( speed ).
	m_speed = velocity.length_2d();

	// compute the ideal strafe angle for our velocity.
	m_ideal = (m_speed > 0.f) ? math::rad_to_deg(std::asin(15.f / m_speed)) : 90.f;
	m_ideal2 = (m_speed > 0.f) ? math::rad_to_deg(std::asin(30.f / m_speed)) : 90.f;

	// some additional sanity.
	math::clamp(m_ideal, 0.f, 90.f);
	math::clamp(m_ideal2, 0.f, 90.f);

	// save entity bounds ( used much in circle-strafer ).
	m_mins = g_cl.m_local->m_vecMins();
	m_maxs = g_cl.m_local->m_vecMaxs();

	// save our origin
	m_origin = g_cl.m_local->m_vecOrigin();

	// for changing direction.
	// we want to change strafe direction every call.
	m_switch_value *= -1.f;

	// for allign strafer.
	++m_strafe_index;

	// do allign strafer.
	if (g_input.GetKeyState(g_menu.main.movement.astrafe.get())) {
		float angle = std::max(m_ideal2, 4.f);

		if (angle > m_ideal2 && !(m_strafe_index % 5))
			angle = m_ideal2;

		// add the computed step to the steps of the previous circle iterations.
		m_circle_yaw = math::NormalizedAngle(m_circle_yaw + angle);

		// apply data to usercmd.
		g_cl.m_strafe_angles.y = m_circle_yaw;
		g_cl.m_cmd->m_side_move = -450.f;

		return;
	}

	// do ciclestrafer
	else if (g_input.GetKeyState(g_menu.main.movement.cstrafe.get())) {
		// if no duck jump.
		if (!g_menu.main.movement.airduck.get()) {
			// crouch to fit into narrow areas.
			g_cl.m_cmd->m_buttons |= IN_DUCK;
		}

		DoPrespeed();
		return;
	}

	else if (g_input.GetKeyState(g_menu.main.movement.zstrafe.get())) {
		float freq = (g_menu.main.movement.z_freq.get() * 0.2f) * g_csgo.m_globals->m_realtime;

		// range [ 1, 100 ], aka grenerates a factor.
		float factor = g_menu.main.movement.z_dist.get() * 0.5f;

		g_cl.m_strafe_angles.y += (factor * std::sin(freq));
	}

	if (!g_menu.main.movement.autostrafe.get())
		return;

	m_circle_yaw = m_old_yaw = g_cl.m_strafe_angles.y;

	static float yaw_add = 0.f;
	static const auto cl_sidespeed = g_csgo.m_cvar->FindVar(HASH("cl_sidespeed"));

	bool back = g_cl.m_cmd->m_buttons & IN_BACK;
	bool forward = g_cl.m_cmd->m_buttons & IN_FORWARD;
	bool right = g_cl.m_cmd->m_buttons & IN_MOVELEFT;
	bool left = g_cl.m_cmd->m_buttons & IN_MOVERIGHT;

	if (back) {
		yaw_add = -180.f;
		if (right)
			yaw_add -= 45.f;
		else if (left)
			yaw_add += 45.f;
	}
	else if (right) {
		yaw_add = 90.f;
		if (back)
			yaw_add += 45.f;
		else if (forward)
			yaw_add -= 45.f;
	}
	else if (left) {
		yaw_add = -90.f;
		if (back)
			yaw_add -= 45.f;
		else if (forward)
			yaw_add += 45.f;
	}
	else {
		yaw_add = 0.f;
	}

	g_cl.m_strafe_angles.y += yaw_add;
	g_cl.m_cmd->m_forward_move = 0.f;
	g_cl.m_cmd->m_side_move = 0.f;

	delta = math::NormalizedAngle(g_cl.m_strafe_angles.y - math::rad_to_deg(atan2(g_cl.m_local->m_vecVelocity().y, g_cl.m_local->m_vecVelocity().x)));

	g_cl.m_cmd->m_side_move = delta > 0.f ? -cl_sidespeed->GetFloat() : cl_sidespeed->GetFloat();

	g_cl.m_strafe_angles.y = math::NormalizedAngle(g_cl.m_strafe_angles.y - delta);

	//// get our viewangle change.
	//delta = math::NormalizedAngle( g_cl.m_cmd->m_view_angles.y - m_old_yaw );

	//// convert to absolute change.
	//abs_delta = std::abs( delta );

	//// save old yaw for next call.
	//m_circle_yaw = m_old_yaw = g_cl.m_cmd->m_view_angles.y;

	//// set strafe direction based on mouse direction change.
	//if( delta > 0.f )
	//	g_cl.m_cmd->m_side_move = -450.f;

	//else if( delta < 0.f )
	//	g_cl.m_cmd->m_side_move = 450.f;

	//// we can accelerate more, because we strafed less then needed
	//// or we got of track and need to be retracked.

	///*
	//* data struct
	//* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
	//*/

	//if( abs_delta <= m_ideal || abs_delta >= 30.f ) {
	//	// compute angle of the direction we are traveling in.
	//	velocity_angle = math::rad_to_deg( std::atan2( velocity.y, velocity.x ) );

	//	// get the delta between our direction and where we are looking at.
	//	velocity_delta = math::NormalizedAngle( g_cl.m_cmd->m_view_angles.y - velocity_angle );

	//	// correct our strafe amongst the path of a circle.
	//	correct = m_ideal2 * 2.f;

	//	if( velocity_delta <= correct || m_speed <= 15.f ) {
	//		// not moving mouse, switch strafe every tick.
	//		if( -correct <= velocity_delta || m_speed <= 15.f ) {
	//			g_cl.m_cmd->m_view_angles.y += ( m_ideal * m_switch_value );
	//			g_cl.m_cmd->m_side_move = 450.f * m_switch_value;
	//		}

	//		else {
	//			g_cl.m_cmd->m_view_angles.y = velocity_angle - correct;
	//			g_cl.m_cmd->m_side_move = 450.f;
	//		}
	//	}

	//	else {
	//		g_cl.m_cmd->m_view_angles.y = velocity_angle + correct;
	//		g_cl.m_cmd->m_side_move = -450.f;
	//	}
	//}
}

void Movement::DoPrespeed() {
	float   mod, min, max, step, strafe, time, angle;
	vec3_t  plane;

	// min and max values are based on 128 ticks.
	mod = g_csgo.m_globals->m_interval * 128.f;

	// scale min and max based on tickrate.
	min = 2.25f * mod;
	max = 5.f * mod;

	// compute ideal strafe angle for moving in a circle.
	strafe = m_ideal * 2.f;

	// clamp ideal strafe circle value to min and max step.
	math::clamp(strafe, min, max);

	// calculate time.
	time = 320.f / m_speed;

	// clamp time.
	math::clamp(time, 0.35f, 1.f);

	// init step.
	step = strafe;

	while (true) {
		// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
		if (!WillCollide(time, step) || max <= step)
			break;

		// if we will collide with an object with the current strafe step then increment step to prevent a collision.
		step += 0.2f;
	}

	if (step > max) {
		// reset step.
		step = strafe;

		while (true) {
			// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
			if (!WillCollide(time, step) || step <= -min)
				break;

			// if we will collide with an object with the current strafe step decrement step to prevent a collision.
			step -= 0.2f;
		}

		if (step < -min) {
			if (GetClosestPlane(plane)) {
				// grab the closest object normal
				// compute the angle of the normal
				// and push us away from the object.
				angle = math::rad_to_deg(std::atan2(plane.y, plane.x));
				step = -math::NormalizedAngle(m_circle_yaw - angle) * 0.1f;
			}
		}

		else
			step -= 0.2f;
	}

	else
		step += 0.2f;

	// add the computed step to the steps of the previous circle iterations.
	m_circle_yaw = math::NormalizedAngle(m_circle_yaw + step);

	// apply data to usercmd.
	g_cl.m_cmd->m_view_angles.y = m_circle_yaw;
	g_cl.m_cmd->m_side_move = (step >= 0.f) ? -450.f : 450.f;
}

bool Movement::GetClosestPlane(vec3_t& plane) {
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;
	vec3_t                start{ m_origin };
	float                 smallest{ 1.f };
	const float		      dist{ 75.f };

	// trace around us in a circle
	for (float step{ }; step <= math::pi_2; step += (math::pi / 10.f)) {
		// extend endpoint x units.
		vec3_t end = start;
		end.x += std::cos(step) * dist;
		end.y += std::sin(step) * dist;

		g_csgo.m_engine_trace->TraceRay(Ray(start, end, m_mins, m_maxs), CONTENTS_SOLID, &filter, &trace);

		// we found an object closer, then the previouly found object.
		if (trace.m_fraction < smallest) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// did we find any valid object?
	return smallest != 1.f && plane.z < 0.1f;
}

bool Movement::WillCollide(float time, float change) {
	struct PredictionData_t {
		vec3_t start;
		vec3_t end;
		vec3_t velocity;
		float  direction;
		bool   ground;
		float  predicted;
	};

	PredictionData_t      data;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// set base data.
	data.ground = g_cl.m_flags & FL_ONGROUND;
	data.start = m_origin;
	data.end = m_origin;
	data.velocity = g_cl.m_local->m_vecVelocity();
	data.direction = math::rad_to_deg(std::atan2(data.velocity.y, data.velocity.x));

	for (data.predicted = 0.f; data.predicted < time; data.predicted += g_csgo.m_globals->m_interval) {
		// predict movement direction by adding the direction change.
		// make sure to normalize it, in case we go over the -180/180 turning point.
		data.direction = math::NormalizedAngle(data.direction + change);

		// pythagoras.
		float hyp = data.velocity.length_2d();

		// adjust velocity for new direction.
		data.velocity.x = std::cos(math::deg_to_rad(data.direction)) * hyp;
		data.velocity.y = std::sin(math::deg_to_rad(data.direction)) * hyp;

		// assume we bhop, set upwards impulse.
		if (data.ground)
			data.velocity.z = g_csgo.sv_jump_impulse->GetFloat();

		else
			data.velocity.z -= g_csgo.sv_gravity->GetFloat() * g_csgo.m_globals->m_interval;

		// we adjusted the velocity for our new direction.
		// see if we can move in this direction, predict our new origin if we were to travel at this velocity.
		data.end += (data.velocity * g_csgo.m_globals->m_interval);

		// trace
		g_csgo.m_engine_trace->TraceRay(Ray(data.start, data.end, m_mins, m_maxs), MASK_PLAYERSOLID, &filter, &trace);

		// check if we hit any objects.
		if (trace.m_fraction != 1.f && trace.m_plane.m_normal.z <= 0.9f)
			return true;
		if (trace.m_startsolid || trace.m_allsolid)
			return true;

		// adjust start and end point.
		data.start = data.end = trace.m_endpos;

		// move endpoint 2 units down, and re-trace.
		// do this to check if we are on th floor.
		g_csgo.m_engine_trace->TraceRay(Ray(data.start, data.end - vec3_t{ 0.f, 0.f, 2.f }, m_mins, m_maxs), MASK_PLAYERSOLID, &filter, &trace);

		// see if we moved the player into the ground for the next iteration.
		data.ground = trace.hit() && trace.m_plane.m_normal.z > 0.7f;
	}

	// the entire loop has ran
	// we did not hit shit.
	return false;
}

void Movement::MoonWalk(CUserCmd* cmd) {
	if (g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;

	// slide walk
	g_cl.m_cmd->m_buttons |= IN_BULLRUSH;

	if (g_menu.main.misc.slide_walk.get()) {
		if (cmd->m_side_move < 0.f)
		{
			cmd->m_buttons |= IN_MOVERIGHT;
			cmd->m_buttons &= ~IN_MOVELEFT;
		}

		if (cmd->m_side_move > 0.f)
		{
			cmd->m_buttons |= IN_MOVELEFT;
			cmd->m_buttons &= ~IN_MOVERIGHT;
		}

		if (cmd->m_forward_move > 0.f)
		{
			cmd->m_buttons |= IN_BACK;
			cmd->m_buttons &= ~IN_FORWARD;
		}

		if (cmd->m_forward_move < 0.f)
		{
			cmd->m_buttons |= IN_FORWARD;
			cmd->m_buttons &= ~IN_BACK;
		}
	}

}

void Movement::FixMove(CUserCmd* cmd, const ang_t& wish_angles) {

	vec3_t  move, dir;
	float   delta, len;
	ang_t   move_angle;

	// roll nospread fix.
	if (!(g_cl.m_flags & FL_ONGROUND) && cmd->m_view_angles.z != 0.f)
		cmd->m_side_move = 0.f;

	// convert movement to vec3_t.
	move = { cmd->m_forward_move, cmd->m_side_move, 0.f };

	// get move length and ensure we're using a unit vec3_t ( vec3_t with length of 1 ).
	len = move.normalize();
	if (!len)
		return;

	// convert move to an angle.
	math::VectorAngles(move, move_angle);

	// calculate yaw delta.
	delta = (cmd->m_view_angles.y - wish_angles.y);

	// accumulate yaw delta.
	move_angle.y += delta;

	// calculate our new move direction.
	// dir = move_angle_forward * move_length
	math::AngleVectors(move_angle, &dir);

	// scale to og movement.
	dir *= len;

	// strip old flags.
	g_cl.m_cmd->m_buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);

	// fix ladder and noclip.
	if (g_cl.m_local->m_MoveType() == MOVETYPE_LADDER) {
		// invert directon for up and down.
		if (cmd->m_view_angles.x >= 45.f && wish_angles.x < 45.f && std::abs(delta) <= 65.f)
			dir.x = -dir.x;

		// write to movement.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		// set new button flags.
		if (cmd->m_forward_move > 200.f)
			cmd->m_buttons |= IN_FORWARD;

		else if (cmd->m_forward_move < -200.f)
			cmd->m_buttons |= IN_BACK;

		if (cmd->m_side_move > 200.f)
			cmd->m_buttons |= IN_MOVERIGHT;

		else if (cmd->m_side_move < -200.f)
			cmd->m_buttons |= IN_MOVELEFT;
	}

	// we are moving normally.
	else {
		// we must do this for pitch angles that are out of bounds.
		if (cmd->m_view_angles.x < -90.f || cmd->m_view_angles.x > 90.f)
			dir.x = -dir.x;

		// set move.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		// set new button flags.
		if (cmd->m_forward_move > 0.f)
			cmd->m_buttons |= IN_FORWARD;

		else if (cmd->m_forward_move < 0.f)
			cmd->m_buttons |= IN_BACK;

		if (cmd->m_side_move > 0.f)
			cmd->m_buttons |= IN_MOVERIGHT;

		else if (cmd->m_side_move < 0.f)
			cmd->m_buttons |= IN_MOVELEFT;
	}
}

void Movement::AutoPeek(float wish_yaw) {
	/* sanity checks */
	if (!g_csgo.m_engine->IsInGame() || !g_csgo.m_engine->IsConnected())
		return;

	if (g_input.GetKeyState(g_menu.main.movement.autopeek.get())) {
		if (start_pos.is_zero())
		{
			start_pos = g_cl.m_local->GetAbsOrigin();

			if (!(g_cl.m_local->m_fFlags() & FL_ONGROUND))
			{
				Ray ray;
				CTraceFilterWorldOnly filter;
				CGameTrace trace;

				ray = Ray(start_pos, start_pos - vec3_t(0.f,0.f,1000.f));
				g_csgo.m_engine_trace->TraceRay(ray, MASK_SOLID, &filter, &trace);

				if (trace.m_fraction < 1.0f)
					start_pos = trace.m_endpos + vec3_t(0.f, 0.f, 2.f);
			}
		}
		else
		{
			if (g_cl.m_cmd->m_buttons & IN_ATTACK) {
				fired_shot = true;
			}

			if (fired_shot)
			{
				auto current_pos = g_cl.m_local->GetAbsOrigin();
				differencebog = current_pos - start_pos;

				if (current_pos != start_pos)
				{
					auto velocity = vec3_t(differencebog.x * cos(wish_yaw / 180.0f * M_PI) + differencebog.y * sin(wish_yaw / 180.0f * M_PI), differencebog.y * cos(wish_yaw / 180.0f * M_PI) - differencebog.x * sin(wish_yaw / 180.0f * M_PI), differencebog.z);

					g_cl.m_cmd->m_forward_move = -velocity.x * 20.f;
					g_cl.m_cmd->m_side_move = velocity.y * 20.f;

					if (differencebog.length_2d() < 1)
					{
						fired_shot = false;
						start_pos = vec3_t(0.f, 0.f, 0.f);
					}
				}
				else
				{
					fired_shot = false;
					start_pos = vec3_t(0.f,0.f,0.f);
				}
			}
		}
	}
	else
	{
		fired_shot = false;
		start_pos = vec3_t(0.f, 0.f, 0.f);
	}





	//if (!g_cl.m_local || !g_cl.m_cmd)
	//	return;

	//static std::deque<peekcmd_t> known_cmds;

	//const auto rotate_movement = [&](float yaw) {
	//	float rotation = DEG2RAD(g_cl.m_cmd->m_view_angles.y - yaw);

	//	float cos_rot = cos(rotation);
	//	float sin_rot = sin(rotation);

	//	float new_forwardmove = (cos_rot * g_cl.m_cmd->m_forward_move) - (sin_rot * g_cl.m_cmd->m_side_move);
	//	float new_sidemove = (sin_rot * g_cl.m_cmd->m_forward_move) + (cos_rot * g_cl.m_cmd->m_side_move);

	//	g_cl.m_cmd->m_forward_move = new_forwardmove;
	//	g_cl.m_cmd->m_side_move = new_sidemove;
	//};

	///* reset */
	//if (!g_input.GetKeyState(g_menu.main.movement.autopeek.get())) {
	//	selfpeek_state = e_selfpeek_states::STATE_REST;
	//	known_cmds.clear();
	//	return;
	//}

	///* if we were resting and we got here store origin and start storing cmds */
	//if (selfpeek_state == e_selfpeek_states::STATE_REST) {
	//	last_selfpeek_origin = g_cl.m_local->GetAbsOrigin();
	//	selfpeek_state = e_selfpeek_states::STATE_PEEKING;

	//	/* reset just in case */
	//	known_cmds.clear();
	//}

	///* store cmds - TODO: make max stored cmds something like 128 and after just go to origin to save some memory ~swoopae */
	//if (selfpeek_state == e_selfpeek_states::STATE_PEEKING) {
	//	/* construct our cmd */
	//	peekcmd_t peekcmd{};
	//	peekcmd.m_yaw = g_cl.m_cmd->m_view_angles.y;
	//	peekcmd.m_fmove = g_cl.m_cmd->m_forward_move;
	//	peekcmd.m_smove = g_cl.m_cmd->m_side_move;

	//	/* store it */
	//	known_cmds.push_front(peekcmd);

	//	/* did we just shoot? stop storing and start going back to cover */
	//	if (g_cl.m_cmd->m_buttons & IN_ATTACK || g_cl.m_old_shot)
	//		selfpeek_state = e_selfpeek_states::STATE_RETURNING;

	//	/* we don't want to start moving back this exact tick in case we just shot */
	//	return;
	//}

	///* go thru cmds and go back */
	//if (selfpeek_state == e_selfpeek_states::STATE_RETURNING) {
	//	/* no more cmds to go through , keep drawing origin but stop doing anything */
	//	if (known_cmds.empty() && g_cl.m_local->GetAbsOrigin().dist_to(last_selfpeek_origin) < 30.f) {
	//		selfpeek_state = e_selfpeek_states::STATE_MAX;
	//	}
	//	else if (known_cmds.empty()) {
	//		/* move to our origin manually */
	//		ang_t angle{};
	//		vec3_t final_vec = last_selfpeek_origin - g_cl.m_local->GetAbsOrigin();
	//		math::VectorAngles(final_vec, angle);

	//		g_cl.m_cmd->m_forward_move = 450.f;

	//		/* rotate move value */
	//		rotate_movement(angle.y);

	//		return;
	//	}
	//	else {
	//		/* let's move */
	//		/* figure out the fastest possible values for fmove and smove without completely fucking direction up */
	//		/* this obviously needs some work such as removing a number of commands to prevent overshooting the target but it should work in most scenarios where you'll release the selfpeek key after you reach safety */
	//		auto factor = 450.f / (std::max(std::abs(known_cmds.begin()->m_fmove), std::abs(known_cmds.begin()->m_smove)));

	//		g_cl.m_cmd->m_forward_move = -known_cmds.begin()->m_fmove * factor;
	//		g_cl.m_cmd->m_side_move = -known_cmds.begin()->m_smove * factor;

	//		/* rotate old move values */
	//		rotate_movement(known_cmds.begin()->m_yaw);

	//		/* delete current cmd */
	//		known_cmds.erase(known_cmds.begin());

	//		return;
	//	}
	//}

	///* cuck our movement until the user lets go of the selfpeek key */
	//if (selfpeek_state == e_selfpeek_states::STATE_MAX) {
	//	g_cl.m_cmd->m_forward_move = std::clamp(g_cl.m_cmd->m_forward_move, -10.f, 10.f);
	//	g_cl.m_cmd->m_side_move = std::clamp(g_cl.m_cmd->m_side_move, -10.f, 10.f);
	//}

	//// set to invert if we press the button.
	//if (g_input.GetKeyState(g_menu.main.movement.autopeek.get())) {
	//	if (g_cl.m_old_shot)
	//		m_invert = true;

	//	vec3_t move{ g_cl.m_cmd->m_forward_move, g_cl.m_cmd->m_side_move, 0.f };

	//	if (m_invert) {
	//		move *= -1.f;
	//		g_cl.m_cmd->m_forward_move = move.x;
	//		g_cl.m_cmd->m_side_move = move.y;
	//	}
	//}

	//else m_invert = false;

	//bool can_stop = g_menu.main.movement.autostop_always_on.get() || (!g_menu.main.movement.autostop_always_on.get() && g_input.GetKeyState(g_menu.main.movement.autostop.get()));
	//if ((g_input.GetKeyState(g_menu.main.movement.autopeek.get()) || can_stop) && g_aimbot.m_stop && !g_input.GetKeyState(g_menu.main.movement.nefariouswalk.get())) {
	//	Movement::QuickStop();
	//}
}

void Movement::QuickStopFix()
{
	bool can_stop = g_menu.main.movement.autostop_always_on.get() || (!g_menu.main.movement.autostop_always_on.get() && g_input.GetKeyState(g_menu.main.movement.autostop.get()));
	if ((g_input.GetKeyState(g_menu.main.movement.autopeek.get()) || can_stop) && g_aimbot.m_stop && !g_input.GetKeyState(g_menu.main.movement.nefariouswalk.get())) {
		Movement::QuickStop();
	}
}

void Movement::QuickStop() {
	if (!(g_cl.m_local->m_fFlags() & FL_ONGROUND))
		return;

	// convert velocity to angular momentum.
	ang_t angle;
	math::VectorAngles(g_cl.m_local->m_vecVelocity(), angle);

	// get our current speed of travel.
	vec3_t vel = g_cl.m_local->m_vecVelocity();
	float speed = vel.length();

	// fix direction by factoring in where we are looking.
	angle.y = g_cl.m_view_angles.y - angle.y;

	// convert corrected angle back to a direction.
	vec3_t direction;
	math::AngleVectors(angle, &direction);

	vec3_t stop = direction * -speed;

	if (g_cl.m_speed < 0.1f)
	{
		g_cl.m_cmd->m_forward_move = 0.f;
		g_cl.m_cmd->m_side_move = 0.f;
		return;
	}

	static auto sv_accelerate = g_csgo.m_cvar->FindVar(HASH("sv_accelerate"));
	float accel = sv_accelerate->GetFloat();
	float max_speed = g_cl.m_local->GetActiveWeapon()->GetWpnData()->m_max_player_speed;

	if (g_cl.m_local->m_bIsScoped()) {
		max_speed = g_cl.m_local->GetActiveWeapon()->GetWpnData()->m_max_player_speed_alt;
	}

	max_speed = std::min< float >(max_speed, 250.f);

	float surf_friction = 1.f;
	float max_accelspeed = accel * g_csgo.m_globals->m_interval * max_speed * surf_friction;

	float wishspeed{ };

	if (speed - max_accelspeed <= -1.f) {
		wishspeed = max_accelspeed / (speed / (accel * g_csgo.m_globals->m_interval));
	}
	else {
		wishspeed = max_accelspeed;
	}

	vec3_t ndir = math::vector_angles(vel * -1.f);
	ndir.y = g_cl.m_cmd->m_view_angles.y - ndir.y;
	ndir = math::angle_vectors(ndir);

	g_cl.m_cmd->m_forward_move = ndir.x * wishspeed * (0.25);
	g_cl.m_cmd->m_side_move = ndir.y * wishspeed * (0.25);
}

void Movement::FakeWalk() {
	vec3_t velocity{ g_cl.m_local->m_vecVelocity( ) };
	int    ticks{ }, max{ 16 };

	if( !g_input.GetKeyState( g_menu.main.movement.fakewalk.get( ) ) )
		return;

	if( !g_cl.m_local->GetGroundEntity( ) )
		return;

	// not on ground dont fakewalk.
	if( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) )
		return;

	// reference:
	// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/shared/gamemovement.cpp#L1612

	// calculate friction.
	float friction = g_csgo.sv_friction->GetFloat( ) * g_cl.m_local->m_surfaceFriction( );

	// check for the amount of ticks when breaking LBY.
	int pred_ticks = g_cl.GetNextUpdate( ) - 1;

	for( ticks; ticks < g_cl.m_max_lag; ++ticks ) {
		// calculate speed.
		float speed = velocity.length( );

		// if too slow return.
		if( speed <= 0.1f )
			break;

		// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
		float control = std::max( speed, g_csgo.sv_stopspeed->GetFloat( ) );

		// calculate the drop amount.
		float drop = control * friction * g_csgo.m_globals->m_interval;

		// scale the velocity.
		float newspeed = std::max( 0.f, speed - drop );

		if( newspeed != speed ) {
			// determine proportion of old speed we are using.
			newspeed /= speed;

			// adjust velocity according to proportion.
			velocity *= newspeed;
		}
	}

	// zero forwardmove and sidemove.
	if( ticks > ( ( max - 1 ) - g_csgo.m_cl->m_choked_commands ) || !g_csgo.m_cl->m_choked_commands ) {
		g_cl.m_cmd->m_forward_move = g_cl.m_cmd->m_side_move = 0.f;
	}
}

void Movement::serverside()
{
	vec3_t velocity{ g_cl.m_local->m_vecVelocity() };
	int    ticks{ }, max{ 16 };

	if (!g_input.GetKeyState(g_menu.main.movement.serverside.get()))
		return;

	if (!g_cl.m_local->GetGroundEntity())
		return;

	// user was running previously and abrubtly held the fakewalk key
	// we should quick-stop under this circumstance to hit the 0.22 flick
	// perfectly, and speed up our fakewalk after running even more.
	//if( g_cl.m_initial_flick ) {
	//	Movement::QuickStop( );
	//	return;
	//}

	// reference:
	// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/shared/gamemovement.cpp#L1612

	// calculate friction.
	float friction = g_csgo.sv_friction->GetFloat() * g_cl.m_local->m_surfaceFriction();

	for (; ticks < g_cl.m_max_lag; ++ticks) {
		// calculate speed.
		float speed = velocity.length();

		// if too slow return.
		if (speed <= 0.1f)
			break;

		// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
		float control = std::max(speed, g_csgo.sv_stopspeed->GetFloat());

		// calculate the drop amount.
		float drop = control * friction * g_csgo.m_globals->m_interval;

		// scale the velocity.
		float newspeed = std::max(0.f, speed - drop);

		if (newspeed != speed) {
			// determine proportion of old speed we are using.
			newspeed /= speed;

			// adjust velocity according to proportion.
			velocity *= newspeed;
		}
	}

	// zero forwardmove and sidemove.
	if (ticks > ((max - 1) - g_csgo.m_cl->m_choked_commands) || !g_csgo.m_cl->m_choked_commands) {
		g_cl.m_cmd->m_forward_move = g_cl.m_cmd->m_side_move = 0.f;
	}
}

void Movement::susAF()
{
	if (!g_cl.aa_disable)
		return;

	if (!g_cl.exploit_check1)
		return;

	if (g_cl.exploit_check2 > (g_menu.main.movement.seanjog_safety.get() - 1)) {

		if (!g_input.GetKeyState(g_menu.main.movement.nefariouswalk.get()))
			return;

		static int oldcmds;

		if (oldcmds != g_cl.m_cmd->m_command_number)
			oldcmds = g_cl.m_cmd->m_command_number;

		if (*g_cl.m_packet) {
			g_cl.m_cmd->m_command_number = oldcmds;
			g_cl.m_cmd->m_tick = INT_MAX;
		}
	}
}

