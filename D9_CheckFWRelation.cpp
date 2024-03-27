#include "extension.h"
#include "errorMsg.h"
#include "util.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);

#ifdef __cplusplus
extern "C" {
#endif

	int D9_CheckFWRelation(EPM_rule_message_t msg)
	{
		EPM_decision_t decision = EPM_go;
		int
			ifail = ITK_ok,
			attach_object_num = 0,
			represent_num = 0;

		tag_t
			root_task = NULL_TAG,
			tItemRev = NULL_TAG;

		tag_t
			* attach_object_list = NULLTAG,
			* represent_object_list = NULLTAG;

		char
			*itemId = NULL,
			*version = NULL,
			*task_name = NULL,
			*error_msg = NULL;

		TC_write_syslog("\n=================   D9_CheckFWRelation   start   ==================\n");

		error_msg = (char*)MEM_alloc(sizeof(char) * 9000);
		memset(error_msg, '\0', 9000);
		tc_strcpy(error_msg, "");

		POM_AM__set_application_bypass(true); // 开启旁路

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // 获取流程根任务
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &task_name));

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (size_t i = 0; i < attach_object_num; i++) {
			logical
				isRev = false;

			tItemRev = attach_object_list[i];

			ITKCALL(GTCTYPE_is_type_of(tItemRev, "D9_FWDesignRevision", &isRev));
			if (!isRev) {
				continue;
			}

			char tempStr[1024];
			memset(tempStr, '\0', 1024);
			tc_strcpy(tempStr, "");

			ITKCALL((AOM_ask_value_string(tItemRev, "item_id", &itemId))); // 获取ID
			TC_write_syslog("itemId == %s\n", itemId);

			ITKCALL((AOM_ask_value_string(tItemRev, "item_revision_id", &version))); // 获取版本号
			TC_write_syslog("version == %s\n", version);

			ITKCALL(AOM_refresh(tItemRev, false));

			ITKCALL(AOM_ask_value_tags(tItemRev, "representation_for", &represent_num, &represent_object_list));

			int count = 0;
			for (size_t j = 0; j < represent_num; j++) {
				logical isVirtualRev = false;
				ITKCALL(GTCTYPE_is_type_of(represent_object_list[j], "D9_VirtualPartRevision", &isVirtualRev));
				if (isVirtualRev) {
					count++;
				}
			}

			if (count == 0) {
				sprintf(tempStr, G2B5("对象ID为:%s, 版本号为:%s, 没有关联虚拟料号,请先关联虚拟料号后再起草流程,谢谢!"), itemId, version);
			}
			else if (count > 1) {
				sprintf(tempStr, G2B5("对象ID为:%s, 版本号为:%s, 关联的虚拟料号超过一个,请移除多余的虚拟料号后再起草流程,谢谢!"), itemId, version);
			}

			tc_strcat(error_msg, tempStr);
		}

		if (tc_strcmp(error_msg, "") != 0) {
			TC_write_syslog("error tip return");
			decision = EPM_nogo;
//			error_msg[strlen(error_msg) - 1] = 0;
			EMH_store_error_s1(EMH_severity_user_error, VER_RULE_ERROR, error_msg);
			ifail = VER_RULE_ERROR;
			goto CLEANUP;
		}

	CLEANUP:
		DOFREE(task_name);
		DOFREE(attach_object_list);
		DOFREE(itemId);
		DOFREE(version);
		DOFREE(represent_object_list);
		DOFREE(attach_object_list);
		TC_write_syslog("\n=================   D9_CheckFWRelation   end   ==================\n");
		return decision;
	}

#ifdef __cplusplus
}
#endif
