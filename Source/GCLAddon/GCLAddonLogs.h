// Copyright (C) 2023 owoDra

#pragma once

#include "Logging/LogMacros.h"

GCLADDON_API DECLARE_LOG_CATEGORY_EXTERN(LogGCLA, Log, All);

#if !UE_BUILD_SHIPPING

#define GCLALOG(FormattedText, ...) UE_LOG(LogGCLA, Log, FormattedText, __VA_ARGS__)

#define GCLAENSURE(InExpression) ensure(InExpression)
#define GCLAENSURE_MSG(InExpression, InFormat, ...) ensureMsgf(InExpression, InFormat, __VA_ARGS__)
#define GCLAENSURE_ALWAYS_MSG(InExpression, InFormat, ...) ensureAlwaysMsgf(InExpression, InFormat, __VA_ARGS__)

#define GCLACHECK(InExpression) check(InExpression)
#define GCLACHECK_MSG(InExpression, InFormat, ...) checkf(InExpression, InFormat, __VA_ARGS__)

#else

#define GCLALOG(FormattedText, ...)

#define GCLAENSURE(InExpression) InExpression
#define GCLAENSURE_MSG(InExpression, InFormat, ...) InExpression
#define GCLAENSURE_ALWAYS_MSG(InExpression, InFormat, ...) InExpression

#define GCLACHECK(InExpression) InExpression
#define GCLACHECK_MSG(InExpression, InFormat, ...) InExpression

#endif