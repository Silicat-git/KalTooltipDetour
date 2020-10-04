#include <Windows.h>
#include <string>
#include <vector>
#include <regex>
#include "detours.h"
#pragma comment(lib, "detours.lib")


DWORD iAddressOfEquipTooltipCreator = 0x0069EDF0;
int const m_nMaxLines = 45;
std::vector<std::string> m_vRegexPattern;

typedef void(*func)(void* iAddressOfString, int32_t a2, int32_t a3);

int countSubstring(const std::string& strString, const std::string& strSub)
{
	if (strSub.length() == 0) return 0;
	int nCount = 0;
	for (size_t s_tOffset = strString.find(strSub); s_tOffset != std::string::npos;
		s_tOffset = strString.find(strSub, s_tOffset + strSub.length()))
	{
		++nCount;
	}
	return nCount;
}

std::string replaceString(std::string const strSource,std::string const strPattern)
{
	std::string strResult = strSource;
	std::regex re(".*" + strPattern + ".*");
	std::smatch matches;
	std::regex_match(strSource, matches, re);

	for (auto i = 1U; i < matches.size(); i++)
	{
		strResult.replace(strResult.find(matches[i].str()), matches[i].str().size(), "");
	}
	
	return strResult;
}

void EquipTooltipString(void* iAddressOfString, int32_t a2, int32_t a3)
{
	char* chOriginalString = (char*)iAddressOfString;
	std::string strTooltip((char*)iAddressOfString);
	std::string strLineTerminator = "#n";

	if (countSubstring(strTooltip, strLineTerminator) > m_nMaxLines)
	{
		for (auto regexPattern : m_vRegexPattern)
		{
			strTooltip = replaceString(strTooltip,regexPattern);
			if (countSubstring(strTooltip, strLineTerminator) < m_nMaxLines)
			{
				break;
			}
		}
		if (countSubstring(strTooltip, strLineTerminator) > m_nMaxLines) //Not a Weapon huh
		{
			while(countSubstring(strTooltip, strLineTerminator) > m_nMaxLines) //Possible Loop Trap
			{
				strTooltip = replaceString(strTooltip, m_vRegexPattern.back());
			}
		}

		const char* chReplaceString = strTooltip.c_str();
		strcpy_s(chOriginalString, strlen(chOriginalString), chReplaceString);
	}

	func originals0 = (func)iAddressOfEquipTooltipCreator;
	originals0(iAddressOfString,a2,a3);
	return;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		m_vRegexPattern.push_back("Power of .+?#n(.+?)#1");					//Moon Sun Orb enchantment
		m_vRegexPattern.push_back("Explosive Blow .+?#n(.+?)#8#n");			//Explosive Blow
		m_vRegexPattern.push_back("#n(.+)#n");								//Last Line - Has to be last for it to work!

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(LPVOID&)iAddressOfEquipTooltipCreator, &EquipTooltipString);

		DetourTransactionCommit();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourDetach(&(LPVOID&)iAddressOfEquipTooltipCreator, &EquipTooltipString);

		DetourTransactionCommit();
	}
	return TRUE;
}