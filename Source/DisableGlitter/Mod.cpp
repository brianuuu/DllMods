extern "C" void __declspec(dllexport) OnFrame()
{

}

std::set<std::string> disableList;
HOOK(uint32_t*, __fastcall, CreateParticleEffect, 0x6B39A0, void* This, void* Edx, void* a2, hh::base::CSharedString const& name, void* a4, int a5)
{
	for (std::string const& n : disableList)
	{
		if (std::strstr(name.c_str(), n.c_str()) != nullptr)
		{
			return originalCreateParticleEffect(This, Edx, a2, "", a4, a5);
		}
	}

	return originalCreateParticleEffect(This, Edx, a2, name, a4, a5);
}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{
	char const* listFile = "DisableList.txt";
	if (Common::IsFileExist(listFile))
	{
		std::string content;
		std::ifstream in(listFile);
		if (in)
		{
			std::string line;
			while (getline(in, line))
			{
				if (!line.empty() && line.find("//", 0) != 0)
				{
					disableList.insert(line);
				}
			}
			in.close();
		}
	}

	if (!disableList.empty())
	{
		INSTALL_HOOK(CreateParticleEffect);
	}
}