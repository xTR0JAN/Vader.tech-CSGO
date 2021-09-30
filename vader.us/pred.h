#pragma once

class InputPrediction {
public:
	float m_curtime;
	float m_frametime;
	bool m_first_time_predicted;
	bool m_in_prediction;

	void SetUserCmd( CUserCmd* cmd1, CUserCmd* cmd2 ) {
		cmd1->m_command_number = cmd2->m_command_number;
		cmd1->m_tick = cmd2->m_tick;
		cmd1->m_view_angles.x = cmd2->m_view_angles.x;
		cmd1->m_view_angles.y = cmd2->m_view_angles.y;
		cmd1->m_view_angles.z = cmd2->m_view_angles.z;
		cmd1->m_aimdirection.x = cmd2->m_aimdirection.x;
		cmd1->m_aimdirection.y = cmd2->m_aimdirection.y;
		cmd1->m_aimdirection.z = cmd2->m_aimdirection.z;
		cmd1->m_forward_move = cmd2->m_forward_move;
		cmd1->m_side_move = cmd2->m_side_move;
		cmd1->m_up_move = cmd2->m_up_move;
		cmd1->m_buttons = cmd2->m_buttons;
		cmd1->m_impulse = cmd2->m_impulse;
		cmd1->m_weapon_select = cmd2->m_weapon_select;
		cmd1->m_weapon_subtype = cmd2->m_weapon_subtype;
		cmd1->m_random_seed = cmd2->m_random_seed;
		cmd1->m_mousedx = cmd2->m_mousedx;
		cmd1->m_mousedy = cmd2->m_mousedy;
		cmd1->m_predicted = cmd2->m_predicted;
		cmd1->m_head_angles.x = cmd2->m_head_angles.x;
		cmd1->m_head_angles.y = cmd2->m_head_angles.y;
		cmd1->m_head_angles.z = cmd2->m_head_angles.z;
		cmd1->m_head_offset.x = cmd2->m_head_offset.x;
		cmd1->m_head_offset.y = cmd2->m_head_offset.y;
		cmd1->m_head_offset.z = cmd2->m_head_offset.z;
	}
	struct {
		CMoveData data{ };
		int m_tickbase{ };
		int m_flags{ };
		int m_move_type{ };
		vec3_t m_origin{ }, m_view_offset{ }, m_velocity{ };
		ang_t  m_view_punch{ }, m_aim_punch{ }, m_aim_punch_vel{ };
	} m_data;
	void ApplyPredictedNetvars( CMoveData data );
	void StorePredictedNetvars( CMoveData data );

public:
	void UpdateGamePrediction( CUserCmd* cmd );
	void RunGamePrediction( CUserCmd* cmd );
	void PredictGamePrediction( CMoveData data, CUserCmd* cmd );
	void RestoreGamePrediction( CUserCmd* cmd );
};

extern InputPrediction g_inputpred;