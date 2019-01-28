#include "file/yml.h"

#include <iostream>
#include <fstream>

namespace YAML
{
    void operator>>(const YAML::Node &node, QString &q)
    {
        std::string sstr;
        sstr = node.as<std::string>();
        q    = QString(sstr.c_str());
    }

    void operator<<(YAML::Node &node, const QString &q)
    {
        std::string sstr = q.toStdString();
        node             = sstr;
    }

    void operator>>(const YAML::Node &node, QStringList &v)
    {
        for (const auto &i : node) {
            v.append(QString::fromStdString(i.as<std::string>()));
        }
    }

    const QRegExp _yamlScalarTrueValues  = QRegExp("true|True|TRUE|on|On|ON");
    const QRegExp _yamlScalarFalseValues = QRegExp("false|False|FALSE|off|Off|OFF");

    QVariant yamlToVariant(const YAML::Node &node)
    {
        switch (node.Type()) {
        case YAML::NodeType::Scalar:
            return yamlScalarToVariant(node);
        case YAML::NodeType::Sequence:
            return yamlSequenceToVariant(node);
        case YAML::NodeType::Map:
            return yamlMapToVariant(node);
        case YAML::NodeType::Null:
        case YAML::NodeType::Undefined:
            return QVariant();
        }

        // fallback for undefined node type
        return QVariant();
    }

    QVariant yamlScalarToVariant(const YAML::Node &scalarNode)
    {
        std::string stdScalar = scalarNode.as<std::string>();
        QString scalarString  = QString::fromStdString(stdScalar);
        if (_yamlScalarTrueValues.exactMatch(scalarString))
            return QVariant(true);
        if (_yamlScalarFalseValues.exactMatch(scalarString))
            return QVariant(false);
        if (QRegExp("[-+]?\\d+").exactMatch(scalarString))
            return QVariant(scalarString.toInt());
        if (QRegExp(R"([-+]?\d*\.?\d+)").exactMatch(scalarString))
            return QVariant(scalarString.toDouble());
        return QVariant(scalarString);
    }

    QVariant yamlSequenceToVariant(const YAML::Node &sequenceNode)
    {
        QVariantList vl;
        for (YAML::const_iterator it = sequenceNode.begin(); it != sequenceNode.end(); ++it) {
            vl << yamlToVariant(*it);
        }
        return vl;
    }

    QVariant yamlMapToVariant(const YAML::Node &mapNode)
    {
        QVariantMap vm;
        for (YAML::const_iterator it = mapNode.begin(); it != mapNode.end(); ++it) {
            vm.insert(QString::fromStdString(it->first.as<std::string>()), yamlToVariant(it->second));
        }
        return vm;
    }
} // namespace YAML

namespace File
{
    YAML::Node Yml::load(const QString &fileName) { return YAML::LoadFile(fileName.toStdString()); }

    std::string toYaml(YAML::Node const &node)
    {
        YAML::Emitter emitter;
        emitter.SetIndent(4);

        // toYaml(node, emitter);
        emitter << node;

        return emitter.c_str();
    }

    bool Yml::saveConfig(const QString &filename, const YAML::Node &node)
    {
        QString configHeaderString = QStringLiteral(
            "#\n"
            "# WPN-XM Server Stack\n"
            "#\n"
            "# MongoDB Configuration File\n"
            "#\n"
            "# MongoDB : File Based Configuration\n"
            "# http://api.mongodb.org/wiki/current/File%20Based%20Configuration.html\n"
            "#\n"
            "# MongoD - Configuration Options\n"
            "# https://docs.mongodb.com/manual/reference/configuration-options/\n"
            "#\n");

        std::ofstream fileout(filename.toLatin1());
        fileout << configHeaderString.toStdString() << node;

        return true;
    }

    /*bool Yml::save(const QString &filename, YAML::Node node)
    {

        //qDebug() << YAML::yamlToVariant(node).toMap();

        QFile file(filename);

        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        QTextStream out(&file);
        out << node; // this doesn't work

        file.close();

        return true;
    }*/
} // namespace File
