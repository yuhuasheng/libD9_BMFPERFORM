#ifdef __cplusplus
extern "C" {
#endif


#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <epm/epm.h>
#include <itk/bmf.h>
#include <bmf/libuserext_exports.h>
	/*=============================================================================*
	* FUNCTION: fillIDToDrawingNo
	* PURPOSE : extension calling in function, check the item revison, permit it to revise or not
	* INPUT:
	*     METHOD_message_t* msg 	// extension method message struct
	*     va_list args     		// extension parameters
	*
	* RETURN:
	*     int // ITK_ok:call successfully; not ITK_ok:error occur
	*============================================================================*/
	//extern USER_EXT_DLL_API int FX8_create_change_notice(METHOD_message_t* msg, va_list args);
	//extern USER_EXT_DLL_API int FX8_Delete_Classification(METHOD_message_t* msg, va_list args);
	/*extern USER_EXT_DLL_API int FX8_create_revision_smchange(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_paste_rel(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_delete_rel(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_FinishedSchedule(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_OngoingSchedule(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_OnPlanningSchedule(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_PassRate(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_FailedRate(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_CreateProgramPostAction(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int FX8_SaveProgramPostAction(METHOD_message_t* msg, va_list args);*/
	extern USER_EXT_DLL_API int FX8_CreatePRTFxItemPostAction(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int design_create_post_action(METHOD_message_t* m, va_list args);
	extern USER_EXT_DLL_API int itemrev_revise_post_action(METHOD_message_t* m, va_list args);
	extern USER_EXT_DLL_API int D9_SetRealEmailPostAction(METHOD_message_t* m, va_list args);
	extern USER_EXT_DLL_API int D9_CreateDCNPostAction(METHOD_message_t* m, va_list args);
	extern USER_EXT_DLL_API int D9_CreateItemPostAction(METHOD_message_t* m, va_list args);
	extern USER_EXT_DLL_API int D9_IRRPNIRDELL(METHOD_message_t* msg, va_list args);
	extern USER_EXT_DLL_API int D9_IRSeverityIRDELL(METHOD_message_t* msg, va_list args);

	extern int POM_AM__set_application_bypass(logical bypass);
	extern int D9_Action_Handler(int *decision, va_list args);
	int D9_AutoArchiveByRCP(EPM_action_message_t msg);
	int SendPublicMail(EPM_action_message_t msg);
	int D9_SystemDesignArch(EPM_action_message_t msg);
	int D9_SendMatrixEmail(EPM_action_message_t msg);
	int D9_SendIssueEmail(EPM_action_message_t msg);
	int D9_Set_Property(EPM_action_message_t msg);
	int D9_SetMaterialGroupInfo(EPM_action_message_t msg);
	int D9_CheckBOMProperties(EPM_rule_message_t msg);
	int D9_CheckFWRelation(EPM_rule_message_t msg);
	extern int POM_AM__set_application_bypass(logical bypass);
	int sync_draw_version(void *retValType);	
	int D9_CheckPlacement(EPM_action_message_t msg);
	int D9_SaveDCNChange(EPM_action_message_t msg);	
	int D9_ApplyRelativeBOM(EPM_action_message_t msg);
	int D9_CheckAssembly(EPM_rule_message_t msg);
	int D9_ApplyCustomPN(EPM_action_message_t msg);
	int D9_SendMailForApprove(EPM_action_message_t msg);
	int D9_SendMailForReject(EPM_action_message_t msg);
	int D9_HPEBOMExport(EPM_action_message_t msg);
	int D9_CheckCustomPN(EPM_rule_message_t msg);
	int D9_PostCustomPN(EPM_action_message_t msg);
	int D9_CustomPNArch(EPM_rule_message_t msg);
	int D9_2ndSourceChangeList(EPM_action_message_t msg);	
	int D9_SendTechDocMail(EPM_action_message_t msg);
	int D9_StpToJT(EPM_action_message_t msg);
	int D9_DellEBOMExport(EPM_action_message_t msg);
	int D9_ScheduleTaskNotify(EPM_action_message_t msg);
	int D9_CreateMoldFolder(EPM_rule_message_t msg);
	int D9_CheckTriggerPlacement(EPM_action_message_t msg);
	int D9_NewMoldFee(EPM_action_message_t msg);
	int D9_DCNFee(EPM_action_message_t msg);
	int D9_FWAutoArch(EPM_action_message_t msg);
	int D9_Save_MNT_L10_AltGroup(EPM_action_message_t msg);
	int D9_Check_MNT_DCN(EPM_action_message_t msg);

	int D9_PostDCN2Agile(EPM_rule_message_t msg);
	int D9_CheckBOMPropertiesForL10(EPM_rule_message_t msg);
	int D9_SubmitToRPA(EPM_action_message_t msg);
	int D9_NoDifferenceNotice(EPM_action_message_t msg);
	int set_bypass(void *retValType); //开启旁路
	int close_bypass(void *retValType); //关闭旁路


#include <bmf/libuserext_undef.h>
#include <tccore/method.h>
#include <tccore/item.h>




#endif

#ifdef __cplusplus
}
#endif