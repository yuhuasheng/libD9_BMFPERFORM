/*===================================================================================================
					Copyright(c) 2011 Siemens PLM Software Corp. All rights reserved.
							 Unpublished - All rights reserved
====================================================================================================
File description:

	Filename : epm_register_handler.c

	This file registers functions which are called when Teamcenter is being initialized

====================================================================================================
Date               Name                  Description of Change
2011-8-21			Ray wei              creation

$HISTORY$
==================================================================================================*/

#pragma warning (disable: 4819) 

/**
* @headerfile			tcua 头文件
*/
#include <server_exits/user_server_exits.h>
#include <tccore/custom.h>
#include <epm/epm.h>

/**
* @headerfile			standard c & cpp header files
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <tccore\item_msg.h>
#include <bom\bom_msg.h>
#include <ae\dataset_msg.h>
#include <res\res_itk.h>



/**
* @headerfile			user's header files
*/
#include "epm_register_handler.h"
#include "extension.h"
//#include <tccore\iman_msg.h>  ----------------20221222
#include <tccore\wso_msg.h>
#include  <tccore\grm_msg.h>
#include <tccore/tc_msg.h>
//#include "origin_CreateItemWithDataset.h"
#define LIMITDAY 2015

#ifdef __cplusplus
extern "C" {
#endif

	int checkvalue(char* str)
	{

		int i;
		for (i = 0; i < strlen(str); i++)
		{
			if (isdigit(str[i]) == 0)
				return 0;
		}
		return 1;
	}


	char* encrypt(char* str)
	{
		char* p = str;
		int i = 0;
		while (*p != '\0')
			*p++ ^= i++;
		return str;
	}

	// Method and Workflow Handler
	extern DLLAPI int CUST_init_module(int *decision, va_list args)
	{
		int ifail = ITK_ok, n = 0;
		char date_buf[80], *expire_date, env[512], temp1[512], temp2[512];
		time_t now;
		struct   tm   *p;
		METHOD_id_t mth_tag;

		*decision = ALL_CUSTOMIZATIONS;		
		

//		ifail = METHOD_find_method("D9_DT_DCNRevision", "GRM_create", &mth_tag);
		
		/*METHOD_find_method("IMAN_specification", GRM_create_msg, &mth_tag);		
		if (mth_tag.id != 0)
		{
			ifail = METHOD_add_action(mth_tag, METHOD_post_action_type, D9_CreateDCNPostAction, NULL);			
			if (ifail == ITK_ok)
			{
				printf("\nIMAN_specification post action successfully registered!\n");
				TC_write_syslog("\nIMAN_specification post action successfully registered!\n");
			}
		}*/

		return ifail;
	}


//	// Method and Workflow Handler
//	extern DLLAPI int CUST_init_gs_shell_module(int *decision, va_list args)
//	{
//		int ifail = ITK_ok, n = 0;
//		char date_buf[80], *expire_date, env[512], temp1[512], temp2[512];
//		time_t now;
//		struct   tm   *p;
//		METHOD_id_t mth_tag;
//
//		*decision = ALL_CUSTOMIZATIONS;
//
//		ifail = EPM_register_action_handler("D9_CheckPlacement", "检查正反面信息", (EPM_action_handler_t)D9_CheckPlacement);
//		if (ifail == ITK_ok) {
//			TC_write_syslog("Registering action handler libworkflowZL FX_Set_Confirm completed!\n", ifail);
//		}
//
//
//
//		ifail = EPM_register_rule_handler("D9_CheckAssembly", "检查装配", (EPM_rule_handler_t)D9_CheckAssembly);
//		if (ifail == ITK_ok) {
//			TC_write_syslog("Registering rule handler libworkflowZL FX_CheckSchedulePerson completed!\n", ifail);
//		}
//
//		ifail = EPM_register_action_handler("Auto-Archive-RCP", "Auto Archive For RCP", AutoArchiveByRCP);
//		if (ifail == ITK_ok)
//		{
//			TC_write_syslog("Successfully Registered Auto-Archive:ifail %d\n", ifail);
//		}
//
//		ifail = METHOD_find_method("ItemRevision", "ITEM_copy_rev", &mth_tag);
////		ifail = METHOD_find_method("ItemRevision", "ITEM_save_rev", &mth_tag);
//
//		if (mth_tag.id != 0)
//		{
//			ifail = METHOD_add_action(mth_tag, METHOD_post_action_type, itemrev_revise_post_action, NULL);
//			if (ifail == ITK_ok)
//			{
//				printf("\nItemRevision revise_post_action Item creation post successfully registered!\n");
//				TC_write_syslog("\nItemRevision revise_post_action Item creation post successfully registered!\n");
//			}
//		}
//
//
//		//Register_revise_msg();
//		return ifail;
//	}
	//
	////register service method
	extern DLLAPI int USERSERVICE_custom_register_methods(int *decision, va_list args)
	{
		int
			status = ITK_ok,
			numberOfArguments = 0,
			returnValueType = USERARG_STRING_TYPE,
			*argumentList = NULL;
		USER_function_t functionPtr;
		*decision = ALL_CUSTOMIZATIONS;

		numberOfArguments = 1;
		functionPtr = set_bypass;
		argumentList = (int*)MEM_alloc(numberOfArguments * sizeof(int));
		argumentList[0] = USERARG_STRING_TYPE;
		//argumentList[1] = USERARG_STRING_TYPE + USERARG_ARRAY_TYPE;
		returnValueType = USERARG_STRING_TYPE;
		USERSERVICE_register_method("set_bypass", functionPtr, numberOfArguments,
			argumentList, returnValueType);
		MEM_free(argumentList);
		TC_write_syslog("\n Registering set_bypass finished\n");

		numberOfArguments = 1;
		functionPtr = close_bypass;
		argumentList = (int*)MEM_alloc(numberOfArguments * sizeof(int));
		argumentList[0] = USERARG_STRING_TYPE;
		//argumentList[1] = USERARG_STRING_TYPE + USERARG_ARRAY_TYPE;
		returnValueType = USERARG_STRING_TYPE;
		USERSERVICE_register_method("close_bypass", functionPtr, numberOfArguments,
			argumentList, returnValueType);
		MEM_free(argumentList);
		TC_write_syslog("\n Registering close_bypass finished\n");

		numberOfArguments = 2;
		functionPtr = sync_draw_version;
		argumentList = (int*)MEM_alloc(numberOfArguments * sizeof(int));
		argumentList[0] = USERARG_STRING_TYPE;
		argumentList[1] = USERARG_STRING_TYPE;
		//argumentList[1] = USERARG_STRING_TYPE + USERARG_ARRAY_TYPE;
		returnValueType = USERARG_STRING_TYPE;
		USERSERVICE_register_method("sync_draw_version", functionPtr, numberOfArguments,
			argumentList, returnValueType);
		MEM_free(argumentList);
		TC_write_syslog("\n Registering sync_draw_version finished\n");

		return(status);
	}

#ifdef __cplusplus
}
#endif