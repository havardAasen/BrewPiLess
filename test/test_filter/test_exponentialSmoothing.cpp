/*
* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Håvard F. Aasen
 */

#include <ExponentialSmoothing.h>
#include <gtest/gtest.h>
#include <vector>

TEST(ExponentialSmoothing, DefaultConstruction)
{
    const ExponentialSmoothing filter;
    EXPECT_FLOAT_EQ(filter.alpha(), 0.1f);
    EXPECT_FLOAT_EQ(filter.output(), 0.0f);
}

TEST(ExponentialSmoothing, SetInitial)
{
    ExponentialSmoothing filter;
    filter.setInitial(5.0f);
    EXPECT_FLOAT_EQ(filter.output(), 5.0f);
}

TEST(ExponentialSmoothing, AlphaSetterGetter)
{
    ExponentialSmoothing filter;
    filter.setAlpha(0.75f);
    EXPECT_FLOAT_EQ(filter.alpha(), 0.75f);
}

TEST(ExponentialSmoothing, SingleUpdate)
{
    ExponentialSmoothing filter(0.2f);
    filter.setInitial(1.0f);
    const float y = filter.addSample(3.0f);   // y = 1 + 0.2*(3-1) = 1.4
    EXPECT_FLOAT_EQ(y, 1.4f);
    EXPECT_FLOAT_EQ(filter.output(), 1.4f);
}

TEST(ExponentialSmoothing, MultipleUpdates)
{
    ExponentialSmoothing filter(0.5f);
    filter.setInitial(0.0f);
    const std::vector in  = {2.0f, 4.0f, 6.0f};
    const std::vector out = {1.0f, 2.5f, 4.25f}; // known EMA values

    for (size_t i = 0; i < in.size(); ++i) {
        EXPECT_FLOAT_EQ(filter.addSample(in[i]), out[i]);
    }
}

TEST(ExponentialSmoothing, AlphaOnePassesThrough)
{
    ExponentialSmoothing filter(1.0f);
    filter.setInitial(0.0f);
    EXPECT_FLOAT_EQ(filter.addSample(7.3f), 7.3f);
    EXPECT_FLOAT_EQ(filter.addSample(-2.1f), -2.1f);
}
