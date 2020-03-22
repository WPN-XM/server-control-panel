
#include "ini.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include <QString>

namespace File
{

//#define log qDebug
#define log

    INI::INI(const char *fileName) : autoSave(false)
    {
        strcpy(iniFileName, fileName);
        loadConfigFile();
    }

    void INI::loadConfigFile()
    {
        std::fstream fStream;
        std::string p = iniFileName;

        fStream.open(p, std::ios::in);
        if (!fStream) {
            /*if (!autoCreate) {
                log("[INI] config file [%s] does not exist", iniFileName);
            } else {
                log("[INI] config file [%s] not found. Creating new file (auto-create "
                    "on)",
                    iniFileName);
            }*/
            return;
        } else {
            // log("[INI] Reading config file [%s]", iniFileName);
        }

        char line[4096];
        char ch;
        int i = 0;
        std::string index;
        std::string str;
        bool isComment = false;

        while (!fStream.eof()) {
            fStream.read(&ch, 1);

            INIEntry entry;

            // handle empty lines
            if (!isComment && ch == '\n' && i == 0) {
                // entry.index = str;
                entry.isEmptyLine = true;
                datas.push_back(entry);
                // log("[INI] entry: empty line");
                continue;
            }

            // handle comments
            if ((ch == '#' || ch == ';') && i == 0) {
                isComment = true;
            }

            if (isComment && (ch == '\n' || ch == '\r')) {
                isComment       = false;
                line[i++]       = '\0';
                i               = 0;
                entry.isComment = true;
                entry.comment   = line;
                datas.push_back(entry);
            }

            if (isComment) {
                line[i++] = ch;
                continue;
            }

            // handle data
            if (ch != '\n' || ch == '\r') {
                line[i++] = ch;
            } else {
                if (i == 0)
                    continue;
                line[i] = '\0';
                str     = std::string(line);
                // log("[INI] read line: %s", str.c_str());
                // [section] start
                if (line[0] == '[') {
                    index = str;
                } else {
                    // key value pair
                    entry.index = index.substr(1, index.length() - 2);
                    int fIndex  = static_cast<int>(str.find_first_of('='));
                    entry.name  = str.substr(0, fIndex);
                    entry.value = str.substr(fIndex + 1, str.length() - fIndex - 1);
                    // trim spaces
                    entry.name  = trim(entry.name);
                    entry.value = trim(entry.value);
                    // insert
                    datas.push_back(entry);
                    // log("[INI] entry: section[%s]\t name[%s]\t value[%s]", entry.index.c_str(), entry.name.c_str(),
                    // entry.value.c_str());
                }
                i = 0;
            }
        }
        // handle last entry
        if (i != 0) {
            INIEntry entry;
            entry.index = str;
            int fIndex  = static_cast<int>(str.find_first_of('='));
            entry.name  = str.substr(0, fIndex);
            entry.value = str.substr(fIndex + 1, str.length() - fIndex - 1);
            // trim spaces
            entry.name  = trim(entry.name);
            entry.value = trim(entry.value);
            // insert
            datas.push_back(entry);
            // log("[INI] last add entry: section[%s]\t name[%s]\t value[%s]", entry.index.c_str(), entry.name.c_str(),
            // entry.value.c_str());
        }
        fStream.close();
    }

    INI::~INI()
    {
        if (autoSave) {
            // log("[INI] Deconstructor. AutoSaving config file: [%s]", iniFileName);
            writeConfigFile();
        }
    }

    std::string INI::trim(std::string &str)
    {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

    void INI::writeConfigFile(const char *fileName)
    {
        autoSave = false;
        if (fileName == nullptr)
            fileName = iniFileName;
        std::fstream fStream;
        fStream.open(fileName, std::ios_base::out | std::ios_base::trunc);
        // log("[INI] start writing file[%s]", fileName);
        std::string index = std::string("");
        bool withComment  = false;
        bool isStart      = true;
        for (auto it = datas.begin(); it != datas.end(); it++) {
            INIEntry entry = *it;
            if (entry.isEmptyLine) {
                fStream << "\n";
                // log("[INI] write empty line");
                continue;
            }
            if (entry.isComment) {
                withComment = true;
                if (isStart)
                    fStream << entry.comment.c_str() << std::endl;
                else
                    fStream << entry.comment.c_str() << std::endl;
                isStart = false;

                // log("[INI] write comment: %s", entry.comment.c_str());
                continue;
            }
            if (strlen(entry.name.c_str()) == 0 || strlen(entry.value.c_str()) == 0) {
                // log("[INI] skip invalid entry");
                continue;
            }
            if (strcmp(index.c_str(), entry.index.c_str()) != 0) {
                index = entry.index;
                if (withComment || isStart) {
                    fStream << '[' << entry.index << ']' << std::endl;
                    withComment = false;
                    isStart     = false;
                    // log("[INI] write section [%s]", entry.index.c_str());
                } else {
                    fStream << std::endl << '[' << entry.index << ']' << std::endl;
                    // log("[INI] write section [%s]", entry.index.c_str());
                }
            }
            fStream << entry.name << " = " << entry.value << std::endl;
            // log("[INI] write value | %s = %s", entry.name.c_str(), entry.value.c_str());
        }
        fStream << std::endl;
        fStream.close();
        // log("[INI] Saved config file [%s]. Done.", fileName);
    }

    void INI::setStringValueWithIndex(const char *index, const char *name, const char *value)
    {
        autoSave = true;
        INIEntry entry;
        entry.index = index;
        entry.name  = name;
        entry.value = value;
        if (datas.size() == 0) { /*cout<<"data is NULL, push and return"<<endl; */
            datas.push_back(entry);
            return;
        }
        auto it        = datas.begin();
        bool findIndex = false;
        bool findName  = false;
        std::vector<INIEntry>::iterator itInsertPos;
        for (it = datas.begin(); it != datas.end(); it++) {
            if (!findIndex) {
                if (strcmp(it->index.c_str(), index) == 0) {
                    findIndex = true;
                }
            }
            if (findIndex) {
                if (strcmp(it->index.c_str(), index) != 0) {
                    break;
                } else {
                    itInsertPos = it;
                }
                if (strcmp(it->name.c_str(), name) == 0) {
                    findName    = true;
                    itInsertPos = it;
                    break;
                }
                continue;
            }
            itInsertPos = it;
        }
        if (findIndex && findName) {
            itInsertPos->value = std::string(value);
            return;
        }

        datas.insert(++itInsertPos, 1, entry);
    }

    // getter
    bool INI::getBoolValue(const char *index, const char *name)
    {
        const char *str = getStringValue(index, name);
        if (str == nullptr) {
            return false;
        }
        return (strcmp(str, "true") == 0);
    }

    int INI::getIntValue(const char *index, const char *name)
    {
        const char *str = getStringValue(index, name);
        if (!str) {
            return -1;
        }
        return atoi(str);
    }

    float INI::getFloatValue(const char *index, const char *name)
    {
        const char *str = getStringValue(index, name);
        if (str == nullptr) {
            return -1.0;
        }
        return atof(str);
    }

    const char *INI::getStringValue(const char *index, const char *name)
    {
        // log("find section[%s]-name[%s]", index, name);
        for (unsigned int i = 0; i < datas.size(); i++) {
            if (strcmp(datas[i].index.c_str(), index) == 0) {
                // log("find section[%s]", datas[i].index.c_str());
                for (; i < datas.size(); i++) {
                    if (strcmp(datas[i].name.c_str(), name) == 0)
                        return datas[i].value.c_str();
                }
            }
        }
        std::cout << "DEBUG: [" << index << "] of--[" << name << "] not found" << std::endl;
        return nullptr;
    }

    // setter
    void INI::setBoolValue(const char *index, const char *name, bool value)
    {
        const char *string = (value) ? "true" : "false";

        setStringValueWithIndex(index, name, string);
    }

    void INI::setIntValue(const char *index, const char *name, int value)
    {
        std::stringstream stream;
        stream << static_cast<int>(value);

        setStringValueWithIndex(index, name, stream.str().c_str());
    }

    void INI::setFloatValue(const char *index, const char *name, float value)
    {
        std::stringstream stream;
        stream << static_cast<float>(value);

        setStringValueWithIndex(index, name, stream.str().c_str());
    }

    void INI::setStringValue(const char *index, const char *name, const char *value)
    {
        setStringValueWithIndex(index, name, value);
    }

    // debug
    void INI::debug()
    {
        for (auto it = datas.begin(); it != datas.end(); it++) {
            INIEntry entry = *it;
            // log(" ------------ INI item of [%s] ------------ ", iniFileName);
            if (entry.isComment) {
                std::cout << entry.comment << std::endl;
                continue;
            }
            // log("  index : %s", entry.index.c_str());
            // log("  name  : %s", entry.name.c_str());
            // log("  value : %s", entry.value.c_str());
        }
    }
}; // namespace File