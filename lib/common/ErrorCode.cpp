/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Håvard F. Aasen
 */

#include "ErrorCode.h"

const char *bpl::errorCodeToString(const ErrorCode error)
{
    switch (error) {
        case ErrorCode::None:
            return "No error";

        case ErrorCode::AuthenticationRequired:
            return "Authentication required";

        case ErrorCode::InvalidJson:
            return "Invalid JSON";

        case ErrorCode::MissingField:
            return "Missing field";

        case ErrorCode::UnknownSource:
            return "Unknown source";

        default:
            return "Unknown error";
    }
}
