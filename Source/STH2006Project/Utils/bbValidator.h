/*----------------------------------------------------------*/
///	Author: brianuuuSonic https://github.com/brianuuu
///	Year: 2026
///	Description: Validate bbX.ini if it has missing archive
/*----------------------------------------------------------*/

#pragma once
class bbValidator
{
public:
	static void validate();

private:
	static void validateBB(std::string const& path, std::string const& fileName);
};

