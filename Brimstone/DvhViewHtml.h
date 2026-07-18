// DvhViewHtml.h
//
// The self-contained HTML/JS page rendered in the DVH WebView2 host. It shows a
// dose-volume-histogram chart plus an interactive structure/prescription editor:
// three lists (Target / OAR / None), each a grid of Name/Color/Min/Max/Weight,
// with drag-to-reorder (priority) and drag-between-lists (type).
//
// C++ -> page (via ExecScript):
//    setStructures([{id,name,color,type,min,max,weight,priority}, ...])
//    setDvh([{id,color,target,pts:[[dose,vol],...]}, ...])
//
// page -> C++ (via window.chrome.webview.postMessage, pipe-delimited strings):
//    "type|<id>|<0|1|2>"          structure moved to None/Target/OAR
//    "weight|<id>|<w>"            weight edited
//    "interval|<id>|<min>|<max>"  min/max dose edited
//    "color|<id>|<rrggbb>"        color edited
//    "order|<id0>,<id1>,..."      full display order -> priorities
//
// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
#pragma once

extern const wchar_t* g_dvhViewHtml;
