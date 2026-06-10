#pragma once

#include <QtCore/QtGlobal>

#if defined(GPSCOMM_LIBRARY)
#  define GPSCOMM_EXPORT Q_DECL_EXPORT
#else
#  define GPSCOMM_EXPORT Q_DECL_IMPORT
#endif
