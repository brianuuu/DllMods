#include "NextGenSonic.h"
#include "Configuration.h"

void NextGenSonic::setAnimationSpeed_Sonic(NextGenAnimation& data)
{
    if (Configuration::m_physics)
    {
        data.jog_speedFactor = 2.7f;
        data.run_speedFactor = 3.2f;
        data.dash_speedFactor = 5.0f;
        data.jet_playbackSpeed = 6.0f;
        data.jet_speedFactor = -1.0f;
        data.jetWall_playbackSpeed = 6.0f;
        data.jetWall_speedFactor = -1.0f;
        data.boost_playbackSpeed = 7.0f;
        data.boost_speedFactor = -1.0f;
        data.boostWall_playbackSpeed = 7.0f;
        data.boostWall_speedFactor = -1.0f;
    }
    else
    {
        data.run_speedFactor = 6.0f;
        data.dash_speedFactor = 9.0f;
    }
}

void NextGenSonic::setAnimationSpeed_Elise(NextGenAnimation& data)
{
    if (Configuration::m_physics)
    {
        data.run_speedFactor = 5.0f;
        data.dash_speedFactor = 7.0f;
    }
    else
    {
        data.run_speedFactor = 6.0f;
        data.dash_speedFactor = 9.0f;
    }
}
