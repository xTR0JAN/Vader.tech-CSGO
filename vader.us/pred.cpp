#include "includes.h"
#include "pred.h"

InputPrediction g_inputpred{ };

// thanks chance :)

void InputPrediction::ApplyPredictedNetvars( CMoveData data ) {
	// restore move data.
	std::memcpy( &data, &m_data.data, sizeof( CMoveData ) );

	g_cl.m_local->m_aimPunchAngleVel( ) = m_data.m_aim_punch_vel;
	g_cl.m_local->m_aimPunchAngle( ) = m_data.m_aim_punch;
	g_cl.m_local->m_viewPunchAngle( ) = m_data.m_view_punch;
	g_cl.m_local->m_nTickBase( ) = m_data.m_tickbase;
	g_cl.m_local->m_fFlags( ) = m_data.m_flags;
	g_cl.m_local->m_MoveType( ) = m_data.m_move_type;
	g_cl.m_local->m_vecOrigin( ) = m_data.m_origin;
	g_cl.m_local->m_vecVelocity( ) = m_data.m_velocity;
	g_cl.m_local->m_vecViewOffset( ) = m_data.m_view_offset;
}

void InputPrediction::StorePredictedNetvars( CMoveData data ) {
	// store move data.
	std::memcpy( &m_data.data, &data, sizeof( CMoveData ) );

	m_data.m_aim_punch_vel = g_cl.m_local->m_aimPunchAngleVel( );
	m_data.m_aim_punch = g_cl.m_local->m_aimPunchAngle( );
	m_data.m_view_punch = g_cl.m_local->m_viewPunchAngle( );
	m_data.m_tickbase = g_cl.m_local->m_nTickBase( );
	m_data.m_flags = g_cl.m_local->m_fFlags( );
	m_data.m_move_type = g_cl.m_local->m_MoveType( );
	m_data.m_origin = g_cl.m_local->m_vecOrigin( );
	m_data.m_velocity = g_cl.m_local->m_vecVelocity( );
	m_data.m_view_offset = g_cl.m_local->m_vecViewOffset( );
}

void InputPrediction::UpdateGamePrediction( CUserCmd * cmd ) {
	const int m_tick = g_csgo.m_cl->m_delta_tick;
	if( m_tick > 0 ) {
		g_csgo.m_prediction->Update( m_tick, true, g_csgo.m_cl->m_last_command_ack,
			g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands );
	}

	static bool unlocked_fakelag = false;
	if( !unlocked_fakelag ) {
		auto cl_move_clamp = pattern::find( g_csgo.m_engine_dll, XOR( "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC" ) ) + 1;
		unsigned long protect = 0;

		VirtualProtect( ( void * ) cl_move_clamp, 4, PAGE_EXECUTE_READWRITE, &protect );
		*( std::uint32_t * )cl_move_clamp = 62;
		VirtualProtect( ( void * ) cl_move_clamp, 4, protect, &protect );
		unlocked_fakelag = true;
	}
}

void InputPrediction::RunGamePrediction( CUserCmd * cmd ) {
	// backup data.
	m_curtime = g_csgo.m_globals->m_curtime;
	m_frametime = g_csgo.m_globals->m_frametime;
	m_first_time_predicted = g_csgo.m_prediction->m_first_time_predicted;
	m_in_prediction = g_csgo.m_prediction->m_in_prediction;

	CUserCmd pPredictedCmd{ };
	SetUserCmd( &pPredictedCmd, cmd );

	*g_csgo.m_nPredictionRandomSeed = g_cl.m_cmd->m_random_seed;
	g_csgo.m_pPredictionPlayer = g_cl.m_local;

	g_csgo.m_globals->m_curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	g_csgo.m_game_movement->StartTrackPredictionErrors( g_cl.m_local );

	CMoveData pMoveData{ };
	std::memset( &pMoveData, 0, sizeof( CMoveData ) );

	g_csgo.m_prediction->SetupMove( g_cl.m_local, &pPredictedCmd, g_csgo.m_move_helper, &pMoveData );

	// store predicted netvars.
	StorePredictedNetvars( pMoveData );

	// lets re-predict them.
	PredictGamePrediction( pMoveData, &pPredictedCmd );
}

void InputPrediction::PredictGamePrediction( CMoveData data, CUserCmd * cmd ) {
	// update game prediction.
	UpdateGamePrediction( cmd );

	// apply predicted netvars.
	ApplyPredictedNetvars( data );

	// set this shit niggga.
	data.m_flForwardMove = cmd->m_forward_move;
	data.m_flSideMove = cmd->m_side_move;
	data.m_flUpMove = cmd->m_up_move;
	data.m_nButtons = cmd->m_buttons;
	data.m_vecViewAngles = cmd->m_view_angles;
	data.m_vecAbsViewAngles = cmd->m_view_angles;
	data.m_nImpulseCommand = cmd->m_impulse;

	g_csgo.m_prediction->m_first_time_predicted = false;
	g_csgo.m_prediction->m_in_prediction = true;

	*g_csgo.m_nPredictionRandomSeed = g_cl.m_cmd->m_random_seed;
	g_csgo.m_pPredictionPlayer = g_cl.m_local;

	g_csgo.m_globals->m_curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );
	g_csgo.m_globals->m_frametime = g_cl.m_local->m_fFlags( ) & FL_ATCONTROLS ? 0.f : g_csgo.m_globals->m_interval;

	g_csgo.m_move_helper->SetHost( g_cl.m_local );
	{
		g_csgo.m_game_movement->StartTrackPredictionErrors( g_cl.m_local );
		{
			g_csgo.m_prediction->SetupMove( g_cl.m_local, g_cl.m_cmd, g_csgo.m_move_helper, &data );

			g_csgo.m_game_movement->ProcessMovement( g_cl.m_local, &data );

			g_cl.m_local->SetAbsOrigin( g_cl.m_local->m_vecOrigin( ) );

			g_csgo.m_prediction->FinishMove( g_cl.m_local, g_cl.m_cmd, &data );
		}
		g_csgo.m_game_movement->FinishTrackPredictionErrors( g_cl.m_local );
	}
	g_csgo.m_move_helper->SetHost( nullptr );

	auto weapon = g_cl.m_local->GetActiveWeapon( );
	if( weapon ) {
		weapon->GetSpread( );
		weapon->UpdateAccuracyPenalty( );
	}
}

void InputPrediction::RestoreGamePrediction( CUserCmd* cmd ) {
	g_csgo.m_prediction->m_in_prediction = false;

	*g_csgo.m_nPredictionRandomSeed = -1;
	g_csgo.m_pPredictionPlayer = nullptr;

	// restore curtime/frametime
	// and prediction seed/player.
	g_csgo.m_globals->m_curtime = g_inputpred.m_curtime;
	g_csgo.m_globals->m_frametime = g_inputpred.m_frametime;
	g_csgo.m_prediction->m_first_time_predicted = g_inputpred.m_first_time_predicted;
	g_csgo.m_prediction->m_in_prediction = g_inputpred.m_in_prediction;
}