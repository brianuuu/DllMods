#include "MstManager.h"

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
	m_mstCollection[name].Load(fullPath, errorMsg);

	return true;
}

mst::TextEntry MstManager::GetSubtitle
(
	std::string const& name, 
	std::string const& id
)
{
	if (!IsRequested(name))
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
