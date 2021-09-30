#include "includes.h"

NetData g_netdata{};;

void NetData::store( ) {
	int          tickbase;
	int          slot;
	StoredData_t *data;

	if( !g_cl.m_processing ) {
		reset( );
		return;
	}

	tickbase = g_cl.m_local->m_nTickBase( );
	slot = g_cl.m_cmd->m_command_number;

	// get current record and store data.
	data = &m_data[ slot % MULTIPLAYER_BACKUP ];

	data->m_tickbase = tickbase;
	data->m_command = slot;
	data->m_punch = g_cl.m_local->m_aimPunchAngle( );
	data->m_punch_vel = g_cl.m_local->m_aimPunchAngleVel( );
	data->m_view_offset = g_cl.m_local->m_vecViewOffset( );
}

void NetData::apply( ) {
    int          tickbase;
    StoredData_t *data;
    ang_t        punch_delta, punch_vel_delta;
	vec3_t       view_delta;

    if( !g_cl.m_processing ) {
		reset( );
		return;
	}

    tickbase = g_cl.m_local->m_nTickBase( );
    
    // get current record and validate.
    data = &m_data[ tickbase % MULTIPLAYER_BACKUP ];

    if( g_cl.m_local->m_nTickBase( ) != data->m_tickbase )
    	return;
    
    // get deltas.
    // note - dex;  before, when you stop shooting, punch values would sit around 0.03125 and then goto 0 next update.
    //              with this fix applied, values slowly decay under 0.03125.
    punch_delta     = g_cl.m_local->m_aimPunchAngle( ) - data->m_punch;
    punch_vel_delta = g_cl.m_local->m_aimPunchAngleVel( ) - data->m_punch_vel;
	view_delta      = g_cl.m_local->m_vecViewOffset( ) - data->m_view_offset;
    
	// get deltas.
	 // note - dex;  before, when you stop shooting, punch values would sit around 0.03125 and then goto 0 next update.
	 //              with this fix applied, values slowly decay under 0.03125.
	auto delta = [ ]( const ang_t& a, const ang_t& b ) {
		auto delta = a - b;
		return std::sqrt( delta.x * delta.x + delta.y * delta.y + delta.z * delta.z );
	};

	// set data.
	if( delta( g_cl.m_local->m_aimPunchAngle( ), data->m_punch ) < 0.03125f ) {
		g_cl.m_local->m_aimPunchAngle( ) = data->m_punch;
	}

	if( delta( g_cl.m_local->m_aimPunchAngleVel( ), data->m_punch_vel ) < 0.03125f ) {
		g_cl.m_local->m_aimPunchAngleVel( ) = data->m_punch_vel;
	}

	if( g_cl.m_local->m_vecViewOffset( ).dist_to( data->m_view_offset ) < 0.03125f ) {
		g_cl.m_local->m_vecViewOffset( ) = data->m_view_offset;
	}
}

void NetData::reset( ) {
	m_data.fill( StoredData_t( ) );
}

void NetData::reduction( CUserCmd* cmd ) {
	if( !g_cl.m_local || !m_data.empty( ) )
		return;

	// we predicted him, but our tickbase breaks
	// so lets fix it.
	if( cmd->m_predicted ) {
		for( auto& data : m_data ) {
			data.m_tickbase--;
		}

		// exit out now.
		return;
	}
}