/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Various physics change on grinding rails
//				 Common: Disable GrindSquat and use GrindSwitch immediately
//				 06 Physics: GrindSwitch speed up and rail lock-on
/*----------------------------------------------------------*/

#pragma once

struct KnotData
{
	Eigen::Vector3f m_invec;
	Eigen::Vector3f m_outvec;
	Eigen::Vector3f m_point;
};

struct PathData
{
	std::string m_name;
	std::string m_url;
	Eigen::Vector3f m_translate;
	Eigen::Vector3f m_scale;
	Eigen::Quaternionf m_rotate;

	std::vector<Eigen::Vector3f> m_points;

	// Bounding sphere
	Eigen::Vector3f m_centre;
	float m_radius;

	bool m_enabled;
};

class RailPhysics
{
public:
	static void applyPatches();

	static uint32_t* m_pHomingTargetObj;
	static uint32_t* m_pHomingTargetCEventCollision;

	static float m_grindSpeed;
	static float m_grindAccelTime;

	static void updateHomingTargetPos();
	static bool getHomingTargetPos(Eigen::Vector3f& pos);
	static void setHomingTargetPos(Eigen::Vector3f pos);

	// Path data
	static bool xmlTextToVector3f(std::string str, Eigen::Vector3f& v);
	static bool xmlTextToQuaternionf(std::string str, Eigen::Quaternionf& q);
	static void applyTransformationToVector(PathData const& pathData, Eigen::Vector3f& v);
	static void getBoundingSphere(PathData& pathData);
	static Eigen::Vector3f interpolateSegment(std::vector<KnotData> const& knotDataList, uint32_t index, float t);
	static tinyxml2::XMLError parsePathXmlData(char const* pData, uint32_t size);
	static std::vector<PathData> m_pathData;
};

