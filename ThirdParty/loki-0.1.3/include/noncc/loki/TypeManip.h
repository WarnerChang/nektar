/////////////////////////////////
// Generated header: TypeManip.h
// Forwards to the appropriate code
// that works on the detected compiler
// Generated on Mon Sep 30 23:14:48 2002
/////////////////////////////////

#ifdef LOKI_USE_REFERENCE
#	include "../../loki/TypeManip.h"
#else
#	if (__INTEL_COMPILER)
#		include "../../loki/TypeManip.h"
#	elif (__MWERKS__)
#		include "../../loki/TypeManip.h"
#	elif (__BORLANDC__ >= 0x560)
#		include "../Borland/TypeManip.h"
#	elif (_MSC_VER >= 1301)
#		include "../../loki/TypeManip.h"
#	elif (_MSC_VER >= 1300)
#		include "../MSVC/1300/TypeManip.h"
#	elif (_MSC_VER >= 1200)
#		include "../MSVC/1200/TypeManip.h"
#	else
#		include "../../loki/TypeManip.h"
#	endif
#endif
