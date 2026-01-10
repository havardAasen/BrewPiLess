/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Håvard F. Aasen
 */

#include <QueueBuffer.h>
#include <gtest/gtest.h>

static constexpr int TEST_BUF_SIZE = 8;

TEST(QueueBuffer, StartsEmpty) {
    QueueBuffer qb(TEST_BUF_SIZE);
    EXPECT_EQ(qb.available(), 0);
    EXPECT_EQ(qb.read(), -1);
}

TEST(QueueBuffer, WriteAndReadSingleChar) {
    QueueBuffer qb(TEST_BUF_SIZE);

    qb.print('A');
    EXPECT_EQ(qb.available(), 1);

    const int c = qb.read();
    EXPECT_EQ(c, 'A');
    EXPECT_EQ(qb.available(), 0);
}

TEST(QueueBuffer, WriteAndReadString) {
    QueueBuffer qb(TEST_BUF_SIZE);

    qb.print("Hi");
    EXPECT_EQ(qb.available(), 2);

    EXPECT_EQ(qb.read(), 'H');
    EXPECT_EQ(qb.read(), 'i');
    EXPECT_EQ(qb.read(), -1);
}

TEST(QueueBuffer, WrapAround) {
    QueueBuffer qb(TEST_BUF_SIZE);

    qb.print('A');
    qb.print('B');
    qb.print('C');
    qb.print('D');
    qb.print('E');
    qb.print('F');
    qb.print('G');

    EXPECT_EQ(qb.available(), 7);

    EXPECT_EQ(qb.read(), 'A');
    EXPECT_EQ(qb.read(), 'B');
    EXPECT_EQ(qb.available(), 5);

    qb.print('X');
    qb.print('Y');

    EXPECT_EQ(qb.available(), 7);

    EXPECT_EQ(qb.read(), 'C');
    EXPECT_EQ(qb.read(), 'D');
    EXPECT_EQ(qb.read(), 'E');
    EXPECT_EQ(qb.read(), 'F');
    EXPECT_EQ(qb.read(), 'G');
    EXPECT_EQ(qb.read(), 'X');
    EXPECT_EQ(qb.read(), 'Y');
    EXPECT_EQ(qb.read(), -1);
}

TEST(QueueBuffer, OverwriteBehavior) {
    QueueBuffer qb(4);

    qb.print('A');
    qb.print('B');
    qb.print('C');

    qb.print('X');
    qb.print('Y');

    EXPECT_EQ(qb.available(), 3);

    EXPECT_EQ(qb.read(), 'C');
    EXPECT_EQ(qb.read(), 'X');
    EXPECT_EQ(qb.read(), 'Y');

    EXPECT_EQ(qb.read(), -1);
}

