#pragma once

enum class e_selfpeek_states {
	STATE_REST = 0,
	STATE_PEEKING,
	STATE_RETURNING,
	STATE_MAX
};

struct peekcmd_t {
	float m_yaw, m_fmove, m_smove;
};

class Movement {
public:
	float  m_speed;
	float  m_ideal;
	float  m_ideal2;
	vec3_t m_mins;
	vec3_t m_maxs;
	vec3_t m_origin;
	float  m_switch_value = 1.f;
	int    m_strafe_index;
	float  m_old_yaw;
	float  m_circle_yaw;
	bool   m_invert;

	vec3_t start_pos;
	bool fired_shot;
	vec3_t differencebog;

	e_selfpeek_states selfpeek_state;

	vec3_t last_selfpeek_origin;

public:
	void JumpRelated();
	void Strafe();
	void DoPrespeed();
	bool GetClosestPlane(vec3_t& plane);
	bool WillCollide(float time, float step);
	void MoonWalk(CUserCmd* cmd);
	void FixMove(CUserCmd* cmd, const ang_t& old_angles);
	void AutoPeek(float wish_yaw);
	void QuickStop();
	void QuickStopFix();
	void FakeWalk();
	void susAF();
	void panzerNigger();
	void serverside();
};

extern Movement g_movement;