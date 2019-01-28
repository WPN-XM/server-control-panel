#ifndef WINDOWSAPI_H
#define WINDOWSAPI_H

// this is needed for "createShellLink"
#include <objbase.h>
#include <ShlObj.h> // type defintion for IShellLink
#include <Windows.h>
//#include <Shobjidl.h>

#include <QtCore>

namespace WindowsAPI
{
    class QtWin : public QObject
    {
        Q_OBJECT

    public:
        static IShellLink *CreateShellLink(const QString &target_app_path,
                                           const QString &app_args,
                                           const QString &description,
                                           const QString &icon_path,
                                           int icon_index,
                                           const QString &working_dir,
                                           const QString &linkShortcut);

        static BOOL IsWow64();
        static bool running_on_64_bits_os();
    };
} // namespace WindowsAPI

#endif // WINDOWSAPI_H
