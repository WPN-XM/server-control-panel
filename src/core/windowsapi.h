#ifndef WINDOWSAPI_H
#define WINDOWSAPI_H

#include "common.h"
#include "version.h"

// this is needed for "createShellLink"
#include <objbase.h>
#include <ShlObj.h> // type definition for IShellLink
#include <Windows.h>

#include <QtCore>

namespace WindowsAPI
{
    class APP_CORE_EXPORT Console
    {
        static HANDLE hConsole;
        static WORD oldConsoleAttributes;
        static WORD GetConsoleTextAttribute(HANDLE hConsole);

    public:
        static const char *printColoredMsg(int prefix, int color, const char *msg);
    };

    class APP_CORE_EXPORT QtWin : public QObject
    {
        Q_OBJECT

        typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

    public:
        static IShellLink *CreateShellLink(const QString &target_app_path,
                                           const QString &app_args,
                                           const QString &description,
                                           const QString &icon_path,
                                           int icon_index,
                                           const QString &working_dir,
                                           const QString &linkShortcut);

        static BOOL IsWow64();
        static bool running_on_64bit_os();
    };

} // namespace WindowsAPI

#endif // WINDOWSAPI_H
