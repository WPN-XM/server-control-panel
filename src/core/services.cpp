#include "services.h"

/**
 * @brief Services::Services
 *
 * Windows Service Control Manager
 * API functions to check the status of a windows service and modify Windows
 * Services.
 */
Services::Services() = default;

void Services::status(QString serviceName) { Q_UNUSED(serviceName); }

void Services::available(QString serviceName) { Q_UNUSED(serviceName); }

/**
 * @brief Services::install
 * @param serviceName
 *
 * Install a Windows Service
 */
void Services::install(QString serviceName) { Q_UNUSED(serviceName); }

/**
 * @brief Services::remove
 * @param serviceName
 *
 * Remove a Windows Service
 */
void Services::remove(QString serviceName) { Q_UNUSED(serviceName); }

/**
 * @brief Services::reinstall
 * @param serviceName
 *
 * Reinstall Windows Service
 * equivalent to remove() followed by install()
 */
void Services::reinstall(const QString &serviceName)
{
    remove(serviceName);
    install(serviceName);
}
