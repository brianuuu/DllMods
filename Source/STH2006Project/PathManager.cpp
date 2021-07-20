#include "PathManager.h"

bool PathManager::xmlTextToVector3f(std::string str, Eigen::Vector3f& v)
{
    if (std::count(str.begin(), str.end(), ' ') != 2) return false;

    float f[3] = { 0,0,0 };
    std::string delimiter = " ";

    int index = 0;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos)
    {
        token = str.substr(0, pos);
        f[index] = static_cast<float>(std::atof(token.c_str()));
        str.erase(0, pos + delimiter.length());

        index++;
    }
    f[index] = static_cast<float>(std::atof(str.c_str()));

    v.x() = f[0];
    v.y() = f[1];
    v.z() = f[2];

    return true;
}

bool PathManager::xmlTextToQuaternionf(std::string str, Eigen::Quaternionf& q)
{
    if (std::count(str.begin(), str.end(), ' ') != 3) return false;

    float f[4] = { 0,0,0,0 };
    std::string delimiter = " ";

    int index = 0;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos)
    {
        token = str.substr(0, pos);
        f[index] = static_cast<float>(std::atof(token.c_str()));
        str.erase(0, pos + delimiter.length());

        index++;
    }
    f[index] = static_cast<float>(std::atof(str.c_str()));

    q.x() = f[0];
    q.y() = f[1];
    q.z() = f[2];
    q.w() = f[3];

    return true;
}

void PathManager::applyTransformationToVector(PathData const& pathData, Eigen::Vector3f& v)
{
    v = pathData.m_rotate * v;
    v.x() *= pathData.m_scale.x();
    v.y() *= pathData.m_scale.y();
    v.z() *= pathData.m_scale.z();
    v += pathData.m_translate;
}

Eigen::Vector3f PathManager::interpolateSegment(KnotDataCollection const& knotDataList, uint32_t index, float t)
{
    float coeff0 = powf(1 - t, 3);
    float coeff1 = 3 * powf(1 - t, 2) * t;
    float coeff2 = 3 * (1 - t) * powf(t, 2);
    float coeff3 = powf(t, 3);

    Eigen::Vector3f point(0, 0, 0);
    point += knotDataList[index].m_point * coeff0;
    point += knotDataList[index].m_outvec * coeff1;
    point += knotDataList[index + 1].m_invec * coeff2;
    point += knotDataList[index + 1].m_point * coeff3;

    return point;
}

Eigen::Vector3f PathManager::interpolateTangent(KnotDataCollection const& knotDataList, uint32_t index, float t)
{
    float coeff0 = -3 * powf(1 - t, 2);
    float coeff1 = 3 * powf(1 - t, 2) - 6 * (1 - t) * t;
    float coeff2 = 6 * (1 - t) * t - 3 * powf(t, 2);
    float coeff3 = 3 * powf(t, 2);

    Eigen::Vector3f tangent(0, 0, 0);
    tangent += knotDataList[index].m_point * coeff0;
    tangent += knotDataList[index].m_outvec * coeff1;
    tangent += knotDataList[index + 1].m_invec * coeff2;
    tangent += knotDataList[index + 1].m_point * coeff3;

    return tangent;
}

tinyxml2::XMLError PathManager::parsePathXml(PathDataCollection& collection, bool closedPaths, char const* pDataOrFileName, size_t pDataSize)
{
    // if size is provided, assume it is already loaded
    tinyxml2::XMLDocument pathXml;
    if (pDataSize > 0)
    {
        tinyxml2::XMLError result = pathXml.Parse(pDataOrFileName, pDataSize);
        XMLCheckResult(result);
    }
    else
    {
        tinyxml2::XMLError result = pathXml.LoadFile(pDataOrFileName);
        XMLCheckResult(result);
    }

    tinyxml2::XMLNode* pRoot = pathXml.FirstChildElement("SonicPath");
    if (pRoot == nullptr) return tinyxml2::XML_ERROR_FILE_READ_ERROR;

    std::map<std::string, PathData> urlToNodeMap;
    tinyxml2::XMLElement* pSceneElement = pRoot->FirstChildElement("scene");
    if (pSceneElement)
    {
        tinyxml2::XMLElement* pNodeElement = pSceneElement->FirstChildElement("node");
        while (pNodeElement != nullptr)
        {
            PathData data;

            // We only care about @GR paths
            data.m_name = std::string(pNodeElement->Attribute("name"));
            //printf("-------------------------\nName: %s\n", data.m_name.c_str());

            tinyxml2::XMLElement* pInstanceElement = pNodeElement->FirstChildElement("instance");
            data.m_url = std::string(pInstanceElement->Attribute("url"));
            data.m_url = data.m_url.substr(1);
            //printf("Url: %s\n", data.m_url.c_str());

            tinyxml2::XMLElement* pTranslateElement = pNodeElement->FirstChildElement("translate");
            xmlTextToVector3f(pTranslateElement->GetText(), data.m_translate);
            //printf("Translate: %.3f, %.3f, %.3f\n", data.m_translate.x(), data.m_translate.y(), data.m_translate.z());

            tinyxml2::XMLElement* pScaleElement = pNodeElement->FirstChildElement("scale");
            xmlTextToVector3f(pScaleElement->GetText(), data.m_scale);
            //printf("Scale: %.3f, %.3f, %.3f\n", data.m_scale.x(), data.m_scale.y(), data.m_scale.z());

            tinyxml2::XMLElement* pRotateElement = pNodeElement->FirstChildElement("rotate");
            xmlTextToQuaternionf(pRotateElement->GetText(), data.m_rotate);
            //printf("Rotate: %.3f, %.3f, %.3f\n", data.m_rotate.x(), data.m_rotate.y(), data.m_rotate.z(), data.m_rotate.w());

            urlToNodeMap[data.m_url] = data;
            pNodeElement = pNodeElement->NextSiblingElement("node");
        }
    }

    tinyxml2::XMLElement* pLibraryElement = pRoot->FirstChildElement("library");
    if (pLibraryElement)
    {
        if (std::string(pLibraryElement->Attribute("type")) == "GEOMETRY")
        {
            tinyxml2::XMLElement* pGeometryElement = pLibraryElement->FirstChildElement("geometry");
            while (pGeometryElement != nullptr)
            {
                // Check if url exist
                std::string url(pGeometryElement->Attribute("name"));
                auto iter = urlToNodeMap.find(url);
                if (iter != urlToNodeMap.end())
                {
                    PathData& pathData = iter->second;
                    printf("[PathManager] Reading path: %s\n", url.c_str());

                    int spline3dCount = 0;
                    std::vector<KnotData>& knotDataList = pathData.m_knots;
                    tinyxml2::XMLElement* pSplineElement = pGeometryElement->FirstChildElement("spline");
                    tinyxml2::XMLElement* pSpline3dElement = pSplineElement->FirstChildElement("spline3d");
                    while (pSpline3dElement != nullptr)
                    {
                        spline3dCount++;
                        if (spline3dCount >= 3)
                        {
                            // wtf there shouldn't be path more than 3 splines
                            break;
                        }

                        //printf("-------------Splind3d-------------\n");

                        int knotIndex = 0;
                        int knotCount = std::stoi(pSpline3dElement->Attribute("count"));
                        knotDataList.resize(knotCount);
                        tinyxml2::XMLElement* pKnotElement = pSpline3dElement->FirstChildElement("knot");
                        while (pKnotElement != nullptr)
                        {
                            if (knotIndex >= knotCount) break;

                            KnotData tempData;

                            tinyxml2::XMLElement* pInvecElement = pKnotElement->FirstChildElement("invec");
                            xmlTextToVector3f(pInvecElement->GetText(), tempData.m_invec);
                            //printf("Invec: %.3f, %.3f, %.3f\n", tempData.m_invec.x(), tempData.m_invec.y(), tempData.m_invec.z());

                            tinyxml2::XMLElement* pOutvecElement = pKnotElement->FirstChildElement("outvec");
                            xmlTextToVector3f(pOutvecElement->GetText(), tempData.m_outvec);
                            //printf("Outvec: %.3f, %.3f, %.3f\n", tempData.m_outvec.x(), tempData.m_outvec.y(), tempData.m_outvec.z());

                            tinyxml2::XMLElement* pPointElement = pKnotElement->FirstChildElement("point");
                            xmlTextToVector3f(pPointElement->GetText(), tempData.m_point);
                            //printf("Point: %.3f, %.3f, %.3f\n", tempData.m_point.x(), tempData.m_point.y(), tempData.m_point.z());

                            KnotData& data = knotDataList[knotIndex];
                            if (spline3dCount == 1)
                            {
                                // Add new knot
                                data = tempData;
                            }
                            else
                            {
                                // Get average
                                data.m_invec = (data.m_invec + tempData.m_invec) / 2;
                                data.m_outvec = (data.m_outvec + tempData.m_outvec) / 2;
                                data.m_point = (data.m_point + tempData.m_point) / 2;
                            }

                            knotIndex++;
                            pKnotElement = pKnotElement->NextSiblingElement("knot");
                        }

                        pSpline3dElement = pSpline3dElement->NextSiblingElement("spline3d");
                    }

                    // If it's a closed path, we need to insert the first knot again
                    if (closedPaths)
                    {
                        knotDataList.push_back(knotDataList[0]);
                    }

                    // Now scale, rotate and translate all knots
                    for (KnotData& knotData : knotDataList)
                    {
                        applyTransformationToVector(pathData, knotData.m_invec);
                        applyTransformationToVector(pathData, knotData.m_outvec);
                        applyTransformationToVector(pathData, knotData.m_point);
                        //printf("Point: %.3f, %.3f, %.3f\n", knotData.m_point.x(), knotData.m_point.y(), knotData.m_point.z());
                    }

                    pathData.m_segmentLengths.resize(knotDataList.size() - 1);
                    for (uint32_t i = 0; i < knotDataList.size() - 1; i++)
                    {
                        // Estimate the length for each segment
                        float segmentLength = 0;
                        Eigen::Vector3f points[SAMPLE_COUNT + 1];
                        for (uint32_t j = 0; j <= SAMPLE_COUNT; j++)
                        {
                            float t = (float)j / SAMPLE_COUNT;
                            points[j] = interpolateSegment(knotDataList, i, t);

                            if (j > 0)
                            {
                                segmentLength += (points[j] - points[j - 1]).norm();
                            }
                        }
                        pathData.m_segmentLengths[i] = segmentLength;
                    }

                    // Push path data
                    collection.push_back(pathData);
                }

                pGeometryElement = pGeometryElement->NextSiblingElement("geometry");
            }
        }
    }

	return tinyxml2::XML_SUCCESS;
}

//-------------------------------------------------------------
// Get the next position and rotation of the follow object
//-------------------------------------------------------------
void PathManager::followAdvance(PathFollowData& followData, float dt)
{
    if (!followData.m_pPathData || !followData.m_enabled) return;

    std::vector<float> const& segmentLengths = followData.m_pPathData->m_segmentLengths;

    // Find the target segment we end up at
    float distRemaining = followData.m_speed * dt;
    float segmentLength = (1.0f - followData.m_segmentTime) * segmentLengths[followData.m_segmentID];
    if (distRemaining > segmentLength)
    {
        while (distRemaining > segmentLength)
        {
            distRemaining -= segmentLength;

            followData.m_segmentID++;
            if (followData.m_segmentID >= segmentLengths.size())
            {
                followData.m_segmentID = 0;
            }
            segmentLength = segmentLengths[followData.m_segmentID];
        }
        followData.m_segmentTime = distRemaining / segmentLength;
    }
    else
    {
        followData.m_segmentTime = 1.0f - ((segmentLength - distRemaining) / segmentLengths[followData.m_segmentID]);
    }

    // Calculate the new position and rotation
    Eigen::Vector3f dir = interpolateTangent(followData.m_pPathData->m_knots, followData.m_segmentID, followData.m_segmentTime);
    Eigen::Quaternionf rotation(0, 0, 0, 1);
    if (followData.m_yawOnly)
    {
        dir.y() = 0;
        dir.normalize();
        float yaw = acos(dir.z());
        if (dir.dot(Eigen::Vector3f::UnitX()) < 0) yaw = -yaw;
        DebugDrawText::draw(format("yaw = %.4f", yaw * 180/PI), { 0,0 }, 3);
        rotation = Eigen::AngleAxisf(yaw, Eigen::Vector3f::UnitY());
    }
    else
    {
        dir.normalize();
        rotation = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f::UnitZ(), dir);
    }
    followData.m_position = interpolateSegment(followData.m_pPathData->m_knots, followData.m_segmentID, followData.m_segmentTime);
    followData.m_rotation = rotation;
}
