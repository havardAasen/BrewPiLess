#ifndef BREWPILESS_FILEMANAGER_H
#define BREWPILESS_FILEMANAGER_H

#include <ArduinoJson.h>
#include <FS.h>

namespace bpl {
    class FileManager {
    public:
        explicit FileManager(fs::FS &filesystem);

        bool exists(const String &path) const;
        bool isFile(const String &path) const;
        bool isDirectory(const String &path) const;

        bool listFiles(const String &path, JsonDocument &result);
        bool readFile(const String &path, Stream &output);
        bool writeFile(const String &path, const Stream &input, std::size_t length);
        bool deleteFile(const String &path);
        bool rename(const String &oldPath, const String &newPath);

    private:
        fs::FS &fs_;

        bool validPath(const String &path) const;
    };

}

#endif
