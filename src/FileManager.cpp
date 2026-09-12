#include "FileManager.h"

bool bpl::FileManager::exists(const String &path) const
{
    if (!validPath(path)) {
        return false;
    }

    return fs_.exists(path);
}


bool bpl::FileManager::isFile(const String &path) const
{
    if (!validPath(path)) {
        return false;
    }

    File file = fs_.open(path, "r");
    if (!file) {
        return false;
    }

    const bool result = !file.isDirectory();
    file.close();

    return result;
}


bool bpl::FileManager::isDirectory(const String &path) const
{
    if (!validPath(path)) {
        return false;
    }

    File file = fs_.open(path, "r");
    if (!file) {
        return false;
    }

    const bool result = file.isDirectory();
    file.close();

    return result;
}

bool bpl::FileManager::listFiles(const String &path, JsonDocument &output)
{
    output.clear();

    if (!validPath(path) || isDirectory(path)) {
        return false;
    }

    File directory = fs_.open(path, "r");
    if (!directory) {
        return false;
    }

    if (!directory.isDirectory()) {
        directory.close();
        return false;
    }

    File entry = directory.openNextFile();
    while (entry) {
        JsonObject item = output.add<JsonObject>();

        item["type"] = entry.isDirectory() ? "dir" : "file";
        item["name"] = entry.name();

        entry.close();
        entry.openNextFile();
    }

    directory.close();
    return true;
}


bool bpl::FileManager::validPath(const String &path) const
{
    return true;
}
