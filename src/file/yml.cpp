#include "src/file/yml.h"

namespace File
{
   YAML::Node Yml::load(const QString &fileName)
   {
       return YAML::LoadFile(fileName.toStdString());
   }

   bool Yml::save(const QString &filename, YAML::Node data)
   {
       QFile file(filename);
       if (!file.open(QIODevice::WriteOnly)) {
           return false;
       }

       QTextStream out(&file);
       out.setCodec("UTF-8");
       out.autoDetectUnicode();

       out << data;

       file.close();

       return true;
   }
}
