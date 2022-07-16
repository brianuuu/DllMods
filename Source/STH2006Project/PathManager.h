/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: A helper manager that deals with paths
/*----------------------------------------------------------*/

#pragma once

#define SAMPLE_COUNT 20

struct KnotData
{
	Eigen::Vector3f m_invec;
	Eigen::Vector3f m_outvec;
	Eigen::Vector3f m_point;
};
typedef std::vector<KnotData> KnotDataCollection;

struct PathData
{
	std::string m_name;
	std::string m_url;
	Eigen::Vector3f m_translate;
	Eigen::Vector3f m_scale;
	Eigen::Quaternionf m_rotate;

	KnotDataCollection m_knots;
	std::vector<float> m_segmentLengths;
};
typedef std::vector<PathData> PathDataCollection;

struct PathFollowData
{
	bool m_yawOnly;
	bool m_enabled;

	bool m_loop;
	bool m_finished;

	size_t m_segmentID;
	float m_segmentTime;

	float m_speed;
	Eigen::Vector3f m_position;
	Eigen::Quaternionf m_rotation;

	PathData* m_pPathData;

	PathFollowData()
		: m_yawOnly(false)
		, m_enabled(true)
		, m_loop(true)
		, m_finished(false)
		, m_segmentID(0)
		, m_segmentTime(0.0f)
		, m_speed(0.0f)
		, m_pPathData(nullptr)
	{}
};

class PathManager
{
public:
	static bool xmlTextToVector3f(std::string str, Eigen::Vector3f& v);
	static bool xmlTextToQuaternionf(std::string str, Eigen::Quaternionf& q);
	static void applyTransformationToVector(PathData const& pathData, Eigen::Vector3f& v);

	static Eigen::Vector3f interpolateSegment(KnotDataCollection const& knotDataList, uint32_t index, float t);
	static Eigen::Vector3f interpolateTangent(KnotDataCollection const& knotDataList, uint32_t index, float t);

	static tinyxml2::XMLError parsePathXml(PathDataCollection& collection, bool closedPaths, char const* pDataOrFileName, size_t pDataSize = 0);
	
	static void followAdvance(PathFollowData& followData, float dt);
};

