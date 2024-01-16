// Copyright (C) 2024 owoDra

#pragma once

#include "Logging/LogMacros.h"

GLEXT_API DECLARE_LOG_CATEGORY_EXTERN(LogGLE, Log, All);

#if !UE_BUILD_SHIPPING

#define GLELOG(FormattedText, ...) UE_LOG(LogGLE, Log, FormattedText, __VA_ARGS__)

#define GLEENSURE(InExpression) ensure(InExpression)
#define GLEENSURE_MSG(InExpression, InFormat, ...) ensureMsgf(InExpression, InFormat, __VA_ARGS__)
#define GLEENSURE_ALWAYS_MSG(InExpression, InFormat, ...) ensureAlwaysMsgf(InExpression, InFormat, __VA_ARGS__)

#define GLECHECK(InExpression) check(InExpression)
#define GLECHECK_MSG(InExpression, InFormat, ...) checkf(InExpression, InFormat, __VA_ARGS__)

#else

#define GLELOG(FormattedText, ...)

#define GLEENSURE(InExpression) InExpression
#define GLEENSURE_MSG(InExpression, InFormat, ...) InExpression
#define GLEENSURE_ALWAYS_MSG(InExpression, InFormat, ...) InExpression

#define GLECHECK(InExpression) InExpression
#define GLECHECK_MSG(InExpression, InFormat, ...) InExpression

#endif