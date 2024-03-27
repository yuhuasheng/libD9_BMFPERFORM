#include "extension.h"
#include "search.h"
#include "util.h"
#include <tc/folder.h>

int getFWFolder(char* ids, tag_t& folder);
extern "C" int POM_AM__set_application_bypass(logical bypass);

#ifdef __cplusplus
extern "C" {
#endif

	int D9_FWAutoArch(EPM_action_message_t msg)
	{
		int
			ifail = ITK_ok,
			attach_object_num = 0,
			represent_num = 0;			

		tag_t
			root_task = NULL_TAG,
			FWItemRev = NULL_TAG,
			FWItem = NULL_TAG,
			virtualPartRev = NULL_TAG,
			FWFolder = NULL_TAG;

		tag_t
			* attach_object_list = NULLTAG,
			* represent_object_list = NULLTAG;

		char
			* FWItemId = NULL,
			* FWVersion = NULL,
			* FWObjectName = NULL,
			* virtualPartItemId = NULL,
			* virtualPartVersion = NULL,
			* task_name = NULL,
			* ids = NULL;		

		string newObjectName = "";

		TC_write_syslog("\n=================   D9_FWAutoArch  start   ==================\n");

		POM_AM__set_application_bypass(true); // 开启旁路

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // 获取流程根任务
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &task_name));

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (size_t i = 0; i < attach_object_num; i++) {
			logical
				isRev = false;
			int markpoint_number = 0;
			FWItemRev = attach_object_list[i];

			ITKCALL(GTCTYPE_is_type_of(FWItemRev, "D9_FWDesignRevision", &isRev));
			if (!isRev) {
				continue;
			}

			ITKCALL(ITEM_ask_item_of_rev(FWItemRev, &FWItem));

			ITKCALL((AOM_ask_value_string(FWItemRev, "item_id", &FWItemId))); // 获取ID
			TC_write_syslog("FWItemId == %s\n", FWItemId);

			ITKCALL((AOM_ask_value_string(FWItemRev, "item_revision_id", &FWVersion))); // 获取版本号
			TC_write_syslog("FWVersion == %s\n", FWVersion);

			ITKCALL((AOM_ask_value_string(FWItemRev, "object_name", &FWObjectName))); // 获取版本号
			TC_write_syslog("FWObjectName == %s\n", U2G(FWObjectName));


			ITKCALL(AOM_ask_value_string(FWItemRev, "project_ids", &ids));
			strReplace(ids, " ", "");
			TC_write_syslog("ids == %s\n", ids);
			if (ids == NULL || tc_strcmp(ids, "") == 0) {
				TC_write_syslog("\n 零组件ID为: %s, 版本号为: %s, 未指派专案\n", FWItemId, FWVersion);
				goto CLEANUP;
			}


			ITKCALL(AOM_refresh(FWItemRev, false));

			ITKCALL(AOM_ask_value_tags(FWItemRev, "representation_for", &represent_num, &represent_object_list));

			for (size_t j = 0; j < represent_num; j++) {
				logical isVirtualRev = false;
				ITKCALL(GTCTYPE_is_type_of(represent_object_list[j], "D9_VirtualPartRevision", &isVirtualRev));
				if (isVirtualRev) {
					virtualPartRev = represent_object_list[j];
					break;
				}
			}

			if (virtualPartRev != NULL) {

				ITKCALL((AOM_ask_value_string(virtualPartRev, "item_id", &virtualPartItemId))); // 获取ID
				TC_write_syslog("virtualPartItemId == %s\n", virtualPartItemId);

				ITKCALL((AOM_ask_value_string(virtualPartRev, "item_revision_id", &virtualPartVersion))); // 获取版本号
				TC_write_syslog("virtualPartVersion == %s\n", virtualPartVersion);

				getFWFolder(ids, FWFolder);
				if (FWFolder == NULL) {
					TC_write_syslog("\n 获取FW文件夹失败\n");
					goto CLEANUP;
				}

				if (checkExistItem(FWFolder, FWItem, "contents")) {
					TC_write_syslog("\n 专案id为:%s, FW文件夹已经存在零组件ID为: %s, Firmware Design对象，无需重复添加\n", ids, FWItemId);
					goto CLEANUP;
				}
				try {
					ITKCALL(POM_place_markpoint(&markpoint_number))
					TC_write_syslog("设置markpoint[%d]", markpoint_number);
					
					newObjectName = (string)virtualPartItemId + "_" + (string)FWObjectName;
					updateProp(FWItem, "object_name", newObjectName.c_str()); // 修改属性
					updateProp(FWItemRev, "object_name", newObjectName.c_str()); // 修改属性				

					ITKCALL(AOM_refresh(FWFolder, false));
					ITKCALL(AOM_lock(FWFolder)) //锁定设计对象
					ITKCALL(AOM_load(FWFolder));
					ITKCALL(FL_insert(FWFolder, FWItem, 999));
					ITKCALL(AOM_save_with_extensions(FWFolder));
					ITKCALL(AOM_unlock(FWFolder)); //解锁设计对象


					ITKCALL(POM_forget_markpoint(markpoint_number)); //释放markpoint
					TC_write_syslog("释放markpoint[%d]", markpoint_number);
				}
				catch (int& error_code) {
					ifail = error_code;

					//回滚markpoint
					try {
						logical state_has_changed = true;//默认TC中数据有变化
						ITKCALL(POM_roll_to_markpoint(markpoint_number, &state_has_changed));
						if (!state_has_changed) {
							TC_write_syslog("回滚markpoint[%d]成功", markpoint_number);
						}
						else {
							TC_write_syslog("回滚markpoint[%d]失败", markpoint_number);
						}
					}
					catch (int& rollbackerror_code) {
						ifail = rollbackerror_code;
						TC_write_syslog("POM_roll_to_markpoint markpoint[%d] fail", markpoint_number);
					}
				}

			}
		}		

	CLEANUP:
		DOFREE(task_name);
		DOFREE(attach_object_list);
		DOFREE(FWItemId);
		DOFREE(FWVersion);
		DOFREE(FWObjectName);
		DOFREE(represent_object_list);
		DOFREE(attach_object_list);
		POM_AM__set_application_bypass(false); // 开启旁路
		TC_write_syslog("\n=================   D9_FWAutoArch  end   ==================\n");
		return ifail;
	}
#ifdef __cplusplus
}
#endif


/**
* 获取专案文件夹下的FW文件夹
*/
int getFWFolder(char* ids, tag_t& folder) {
	int rcode = ITK_ok;
	char** qry_entries = (char**)MEM_alloc(sizeof(char**));
	char** qry_values = (char**)MEM_alloc(sizeof(char**));
	qry_entries[0] = (char*)MEM_alloc(sizeof(char*) * 200);
	qry_values[0] = (char*)MEM_alloc(sizeof(char*) * 200);

	tc_strcpy(qry_entries[0], SPAS_ID);
	tc_strcpy(qry_values[0], ids);

	char
		* objectName = NULL;

	tag_t
		* queryResults = NULLTAG,
		* contents = NULLTAG;

	tag_t
		projectFolder = NULLTAG;

	int
		resultLength = 0,
		count = 0;

	ITKCALL(query(D9_FIND_PROJECT_FOLDER, qry_entries, qry_values, 1, &queryResults, &resultLength));
	TC_write_syslog(" -- >> project ids : %s queryresult : %d \n", ids, resultLength);

	if (resultLength > 0) {
		projectFolder = queryResults[0];
		AOM_ask_value_tags(projectFolder, "contents", &count, &contents);
		for (size_t i = 0; i < count; i++) {
			logical isFolder = false;
			GTCTYPE_is_type_of(contents[i], "Folder", &isFolder);
			if (!isFolder) {
				continue;
			}

			ITKCALL(AOM_ask_value_string(contents[i], "object_name", &objectName)); // 获取文件夹名称
			TC_write_syslog("objectName == %s\n", U2G(objectName));

			if (tc_strcmp(U2G(objectName), "FW") == 0) {
				folder = contents[i];
				break;
			}
		}
	}

CLEANUP:	
	DOFREE(contents);
	DOFREE(objectName);
	return rcode;
}
