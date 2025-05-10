/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2025
///	Description: Manager that manages loading mst files
/*----------------------------------------------------------*/

#pragma once
#include "Utils/mst.h"

class MstManager
{
public:
	static bool IsRequested(std::string const& name);
	static bool RequestMst(std::string const& name);
	static bool RequestMstRawPath(std::string const& name);

	static mst::TextEntry GetSubtitle(std::string const& name, std::string const& id);
	static std::string GetLanguagePrefix();
	static mst const& GetMst(std::string const& name);

private:
	static std::mutex m_requestMutex;
	static std::set<std::string> m_mstRequested;

	static std::shared_mutex m_collectionMutex;
	static std::map<std::string, mst> m_mstCollection;
};

