
#include <QDebug>
#include "HelloWorldPlugin.h"

namespace Plugin_HelloWorld_NS
{
    Plugin_HelloWorld::Plugin_HelloWorld() : QObject(){};

    Plugin_HelloWorld::~Plugin_HelloWorld() = default;

    void Plugin_HelloWorld::init(InitState state)
    {
        if (state == InitState::StartupInitState) {
            qDebug() << "[Plugin: HelloWorld] StartupInitState";
        }
        if (state == InitState::LateInitState) {
            qDebug() << "[Plugin: HelloWorld] LateInitState";
        }
    }

    void Plugin_HelloWorld::unload()
    {
        // unload
        qDebug() << __FUNCTION__ << "called";
    }

    QString Plugin_HelloWorld::getName() const
    {
        // the name
        return "Hello World from Plugin!";
    }

    // register page with addWidgetToStack()
    QWidget *Plugin_HelloWorld::getConfigDialog()
    {
        // returns the config dialog page
        return new Plugin_HelloWorld_ConfigDialog();
    }

    // register menu item with addItemToTreeMenu()
    QString Plugin_HelloWorld::getConfigDialogTreeMenuEntry()
    {
        // return the name of the menu item for the config tree menu
        return "HelloWorld";
    }

    /*
     * Shows the Settings Dialog for this plugin,
     * when the Button "Settings" is clicked in the "Configuration > Plugins" dialog.
     */
    void Plugin_HelloWorld::showSettings(QWidget *parent)
    {
        if (!settings) {
            settings = new QDialog(parent);
            settings.data()->setWindowFlags(settings.data()->windowFlags() & ~Qt::WindowContextHelpButtonHint);

            QPushButton *b           = new QPushButton("Hello World Plugin v1.0.0");
            QPushButton *closeButton = new QPushButton(tr("Close"));
            /*QLabel *label            = new QLabel();
            label->setPixmap(QPixmap(":icons/other/about.svg"));*/

            auto *boxLayout = new QVBoxLayout(settings.data());
            // boxLayout->addWidget(label);
            boxLayout->addWidget(b);
            boxLayout->addWidget(closeButton);
            settings.data()->setLayout(boxLayout);

            settings.data()->setMinimumSize(200, 200);
            settings.data()->setAttribute(Qt::WA_DeleteOnClose);
            settings.data()->setWindowTitle(tr("Hello World Plugin Settings"));
            // settings.data()->setWindowIcon(QIcon(":icons/falkon.svg"));
            connect(closeButton, SIGNAL(clicked()), settings.data(), SLOT(close()));
        }

        settings.data()->show();
        settings.data()->raise();
    }

    /*bool Plugin_HelloWorld::saveSettings(Settings::SettingsManager *settings)
    {
        m_settings = settings;
        return true;
    }

    bool Plugin_HelloWorldn::loadDefaultSettings(Settings::SettingsManager *settings)
    {
        m_settings = settings;
        return true;
    }*/

} // namespace Plugin_HelloWorld_NS
