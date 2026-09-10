// OptGraphHtml.h
//
// The self-contained HTML/JS convergence chart rendered in the WebView2 host
// that replaces the legacy GDI iteration graph. Exposes two JS entry points the
// C++ side drives via ExecScript:
//    addPoint(level, x, y)   -- append a point to a pyramid-level series
//    resetChart()            -- clear all series (called when optimization starts)
//
// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
#pragma once

extern const wchar_t* g_optGraphHtml;
