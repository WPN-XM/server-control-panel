#include "QApplication"
#include "QLibrary"

#include "windowsapi.h"

/*
 * Some access methods for Windows libraries
 */
namespace WindowsAPI
{
    /*
     * http://msdn.microsoft.com/en-us/library/windows/desktop/bb776891%28v=vs.85%29.aspx
     */
    IShellLink *QtWin::CreateShellLink(QString target_app_path,
                                       QString app_args,
                                       QString description,
                                       QString icon_path,
                                       int icon_index,
                                       QString working_dir,
                                       QString linkShortcut)
    {
        IShellLink *shell_link = nullptr;

        HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink,
                                        reinterpret_cast<void **>(&(shell_link)));

        if (SUCCEEDED(hres)) {

            IPersistFile *persistFile = NULL;

            shell_link->SetPath(target_app_path.toStdWString().c_str());
            shell_link->SetArguments(app_args.toStdWString().c_str());
            shell_link->SetIconLocation(icon_path.toStdWString().c_str(), icon_index);
            shell_link->SetDescription(description.toStdWString().c_str());
            shell_link->SetWorkingDirectory(working_dir.toStdWString().c_str());

            // Query IShellLink for the IPersistFile interface,
            // used for saving the shortcut in persistent storage.
            hres = shell_link->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&(persistFile)));

            if (SUCCEEDED(hres)) {

                // Save the link by calling IPersistFile::Save.
                hres = persistFile->Save((LPCOLESTR)linkShortcut.toStdWString().c_str(), STGM_WRITE);

                // Release the pointer to the IPersistFile interface.
                persistFile->Release();
            }

            // Release the pointer to the IShellLink interface.
            shell_link->Release();
        }

        return shell_link;
    }

    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

    BOOL QtWin::IsWow64()
    {
        BOOL bIsWow64                        = FALSE;
        LPFN_ISWOW64PROCESS fnIsWow64Process = nullptr;

        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

        if (nullptr != fnIsWow64Process) {
            if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
                // handle error
            }
        }
        return bIsWow64;
    }

    bool QtWin::running_on_64_bits_os()
    {
#if defined(_M_X64) || defined(x86_64)
        return true;
#else
        return (IsWow64() == TRUE);
#endif
    }

    /*QString QtWin::getProcessPathByPid(QString pid)
{
    // get process handle
    DWORD pidwin = pid.toLongLong(); // dword = unsigned long
    GetWindowThreadProcessId(foregroundWindow, &pidwin);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
FALSE, pidwin);

    // GetModuleFileNameEx(hProcess, 0, PChar(result), Length(result)) > 0 then

    // get process path
    WCHAR szProcessPath[MAX_PATH];
    DWORD bufSize = MAX_PATH;
    QueryFullProcessImageName pQueryFullProcessImageName = NULL;
    pQueryFullProcessImageName = (QueryFullProcessImageName)
QLibrary::resolve("kernel32", "QueryFullProcessImageNameW");
    QString processPath;
    if(pQueryFullProcessImageName != NULL) {
        pQueryFullProcessImageName(hProcess, 0, (LPWSTR) &szProcessPath,
&bufSize);
        processPath = QString::fromUtf16((ushort*)szProcessPath, bufSize);
    }

    return processPath;
}*/
} // namespace WindowsAPI
