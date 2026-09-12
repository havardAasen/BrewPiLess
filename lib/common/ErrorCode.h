/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Håvard F. Aasen
 */

#ifndef BREWPILESS_ERRORCODE_H
#define BREWPILESS_ERRORCODE_H

#include <cstdint>

namespace bpl {
    enum class ErrorCode: std::uint8_t {
        None = 0,
        AuthenticationRequired,
        InvalidJson,
        MissingField,
        UnknownSource
    };

    const char *errorCodeToString(ErrorCode error);
}

#endif
