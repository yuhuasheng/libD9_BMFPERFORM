#include "extension.h"
#include "search.h"
#include "util.h"
#include <tc/folder.h>

// #define RepeateReleaseMatRecord "重复发行物料记录.txt"
#define RepeateReleaseMatRecord "Repeate_Release_Material_Record.txt"

extern "C" int POM_AM__set_application_bypass(logical bypass);
int getItemList(tag_t itemRev, vector<tag_t>& list, const char* propName);
int filterSolutionList(vector<tag_t> solutionList, vector<tag_t>& solutionReleaseList, vector<tag_t>& solutionBomList);
int filterProblemAndImpact(vector<tag_t> solutionReleaseList, vector<tag_t>& list, const char* propName);
int removeItemRevByRelation(tag_t itemRev, vector<tag_t> list, const char* propName);
vector<string> recordPartAllDCN(vector<tag_t> list, char* curDCNId);
tag_t checkDataset(tag_t itemRev);
int getSolutionBomNotReleaseItem(vector<tag_t> solutionBomList, vector<tag_t>& solutionNotReleaseList);
int checkItemRevExist(vector<tag_t> sourceList, vector<tag_t>& targetList, const char* propName);
int getLastItemRevList(vector<tag_t> curItemRevList, vector<tag_t>& lastItemRevList);
int addItemRevByRelation(tag_t itemRev, vector<tag_t> list, const char* propName);

#ifdef __cplusplus
extern "C" {
#endif

	int D9_Check_MNT_DCN(EPM_action_message_t msg)
	{
		int
			ifail = ITK_ok,
			attach_object_num = 0;

		tag_t
			target_obj = NULLTAG,
			root_task = NULL_TAG,
			MNT_DCN_Revision = NULLTAG,
			recordDataset = NULLTAG;


		tag_t
			* attach_object_list = NULLTAG;

		char
			* task_name = NULL,
			* curDcnId = NULL,
			* version = NULL,
			* fullfilename = NULL;

		string
			logFilePath;


		vector<tag_t>
			solutionList,
			solutionReleaseList,
			solutionNotReleaseList,
			solutionBomList,
			problemList,
			impactedList,
			lastItemRevList,
			problemExistLastItemRevList;
			

		TC_write_syslog("\n=================   D9_Check_MNT_DCN  start   ==================\n");

		POM_AM__set_application_bypass(true); // 开启旁路

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // 获取流程根任务
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &task_name));

		TC_write_syslog("\n 开始处理流程：%s\n", task_name);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (size_t i = 0; i < attach_object_num; i++) {
			logical
				isMNTDCNRev = false;
			target_obj = attach_object_list[i];

			ITKCALL(GTCTYPE_is_type_of(target_obj, "D9_MNT_DCNRevision", &isMNTDCNRev));
			if (isMNTDCNRev) {
				MNT_DCN_Revision = target_obj;
				break;
			}
		}


		if (MNT_DCN_Revision != NULLTAG)
		{
			ITKCALL((AOM_ask_value_string(MNT_DCN_Revision, "item_id", &curDcnId))); // 获取ID
			TC_write_syslog("curDcnId == %s\n", curDcnId);

			ITKCALL((AOM_ask_value_string(MNT_DCN_Revision, "item_revision_id", &version))); // 获取ID
			TC_write_syslog("version == %s\n", version);

			ITKCALL(getItemList(MNT_DCN_Revision, solutionList, "CMHasSolutionItem")); // 获取解决方案项中对象版本集合

			ITKCALL(filterSolutionList(solutionList, solutionReleaseList, solutionBomList));// 过滤出已经发行的物料集合，含有BOM结构的集合

			ITKCALL(getItemList(MNT_DCN_Revision, problemList, "CMHasProblemItem")); // 获取问题项中的对象版本
			ITKCALL(getItemList(MNT_DCN_Revision, impactedList, "CMHasImpactedItem")); // 获取受影响项中的对象版本

			ITKCALL(filterProblemAndImpact(solutionReleaseList, problemList, "CMHasProblemItem")); // 过滤问题项的对象集合
			ITKCALL(filterProblemAndImpact(solutionReleaseList, impactedList, "CMHasImpactedItem")); // 过滤受影响项的对象集合			

			ITKCALL(removeItemRevByRelation(MNT_DCN_Revision, solutionReleaseList, "CMHasSolutionItem")); // 移除解决方案项下的部分对象
			ITKCALL(removeItemRevByRelation(MNT_DCN_Revision, problemList, "CMHasProblemItem")); // 移除问题项下的部分对象
			ITKCALL(removeItemRevByRelation(MNT_DCN_Revision, impactedList, "CMHasImpactedItem")); // 移除受影响项下的部分对象

			ITKCALL(getSolutionBomNotReleaseItem(solutionBomList, solutionNotReleaseList)); // 获取解决方案项BOM中未发行的对象集合

			ITKCALL(checkItemRevExist(solutionList, solutionNotReleaseList, "CMHasSolutionItem")); // 判断对象是否在解决方案项中已经存在
			

			ITKCALL(getLastItemRevList(solutionNotReleaseList, lastItemRevList)); // 获取解决方案项中新增的对象当前版本的上一个版本

			ITKCALL(getItemList(MNT_DCN_Revision, problemList, "CMHasProblemItem")); // 重新获取问题项中的对象版本
//			ITKCALL(getItemList(MNT_DCN_Revision, impactedList, "CMHasImpactedItem")); // 重新获取受影响项中的对象版本

			problemExistLastItemRevList.assign(lastItemRevList.begin(), lastItemRevList.end()); // vector集合的复制
//			impactExistLastItemRevList.assign(lastItemRevList.begin(), lastItemRevList.end());

			ITKCALL(checkItemRevExist(problemList, problemExistLastItemRevList, "CMHasProblemItem")); // 判断对象是否存在问题项中
//			ITKCALL(checkItemRevExist(impactedList, impactExistLastItemRevList, "CMHasImpactedItem")); // 判断对象是否存在受影响项中

			ITKCALL(addItemRevByRelation(MNT_DCN_Revision, solutionNotReleaseList, "CMHasSolutionItem")); // 解决方案项添加未发行对象
			ITKCALL(addItemRevByRelation(MNT_DCN_Revision, problemExistLastItemRevList, "CMHasProblemItem")); // 问题项添加未发行对象上一版本
//			ITKCALL(addItemRevByRelation(MNT_DCN_Revision, impactExistLastItemRevList, "CMHasImpactedItem")); // 受影响项项添加未发行对象上一版本

			if (solutionReleaseList.size() <=0)
			{
				goto CLEANUP;
			}

			vector<string> logList = recordPartAllDCN(solutionReleaseList, curDcnId);
			if (logList.size() <= 0)
			{
				goto CLEANUP;
			}

			string dir = getTempFolder((string)curDcnId + "_" + (string)version + "_log");
			TC_write_syslog("dir == %s\n", dir);

			removeDir(dir.c_str()); // 删除目录中的文件

			logFilePath = dir + "\\" + (string)RepeateReleaseMatRecord;
			TC_write_syslog("logFilePath == %s\n", logFilePath);

			bool create = createFile(logFilePath); // 创建文件
			if (!create)
			{
				TC_write_syslog("记录存放记录移除解决方案项发行物料的日志文件创建失败\n");
				goto CLEANUP;
			}

			const int len = logFilePath.length();
			fullfilename = new char[len + 1];
			strcpy(fullfilename, logFilePath.c_str());

			appendContent(logFilePath.c_str(), logList); // 将日志内容追加到txt中

			recordDataset = checkDataset(MNT_DCN_Revision);

			if (recordDataset == NULL)
			{
				create_dataset("Text", "Text", RepeateReleaseMatRecord, fullfilename, "IMAN_specification", MNT_DCN_Revision, &recordDataset); // 创建数据集
			}
			
			ITKCALL(RES_checkout2(recordDataset, "", "", "", RES_EXCLUSIVE_RESERVE)); // 签出数据集
			import_dataset_file(recordDataset, "Text", "txt", fullfilename, RepeateReleaseMatRecord);
			ITKCALL(RES_checkin(recordDataset)); // 签入数据集
		}
	CLEANUP:
		DOFREE(task_name);
		DOFREE(attach_object_list);		
		DOFREE(curDcnId);
		DOFREE(version);
		POM_AM__set_application_bypass(false); // 关闭旁路
		solutionList.clear();
		solutionReleaseList.clear();
		solutionNotReleaseList.clear();
		solutionBomList.clear();
		problemList.clear();
		impactedList.clear();
		lastItemRevList.clear();
		problemExistLastItemRevList.clear();
		TC_write_syslog("\n=================   D9_Check_MNT_DCN  end   ==================\n");
		return ifail;
	}
#ifdef __cplusplus
}
#endif


/*
* 使用引用传递减创建拷贝vector的操作，提高运行速度
*/
int getItemList(tag_t itemRev, vector<tag_t>& list, const char* propName) {
	int 
		rcode = ITK_ok,
		tagCount = 0;

	tag_t
		* tag_list = NULLTAG;

	ITKCALL(AOM_ask_value_tags(itemRev, propName, &tagCount, &tag_list));

//	vector<tag_t>* itemList = new vector<tag_t>();
	for (size_t i = 0; i < tagCount; i++)
	{
		logical
			isRev = false;

		ITKCALL(GTCTYPE_is_type_of(tag_list[i], "ItemRevision", &isRev));
		if (isRev)
		{
			list.push_back(tag_list[i]);

			//if (isSolutionFlag) {
			//	if (isRelease(tag_list[i])) // 假如是解决方案项，需要进一步判断对象是否已经发行
			//	{
			//		list.push_back(tag_list[i]);
			//	}

			//	if (isSolutionBom) 
			//	{
			//		if (isBom(tag_list[i])) // 判断是否为BOM
			//		{
			//			list.push_back(tag_list[i]);
			//		}
			//	}
			//}
			//else {
			//	list.push_back(tag_list[i]);
			//}
			
		}
	}

	CLEANUP:
		DOFREE(tag_list);
		return rcode;
}



/*
* 过滤解决方案项中的零组件集合，过滤出已经发行的物料集合，含有BOM结构的集合
*/
int filterSolutionList(vector<tag_t> solutionList, vector<tag_t>& solutionReleaseList, vector<tag_t>& solutionBomList) {
	int
		rcode = ITK_ok;

	tag_t
		target_obj = NULLTAG;


	for (size_t i = 0; i < solutionList.size(); i++) {
		target_obj = solutionList[i];

		if (isRelease(target_obj)) { // 判断是否已经发行
			solutionReleaseList.push_back(target_obj);
		}

		if (isBom(target_obj)) { // 判断是否为BOM
			solutionBomList.push_back(target_obj);
		}	
	}

CLEANUP:
	return rcode;
}


/*
* 过滤问题项和受影响项集合记录
*/
int filterProblemAndImpact(vector<tag_t> solutionReleaseList, vector<tag_t>& list, const char* propName) {
	int 
		rcode = ITK_ok;

	char
		* sourceItemId = NULL,
		* targetItemId = NULL;

	for (auto it = list.begin(); it != list.end();) {
		tag_t target = *it;
		ITKCALL((AOM_ask_value_string(target, "item_id", &targetItemId)));
		auto flag = find_if(solutionReleaseList.begin(), solutionReleaseList.end(), [&](const tag_t& itemRev) {
			ITKCALL((AOM_ask_value_string(itemRev, "item_id", &sourceItemId))); // 获取ID
			return tc_strcmp(sourceItemId, targetItemId) == 0;
		});

		bool found = flag != solutionReleaseList.end();

		if (!found)
		{
			it = list.erase(it);

			if (it == list.end()) {
				break;
			}
		}
		else
		{
			TC_write_syslog("关系名为:%s, 零组件ID为:%s,匹配成功\n", propName, targetItemId);
			it++;			
		}
	}

CLEANUP:
	DOFREE(sourceItemId);
	DOFREE(targetItemId);
	return rcode;
}


/*
* 移除关系
*/
int removeItemRevByRelation(tag_t itemRev, vector<tag_t> list, const char* propName) {
	int 
		rcode = ITK_ok;

	tag_t
		relType = NULLTAG,
		target_obj = NULLTAG,
		relation = NULLTAG;	
	char
		* itemId = NULL,
		* version = NULL;

	ITKCALL(AOM_lock(itemRev)) //锁定设计对象
	ITKCALL(GRM_find_relation_type(propName, &relType)); //返回指定关系类型名称物件的关系类型
	for (size_t i = 0; i < list.size(); i++)
	{
		target_obj = list[i];
		ITKCALL((AOM_ask_value_string(target_obj, "item_id", &itemId))); // 获取ID
		TC_write_syslog("itemId == %s\n", itemId);

		ITKCALL((AOM_ask_value_string(target_obj, "item_revision_id", &version))); // 获取版本
		TC_write_syslog("version == %s\n", version);

		ITKCALL(GRM_find_relation(itemRev, target_obj, relType, &relation));
		if (relation != NULL)
		{
			ITKCALL(GRM_delete_relation(relation));
			TC_write_syslog("移除关系名为:%s, 零组件ID为:%s, 版本号为:%s成功\n", propName, itemId, version);
		}
	}

	ITKCALL(AOM_save_with_extensions(itemRev));
	ITKCALL(AOM_unlock(itemRev));
CLEANUP:
	DOFREE(itemId);
	DOFREE(version);
	return rcode;
}


/*
* 添加关系
*/
int addItemRevByRelation(tag_t itemRev, vector<tag_t> list, const char* propName) {
	int
		rcode = ITK_ok;

	tag_t
		relType = NULLTAG,
		target_obj = NULLTAG,
		relation = NULLTAG;
	char
		* itemId = NULL,
		* version = NULL;

	ITKCALL(AOM_lock(itemRev)) //锁定设计对象
	ITKCALL(GRM_find_relation_type(propName, &relType)); //返回指定关系类型名称物件的关系类型
	for (size_t i = 0; i < list.size(); i++) {
		target_obj = list[i];
		ITKCALL((AOM_ask_value_string(target_obj, "item_id", &itemId))); // 获取ID
		TC_write_syslog("itemId == %s\n", itemId);

		ITKCALL((AOM_ask_value_string(target_obj, "item_revision_id", &version))); // 获取版本
		TC_write_syslog("version == %s\n", version);

		TC_write_syslog("create relation ... \n");
		ITKCALL(GRM_create_relation(itemRev, target_obj, relType, NULL, &relation)); //添加关联关系
		ITKCALL(GRM_save_relation(relation));
		if (relation != NULL) {
			TC_write_syslog("添加关系名为:%s, 零组件ID为:%s, 版本号为:%s成功\n", propName, itemId, version);
		}
	}

	
	ITKCALL(AOM_save_with_extensions(itemRev)); //保存设计对象的改变
	ITKCALL(AOM_unlock(itemRev));

CLEANUP:
	DOFREE(itemId);
	DOFREE(version);
	return rcode;
}


/*
* 查询物料在DCN的解决方案项的所有MNT DCN
*/
vector<string> recordPartAllDCN(vector<tag_t> list, char* curDCNId) {

	tag_t 
		target_obj = NULLTAG;

	tag_t
		* queryResults = NULLTAG;

	char
		* partId = NULL,
		* version = NULL,
		* dcnId = NULL;
	int
		resultLength = 0;

	char** qry_entries = (char**)MEM_alloc(sizeof(char**));
	char** qry_values = (char**)MEM_alloc(sizeof(char**));
	qry_entries[0] = (char*)MEM_alloc(sizeof(char*) * 32);
	qry_entries[1] = (char*)MEM_alloc(sizeof(char*) * 32);

	qry_values[0] = (char*)MEM_alloc(sizeof(char*) * 200);
	qry_values[1] = (char*)MEM_alloc(sizeof(char*) * 200);

	vector<string>* logList = new vector<string>();

	for (size_t i = 0; i < list.size(); i++) {

		target_obj = list[i];
		ITKCALL((AOM_ask_value_string(target_obj, "item_id", &partId))); // 获取ID
		TC_write_syslog("partId == %s\n", partId);

		ITKCALL((AOM_ask_value_string(target_obj, "item_revision_id", &version))); // 获取版本
		TC_write_syslog("version == %s\n", version);

		tc_strcpy(qry_entries[0], ID);
		tc_strcpy(qry_values[0], partId);

		tc_strcpy(qry_entries[1], Rev);
		tc_strcpy(qry_values[1], version);

		ITKCALL(query(D9_Find_MNT_DCNRev, qry_entries, qry_values, 2, &queryResults, &resultLength));
		TC_write_syslog(" -- >> partId: %s, version: %s queryresult : %d \n", partId, version, resultLength);

		for (size_t k = 0; k < resultLength; k++)
		{
			logical
				isMNTDCNRev = false;
			string
				msg;

			ITKCALL(GTCTYPE_is_type_of(queryResults[k], "D9_MNT_DCNRevision", &isMNTDCNRev));
			if (!isMNTDCNRev) {
				continue;
			}

			ITKCALL((AOM_ask_value_string(queryResults[k], "item_id", &dcnId))); // 获取ID
			TC_write_syslog("find by dcnId == %s\n", dcnId);

			if (tc_strcmp(curDCNId, dcnId) != 0) {
				msg = "【" + (string)partId + "/" + version + "】" + "已在【" + (string)dcnId + "】中生效," + "该版本料号从本【" + (string)curDCNId + "】的解决方案项中移除";
				logList->push_back(msg);
			}
		}
	}

CLEANUP:
	DOFREE(partId);
	DOFREE(version);
	DOFREE(dcnId);
	return *logList;
}


/*
* 判断DCN规范关系下 RepeateReleaseMatRecord 数据集是否存在
*/
tag_t checkDataset(tag_t itemRev) {

	char
		* targetType = NULL,
		* datasetName = NULL;

	tag_t
	
		* tag_list = NULLTAG;

	tag_t
		dataset = NULLTAG;

	int
		tagCount = 0;

	ITKCALL(AOM_ask_value_tags(itemRev, "IMAN_specification", &tagCount, &tag_list));
	for (size_t i = 0; i < tagCount; i++) {
		logical
			isDataset = false;

		ITKCALL(GTCTYPE_is_type_of(tag_list[i], "Dataset", &isDataset));
		if (!isDataset) {
			continue;
		}

		ITKCALL(WSOM_ask_object_type2(tag_list[i], &targetType));
		TC_write_syslog("\n IMAN_specification属性下的targetType == %s\n", targetType);

		if (tc_strcmp(targetType, "Text") == 0) {
			ITKCALL(AOM_ask_value_string(tag_list[i], "object_name", &datasetName)); // 获取数据名称
			TC_write_syslog("\n Text数据集名称为 == %s\n", U2G(datasetName));
			if (tc_strcmp(U2G(datasetName), RepeateReleaseMatRecord) == 0) {
				dataset = tag_list[i];
				break;
			}
		}
	}

CLEANUP:
	DOFREE(tag_list);
	DOFREE(targetType);
	DOFREE(datasetName);
	return dataset;
}


/*
* 获取解决方案项BOM中未发行的物料集合
*/
int getSolutionBomNotReleaseItem(vector<tag_t> solutionBomList, vector<tag_t>& solutionNotReleaseList) {
	int
		rcode = ITK_ok,
		child_count = 0,
		sub_child_count = 0;

	char
		* parentItemId = NULL,
		* parentVersion = NULL,
		* childItemId = NULL,
		* childVersion = NULL,
		* subChildItemId = NULL,
		* subChildVersion = NULL;


	tag_t
		parentItem = NULLTAG,
		childItem = NULLTAG,
		childSubItem = NULLTAG,
		window = NULLTAG,
		target_obj = NULLTAG,
		topLine = NULLTAG,
		childBOMLine = NULLTAG,
		childSubBOMLine = NULLTAG;

	tag_t
		* children = NULLTAG,
		* sub_children = NULLTAG;

	for (size_t i = 0; i < solutionBomList.size(); i++) {
		target_obj = solutionBomList[i];
		ITKCALL(ITEM_ask_item_of_rev(target_obj, &parentItem));
		try
		{
			if (!isBom(target_obj))
			{
				continue;
			}

			ITKCALL(ITEM_ask_item_of_rev(target_obj, &parentItem)); // 对象获取对象版本

			ITKCALL((AOM_ask_value_string(parentItem, "item_id", &parentItemId)));
			TC_write_syslog("parentItemId == %s\n", parentItemId);

			ITKCALL((AOM_ask_value_string(target_obj, "item_revision_id", &parentVersion)));
			TC_write_syslog("parentVersion == %s\n", parentVersion);

			ITKCALL(BOM_create_window(&window)); // 创建BOMWindow

			ITKCALL(BOM_set_window_top_line(window, parentItem, target_obj, NULLTAG, &topLine)); // 获取顶层BOMLine

			ITKCALL(BOM_line_ask_child_lines(topLine, &child_count, &children));
			for (int j = 0; j < child_count; j++) {
				logical
					isSubstitute = false,
					hasSubstitutes = false;
				childBOMLine = children[j];
				ITKCALL(BOM_line_ask_is_substitute(childBOMLine, &isSubstitute)); // 判断是否为替代料

				if (isSubstitute) {
					continue;
				}

				ITKCALL(AOM_ask_value_tag(childBOMLine, "bl_line_object", &childItem));
				if (!isRelease(childItem)) { // 判断是否已经发行

					ITKCALL((AOM_ask_value_string(childItem, "item_id", &childItemId)));
					ITKCALL((AOM_ask_value_string(childItem, "item_revision_id", &childVersion)));

					TC_write_syslog("childItem == %s, childVersion=%s, 主料未发行\n", childItemId, childVersion);

					solutionNotReleaseList.push_back(childItem);
				}

				ITKCALL(BOM_line_ask_has_substitutes(childBOMLine, &hasSubstitutes)); // 判断是否含有替代料
				if (hasSubstitutes) {
					ITKCALL(BOM_line_list_substitutes(childBOMLine, &sub_child_count, &sub_children)); // 获取替代料
					for (size_t k = 0; k < sub_child_count; k++) {
						childSubBOMLine = sub_children[k];
						ITKCALL(AOM_ask_value_tag(childSubBOMLine, "bl_line_object", &childSubItem));
						if (!isRelease(childSubItem)) { // 判断替代料是否已经发行
							ITKCALL((AOM_ask_value_string(childSubItem, "item_id", &subChildItemId)));
							ITKCALL((AOM_ask_value_string(childSubItem, "item_revision_id", &subChildVersion)));

							TC_write_syslog("subChildItemId == %s, subChildVersion=%s, 替代料未发行\n", subChildItemId, subChildVersion);
							solutionNotReleaseList.push_back(childSubItem);
						}
					}
				}
			}

		}
		catch (const std::exception& e)
		{

		}

		ITKCALL(BOM_close_window(window)); // 关闭BOMWindow

	}

CLEANUP:	
	DOFREE(parentItemId);
	DOFREE(parentVersion);
	DOFREE(children);
	DOFREE(childItemId);
	DOFREE(childVersion);
	DOFREE(sub_children);
	DOFREE(subChildItemId);
	DOFREE(subChildVersion);
	return rcode;
}

/*
* 判断对象版本是否存在
*/
int checkItemRevExist(vector<tag_t> sourceList, vector<tag_t>& targetList, const char* propName) {
	int
		count = 0,
		rcode = ITK_ok;
	char
		* sourceItemId = NULL,
		* sourceVersion = NULL,
		* targetItemId = NULL,
		* targetVersion = NULL;

	tag_t
		item = NULLTAG;

	tag_t
		* rev_list = NULLTAG;
		

	for (auto it = targetList.begin(); it != targetList.end();) {
		tag_t target = *it;
		ITKCALL((AOM_ask_value_string(target, "item_id", &targetItemId)));
		ITKCALL((AOM_ask_value_string(target, "item_revision_id", &targetVersion)));

		auto flag = find_if(sourceList.begin(), sourceList.end(), [&](const tag_t& itemRev) {
			ITKCALL((AOM_ask_value_string(itemRev, "item_id", &sourceItemId))); // 获取ID
			ITKCALL((AOM_ask_value_string(itemRev, "item_revision_id", &sourceVersion))); // 获取ID
			return tc_strcmp(sourceItemId, targetItemId) == 0 && tc_strcmp(sourceVersion, targetVersion) == 0;
			});

		bool found = flag != sourceList.end();

		if (found) {
			it = targetList.erase(it);
			if (it == targetList.end()) {
				break;
			}	

		} else {
			TC_write_syslog("关系名为:%s, 零组件ID为:%s, 版本号为:%s, 属于新增物料\n", propName, targetItemId, targetVersion);			
			it++;
		}
	}

CLEANUP:
	DOFREE(sourceItemId);
	DOFREE(sourceVersion);
	DOFREE(targetItemId);
	DOFREE(targetVersion);
	return rcode;
}

/*
* 获取上一个版本集合
*/
int getLastItemRevList(vector<tag_t> curItemRevList, vector<tag_t>& lastItemRevList) {
	int
		count = 0,
		rcode = ITK_ok;

	tag_t
		item = NULLTAG;

	tag_t
		* rev_list = NULLTAG;

	for (size_t i = 0; i < curItemRevList.size(); i++) {
		ITKCALL(ITEM_ask_item_of_rev(curItemRevList[i], &item)); // 对象版本获取对象
		ITKCALL(ITEM_list_all_revs(item, &count, &rev_list)); // 获取对象的所有版本
		if (count > 1) {
			lastItemRevList.push_back(rev_list[count - 2]);
		}
	}

CLEANUP:
	DOFREE(rev_list);
	return rcode;
}