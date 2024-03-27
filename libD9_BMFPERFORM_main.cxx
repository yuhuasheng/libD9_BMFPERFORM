/**
* @defgroup				common		公用函数
* @defgroup				workflow	流程函数
* @defgroup				bmf			业务扩展
* @defgroup				service		服务函数
*/

/**
* @addtogroup			common
* @{
*/

/**
* @file
* @brief				注册主函数
* @date					2011/4/11
* @author				Ray Li
*/

#pragma warning (disable: 4819) 

/**
* @headerfile			tcua 头文件
*/

//#include <tc/tc.h>                -----------20221222
#include <tccore/custom.h>
#include <server_exits/user_server_exits.h>
#include "error_handling.h"
#include "epm_register_handler.h"
#include "extension.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

	void revisePostAction();
	void realEmailPostAction();

	void itemPostAction();
	void IRDellPostAction();

	extern DLLAPI int libD9_BMFPERFORM_register_callbacks()
	{
		int ifail = ITK_ok;

		ITKCALL(ifail = CUSTOM_register_exit("libD9_BMFPERFORM", "USERSERVICE_register_methods", (CUSTOM_EXIT_ftn_t)USERSERVICE_custom_register_methods));
		fprintf(stdout, "\nRegistering USERSERVICE_register_methods completed!\n");
		TC_write_syslog("\nRegistering USERSERVICE_register_methods completed!\n");

		/*ITKCALL(ifail = CUSTOM_register_exit("libD9_BMFPERFORM","USER_gs_shell_init_module",(CUSTOM_EXIT_ftn_t)CUST_init_module));
		fprintf(stdout, "Registering USER_gs_shell_init_module completed!\n");
		TC_write_syslog("\nRegistering USER_gs_shell_init_module completed!\n");*/


		ITKCALL(ifail = CUSTOM_register_exit("libD9_BMFPERFORM", "USER_gs_shell_init_module", (CUSTOM_EXIT_ftn_t)CUST_init_gs_shell_module));
		fprintf(stdout, "Registering USER_gs_shell_init_module completed!\n");
		TC_write_syslog("\nRegistering USER_gs_shell_init_module completed!\n");

		return ifail;
	}
	
	// Method and Workflow Handler
	extern DLLAPI int CUST_init_gs_shell_module(int *decision, va_list args)
	{
		int ifail = ITK_ok, n = 0;
		char date_buf[80], *expire_date, env[512], temp1[512], temp2[512];
		time_t now;
		struct   tm   *p;
		METHOD_id_t mth_tag;

		*decision = ALL_CUSTOMIZATIONS;
		
		ifail = EPM_register_action_handler("D9_CheckPlacement", "检查正反面信息", (EPM_action_handler_t)D9_CheckPlacement);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_CheckPlacement completed!\n", ifail);
		}
		
		ifail = EPM_register_action_handler("D9_SaveDCNChange", "保存DCN变更", (EPM_action_handler_t)D9_SaveDCNChange);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_SaveDCNChange completed!\n", ifail);
		}

		ifail = EPM_register_rule_handler("D9_CheckAssembly", "检查装配", (EPM_rule_handler_t)D9_CheckAssembly);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering rule handler libworkflowZL D9_CheckAssembly completed!\n", ifail);
		}


		ifail = EPM_register_rule_handler("D9_PostDCN2Agile", "抛转DCN to Agile", (EPM_rule_handler_t)D9_PostDCN2Agile);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_PostDCN2Agile completed!\n", ifail);
		}


		ifail = EPM_register_rule_handler("D9_CustomPNArch", "自编物料归档", (EPM_rule_handler_t)D9_CustomPNArch);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering rule handler libworkflowZL D9_CustomPNArch completed!\n", ifail);
		}


		ifail = EPM_register_rule_handler("D9_CheckCustomPN", "检查自编料号", (EPM_rule_handler_t)D9_CheckCustomPN);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering rule handler libworkflowZL D9_CheckCustomPN completed!\n", ifail);
		}

		ifail = EPM_register_rule_handler("D9_CheckBOMProperties", "Check BOM Properties", (EPM_rule_handler_t)D9_CheckBOMProperties);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Registering rule handler libworkflowZL D9_CheckBOMProperties completed!\n", ifail);
		}
		ifail = EPM_register_rule_handler("D9_CheckBOMPropertiesForL10", "D9_CheckBOMPropertiesForL10", (EPM_rule_handler_t)D9_CheckBOMPropertiesForL10);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Registering rule handler libworkflowZL D9_CheckBOMPropertiesForL10 completed!\n", ifail);
		}

		ifail = EPM_register_rule_handler("D9_CheckFWRelation", "Check FW Relation", (EPM_rule_handler_t)D9_CheckFWRelation);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Registering rule handler libworkflowZL D9_CheckFWRelation completed!\n", ifail);
		}


		ifail = EPM_register_action_handler("D9_ApplyCustomPN", "申请自编料号", (EPM_action_handler_t)D9_ApplyCustomPN);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_ApplyCustomPN completed!\n", ifail);
		}


		ifail = EPM_register_action_handler("D9_SendTechDocMail", "技术文档邮件通知", (EPM_action_handler_t)D9_SendTechDocMail);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_SendTechDocMail completed!\n", ifail);
		}


		ifail = EPM_register_action_handler("D9_2ndSourceChangeList", "2ndSource change list", (EPM_action_handler_t)D9_2ndSourceChangeList);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_2ndSourceChangeList completed!\n", ifail);
		}


		ifail = EPM_register_action_handler("D9_PostCustomPN", "抛转自编料号", (EPM_action_handler_t)D9_PostCustomPN);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_PostCustomPN completed!\n", ifail);
		}


		ifail = EPM_register_action_handler("D9_AutoArchiveRCP", "Auto Archive For RCP", D9_AutoArchiveByRCP);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Successfully Registered AutoArchiveRCP:ifail %d\n", ifail);
		}		

		ifail = EPM_register_action_handler("D9_SystemDesignArch", "System Design Arch", D9_SystemDesignArch);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Successfully Registered System Design Arch:ifail %d\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_SendMatrixEmail", "Send Matrix Email", D9_SendMatrixEmail);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Successfully Registered SendMatrixEmail:ifail %d\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_SendIssueEmail", "Send Issue Email", D9_SendIssueEmail);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Successfully Registered SendIssueEmail:ifail %d\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_Set_Property", "Set Property", D9_Set_Property);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Successfully Registered Set Property:ifail %d\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_SetMaterialGroupInfo", "Set Material Group Info", D9_SetMaterialGroupInfo);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Successfully Registered SetMaterialGroupInfo:ifail %d\n", ifail);
		}		


		ifail = EPM_register_action_handler("D9_ApplyRelativeBOM", "Apply Relative BOM", (EPM_action_handler_t)D9_ApplyRelativeBOM);
		if (ifail == ITK_ok)
		{
			TC_write_syslog("Registering action handler libworkflowZL D9_ApplyRelativeBOM completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_HPEBOMExport", "HP_EBOM导出", (EPM_action_handler_t)D9_HPEBOMExport);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_HPEBOMExport completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_StpToJT", "Stp转JT", (EPM_action_handler_t)D9_StpToJT);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_StpToJT completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_DellEBOMExport", "Dell_EBOM导出", (EPM_action_handler_t)D9_DellEBOMExport);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_DellEBOMExport completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_ScheduleTaskNotify", "时间表任务提醒", (EPM_action_handler_t)D9_ScheduleTaskNotify);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_ScheduleTaskNotify completed!\n", ifail);
		}

		ifail = EPM_register_rule_handler("D9_CreateMoldFolder", "建立机种文件夹", (EPM_rule_handler_t)D9_CreateMoldFolder);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering rule handler libworkflowZL D9_CreateMoldFolder completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_CheckTriggerPlacement", "CheckTriggerPlacement", (EPM_action_handler_t)D9_CheckTriggerPlacement);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_CheckTriggerPlacement completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_NewMoldFee", "NewMoldFee", (EPM_action_handler_t)D9_NewMoldFee);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_NewMoldFee completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_DCNFee", "DCNFee", (EPM_action_handler_t)D9_DCNFee);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_DCNFee completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_FWAutoArch", "FWAutoArch", (EPM_action_handler_t)D9_FWAutoArch);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_FWAutoArch completed!\n", ifail);
		}
		ifail = EPM_register_action_handler("D9_SubmitToRPA", "SubmitToRPA", (EPM_action_handler_t)D9_SubmitToRPA);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_SubmitToRPA completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_SendMailForApprove", "SendMailForApprove", (EPM_action_handler_t)D9_SendMailForApprove);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_SendMailForApprove completed!\n", ifail);
		}
		
		ifail = EPM_register_action_handler("D9_SendMailForReject", "SendMailForReject", (EPM_action_handler_t)D9_SendMailForReject);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_SendMailForReject completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_Save_MNT_L10_AltGroup", "SaveMNTL10AltGroup", (EPM_action_handler_t)D9_Save_MNT_L10_AltGroup);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_Save_MNT_L10_AltGroup completed!\n", ifail);
		}

		ifail = EPM_register_action_handler("D9_Check_MNT_DCN", "Check_MNT_DCN", (EPM_action_handler_t)D9_Check_MNT_DCN);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_Check_MNT_DCN completed!\n", ifail);
		}

		//
		ifail = EPM_register_action_handler("D9_NoDifferenceNotice", "NoDifferenceNotice", (EPM_action_handler_t)D9_NoDifferenceNotice);
		if (ifail == ITK_ok) {
			TC_write_syslog("Registering action handler libworkflowZL D9_NoDifferenceNotice completed!\n", ifail);
		}

		// Issue上线在打开使用
		ifail = METHOD_register_prop_method("D9_IR_DELLRevision", "d9_IRRPN", PROP_ask_value_string_msg, D9_IRRPNIRDELL, NULL, &mth_tag);
		if (mth_tag.id != 0) {
			TC_write_syslog("Registering D9_IR_DELLRevision_d9_IRRPN_ask_object_string_value completed！ \n");
		}

		ifail = METHOD_register_prop_method("D9_IR_DELLRevision", "d9_IRSeverity", PROP_ask_value_string_msg, D9_IRSeverityIRDELL, NULL, &mth_tag);
		if (mth_tag.id != 0) {
			TC_write_syslog("Registering D9_IR_DELLRevision_d9_IRSeverity_ask_object_string_value completed！ \n");
		}


		revisePostAction();
		realEmailPostAction();

//		itemPostAction();
		//Register_revise_msg();

		return ifail;
	}

	void revisePostAction() {
		METHOD_id_t mth_tag;
		int ifail = ITK_ok;
		vector<string> vec;
		//获取首选项
		getPreferenceByName("D9_Revise_Control_IR", vec);
		for (int i = 0; i < vec.size(); i++) {
			//printf("item_rev_object_type == %s\n", vec[i].c_str());
			ifail = METHOD_find_method(vec[i].c_str(), "ITEM_copy_rev", &mth_tag);
			//ifail = METHOD_find_method("ItemRevision", "ITEM_save_rev", &mth_tag);			
			if (mth_tag.id != 0)
			{
				ifail = METHOD_add_action(mth_tag, METHOD_post_action_type, itemrev_revise_post_action, NULL);
				if (ifail == ITK_ok)
				{
					char* msg = NULL;
					msg = (char*)MEM_alloc(sizeof(char) * 4096);
					tc_strcat(msg, "\n");
					tc_strcat(msg, vec[i].c_str());
					tc_strcat(msg, "_revise_post_action Item creation post successfully registered!\n");
					printf(msg);
					TC_write_syslog(msg);
				}
			}
		}
	}

	void realEmailPostAction()
	{
		METHOD_id_t mth_tag;
		int ifail = ITK_ok;
		vector<string> vec;
		
		//获取首选项
		getPreferenceByName("D9_RealMail_ObjectControl", vec);

		for (int i = 0; i < vec.size(); i++) {
			//printf("realEmailPostAction item_rev_object_type == %s\n", vec[i].c_str());
			ifail = METHOD_find_method(vec[i].c_str(), "ITEM_create", &mth_tag);			
			if (mth_tag.id != 0)
			{
				ifail = METHOD_add_action(mth_tag, METHOD_post_action_type, D9_SetRealEmailPostAction, NULL);
				if (ifail == ITK_ok)
				{
					char* msg = NULL;
					msg = (char*)MEM_alloc(sizeof(char) * 4096);
					tc_strcat(msg, "\n");
					tc_strcat(msg, vec[i].c_str());
					tc_strcat(msg, "_real_email_post_action Item creation post successfully registered!\n");
					printf(msg);
					TC_write_syslog(msg);
				}
			}
		}
	}

	void itemPostAction() {
		METHOD_id_t mth_tag;
		int ifail = ITK_ok;


		TC_write_syslog("==>> itemPostAction start\n");			
		ifail = METHOD_find_method("Item", TC_save_msg, &mth_tag);
		TC_write_syslog("==>> mth_tag.id:%d\n", mth_tag.id);
		if (mth_tag.id != 0) {
			ifail = METHOD_add_action(mth_tag, METHOD_post_action_type, D9_CreateItemPostAction, NULL);
			if (ifail == ITK_ok) {
				char* msg = NULL;
				msg = (char*)MEM_alloc(sizeof(char) * 4096);
				tc_strcat(msg, "\n");				
				tc_strcat(msg, "Item_post_action creation post successfully registered!\n");
				printf(msg);
				TC_write_syslog(msg);
			}
		}
	}	


#ifdef __cplusplus
}
#endif




/**
* @}
*/