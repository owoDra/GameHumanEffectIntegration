// Copyright (C) 2023 owoDra

#pragma once

#include "Logging/LogMacros.h"

GHFXINTG_API DECLARE_LOG_CATEGORY_EXTERN(LogGHFXI, Log, All);

#if !UE_BUILD_SHIPPING

#define GHFXILOG(FormattedText, ...) UE_LOG(LogGHFXI, Log, FormattedText, __VA_ARGS__)

#define GHFXIENSURE(InExpression) ensure(InExpression)
#define GHFXIENSURE_MSG(InExpression, InFormat, ...) ensureMsgf(InExpression, InFormat, __VA_ARGS__)
#define GHFXIENSURE_ALWAYS_MSG(InExpression, InFormat, ...) ensureAlwaysMsgf(InExpression, InFormat, __VA_ARGS__)

#define GHFXICHECK(InExpression) check(InExpression)
#define GHFXICHECK_MSG(InExpression, InFormat, ...) checkf(InExpression, InFormat, __VA_ARGS__)

#else

#define GHFXILOG(FormattedText, ...)

#define GHFXIENSURE(InExpression) InExpression
#define GHFXIENSURE_MSG(InExpression, InFormat, ...) InExpression
#define GHFXIENSURE_ALWAYS_MSG(InExpression, InFormat, ...) InExpression

#define GHFXICHECK(InExpression) InExpression
#define GHFXICHECK_MSG(InExpression, InFormat, ...) InExpression

#endif