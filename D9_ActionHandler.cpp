#include "extension.h"
#include "util.h"
#include <tc/folder.h>
#include <curl/curl.h>

boolean isExistItem(tag_t folder_tag, tag_t item_tag)
{
	char *itemUID = NULL;
	tag_t* tContents = NULL;
	int iCount = 0;
	AOM_ask_value_tags(folder_tag, "contents", &iCount, &tContents);
	ITK__convert_tag_to_uid(item_tag, &itemUID);
	for (size_t i = 0; i < iCount; i++) {
		tag_t tContent = tContents[i];
		char *compareUID = NULL;
		ITK__convert_tag_to_uid(tContent, &compareUID);
		if (strcmp(itemUID, compareUID) == 0)
		{
			return true;
		}
	}
	return false;
}
string isInUserGroup(vector<string> vec, string group) {
	string::size_type idx;
	for (int i = 0; i < vec.size(); i++) {
		vector<string> spl = split(vec[i], "=");
		string g = spl[0];
		string brand = spl[1];
		idx = group.find(g);
		if (idx != string::npos) {
			return brand;
		}
	}
	return "";
}

bool isInObjectType(vector<string> vec, string objectType) {
	string::size_type idx;
	for (int i = 0; i < vec.size(); i++) {
		idx = objectType.find(vec[i]);
		if (idx != string::npos) {
			return true;
		}
	}
	return false;
}


extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_AutoArchiveByRCP(EPM_action_message_t msg) {
		int 
			status = ITK_ok,
			iAttachmentCount = 0,
			n_entries = 2,
			int_resutl_num = 0;

		logical
			isRev = false,
			isDS = false,
			isBOMViewRev = false;

		tag_t
			tRootTask = NULL_TAG,
			tTargetObj = NULL_TAG,
			tItem = NULL_TAG,
			tItemRev = NULL_TAG;

		tag_t 
			*tAttachments = NULL,
			*tResult = NULL;

		char
			*value = (char *)MEM_alloc(sizeof(char) * 128),
			*objectName = NULL,
			*itemProjectValue = (char *)MEM_alloc(sizeof(char) * 100),
			*folderProjectValue = (char *)MEM_alloc(sizeof(char) * 100),
			*targetType = NULL;

		char 
			**attr_names = (char **)MEM_alloc(sizeof(char *) * n_entries),
			**attr_values = (char **)MEM_alloc(sizeof(char *) * n_entries);

		vector<string>*
			folderPreVec = new vector<string>();

		map<string, string>*
			folderPreMap = new map<string, string>();		

		string
			folderName = "",
			findName = "";

		vector<string> 
			nameLst;

		POM_AM__set_application_bypass(true);

		// 获取首选项		
		memset(value, 0, 128);		
		getPreferenceByName("D9_Design_Archive_Rule", *folderPreVec);
		getHashMapPreference(*folderPreVec, *folderPreMap);

		// 获取目标对象
		EPM_ask_root_task(msg.task, &tRootTask);
		EPM_ask_attachments(tRootTask, EPM_target_attachment, &iAttachmentCount, &tAttachments);
		for (size_t i = 0; i < iAttachmentCount; i++) {
			vector<string> *itemProjectVec = new vector<string>();
			map<string, string>::iterator iter;

			tTargetObj = NULL_TAG;
			tItemRev = NULL_TAG;			
			findName = "";
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
			// 获取对象项目信息				
			getProjectInfo(tItemRev, *itemProjectVec);			
			// 获取相应的文件夹名称		
			ITKCALL(AOM_ask_value_string(tItemRev, "item_id", &objectName));
			nameLst = split(objectName, "-");
			if (nameLst.size() > 2) {
				findName = nameLst[0] + "-" + nameLst[1];
			}

			iter = folderPreMap->find(findName);
			if (iter != folderPreMap->end()) {
				folderName = iter->second;
				cout << "Find, the value is " << iter->second << endl;
				TC_write_syslog("Find, the value is %s\n", folderName);
			}
			else {
				cout << "Do not Find" << endl;
				TC_write_syslog("Do not Find\n");
			}				

			// 查找文件夹						
			attr_names[0] = (char *)MEM_alloc(sizeof(char) * 256);
			attr_names[1] = (char *)MEM_alloc(sizeof(char) * 256);
			memset(attr_names[0], 0, sizeof(char) * 256);
			memset(attr_names[1], 0, sizeof(char) * 256);
			//tc_strcpy(attr_names[0], "Type");
			//tc_strcpy(attr_names[1], "Description");
			//tc_strcpy(attr_names[0], G2U("类型"));
			//tc_strcpy(attr_names[1], G2U("描述"));
			tc_strcpy(attr_names[0], "FolderType");
			tc_strcpy(attr_names[1], "Desc");
			attr_values[0] = (char *)MEM_alloc(sizeof(char) * 256);
			attr_values[1] = (char *)MEM_alloc(sizeof(char) * 256);
			memset(attr_values[0], 0, sizeof(char) * 256);
			memset(attr_values[1], 0, sizeof(char) * 256);
			tc_strcpy(attr_values[0], "D9_WorkAreaFolder");	
			//tc_strcpy(attr_values[0], "Folder");
			tc_strcpy(attr_values[1], folderName.c_str());						
			
			//query("General...", attr_names, attr_values, n_entries, &tResult, &int_resutl_num);
			query("__D9_Find_Folder", attr_names, attr_values, n_entries, &tResult, &int_resutl_num); 
			printf("search  event resultNum == %d\n", int_resutl_num);
			TC_write_syslog("search  event resultNum == %d\n", int_resutl_num);

			printf("itemProjectVec size is: %d\n", (*itemProjectVec).size());
			for (size_t j = 0; j < (*itemProjectVec).size(); j++)
			{
				// 获取目标对象项目信息
				tc_strcpy(itemProjectValue, (*itemProjectVec)[j].c_str());
				TC_write_syslog("itemProjectValue is %s\n", itemProjectValue);
				printf("itemProjectValue is: %s\n", itemProjectValue);

				// 获取文件夹对象项目信息
				for (size_t k = 0; k < int_resutl_num; k++) {
					vector<string> *folderProjectVec = new vector<string>();
					getProjectInfo(tResult[k], *folderProjectVec);
					for (size_t m = 0; m < (*folderProjectVec).size(); m++)
					{
						// 获取目标对象项目信息
						tc_strcpy(folderProjectValue, (*folderProjectVec)[m].c_str());
						TC_write_syslog("folderProjectValue is %s\n", folderProjectValue);
						printf("folderProjectValue is: %s\n", folderProjectValue);

						// 项目信息比对
						if (tc_strcmp(itemProjectValue, folderProjectValue) == 0) {
							// 对象版本获取对象
							ITKCALL(ITEM_ask_item_of_rev(tItemRev, &tItem));

							// 判断文件夹是否关联对象
							if (!isExistItem(tResult[k], tItem)) {
								status = AOM_refresh(tResult[k], false);
								status = FL_insert(tResult[k], tItem, 999);
								status = AOM_save_with_extensions(tResult[k]);
								status = AOM_unload(tResult[k]);
							}
						}
					}
					
					folderProjectVec->clear();
					folderProjectVec = NULL;
				}
			}
			
			itemProjectVec->clear();
			itemProjectVec = NULL;
		}

	CLEANUP:
		POM_AM__set_application_bypass(false);

		folderPreVec->clear();
		folderPreMap->clear();		
		nameLst.clear();

		folderPreVec = NULL;
		folderPreMap = NULL;

		if (value != NULL) MEM_free(value);
		if (objectName != NULL) MEM_free(objectName);
		if (itemProjectValue != NULL) MEM_free(itemProjectValue);
		if (folderProjectValue != NULL) MEM_free(folderProjectValue);
		if (targetType != NULL) MEM_free(targetType);
		if (attr_names != NULL) MEM_free(attr_names);
		if (attr_values != NULL) MEM_free(attr_values);

		return status;
	}

	int D9_SystemDesignArch(EPM_action_message_t msg) {
		int status = ITK_ok;
		POM_AM__set_application_bypass(true);
		vector<string> seArchTypeVec;
		vector<string> seArchUserVec;
		vector<string> seArchAdminVec;
		vector<string> seArchObjectTypeVec;
		tag_t root_task = NULLTAG;
		tag_t owning_group_tag = NULLTAG;
		int att_count = 0;
		tag_t *atts = NULL;
		char objectStr7[8];
		objectStr7[7] = '\0';
		char *object_string = NULL;
		char *taskname = NULL;
		char *owning_group = NULL;
		getPreferenceByName("D9_Item_SeArch_Type", seArchTypeVec);
		getPreferenceByName("D9_User_SeArch", seArchUserVec);
		getPreferenceByName("D9_Admin_SeArch", seArchAdminVec);
		getPreferenceByName("D9_Item_SeArch_Object_Type", seArchObjectTypeVec);
		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &att_count, &atts));
		ITKCALL(AOM_ask_value_string(root_task, "object_name", &taskname));
		printf("开始处理流程：%s\n", taskname);
		for (int i = 0; i < att_count; i++) {
			tag_t target = atts[i];
			ITKCALL(AOM_ask_value_string(target, "object_string", &object_string));
			strncpy(objectStr7, object_string, 7);
			printf("--处理目标：%s(前7位%s)\n", object_string, objectStr7);
			vector<string>::iterator it;
			it = find(seArchTypeVec.begin(), seArchTypeVec.end(), objectStr7);
			if (it == seArchTypeVec.end()) {
				printf("--目标(%s)不在D9_Item_SeArch_Type首选项中，不处理\n", objectStr7);
				continue;
			}
			ITKCALL(AOM_ask_value_tag(target, "owning_group", &owning_group_tag));
			ITKCALL(AOM_ask_value_string(owning_group_tag, "full_name", &owning_group));
			printf("--目标(%s)的用户组是：%s\n", objectStr7, owning_group);
			string brand = isInUserGroup(seArchUserVec, owning_group);
			if (brand.empty()) {
				printf("--目标(%s)的用户组不在D9_User_SeArch首选项中，不处理\n", objectStr7);
				continue;
			}
			char  **queryTypeName = (char **)MEM_alloc(sizeof(char*) * 1);
			char  **queryTypeValue = (char **)MEM_alloc(sizeof(char*) * 1);
			queryTypeName[0] = (char *)MEM_alloc(sizeof(char*) * 64);
			queryTypeValue[0] = (char *)MEM_alloc(sizeof(char*) * 64);
			queryTypeName[1] = (char *)MEM_alloc(sizeof(char*) * 64);
			queryTypeValue[1] = (char *)MEM_alloc(sizeof(char*) * 64);
			tc_strcpy(queryTypeName[0], "Desc");
			string t = objectStr7;
			t.append("=");
			t.append(brand);
			tc_strcpy(queryTypeValue[0], t.c_str());
			tc_strcpy(queryTypeName[1], "AdminName");
			tc_strcpy(queryTypeValue[1], seArchAdminVec[0].c_str());
			tag_t*	resultTags = NULL;
			int resultCount = 0;
			query("__D9_Find_Std_Folder", queryTypeName, queryTypeValue, 1, &resultTags, &resultCount);
			if (resultCount != 0) {
				char *s = NULL;
				ITKCALL(AOM_ask_value_string(resultTags[0], "object_name", &s));
				printf("--查询到文件夹:%s\n", s);
				DOFREE(s);
				tag_t stdFolder = resultTags[0];
				if (!isExistItem(stdFolder, target)) {
					char *objectType = NULL;
					AOM_ask_value_string(target, "object_type", &objectType);
					if (!isInObjectType(seArchObjectTypeVec, objectType)) {
						printf("--目标(%s)的类型是%s,不在首选项D9_Item_SeArch_Object_Type中，跳过\n", objectStr7, objectType);
						continue;
					}					
					DOFREE(objectType);
					tag_t parentTag = NULLTAG;
					ITKCALL(AOM_ask_value_tag(target, "items_tag", &parentTag));
					printf("--目标(%s)正在挂载\n", objectStr7);
					//复制到文件夹
					AOM_refresh(stdFolder, true);
					FL_insert(stdFolder, parentTag, 999);
					ITKCALL(AOM_save_with_extensions(stdFolder));
					ITKCALL(AOM_unlock(stdFolder));
					printf("--目标(%s)挂载成功！\n", objectStr7);
					parentTag = NULL;
				}
				else {
					printf("--目标(%s)重复挂载,结束\n", objectStr7);
				}
				stdFolder = NULL;
				

			}
			else {
				printf("--目标(%s)没有对应文件夹,结束\n", objectStr7);
			}
			target = NULL;
			DOFREE(queryTypeName);
			printf("--释放queryTypeName\n");
			DOFREE(queryTypeValue);
			printf("--释放queryTypeValue\n");
			DOFREE(resultTags);
			printf("--释放resultTags\n");

		}
		
	CLEANUP:
		POM_AM__set_application_bypass(false);
		root_task = NULL;
		owning_group_tag = NULL;
		atts = NULL;
		// delete[](objectStr7);
		printf("--释放objectStr7\n");
		DOFREE(object_string);
		printf("--释放object_string\n");
		DOFREE(owning_group);
		printf("--释放owning_group\n");
		DOFREE(taskname);
		printf("--释放taskname\n");
		
		return status;
	}



	

	int SendPublicMail(EPM_action_message_t msg) {
		int status = ITK_ok;

		tag_t
			tTask = NULL_TAG,
			tRootTask = NULL_TAG,
			tAttach = NULL_TAG,
			tDataSheetRelType = NULL_TAG,
			tDatasetType = NULL_TAG,
			tRefObject = NULL_TAG,
			*tAttachments = NULL,
			*tDatasets = NULL,
			*tRefs = NULL;

		char
			*char_object_type = NULL,
			*char_taskname = NULL,
			*char_taskparentname = NULL,
			*char_datasetTypeName = NULL,
			*char_ext = NULL,
			*char_dir_path = NULL,
			*char_file_name_path = NULL,
			*char_original_name = NULL,
			**char_refNames = NULL,
			char_cmd[200] = "",
			char_cmd_result[100] = "",
			char_taskinfo[100] = "";

		int
			iAttachmentCount = 0,
			iDatasetNum = 0,
			iRefCount = 0;

		AE_reference_type_t reference_type;

		tTask = msg.task;

		ITKCALL(WSOM_ask_object_type2(tTask, &char_object_type));
		ITKCALL(AOM_ask_value_string(tTask, "object_name", &char_taskname));
		ITKCALL(AOM_ask_value_string(tTask, "parent_name", &char_taskparentname));
		tc_strcat(char_taskinfo, char_taskname);
		tc_strcat(char_taskinfo, ",");
		tc_strcat(char_taskinfo, char_taskparentname);

		EPM_ask_root_task(tTask, &tRootTask);
		EPM_ask_attachments(tRootTask, EPM_target_attachment, &iAttachmentCount, &tAttachments);

		if (iAttachmentCount > 0) {
			for (int i = 0; i < iAttachmentCount; i++) {
				tAttach = tAttachments[i];
				if (tAttach != NULL_TAG) {
					ITKCALL(WSOM_ask_object_type2(tAttach, &char_object_type));
					if (tc_strcasecmp(char_object_type, "ItemRevision") == 0) {
						ITKCALL(GRM_find_relation_type("IMAN_specification", &tDataSheetRelType));
						ITKCALL(GRM_list_secondary_objects_only(tAttach, tDataSheetRelType, &iDatasetNum, &tDatasets));
						for (int j = 0; j < iDatasetNum; j++) {
							ITKCALL(WSOM_ask_object_type2(tDatasets[j], &char_datasetTypeName));
							if (tc_strcasecmp(char_datasetTypeName, "Text") == 0) {
								ITKCALL(AE_find_datasettype2(char_datasetTypeName, &tDatasetType));
								ITKCALL(AE_ask_datasettype_refs(tDatasetType, &iRefCount, &char_refNames));
								ITKCALL(AOM_ask_value_tags(tDatasets[j], "ref_list", &iRefCount, &tRefs));
								ITKCALL(AOM_ask_value_string(tRefs[0], "file_ext", &char_ext));																
								
								ITKCALL(AE_ask_dataset_named_ref2(tDatasets[j], char_refNames[0], &reference_type, &tRefObject));
								ITKCALL(IMF_ask_original_file_name2(tRefObject, &char_original_name));
								if (tc_strcasecmp(char_original_name, "PublicMial.txt") == 0) {
									char_dir_path = getenv("TEMP");
									export_dataset_file(tDatasets[j], char_refNames[0], char_ext, &char_file_name_path, &char_original_name, char_dir_path);
									sprintf(char_cmd, "java -jar %%TC_ROOT%%\\portal\\\plugins\\sendmail.jar \"%s\" \"%s\"", char_taskinfo, char_file_name_path);
									TC_write_syslog("[info]char_cmd=%s \n", char_cmd);
									if (1 == execmd(char_cmd, char_cmd_result)) {
										TC_write_syslog("[info]char_cmd_result = %s\n", char_cmd_result);
									}
									else {
										ITKCALL(EMH_store_error_s2(EMH_severity_warning, ERRORCODE_MESSAGE, "\n执行下面的命令失败:", char_cmd));
										return EPM_nogo;
									}
								}								
							}
						}
					}
				}
			}
		}

	CLEANUP:
		tAttachments = NULL;
		tDatasets = NULL;
		tRefs = NULL;
		char_dir_path = NULL;
		char_file_name_path = NULL;
		char_original_name = NULL;

		DOFREE(char_object_type);
		DOFREE(char_taskname);
		DOFREE(char_taskparentname);
		DOFREE(char_datasetTypeName);
		DOFREE(char_refNames);
		DOFREE(char_ext);
		//DOFREE(char_dir_path);
		//DOFREE(char_file_name_path);
		//DOFREE(char_original_name);

		return status;
	}

#ifdef __cplusplus
}
#endif