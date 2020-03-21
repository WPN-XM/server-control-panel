#ifndef YML_H
#define YML_H

#include "common.h"

#include <QVariant>
#include <QStringList>
#include <QRegExp>

#include "yaml-cpp/yaml.h"

namespace YAML
{
    template <>
    struct convert<QString>
    {
        static Node encode(const QString &rhs)
        {
            Node node;
            node = rhs.toStdString();
            return node;
        }

        static bool decode(const Node &node, QString &rhs)
        {
            if (!node.IsScalar()) {
                return false;
            }

            auto string = node.as<std::string>();
            rhs         = QString(string.c_str());

            return true;
        }
    };

    QVariant yamlToVariant(const YAML::Node &node);
    QVariant yamlScalarToVariant(const YAML::Node &scalarNode);
    QVariant yamlSequenceToVariant(const YAML::Node &sequenceNode);
    QVariant yamlMapToVariant(const YAML::Node &mapNode);
} // namespace YAML

namespace File
{
    class APP_CORE_EXPORT Yml
    {
    public:
        YAML::Node load(const QString &fileName);
        // bool save(const QString &filename, YAML::Node node);
        bool saveConfig(const QString &filename, const YAML::Node &node);
    };
} // namespace File

#endif // YML_H
