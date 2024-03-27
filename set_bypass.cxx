#pragma warning (disable: 4996)
#pragma warning (disable: 4819)

/**
* @headerfile			tcua 头文件
*/
#include <ict/ict_userservice.h>
#include <tccore/item.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <property/prop_errors.h>
#include <tccore/workspaceobject.h>
#include <tc/preferences.h>
//#include <tccore/imantype.h>                -----------20221222
#include <tccore/grm.h>
#include <tccore/grmtype.h>
#include <sa/am.h>
#include <cfm/cfm.h>
/**
* @headerfile standard c & cpp header files
*/
#include  <stdio.h>
#include  <stdlib.h>
#include <time.h>

/**
* @headerfile user's header files
*/
#include "error_handling.h"

extern "C" int POM_AM__ask_application_bypass(logical bypass);
extern "C" int POM_AM__set_application_bypass(logical bypass);
extern "C" int AM__set_application_bypass(logical bypass);
extern "C" int RIL_applic_protection(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int set_bypass(void* retValue) {

		printf("=========================================================\n");
		printf("set_bypass 开始执行\n");
		printf("=========================================================\n");
		char* status = NULL;

		USERARG_get_string_argument(&status);


		AM__set_application_bypass(true);

		POM_AM__set_application_bypass(true);

		RIL_applic_protection(false);

		ITK_set_bypass(true);

		printf("=========================================================\n");
		printf("set_bypass 执行完成\n");
		printf("=========================================================\n");

		return ITK_ok;
	}

#ifdef __cplusplus
}
#endif