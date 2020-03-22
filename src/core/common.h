#ifndef COMMON_H
#define COMMON_H

//------------------------------------------------------------------------------
// EXPORT Library
//------------------------------------------------------------------------------

#ifdef APP_CORE_SHAREDLIBRARY
#define APP_CORE_EXPORT Q_DECL_EXPORT
#else
#define APP_CORE_EXPORT Q_DECL_IMPORT
#endif

//------------------------------------------------------------------------------
// Shortcuts
//------------------------------------------------------------------------------

#ifndef QSL
#define QSL(x) QStringLiteral(x)
#endif

#ifndef QL1S
#define QL1S(x) QLatin1String(x)
#endif

#ifndef QL1C
#define QL1C(x) QLatin1Char(x)
#endif

#endif // COMMON_H
