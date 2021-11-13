#pragma once
class CustomCamera
{
public:
	static void applyPatches();
	static void advance();

	static Eigen::Vector3f calculateCameraPos
	(
		Eigen::Vector3f const& playerPosition,
		Eigen::Vector3f const& playerUpAxis,
		Eigen::Vector3f const& pitchAxis,
		float const& pitch,
		float const& targetPitch
	);
};

