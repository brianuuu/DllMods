#include "RailPhysics.h"
#include "Configuration.h"
#include "Application.h"

uint32_t* RailPhysics::m_pHomingTargetObj = nullptr;
uint32_t* RailPhysics::m_pHomingTargetCEventCollision = nullptr;
uint32_t RailPhysics::m_isGettingHomingTarget = false;
uint32_t m_forceRailCollisionQuery = false;
std::set<uint32_t*> RailPhysics::m_pPathContainer;
std::vector<PathData> RailPhysics::m_pathData;

float const c_lockOnRange = 8.0f;
float RailPhysics::m_grindSpeed = 0.0f;
float RailPhysics::m_grindAccelTime = 0.0f;
float const c_grindSpeedInit = 23.0f;
float const c_grindSpeedInitSuper = 30.0f;
float const c_grindSpeedBoard = 28.0f;
float const c_grindSpeedMax = 40.0f;
float const c_grindSpeedMaxSuper = 60.0f;
float const c_grindAccel = 15.0f;
float const c_grindAccelSuper = 25.0f;
float const c_grindAccelTime = 0.333f;
float const c_grindJumpPower = 18.0f;
float const c_grindJumpSpeedMax = 15.0f;

FUNCTION_PTR(void*, __thiscall, processGameObjectMsgSetPosition, 0xD5CEB0, void* This, void* message);
FUNCTION_PTR(void*, __thiscall, processGameObjectMsgSetRotation, 0xD5CE70, void* This, void* message);

HOOK(int, __stdcall, RailPhysics_HomingUpdate, 0xE5FF10, int a1)
{
    RailPhysics::updateHomingTargetPos();
    int result = originalRailPhysics_HomingUpdate(a1);
    m_forceRailCollisionQuery = false;
    return result;
}

HOOK(bool, __stdcall, RailPhysics_ParsePathXml, 0x11E3460, uint32_t* This, char* pData, uint32_t size, char a4)
{
    RailPhysics::m_pPathContainer.insert(This);
    RailPhysics::parsePathXmlData(pData, size);
    return originalRailPhysics_ParsePathXml(This, pData, size, a4);
}

HOOK(uint32_t*, __fastcall, RailPhysics_CHomingTargetDestructor, 0x500280, uint32_t* This, void* Edx, char a2)
{
    if (This == RailPhysics::m_pHomingTargetObj)
    {
        RailPhysics::m_pHomingTargetObj = nullptr;
        RailPhysics::m_pHomingTargetCEventCollision = nullptr;
        printf("[RailPhysics] Homing Target Destructed\n");
    }

    return originalRailPhysics_CHomingTargetDestructor(This, Edx, a2);
}

HOOK(uint32_t*, __fastcall, RailPhysics_CDatabaseDataDestructor, 0x699380, uint32_t* This)
{
    if (!RailPhysics::m_pPathContainer.empty())
    {
        if (RailPhysics::m_pPathContainer.find(This) != RailPhysics::m_pPathContainer.end())
        {
            // Just clear it all
            RailPhysics::m_pPathContainer.clear();
            RailPhysics::m_pathData.clear();
            printf("[RailPhysics] Path Data Cleared\n");
        }
    }

    return originalRailPhysics_CDatabaseDataDestructor(This);
}

HOOK(uint32_t*, __cdecl, RailPhysics_CEventCollisionConstructor, 0x11836F0, int a1, int a2, int a3, int a4, int a5, char a6, int a7)
{
    uint32_t* pObject = originalRailPhysics_CEventCollisionConstructor(a1, a2, a3, a4, a5, a6, a7);

    if (RailPhysics::m_isGettingHomingTarget)
    {
        RailPhysics::m_pHomingTargetCEventCollision = (uint32_t*)*pObject;
        RailPhysics::m_isGettingHomingTarget = false;
    }

    return pObject;
}

HOOK(int, __fastcall, RailPhysics_CSonicPostureGrindBegin, 0x11D8060, void* This)
{
    RailPhysics::m_grindSpeed = Common::IsPlayerOnBoard() ? c_grindSpeedBoard : (Common::IsPlayerSuper() ? c_grindSpeedInitSuper : c_grindSpeedInit);
    RailPhysics::m_grindAccelTime = 0.0f;
    return originalRailPhysics_CSonicPostureGrindBegin(This);
}

HOOK(void, __fastcall, RailPhysics_CSonicPostureGrindAdvance, 0x11D81E0, void* This)
{
    originalRailPhysics_CSonicPostureGrindAdvance(This);

    Eigen::Vector3f velocity;
    if (Common::GetPlayerVelocity(velocity))
    {
        float dt = Application::getDeltaTime();
        bool isSuper = Common::IsPlayerSuper();

        if (Common::GetSonicStateFlags()->Boost)
        {
            // Always allow accleration when boosting, but slower
            RailPhysics::m_grindSpeed += (isSuper ? c_grindAccelSuper : (c_grindAccel * 0.3f)) * dt;
        }
        else if (RailPhysics::m_grindAccelTime > 0.0f)
        {
            // Acclerate after doing grind trick
            RailPhysics::m_grindSpeed += (isSuper ? c_grindAccelSuper : c_grindAccel) * min(RailPhysics::m_grindAccelTime, dt);
            RailPhysics::m_grindAccelTime -= dt;
        }

        // Get Sonic's rotation in case he's going backwards
        Eigen::Vector3f playerPosition;
        Eigen::Quaternionf playerRotation;
        Common::GetPlayerTransform(playerPosition, playerRotation);
        Eigen::Vector3f playerDirection = playerRotation * Eigen::Vector3f(0, 0, 1);
        bool forward = playerDirection.dot(velocity) >= 0.0f;

        // Set current velocity
        RailPhysics::m_grindSpeed = min(RailPhysics::m_grindSpeed, (isSuper ? c_grindSpeedMaxSuper : c_grindSpeedMax));
        velocity = velocity.normalized() * RailPhysics::m_grindSpeed * (forward ? 1.0f : -1.0f);
        Common::SetPlayerVelocity(velocity);
    }
}

HOOK(int, __fastcall, RailPhysics_CSonicStateGrindSquatBegin, 0x1118830, void* This)
{
    // Gain speed
    if (Configuration::m_physics)
    {
        RailPhysics::m_grindAccelTime += c_grindAccelTime;
    }

    // Play grind trick sfx
    static SharedPtrTypeless soundHandle;
    Common::SonicContextPlaySound(soundHandle, 80041020, 1);

    return originalRailPhysics_CSonicStateGrindSquatBegin(This);
}

HOOK(bool, __fastcall, RailPhysics_CSonicStateGrindJumpShortBegin, 0x124A8C0, void* This)
{
    bool result = originalRailPhysics_CSonicStateGrindJumpShortBegin(This);

    if (!Common::IsPlayerOnBoard())
    {
        float* targetVel = (float*)((uint32_t)*PLAYER_CONTEXT + 0x2A0);
        Eigen::Vector3f vel(targetVel[0], 0, targetVel[2]);
        if (vel.squaredNorm() > c_grindJumpSpeedMax * c_grindJumpSpeedMax)
        {
            // Cap horizontal speed
            vel = vel.normalized() * c_grindJumpSpeedMax;
        }

        vel.y() = c_grindJumpPower;
        if (targetVel[1] > 0.0f)
        {
            // Only add vertical speed if going up, otherwise use fixed jump power
            vel.y() += targetVel[1];
        }

        Common::SetPlayerVelocity(vel);
    }

    return result;
}

HOOK(bool, __fastcall, RailPhysics_CheckRailSwitch, 0xDFCC70, void* This, void* Edx, int a2, int a3)
{
    if (Common::IsPlayerIn2D())
    {
        return false;
    }
    else
    {
        return originalRailPhysics_CheckRailSwitch(This, Edx, a2, a3);
    }
}

void __declspec(naked) getHomingTargetObj()
{
    static uint32_t returnAddress = 0xE91412;
    static uint32_t sub_D5DBA0 = 0xD5DBA0;
    __asm
    {
        mov     RailPhysics::m_pHomingTargetObj, esi
        mov     RailPhysics::m_isGettingHomingTarget, 1
        call    [sub_D5DBA0]
        jmp     [returnAddress]
    }
}

void __declspec(naked) createHomingTargetObj()
{
    static uint32_t returnAddress = 0x121EDB0;
    static uint32_t skipAddress = 0x121EE56;
    __asm
    {
        mov     esi, RailPhysics::m_pHomingTargetObj
        test    esi, esi
        jnz     jump

        mov     esi, [ebx+0xAC]
        jmp     [returnAddress]

        jump:
        jmp     [skipAddress]
    }
}

void __declspec(naked) forceQueryHomingCollision()
{
    static uint32_t originalAddress = 0xE744C7;
    static uint32_t returnAddress = 0xE74529;
    static uint32_t tempQueryList = 0;
    static uint32_t* tempQueryListPtr = &tempQueryList;
    __asm
    {
        mov     [esp + 1Ch], eax

        // Are we at HomingUpdate function?
        mov     eax, m_forceRailCollisionQuery
        test    eax, eax
        jz      jump

        // Check we have any rails
        mov     eax, RailPhysics::m_pHomingTargetCEventCollision
        test    eax, eax
        jz      jump
        
        // Force put rail event collision in query list
        mov     edx, [esp + 1Ch]
        mov     tempQueryList, edx
        mov     edx, tempQueryListPtr
        mov     [esp + 1Ch], edx
        jmp     [returnAddress]

        // Original function
        jump:
        mov     eax, [edi]
        cmp     eax, edi
        jmp     [originalAddress]
    }
}

void __declspec(naked) allowJumpDuringGrindSwitch()
{
    static uint32_t returnAddress = 0x1232659;
    static uint32_t successAddress = 0x12326D1;
    static uint32_t sub_DFF060 = 0xDFF060; // GrindJumpShort function
    static uint32_t sub_DFCC70 = 0xDFCC70; // GrindJumpSide function
    __asm
    {
        // Required pop to edi before returning
        push    eax

        // Replicate sub_DFF0B0 but only check GrindJumpShort & GrindJumpSide
        mov     edi, ecx
        mov     esi, eax
        cmp     dword ptr[esi + 11F0h], 0
        jz      jump

        // Check GrindJumpShort
        mov     eax, esi
        call    [sub_DFF060]
        test    al, al
        jnz     success

        // Check GrindJumpSide
        push    edi
        push    esi
        call    [sub_DFCC70]
        test    al, al
        jz      jump

        // State changed
        success:
        pop     edi
        jmp     [successAddress]

        // Original function
        jump:
        pop     edi
        cmp     byte ptr [ebx + 68h], 0
        jmp     [returnAddress]
    }
}

void RailPhysics::applyPatches()
{
    // Don't disable rail sfx when doing GrindSwitch
    WRITE_MEMORY(0x123260D, uint8_t, 0xEB);

    // Play GrindSwitch spark a bit earlier
    static double const grindSwitchSparkTime = 24.0;
    WRITE_MEMORY(0x1232682, double*, &grindSwitchSparkTime);

    // Play normal rail sfx on board grind
    WRITE_MEMORY(0xE4FC78, uint8_t, 0xEB);

    // Add speed at start of GrindSquat
    INSTALL_HOOK(RailPhysics_CSonicStateGrindSquatBegin);

    // Disable GrindSquat and skip to GrindSwitch immdiately
    WRITE_MEMORY(0x1118886, uint8_t, 0x58, 0x90);
    WRITE_NOP(0x1118979, 0xA);
    WRITE_JUMP(0x1232653, allowJumpDuringGrindSwitch);

    // Skip BoardLand on grind rails
    WRITE_NOP(0x111D55A, 0x2);
    WRITE_NOP(0x111D569, 0x2);

    // Don't check for rail switch in 2D
    INSTALL_HOOK(RailPhysics_CheckRailSwitch);

    // Force normal grinding animation even when holding stick after switching
    WRITE_NOP(0xDF26E3, 2);

    if (Configuration::m_physics)
    {
        // Only allow one homing target
        WRITE_JUMP(0x121EDAA, createHomingTargetObj);

        // Grab the target object pointer
        WRITE_JUMP(0xE9140D, getHomingTargetObj);

        // Force rail event collision to be "detected"
        WRITE_JUMP(0xE744C1, forceQueryHomingCollision);

        // Grab the event collision pointer
        INSTALL_HOOK(RailPhysics_CEventCollisionConstructor);

        // Update rail lock-on position
        INSTALL_HOOK(RailPhysics_HomingUpdate);

        // For resetting pointers
        INSTALL_HOOK(RailPhysics_CHomingTargetDestructor);

        // Disable stick rail switching
        WRITE_MEMORY(0xDFCC9F, uint8_t, 0xEB, 0x6D);

        // Force constant velocity on rails
        INSTALL_HOOK(RailPhysics_CSonicPostureGrindBegin);
        INSTALL_HOOK(RailPhysics_CSonicPostureGrindAdvance);

        // Normalize grind jump speed
        INSTALL_HOOK(RailPhysics_CSonicStateGrindJumpShortBegin);

        // Patch "Disable Rail Boosters" by "Hyper"
        WRITE_MEMORY(0x166F238, uint8_t, 0x00);

        // Get all grind rail path data
        INSTALL_HOOK(RailPhysics_ParsePathXml);

        // For resetting path data
        INSTALL_HOOK(RailPhysics_CDatabaseDataDestructor);
    }
}

void RailPhysics::updateHomingTargetPos()
{
    if (!RailPhysics::m_pHomingTargetObj) return;

    Eigen::Vector3f playerWorldDir;
    if (!Common::GetPlayerWorldDirection(playerWorldDir, true)) return;
    //printf("World Dir: %.3f, %.3f, %.3f\n", playerWorldDir.x(), playerWorldDir.y(), playerWorldDir.z());

    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (!Common::GetPlayerTransform(playerPosition, playerRotation)) return;

    // Get the closest paths (filter out far away paths for optimization)
    Eigen::Vector3f testPos = playerPosition + playerWorldDir * 6;
    std::set<uint32_t> closestPathIndices;
    for (uint32_t i = 0; i < m_pathData.size(); i++)
    {
        PathData const& pathData = m_pathData[i];

        // Check if bounding sphere (plus a little) enclose test point
        if ((testPos - pathData.m_centre).squaredNorm() <= pow(pathData.m_radius + 20.0f, 2))
        {
            closestPathIndices.insert(i);
        }
    }

    // Find the closest lock-on position from the list of paths
    float const rangeSquared = c_lockOnRange * c_lockOnRange;
    Eigen::Vector3f lockPos(0, 10000, 0);
    float minDistSquared = FLT_MAX;
    for (uint32_t index : closestPathIndices)
    {
        PathData const& pathData = m_pathData[index];
        for (uint32_t i = 0; i < pathData.m_points.size() - 1; i++)
        {
            Eigen::Vector3f const& p0 = pathData.m_points[i];
            Eigen::Vector3f const& p1 = pathData.m_points[i + 1];

            // Both points a too away, skip
            if ((testPos - p0).squaredNorm() > rangeSquared
             && (testPos - p1).squaredNorm() > rangeSquared)
            {
                continue;
            }

            Eigen::Vector3f dir = p1 - p0;
            float segLength = dir.norm();
            dir.normalize();

            Eigen::Vector3f dist = testPos - p0;
            float t = dist.dot(dir);
            if (t < 0) t = 0;
            if (t > segLength) t = segLength;

            Eigen::Vector3f pt = p0 + dir * t;
            float currentDistSquared = (pt - testPos).squaredNorm();
            if (currentDistSquared <= rangeSquared
             && currentDistSquared < minDistSquared)
            {
                m_forceRailCollisionQuery = true;
                lockPos = pt;
                minDistSquared = currentDistSquared;
            }
        }
    }

    if (m_forceRailCollisionQuery)
    {
        //printf("Lock Pos: %.3f, %.3f, %.3f\n", lockPos.x(), lockPos.y(), lockPos.z());
    }
    setHomingTargetPos(lockPos);
}

bool RailPhysics::getHomingTargetPos(Eigen::Vector3f& pos)
{
    if (m_pHomingTargetObj && m_pHomingTargetCEventCollision)
    {
        float* position = (float*)(RailPhysics::m_pHomingTargetObj[46] + 0x70);
        pos = Eigen::Vector3f(position[0], position[1], position[2]);
        return true;
    }

    return false;
}

void RailPhysics::setHomingTargetPos(Eigen::Vector3f pos)
{
    if (m_pHomingTargetObj && m_pHomingTargetCEventCollision)
    {
        alignas(16) MsgSetPosition msgSetPosition {};
        msgSetPosition.m_position = pos;
        processGameObjectMsgSetPosition(m_pHomingTargetObj, &msgSetPosition);

        /*float* position = (float*)(m_pHomingTargetObj[46] + 0x70);
        position[0] = pos.x();
        position[1] = pos.y();
        position[2] = pos.z();*/

        float* positionPhysics = (float*)((uint32_t)(*(uint32_t**)((uint32_t)m_pHomingTargetCEventCollision + 0x4)) + 0x120);
        positionPhysics[0] = pos.x();
        positionPhysics[1] = pos.y();
        positionPhysics[2] = pos.z();
    }
}

bool RailPhysics::xmlTextToVector3f(std::string str, Eigen::Vector3f& v)
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

bool RailPhysics::xmlTextToQuaternionf(std::string str, Eigen::Quaternionf& q)
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

void RailPhysics::applyTransformationToVector(PathData const& pathData, Eigen::Vector3f& v)
{
    v = pathData.m_rotate * v;
    v.x() *= pathData.m_scale.x();
    v.y() *= pathData.m_scale.y();
    v.z() *= pathData.m_scale.z();
    v += pathData.m_translate;
}

void RailPhysics::getBoundingSphere(PathData& pathData)
{
    if (pathData.m_points.size() < 2) return;

    // Ritter's bounding sphere
    // 1. Pick point x, find point y with largest distance from x
    // 2. Find point z with largest distance from y, get point c at the middle of x & z and radius (x-z)/2
    // 3. If point p is outside circle, new radius is (c-p)

    std::vector<Eigen::Vector3f> points = pathData.m_points;

    // Step 1
    float currentDistSquared = 0;
    Eigen::Vector3f x = points[0];
    Eigen::Vector3f y = points[0];
    for (Eigen::Vector3f const& point : points)
    {
        float distSquared = (point - x).squaredNorm();
        if (distSquared > currentDistSquared)
        {
            currentDistSquared = distSquared;
            y = point;
        }
    }

    // Step 2
    currentDistSquared = 0;
    Eigen::Vector3f z = y;
    for (Eigen::Vector3f const& point : points)
    {
        float distSquared = (point - y).squaredNorm();
        if (distSquared > currentDistSquared)
        {
            currentDistSquared = distSquared;
            z = point;
        }
    }

    // Step 3
    pathData.m_centre = y + (z - y) / 2;
    float maxRadiusSquared = (z - pathData.m_centre).squaredNorm();
    while (!points.empty())
    {
        auto iter = points.begin();
        while (iter != points.end())
        {
            float currentRadiusSquared = (*iter - pathData.m_centre).squaredNorm();
            if (currentRadiusSquared <= maxRadiusSquared)
            {
                // Enclosed point, remove it
                iter = points.erase(iter);
            }
            else
            {
                maxRadiusSquared = currentRadiusSquared;
                points.erase(iter);
                break;
            }
        }
    }

    pathData.m_radius = sqrtf(maxRadiusSquared);
}

Eigen::Vector3f RailPhysics::interpolateSegment(std::vector<KnotData> const& knotDataList, uint32_t index, float t)
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

tinyxml2::XMLError RailPhysics::parsePathXmlData(char const* pData, uint32_t size)
{
    tinyxml2::XMLDocument pathXml;
    tinyxml2::XMLError result = pathXml.Parse(pData, size);
    XMLCheckResult(result);

    //printf("Parsing a path xml file...\n");
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
            if (Common::IsStringEndsWith(data.m_name, "@GR"))
            {
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
            }

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
                    //printf("-------------------------\nName: %s\n", url.c_str());

                    int spline3dCount = 0;
                    std::vector<KnotData> knotDataList;
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
                    
                    if (knotDataList.size() >= 2)
                    {
                        // Now scale, rotate and translate all knots
                        for (KnotData& knotData : knotDataList)
                        {
                            applyTransformationToVector(pathData, knotData.m_invec);
                            applyTransformationToVector(pathData, knotData.m_outvec);
                            applyTransformationToVector(pathData, knotData.m_point);
                            //printf("Point: %.3f, %.3f, %.3f\n", knotData.m_point.x(), knotData.m_point.y(), knotData.m_point.z());
                        }

                        for (uint32_t i = 0; i < knotDataList.size() - 1; i++)
                        {
                            // Estimate the length for each segment
                            float segmentLength = 0;
                            Eigen::Vector3f points[9];
                            for (uint32_t j = 0; j <= 8; j++)
                            {
                                float t = 0.125f * j;
                                points[j] = interpolateSegment(knotDataList, i, t);

                                if (j > 0)
                                {
                                    segmentLength += (points[j] - points[j - 1]).norm();
                                }
                            }

                            // Divide the segment to below target length
                            float const targetSegmentLength = 2.0f;
                            float const targetSegmentCount = ceil(segmentLength / targetSegmentLength);

                            if (targetSegmentCount > 1)
                            {
                                for (int j = 0; j < static_cast<int>(targetSegmentCount); j++)
                                {
                                    float t = j  / targetSegmentCount;
                                    pathData.m_points.push_back(interpolateSegment(knotDataList, i, t));
                                }
                            }
                            else
                            {
                                // No need to divide, just add current point
                                pathData.m_points.push_back(knotDataList[i].m_point);
                            }
                        }
                        pathData.m_points.push_back(knotDataList[knotDataList.size() - 1].m_point);

                        // Push path data
                        getBoundingSphere(pathData);
                        m_pathData.push_back(pathData);
                    }
                }

                pGeometryElement = pGeometryElement->NextSiblingElement("geometry");
            }
        }
    }

    return tinyxml2::XML_SUCCESS;
}
