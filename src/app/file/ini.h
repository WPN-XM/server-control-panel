#ifndef INI_H
#define INI_H

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace File
{

    using namespace std;

    struct INIEntry
    {
        INIEntry() : isComment(false), isEmptyLine(false) {}
        string index;
        string name;
        string value;
        string comment;
        bool isComment;
        bool isEmptyLine;
    };

    /**
     * INI - A reader and writer for the INI configuration file format.
     *
     * QSettings with QSettings::IniFormat doesn't work with comments.
     * Not supporting INI comments (starting with ; or #) is stupid.
     *
     * // Writer
     *
     * INI *ini = new INI("test.ini");
     *
     * ini->setIntValue("section1", "intValue", 1);
     * ini->setFloatValue("section1", "floatValue", 0.1);
     * ini->setStringValue("section2", "stringValue", "hello, world");
     * ini->setBoolValue("section2", "boolValue", true);
     *
     * ini->writeConfigFile();
     *
     * // Reader
     *
     * int intValue = ini->getIntValue("section1", "intValue");
     * float floatValue = ini->getFloatValue("section1", "floatValue");
     * const char *stringValue = ini->getStringValue("section2", "stringValue");
     * bool boolValue = ini->getBoolValue("section2", "boolValue");
     */
    class INI
    {
    public:
        INI(const char *fileName);
        ~INI();

        void writeConfigFile(const char *fileName = NULL);

        // getter
        bool getBoolValue(const char *index, const char *name);
        int getIntValue(const char *index, const char *name);
        float getFloatValue(const char *index, const char *name);
        const char *getStringValue(const char *index, const char *name);
        // setter
        void setBoolValue(const char *index, const char *name, bool value);
        void setIntValue(const char *index, const char *name, int value);
        void setFloatValue(const char *index, const char *name, float value);
        void setStringValue(const char *index, const char *name, const char *value);

        // debug
        void debug();

        // get all Entry
        vector<INIEntry> datas;

    private:
        string trim(string &str);
        char str[4096]; // for temporary string data
        void setStringValueWithIndex(const char *index, const char *name, const char *value);
        char iniFileName[4096];
        char *data;
        const char (*lineData)[4096];
        void loadConfigFile();
        fstream *fStream;
        bool autoSave;
    };
}; // namespace File

#endif // INI_H
