/////////////////////////////////
// Generated header: Singleton.h
// Forwards to the appropriate code
// that works on the detected compiler
// Generated on Mon Sep 30 23:14:48 2002
/////////////////////////////////

#ifdef LOKI_USE_REFERENCE
#	include "../../loki/Singleton.h"
#else
#	if (__INTEL_COMPILER)
#		include "../../loki/Singleton.h"
#	elif (__MWERKS__)
#		include "../../loki/Singleton.h"
#	elif (__BORLANDC__ >= 0x560)
#		include "../Borland/Singleton.h"
#	elif (_MSC_VER >= 1301)
#		include "../../loki/Singleton.h"
#	elif (_MSC_VER >= 1300)
#		include "../MSVC/1300/Singleton.h"
#	elif (_MSC_VER >= 1200)
#		include "../MSVC/1200/Singleton.h"
#	else
#		include "../../loki/Singleton.h"
#	endif
#endif
