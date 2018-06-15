#ifndef YML_H
#define YML_H

#include <QFile>
#include <QTextStream>
#include "yaml-cpp/yaml.h"

namespace YAML
{
    QVariant yamlToVariant(const YAML::Node &node);
    QVariant yamlScalarToVariant(const YAML::Node &scalarNode);
    QVariant yamlSequenceToVariant(const YAML::Node &sequenceNode);
    QVariant yamlMapToVariant(const YAML::Node &mapNode);
}

namespace File
{
    class Yml
    {
    public:
        YAML::Node load(const QString &fileName);
        bool save(const QString &filename, YAML::Node node);
        bool saveConfig(const QString &filename, YAML::Node node);
    };
}

#endif // YML_H
