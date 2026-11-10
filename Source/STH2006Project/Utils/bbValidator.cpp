#include "bbValidator.h"

void bbValidator::validate()
{
	//MessageBox(nullptr, TEXT("Attach debugger"), TEXT("STH2006 Project"), MB_ICONERROR);
	validateBB("core\\", "bb.ini");
	validateBB("core\\", "bb2.ini");
	validateBB("core\\", "bb3.ini");
}

void bbValidator::validateBB(std::string const& path, std::string const& fileName)
{
	std::vector<std::string> lines;
	{
		std::ifstream file(path + fileName);
		if (!file.is_open())
		{
			return;
		}

		std::string temp;
		while (std::getline(file, temp)) 
		{
			lines.push_back(temp);
		}

		file.close();
	}

	int largestPackedNumber = -1;
	bool removedStaleAR = false;
	std::string prevLine;
	std::string prevExtension;

	std::ofstream file(path + fileName);
	for (std::string const& line : lines)
	{
		size_t const split = line.find('=');
		size_t const lastDot = line.find_last_of('.');
		if (split != std::string::npos && lastDot != std::string::npos)
		{
			std::string left = line.substr(0, split);
			left.erase(std::remove(left.begin(), left.end(), '\t'), left.end());
			left.erase(std::remove(left.begin(), left.end(), ' '), left.end());

			std::string right = line.substr(split + 1);
			right.erase(std::remove(right.begin(), right.end(), '\t'), right.end());
			right.erase(std::remove(right.begin(), right.end(), ' '), right.end());

			std::string extension = line.substr(lastDot + 1);
			if (extension == "00")
			{
				largestPackedNumber = -1;
				removedStaleAR = false;
			}

			if (extension == "arl")
			{
				if (removedStaleAR)
				{
					if (largestPackedNumber == -1)
					{
						// no .ar.XX exist, remove .arl
						continue;
					}
				}
				else
				{
					while (true)
					{
						++largestPackedNumber;
						std::string const newExtension = (largestPackedNumber < 10 ? "ar.0" : "ar.") + std::to_string(largestPackedNumber);
						std::string const newAR = std::regex_replace(right, std::regex(extension), newExtension);
						if (Common::IsFileExist(path + newAR))
						{
							// add new .ar.XX
							std::string const newLine = std::regex_replace(prevLine, std::regex("ar." + prevExtension), newExtension);
							file << newLine << std::endl; \
						}
						else
						{
							break;
						}
					}
				}
			}
			else
			{
				bool success = true;
				int packedNumber = 0;
				try { packedNumber = std::stoi(extension); }
				catch (std::exception& err) { success = false; }

				if (success)
				{
					if (!Common::IsFileExist(path + right))
					{
						// remove stale .ar.XX
						removedStaleAR = true;
						continue;
					}
					else
					{
						largestPackedNumber = packedNumber;
					}
				}
			}

			prevExtension = extension;
		}

		prevLine = line;
		file << line << std::endl;
	}
}
