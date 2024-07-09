/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 Håvard F. Aasen
 */

#include <gtest/gtest.h>

#ifdef ARDUINO
#include <Arduino.h>

void setup()
{
    // should be the same value as for the `test_speed` option in "platformio.ini"
    // default value is test_speed=115200
    Serial.begin(115200);

    ::testing::InitGoogleTest();
}

void loop()
{
    if (RUN_ALL_TESTS()) {}
    delay(1000);
}

#else
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    if (RUN_ALL_TESTS()) {}
    return 0;
}
#endif
