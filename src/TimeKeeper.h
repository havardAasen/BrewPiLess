#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <cstdint>
#include <ctime>

class TimeKeeperClass {
public:
    void begin();

    void begin(const char *, const char *, const char *);

    void updateTime();

    std::time_t getTimeSeconds(); // get Epoch time
    std::time_t getLocalTimeSeconds();

    const char *getDateTimeStr();

    void setCurrentTime(std::time_t);

    static void setTimezoneOffset(std::int32_t);

    static std::int32_t getTimezoneOffset();

    [[nodiscard]] bool isSynchronized() const;

private:
    std::time_t reference_epoc_{};
    std::time_t reference_system_time_{};
    std::time_t last_saved_{};
    bool ntp_synced_{};
    inline static char date_time_str_buff_[128];

    static constexpr std::int16_t time_saving_period = 3600;
    static constexpr std::uint32_t resync_time = 43200000;

    /** Time gap in seconds from 01.01.1900 (NTP time) to 01.01.1970 (UNIX time). */
    static constexpr std::uint32_t diff_1900_to_1970 = 2208988800;

    static void saveTime(std::time_t);

    static std::time_t loadTime();

    std::time_t _queryServer();
};

extern TimeKeeperClass TimeKeeper;

#endif
