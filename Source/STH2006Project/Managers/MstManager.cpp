#include "MstManager.h"

#include "Configuration.h"
#include "System/Application.h"

std::mutex MstManager::m_requestMutex;
std::set<std::string> MstManager::m_mstRequested;

std::shared_mutex MstManager::m_collectionMutex;
std::map<std::string, mst> MstManager::m_mstCollection;

bool MstManager::IsRequested
(
	std::string const& name
)
{
	std::unique_lock lock(m_requestMutex);
	return m_mstRequested.count(name);
}

bool MstManager::RequestMst
(
	std::string const& name
)
{
	// load PS3 file instead
	if (Configuration::m_buttonType == Configuration::ButtonType::BT_PS3 && name == "msg_hint_xenon")
	{
		return RequestMst("msg_hint_ps3");
	}

	// check or fill m_mstRequested
	{
		std::scoped_lock lock(m_requestMutex);
		if (m_mstRequested.count(name))
		{
			return true;
		}

		m_mstRequested.insert(name);
	}

	std::unique_lock lock(m_collectionMutex);

	std::string const fullPath = Application::getModDirString() + "Assets\\Message\\" + GetLanguagePrefix() + "\\" + name + ".mst";
	std::string errorMsg;
	return m_mstCollection[name].Load(fullPath, errorMsg);
}

mst::TextEntry MstManager::GetSubtitle
(
	std::string const& name, 
	std::string const& id
)
{
	// load PS3 entry instead
	if (Configuration::m_buttonType == Configuration::ButtonType::BT_PS3 && name == "msg_hint_xenon")
	{
		return GetSubtitle("msg_hint_ps3", id);
	}

	if (!RequestMst(name))
	{
		return mst::TextEntry();
	}

	std::shared_lock lock(m_collectionMutex);
	mst const& mstData = m_mstCollection.at(name);
	return mstData.GetEntry(id);
}

std::string MstManager::GetLanguagePrefix()
{
	switch (Common::GetUILanguageType())
	{
	case LT_English: return "English";
	case LT_Japanese: return "Japanese";
	//case LT_French: return "French";
	//case LT_German: return "German";
	//case LT_Spanish: return "Spanish";
	//case LT_Italian: return "Italian";
	}
	return "English";
}

mst const& MstManager::GetMst
(
	std::string const& name
)
{
	std::shared_lock lock(m_collectionMutex);
	return m_mstCollection.at(name);
}
