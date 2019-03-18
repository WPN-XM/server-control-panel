
#include <QDebug>
#include "HelloWorldPlugin.h"

// HelloWorldPlugin::~HelloWorldPlugin() {}

void HelloWorldPlugin::init(InitState state)
{
    if (state == InitState::StartupInitState) {
        qDebug() << "HelloWorld StartupInitState";
    }
    if (state == InitState::LateInitState) {
        qDebug() << "HelloWorld LateInitState";
    }
}

void HelloWorldPlugin::unload() { qDebug() << __FUNCTION__ << "called"; }

QString HelloWorldPlugin::getName() const { return "Hello World from Plugin!"; }

// QWidget HelloWorldPlugin::getConfigPage() { return new HelloWorldConfigForm(); }

QString HelloWorldPlugin::getConfigDialogTreeMenuEntry() { return "HelloWorld"; }

void HelloWorldPlugin::showSettings(QWidget *parent) { Q_UNUSED(parent) }

/*bool HelloWorldPlugin::saveSettings(Settings::SettingsManager *settings)
{
    m_settings = settings;
    return true;
}

bool HelloWorldPlugin::loadDefaultSettings(Settings::SettingsManager *settings)
{
    m_settings = settings;
    return true;
}*/
