/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 Håvard F. Aasen
 */

#include <conversion.h>
#include <gtest/gtest.h>


TEST(ConversionTemperatures, celsiusToFahrenheit)
{
    EXPECT_DOUBLE_EQ(bpl::celsius_to_fahrenheit<double>(-100.0), -148.0);
    EXPECT_FLOAT_EQ(bpl::celsius_to_fahrenheit<float>(0.0), 32.0);
    EXPECT_DOUBLE_EQ(bpl::celsius_to_fahrenheit<double>(100.0), 212.0);
}

TEST(ConversionTemperatures, FahrenheitToCelsius)
{
    EXPECT_DOUBLE_EQ(bpl::fahrenheit_to_celsius<double>(-148.0), -100.0);
    EXPECT_FLOAT_EQ(bpl::fahrenheit_to_celsius<float>(32.0), 0.0);
    EXPECT_DOUBLE_EQ(bpl::fahrenheit_to_celsius<double>(212.0), 100.0);
}

TEST(ConversionGravity, SpecificGravityToBrix)
{
    EXPECT_NEAR(bpl::specific_gravity_to_brix<double>(1.0), 0.0, 0.002);
    EXPECT_NEAR(bpl::specific_gravity_to_brix<float>(1.02369), 6.0, 0.002);
    EXPECT_NEAR(bpl::specific_gravity_to_brix<double>(1.05005), 12.4, 0.002);
}

TEST(ConversionGravity, BrixToSpecificGravity)
{
    EXPECT_NEAR(bpl::brix_to_specific_gravity<double>(0.0), 1.0, 0.002);
    EXPECT_NEAR(bpl::brix_to_specific_gravity<float>(5.2), 1.020, 0.002);
    EXPECT_NEAR(bpl::brix_to_specific_gravity<float>(10.0), 1.040, 0.002);
    EXPECT_NEAR(bpl::brix_to_specific_gravity<double>(13.7), 1.055, 0.002);
}
