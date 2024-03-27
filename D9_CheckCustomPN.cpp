#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <dispatcher/dispatcher_itk.h>
extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_CheckCustomPN(EPM_rule_message_t msg)
	{
		EPM_decision_t decision = EPM_go;
		int ifail = ITK_ok, counts=0, tagCount = 0,cnt=0;
		char  *tmp_arg = NULL, *para_name = NULL,  * relatedId = NULL,*error_msg, *object_type = NULL,  *tempPN = NULL, *uid = NULL, *values = NULL;
		tag_t root_task = NULLTAG;
		tag_t *target_tags = NULLTAG, *cnTags = NULL;
		tag_t revType = NULLTAG;
		tag_t typeTag = NULLTAG;
		logical isRev;
		//需要检查的对象属性
		char tempStr[256];
		char **qry_entries = NULL, **qry_values = NULL;
		error_msg = (char*)MEM_alloc(sizeof(char) * 9000);
		memset(error_msg, '\0', 9000);
		tc_strcpy(error_msg, "");
		tag_t qry = NULLTAG;
		char *itemId = NULL;
		ITKCALL(ifail = EPM_ask_root_task(msg.task, &root_task));

		//获取流程目标对象
		ITKCALL(ifail = EPM_ask_attachments(root_task, EPM_target_attachment, &counts, &target_tags));
		printf("\n EPM_target_attachment count %d\n", counts);
		for (int i = 0; i < counts; i++) {
	
			ITKCALL(TCTYPE_ask_type("ItemRevision", &revType));
			ITKCALL(TCTYPE_ask_object_type(target_tags[i], &typeTag));
			ITKCALL(TCTYPE_is_type_of(typeTag, revType, &isRev));		
			if (!isRev) {
				continue;
			}
			ITKCALL(ifail = AOM_ask_value_string(target_tags[i], "item_id", &itemId));
			printf("\n checking item  %s\n", itemId);
			ITKCALL(ifail = AOM_ask_value_string(target_tags[i], "d9_TempPN", &tempPN));
			printf("\n TempPN is %s\n", tempPN);
			if (startWith(tempPN,"49")==0 && startWith(tempPN, "629") == 0 && startWith(tempPN, "7351") == 0) {
				printf("\n  item is not start with 49/629/7351 %s\n", itemId);
				continue;
			}

			ITKCALL(ifail = AOM_ask_value_string(target_tags[i], "d9_SupplementInfo", &relatedId));
			printf("79 itemid == %s\n", relatedId);
			if (tc_strcmp(relatedId, "") == 0) {
				memset(tempStr, '\0', 256);
				sprintf(tempStr, G2B5("对象:%s对应的PCA对象不存在!\n"), itemId);
				tc_strcat(error_msg, tempStr);
				goto CLEANUP;
			}
			qry_entries = (char **)MEM_alloc(sizeof(char*) * 1);
			qry_values = (char **)MEM_alloc(sizeof(char*) * 1);
			qry_entries[0] = (char *)MEM_alloc(sizeof(char*) * 64);
			qry_values[0] = (char *)MEM_alloc(sizeof(char*) * 64);

			tc_strcpy(qry_entries[0], "item_id");
			tc_strcpy(qry_values[0], relatedId);

			ITKCALL(QRY_find2("Item_Name_or_ID", &qry));

			ITKCALL(QRY_execute(qry, 1, qry_entries, qry_values, &cnt, &cnTags));
			printf("search  event resultNum == %d\n", cnt);
			TC_write_syslog("search  event resultNum == %d\n", cnt);
			if (cnt <= 0) {
				memset(tempStr, '\0', 256);
				sprintf(tempStr, G2B5("对象:%s对应的PCA对象%s不存在!\n"), itemId, relatedId);
				tc_strcat(error_msg, tempStr);
			}
		}

	CLEANUP:
		printf("\n start handler CLEANUP\n");
		DOFREE(target_tags);
		DOFREE(cnTags);
		printf("\n ending CLEANUP over\n");
		if (tc_strcmp(error_msg, "") != 0)
		{
			decision = EPM_nogo;
			EMH_store_error_s1(EMH_severity_user_error, VER_RULE_ERROR, error_msg);
			ifail = VER_RULE_ERROR;	
		}

		return decision;
	}
#ifdef __cplusplus
}
#endif