#ifndef YML_H
#define YML_H

#include <QFile>
#include <QTextStream>
#include "yaml-cpp/yaml.h"

namespace File
{
    class Yml
    {
        public:
           YAML::Node load(const QString &fileName);
           bool save(const QString &filename, YAML::Node data);
    };
}

#endif // YML_H
