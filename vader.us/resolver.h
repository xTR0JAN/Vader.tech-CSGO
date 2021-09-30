#pragma once

class ShotRecord;

struct AntiFreestandingRecord
{
	int right_damage = 0, left_damage = 0, back_damage = 0;
	float right_fraction = 0.f, left_fraction = 0.f, back_fraction = 0.f;
};


class Resolver {
public:
	enum Modes : size_t {
		RESOLVE_NONE = 0,
		RESOLVE_WALK,
		RESOLVE_STAND,
		RESOLVE_STAND1,
		RESOLVE_STAND2,
		RESOLVE_AIR,
		RESOLVE_BODY,
		RESOLVE_STOPPED_MOVING,
		RESOLVE_OVERRIDE,
		RESOLVE_LASTMOVE,
		RESOLVE_ANTIFREESTAND,
		RESOLVE_UNKNOWM,
		RESOLVE_DELTA,
		RESOLVE_BRUTEFORCE,
	};

public:
	LagRecord* FindIdealRecord(AimPlayer* data);
	LagRecord* FindLastRecord(AimPlayer* data);

	LagRecord* FindFirstRecord(AimPlayer* data);

	float GetLBYRotatedYaw(float lby, float yaw);

	bool IsYawSideways(Player* entity, float yaw);

	void OnBodyUpdate(Player* player, float value);
	float GetAwayAngle(LagRecord* record);

	void MatchShot(AimPlayer* data, LagRecord* record);
	void SetMode(LagRecord* record);

	void SupremAntiFreestanding(LagRecord* record);

	void SupremAntiFreestandingReversed(LagRecord* record);

	void collect_wall_detect(const Stage_t stage);

	bool AntiFreestanding(Player* entity, AimPlayer* data, float& yaw);

	void ResolveAngles(Player* player, LagRecord* record);
	void ResolveWalk(AimPlayer* data, LagRecord* record);
	void ResolveYawBruteforce(LagRecord* record, Player* player, AimPlayer* data);
	float GetDirectionAngle(int index, Player* player);
	void MainResolver(LagRecord* record, AimPlayer* data, Player* player);
	void ResolveStand(AimPlayer* data, LagRecord* record);
	void FindBestAngle( AimPlayer * data );
	void StandNS(AimPlayer* data, LagRecord* record);
	void ResolveAir(AimPlayer* data, LagRecord* record, Player* player);

	void AirNS(AimPlayer* data, LagRecord* record);
	void ResolvePoses(Player* player, LagRecord* record);
	void ResolveOverride(Player* player, LagRecord* record, AimPlayer* data);


public:
	std::array< vec3_t, 64 > m_impacts;
	int	   iPlayers[64];
	bool   m_step_switch;
	int    m_random_lag;
	float  m_next_random_update;
	float  m_random_angle;
	float  m_direction;
	float  m_auto;
	float  m_auto_dist;
	float  m_auto_last;
	float  m_view;
	bool is_flicking;

	int m_iMode;

	AntiFreestandingRecord anti_freestanding_record;

	class PlayerResolveRecord
	{
	public:
		struct AntiFreestandingRecord
		{
			int right_damage = 0, left_damage = 0;
			float right_fraction = 0.f, left_fraction = 0.f;
		};

	public:
		AntiFreestandingRecord m_sAntiEdge;
	};

	PlayerResolveRecord player_resolve_records[33];
private:
	vec3_t last_eye;

	float left_damage[64];
	float right_damage[64];
	float back_damage[64];

	std::vector<vec3_t> last_eye_positions;

};

extern Resolver g_resolver;

extern bool hitPlayer[64];