#pragma once

class ShotRecord {
public:
	__forceinline ShotRecord() : m_target{}, m_record{}, m_time{}, m_lat{}, m_damage{}, m_pos{}, m_matched{}, m_hurt{}, m_matrix{} {}

public:
	Player* m_target;
	LagRecord* m_record;
	float      m_time, m_lat, m_damage;
	vec3_t     m_pos;
	bool       m_matched;
	bool	   m_hurt;
	bool	   m_confirmed;
	vec3_t	   m_server_pos;
	int		   m_hitbox;
	BoneArray* m_matrix;
};

class VisualImpactData_t {
public:
	vec3_t m_impact_pos, m_shoot_pos;
	int    m_tickbase;
	bool   m_ignore, m_hit_player;

public:
	__forceinline VisualImpactData_t(const vec3_t& impact_pos, const vec3_t& shoot_pos, int tickbase) :
		m_impact_pos{ impact_pos }, m_shoot_pos{ shoot_pos }, m_tickbase{ tickbase }, m_ignore{ false }, m_hit_player{ false } {}
};

class ImpactRecord {
public:
	__forceinline ImpactRecord() : m_shot{}, m_pos{}, m_tick{} {}

public:
	ShotRecord* m_shot;
	int         m_tick;
	vec3_t      m_pos;
};

class HitRecord {
public:
	__forceinline HitRecord() : m_impact{}, m_group{ -1 }, m_damage{} {}

public:
	ImpactRecord* m_impact;
	int           m_group;
	float         m_damage;
};

class Shots {

public:
	void OnFrameStage();
	void OnShotFire(Player* target, float damage, int bullets, LagRecord* record);
	void OnImpact(IGameEvent* evt);
	void OnHurt(IGameEvent* evt);

	void OnFire(IGameEvent* evt);

public:
	std::array< std::string, 11 > m_groups = {
   XOR("body"),
   XOR("head"),
   XOR("chest"),
   XOR("stomach"),
   XOR("left arm"),
   XOR("right arm"),
   XOR("left leg"),
   XOR("right leg"),
   XOR("neck"),
   XOR("unknown"),
   XOR("gear")
	};

	std::deque< ShotRecord >          m_shots;
	std::vector< VisualImpactData_t > m_vis_impacts;
	std::deque< ImpactRecord >        m_impacts;
	std::deque< HitRecord >           m_hits;

	float iHitDmg = NULL;
	bool iHit = false;
	bool iHeadshot = false;
	bool canhit = false;

	vec3_t iPlayerOrigin, iPlayermins, iPlayermaxs;
	vec2_t iPlayerbottom, iPlayertop;
};

extern Shots g_shots;