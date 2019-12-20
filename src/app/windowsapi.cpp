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
    IShellLink *QtWin::CreateShellLink(const QString &target_app_path,
                                       const QString &app_args,
                                       const QString &description,
                                       const QString &icon_path,
                                       int icon_index,
                                       const QString &working_dir,
                                       const QString &linkShortcut)
    {
        IShellLink *shell_link = nullptr;

        HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink,
                                        reinterpret_cast<void **>(&(shell_link)));

        if (SUCCEEDED(hres)) {

            IPersistFile *persistFile = nullptr;

            shell_link->SetPath(target_app_path.toStdString().c_str());
            shell_link->SetArguments(app_args.toStdString().c_str());
            shell_link->SetIconLocation(icon_path.toStdString().c_str(), icon_index);
            shell_link->SetDescription(description.toStdString().c_str());
            shell_link->SetWorkingDirectory(working_dir.toStdString().c_str());

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

    bool QtWin::running_on_64bit_os()
    {
#if defined(_M_X64) || defined(x86_64)
        return true;
#else
        return (IsWow64() == TRUE);
#endif
    }

    // initalization of static variables
    HANDLE Console::hConsole;
    WORD Console::oldConsoleAttributes = 0;

    WORD Console::GetConsoleTextAttribute(HANDLE hConsole)
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(hConsole, &info);
        return info.wAttributes;
    }

    const char *Console::printColoredMsg(int prefix, int color, const char *msg)
    {
        if (hConsole == nullptr) {
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        }

        if (!hConsole) {
            return msg;
        }

        oldConsoleAttributes = GetConsoleTextAttribute(hConsole);

        WORD attr = (oldConsoleAttributes & 0x0f0);

        if (prefix)
            attr |= FOREGROUND_INTENSITY;
        if (color == 0)
            attr |= 0; // black
        if (color == 31)
            attr |= FOREGROUND_RED; // red
        if (color == 32)
            attr |= FOREGROUND_GREEN; // green
        if (color == 33)
            attr |= FOREGROUND_GREEN | FOREGROUND_RED; // yellow
        if (color == 34)
            attr |= FOREGROUND_BLUE; // blue
        if (color == 35)
            attr |= FOREGROUND_BLUE | FOREGROUND_RED; // purple/magenta
        if (color == 36)
            attr |= FOREGROUND_BLUE | FOREGROUND_GREEN; // cyan
        if (color == 37)
            attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // white

        SetConsoleTextAttribute(hConsole, attr);

        printf("%s", msg);

        SetConsoleTextAttribute(hConsole, oldConsoleAttributes);
        return "";
    }

} // namespace WindowsAPI
