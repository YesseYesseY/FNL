#include <print>
#include <Windows.h>
#include <stdlib.h>

#define LAUNCH_ARGS (char*)(" -nosplash -skippatchcheck -epicportal -log -epicapp=Fortnite -epicenv=Prod -epiclocale=en-us -nobe -fromfl=eac -fltoken=7a848a93a74ba68876c36C1c -caldera=TODO.TODO.TODO -AUTH_LOGIN=Yes -AUTH_PASSWORD=s -AUTH_TYPE=epic")

PROCESS_INFORMATION SimpleCreateProcess(const std::string& Path, DWORD CreationFlags = CREATE_SUSPENDED)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcessA(
            Path.c_str(), // lpApplicationName
            LAUNCH_ARGS, // lpCommandLine
            NULL, // lpProcessAttributes
            NULL, // lpThreadAttributes
            FALSE, // bInheritHandles
            CreationFlags, // dwCreationFlags
            NULL, // lpEnvironment
            NULL, // lpCurrentDirectory
            &si, // lpStartupInfo
            &pi // lpProcessInformation
            );

    return pi;
}

void InjectDll(PROCESS_INFORMATION pi, char* DllPath)
{
    if (DllPath[0] == '\x00')
        return;
    auto a1 = GetModuleHandleA("kernel32.dll");
    auto a2 = GetProcAddress(a1, "LoadLibraryA");
    auto a3 = VirtualAllocEx(pi.hProcess, NULL, strlen(DllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    auto a4 = WriteProcessMemory(pi.hProcess, a3, DllPath, strlen(DllPath) + 1, NULL);
    auto a5 = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)a2, a3, 0, NULL);
    CloseHandle(a5);
}

int main(int argc, char** argv)
{
    if (argc <= 2)
    {
        std::println("Usage: {} [PathToFolderThatIncludesFortniteGame] (Actions)\nActions:\n    -wNumberToWait // Wait for x milliseconds\n    -iPathToDll // Inject dll", argv[0]);
        return 1;
    }

    std::string Path = std::string(argv[1]);

    if (!Path.ends_with('/') || !Path.ends_with('\\'))
        Path += '/';

    SimpleCreateProcess(Path + "FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping_EAC.exe");
    SimpleCreateProcess(Path + "FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping_BE.exe");
    SimpleCreateProcess(Path + "FortniteGame/Binaries/Win64/FortniteLauncher.exe");
    auto pi = SimpleCreateProcess(Path + "FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe", 0);

    for (int i = 2; i < argc; i++)
    {
        switch (argv[i][1])
        {
            case 'w':
            {
                int WaitNum = atoi(&argv[i][2]);
                Sleep(WaitNum);
                break;
            }
            case 'i':
            {
                char* DllPath = &argv[i][2];
                std::println("Injecting {}", DllPath);
                InjectDll(pi, DllPath);
                break;
            }
            default:
            {
                std::println("Unknown Action: {}", argv[i]);
                break;
            }
        }
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
