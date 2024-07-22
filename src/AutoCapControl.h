#ifndef AUTO_CAP_CONTROL_H
#define AUTO_CAP_CONTROL_H

#include "Actuator.h"
#include "BPLSettings.h"

#include <cstdint>

enum AutoCapMode {
    AutoCapModeNone,
    AutoCapModeManualOpen,
    AutoCapModeManualClose,
    AutoCapModeTime,
    AutoCapModeGravity
};

enum CapStatus { CapStatusInactive, CapStatusActive, CapStatusUnknown };


class AutoCapControl {
    public:
        void begin();

        [[nodiscard]] bool isCapOn() const;
        static void setPhysicalCapOn(bool on) ;

        bool autoCapOn(std::uint32_t current, float gravity);
        void capManualSet(bool capped) const;
        void capAtTime(std::uint32_t now) const;
        void catOnGravity(float sg) const;

        [[nodiscard]] std::uint32_t targetTime() const;
        [[nodiscard]] float targetGravity() const;
        [[nodiscard]] AutoCapMode mode() const;

        static Actuator *capper;

    private:
        AutoCapSettings *_settings{};
        CapStatus _capStatus{CapStatusUnknown};

        void setCapOn(bool on);
        static void saveConfig();
};


inline bool AutoCapControl::isCapOn() const { return _capStatus == CapStatusActive; }


inline void AutoCapControl::saveConfig() { theSettings.save(); }


inline std::uint32_t AutoCapControl::targetTime() const { return _settings->condition.targetTime; }


inline float AutoCapControl::targetGravity() const { return _settings->condition.targetGravity; }


extern AutoCapControl autoCapControl;

#endif
