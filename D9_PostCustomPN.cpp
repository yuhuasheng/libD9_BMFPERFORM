#include "extension.h"
#include "util.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_PostCustomPN(EPM_action_message_t msg)
	{
		int ifail = ITK_ok;

		return ifail;
	}
#ifdef __cplusplus
}
#endif