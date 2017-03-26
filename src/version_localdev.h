#ifndef VERSION_H_
#define VERSION_H_

// Application version.
// Its a token, replaced during the build process.
#define VERSION = 0.8.5 + githash
#define APP_VERSION "0.8.5+githash"
#define APP_VERSION_SHORT "0.8.5"

// Application name
#define APP_NAME "WPИ-XM Server Control Panel"
#define APP_NAME_AND_VERSION "WPИ-XM Server Control Panel @APPVERSIONSHORT@"

// Settings for application_win.rc
// These values are the attributes of the executable, shown when right-clicking
// on it.
#define VER_FILEVERSION 0, 0, 0, 1
#define VER_FILEVERSION_STR "0,0,0,1\0"

#define VER_PRODUCTVERSION VER_FILEVERSION
#define VER_PRODUCTVERSION_STR VER_FILEVERSION_STR

#define VER_COMPANYNAME_STR "Koch Softwaresystemtechnik"
#define VER_COMPANYDOMAIN_STR "http://wpn-xm.org"
#define VER_FILEDESCRIPTION_STR "WPN-XM Server Control Panel"
#define VER_INTERNALNAME_STR "WPN-XM SCP"
#define VER_LEGALCOPYRIGHT_STR "Copyright (c) 2015 Koch Softwaresystemtechnik"
#define VER_LEGALTRADEMARKS1_STR "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR "wpn-xm.exe"
#define VER_PRODUCTNAME_STR "WPN-XM Server Control Panel"

#endif /* VERSION_H_ */
