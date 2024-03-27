#include "extension.h"
#include "util.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);

#ifdef __cplusplus
extern "C" {
#endif

	int D9_SetMaterialGroupInfo(EPM_action_message_t msg)
	{
		int
			status = ITK_ok,
			iAttachmentCount = 0;

		logical
			isRev = false,
			isDS = false,
			isBOMViewRev = false;

		tag_t
			tRootTask = NULL_TAG,
			tTargetObj = NULL_TAG,
			tItemRev = NULL_TAG,
			tGroup = NULL_TAG;

		tag_t
			*tAttachments = NULL;

		char
			*objectBU = NULL,
			*groupName = NULL;			

		vector<string>*
			materialGroupPreVec = new vector<string>();


		POM_AM__set_application_bypass(true);

		cout << "D9_SetMaterialGroupInfo START : " << endl;
		TC_write_syslog("D9_SetMaterialGroupInfo START : \n");

		// 获取Group信息
		POM_ask_default_group(&tGroup);
		POM_ask_group_name(tGroup, &groupName);

		// 获取首选项
		getPreferenceByName("D9_Material_Group_BU_Assign", *materialGroupPreVec);
		cout << "materialGroupPreVec, size is : " << materialGroupPreVec->size() << endl;
		TC_write_syslog("materialGroupPreVec, size is : %d\n", materialGroupPreVec->size());

		// 获取Material Group
		string materialGroup = getMaterialGroup(groupName, *materialGroupPreVec);
		cout << "materialGroup is : " << materialGroup.c_str() << endl;
		TC_write_syslog("materialGroup is : %s\n", materialGroup.c_str());


		// 获取目标对象
		EPM_ask_root_task(msg.task, &tRootTask);
		EPM_ask_attachments(tRootTask, EPM_target_attachment, &iAttachmentCount, &tAttachments);
		for (size_t i = 0; i < iAttachmentCount; i++) {
			
			tTargetObj = NULL_TAG;
			isRev = false;
			isDS = false;
			isBOMViewRev = false;

			tTargetObj = tAttachments[i];

			GTCTYPE_is_type_of(tTargetObj, "Dataset", &isDS);
			if (isDS)
				continue;

			GTCTYPE_is_type_of(tTargetObj, "PSBOMViewRevision", &isBOMViewRev);
			if (isBOMViewRev)
				continue;

			GTCTYPE_is_type_of(tTargetObj, "ItemRevision", &isRev);
			if (!isRev) {
				ITKCALL(ITEM_ask_latest_rev(tTargetObj, &tItemRev));
			}
			else
			{
				tItemRev = tTargetObj;
			}
			if (NULL_TAG == tItemRev)
				continue;

			// 获取相应的文件夹名称		
			ITKCALL(AOM_ask_value_string(tItemRev, "d9_BU", &objectBU));
			cout << "get MaterialGroup is : " << objectBU << endl;
			TC_write_syslog("get MaterialGroup is : %s\n", objectBU);

			if (tc_strcmp(objectBU, "") == 0) {
				ITKCALL(AOM_lock(tItemRev));
				ITKCALL(AOM_load(tItemRev));
				ITKCALL(AOM_set_value_string(tItemRev, "d9_BU", materialGroup.c_str()));
				ITKCALL(AOM_save_with_extensions(tItemRev));
				ITKCALL(AOM_unlock(tItemRev));
				
				
				cout << "set MaterialGroup is : " << materialGroup.c_str() << endl;
				TC_write_syslog("set MaterialGroup is : %s\n", materialGroup.c_str());				
			}			

		}

	CLEANUP:
		POM_AM__set_application_bypass(false);

		if (objectBU != NULL) MEM_free(objectBU);
		if (groupName != NULL) MEM_free(groupName);

		materialGroupPreVec->clear(); materialGroupPreVec = NULL;
		
		cout << "D9_SetMaterialGroupInfo END : " << endl;
		TC_write_syslog("D9_SetMaterialGroupInfo END : \n");

		return status;
	}

#ifdef __cplusplus
}
#endif