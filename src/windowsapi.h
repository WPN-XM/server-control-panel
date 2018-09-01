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
        static IShellLink *CreateShellLink(QString target_app_path,
                                           QString app_args,
                                           QString description,
                                           QString icon_path,
                                           int icon_index,
                                           QString working_dir,
                                           QString linkShortcut);

        static BOOL IsWow64();
        static bool running_on_64_bits_os();
    };
} // namespace WindowsAPI

#endif // WINDOWSAPI_H
