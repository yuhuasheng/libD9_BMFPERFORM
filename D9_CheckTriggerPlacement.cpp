#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <dispatcher/dispatcher_itk.h>
#define Layout "Layout"
#define EE "EE"
#define PSU "PSU"
#define MAXSIZE 3000
bool checkDateset(tag_t itemRev);
tag_t getProjectFolder(char* ids);
void getFolderPath(tag_t current, map<string, tag_t>& map, string path, bool flag, string dept);
bool checkFolder(int count, tag_t* contents);
int getItemRevList(tag_t folder, vector<string> tagTypeVec, vector<tag_t>& list);
int getEERevInfo(vector<tag_t> EEList, string& EERevUid);
boolean compareTag(const tag_t& firstTag, const tag_t& secondTag);
vector<tag_t> filterEEList(vector<tag_t>& list);

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_CheckTriggerPlacement(EPM_action_message_t msg) {
		int ifail = ITK_ok;

		char
			* layoutId = NULL,
			* layoutVersion = NULL,
			* layoutRevUid = NULL,
			* task_name = NULL,
			* ids = NULL,
			* locationPath = NULL,
			* projectName = NULL;


		char
			** locationPathProps = NULL,
			** request_args = (char**)MEM_alloc(sizeof(char**));
		request_args[0] = (char*)MEM_alloc(sizeof(char[200]));
		request_args[1] = (char*)MEM_alloc(sizeof(char[200]));

		tag_t
			current_task = NULLTAG,
			root_task = NULLTAG,
			layoutItemRev = NULLTAG,
			projectFolder = NULLTAG,
			folder = NULLTAG,
			trans_rqst = NULLTAG;

		tag_t
			* attach_object_list = NULLTAG;

		tag_t
			* primaryTag = (tag_t*)MEM_alloc(sizeof(tag_t) * 1024),
			* secondTag = (tag_t*)MEM_alloc(sizeof(tag_t) * 1024);

		string dept,
			EERevUid = "";
		/*tag_t
			secondary[MAXSIZE] = {};*/

		vector<string>
			tagTypeVec,
			layoutPathVec;
			
		vector<tag_t>
			totalEEList;

		tagTypeVec.clear();
		layoutPathVec.clear();
		totalEEList.clear();

		int
			prop_num = 0,
			attach_object_num = 0;

		TC_write_syslog("\n enter D9_CheckTriggerPlacement.....!\n");

		current_task = msg.task;

		getPreferenceByName("D9_Allow_TandBInfo_Item", tagTypeVec); // 获取首选项

		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &task_name));

		TC_write_syslog("\n 开始处理流程：%s\n", task_name);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (int i = 0; i < attach_object_num; i++) {
			if (!checkItemType(tagTypeVec, attach_object_list[i])) { // 判断目标对象文件夹下的对象版本类型是否和首选项D9_Allow_TandBInfo_Item一致
				continue;
			}

			layoutItemRev = attach_object_list[i];

			ITKCALL(AOM_ask_value_string(layoutItemRev, "item_id", &layoutId));
			TC_write_syslog("layoutId == %s\n", layoutId);

			ITKCALL(AOM_ask_value_string(layoutItemRev, "item_revision_id", &layoutVersion));
			TC_write_syslog("layoutVersion == %s\n", layoutVersion);

			ITK__convert_tag_to_uid(layoutItemRev, &layoutRevUid);

			ITKCALL(AOM_ask_value_strings(layoutItemRev, "d9_LocationPath", &prop_num, &locationPathProps));
			for (size_t j = 0; j < prop_num; j++) {
				if (containStr(locationPathProps[j], Layout) > 0) { // 校验填写位置属性栏位值
					layoutPathVec.push_back((string)locationPathProps[j]);
				}
			}

			if (layoutPathVec.size() <= 0) {
				TC_write_syslog("\n d9_LocationPath不存在含有Layout的属性值\n");
				goto CLEANUP;
			}

			if (!checkDateset(layoutItemRev)) { // 判断师傅含有placement数据集
				TC_write_syslog("\n 不存在placement数据集\n");
				goto CLEANUP;
			}

			ITKCALL(AOM_ask_value_string(layoutItemRev, "project_ids", &ids));
			TC_write_syslog("ids == %s\n", ids);
			if (ids == NULL) {
				TC_write_syslog("\n 未指派专案\n");
				goto CLEANUP;
			}

			projectFolder = getProjectFolder(ids); // 获取专案文件夹
			if (projectFolder == NULL) {
				TC_write_syslog("\n 获取专案文件夹失败\n");
				goto CLEANUP;
			}

			AOM_ask_value_string(projectFolder, "object_name", &projectName);
			TC_write_syslog("projectName == %s\n", projectName);


			for (size_t k = 0; k < layoutPathVec.size(); k++) {
				locationPath = (char*)MEM_alloc(sizeof(char) * 4096);
				strcat(locationPath, layoutPathVec[k].c_str());
				TC_write_syslog("locationPath == %s\n", locationPath);

				if (endWith(locationPath, "Power&PI&INV&LED CONV BD") || endWith(locationPath, "LED driver BD") || endWith(locationPath, "LED Lighting BD")
					|| endWith(locationPath, "LED Lighting  Driver BD") || endWith(locationPath, "DC JACK") || endWith(locationPath, "Safety POWER BD")) {
					str_replace(locationPath, Layout, PSU);
					dept = PSU;
				} else {
					str_replace(locationPath, Layout, EE);
					dept = EE;
				}

				map<string, tag_t>* pathMap = new map<string, tag_t>();
				map<string, tag_t>::reverse_iterator* iterator = new map<string, tag_t>::reverse_iterator();
				vector<tag_t> *EEList = new vector<tag_t>;
				vector<tag_t> *filterList = new vector<tag_t>;

				getFolderPath(projectFolder, *pathMap, "", false, dept);

				map<string, tag_t>::iterator iter = pathMap->find(locationPath);

				if (iter != pathMap->end()) {
					folder = iter->second;
				} else {
					TC_write_syslog("locationPath == %s\n", "not exist PCBA ItemRevision");
					continue;
				}

				getItemRevList(folder, tagTypeVec, *EEList); // 获取EE或PSU下面的PCBA对象版本
				if ((*EEList).size() <= 0) {
					continue;
				}

				*filterList = filterEEList(*EEList); // 过滤EE对象版本集合
				if ((*filterList).size() <= 0) {
					continue;
				}

				for (size_t m = 0; m < (*filterList).size(); m++) {
					totalEEList.push_back((*filterList)[m]);
				}

				if (pathMap != NULL) {
					(*pathMap).clear();
					pathMap = NULL;
				}
				iterator = NULL;

				if (EEList != NULL) {
					(*EEList).clear();
					EEList = NULL;
				}
				EEList = NULL;

				if (filterList != NULL) {
					(*filterList).clear();
					filterList = NULL;
				}
				filterList = NULL;
			}
		}

		if (totalEEList.size() <= 0) {
			goto CLEANUP;
		}

		getEERevInfo(totalEEList, EERevUid);

		for (size_t n = 0; n < totalEEList.size(); n++) {
			primaryTag[n] = layoutItemRev;
			secondTag[n] = totalEEList[n];
		}


		/*if (true) {
			goto CLEANUP;
		}*/

		TC_write_syslog("layoutRevUid == %s\n", layoutRevUid);

		TC_write_syslog("EERevUid == %s\n", EERevUid);

//		int length = totalEEList.size() * 2;
		int length = totalEEList.size();
		char ProviderName[128] = "FOXCONN"; //Provider名称
		char ServiceName[128] = "checkplacementnew"; //服务名称

		strcat(request_args[0], "LayoutRevUid=");
		strcat(request_args[0], layoutRevUid);
		strcat(request_args[1], "EERevUid=");
		strcat(request_args[1], EERevUid.c_str());

		TC_write_syslog("\n Call dispatcher start .....\n");
		//Call sipather 服务
		ITKCALL(DISPATCHER_create_request(
			ProviderName,//the name of the provider that will process this request
			ServiceName,//the name of the service that will process this request
			3, //the priority to assign to the request(0 LOW to 3 HIGH)
			0, //the time at which to start this request
			0, //the time at which no more repeating requests of this same type will be processed.  
			//If the interval option for repeating is NOT selected, then this paramater is unused
			0, //the number of times to repeat this request
			// 0 - no repeating
			// 1 2 3 ... - number of times to repeat this task
			length,// the length of the primary and secondary object arrays
			primaryTag,// the array of primary objects  请求对象
			secondTag,//the array of related secondary objects
			2, //the number of request arguments
			(const char**)request_args,//the array of request arguments in the format of
			NULL, // a string for use by the application creating the request for use in defining a type for the request (i.e. SYSTEM, THINCLIENT, etc.)
			0, //the number of datafiles you would like to upload
			0,
			0,
			&trans_rqst));
		TC_write_syslog("\n Call dispatcher end .....\n");
		//		delete secondaryTag;

	CLEANUP:
		POM_AM__set_application_bypass(false);
		TC_write_syslog("\n start D9_CheckTriggerPlacement CLEANUP\n");
		totalEEList.clear();
		DOFREE(primaryTag);
		DOFREE(secondTag);
		DOFREE(task_name);
		DOFREE(attach_object_list);
		DOFREE(layoutId);
		DOFREE(layoutVersion);
		DOFREE(locationPathProps);
		DOFREE(ids);
		TC_write_syslog("\n ending D9_CheckTriggerPlacement CLEANUP over\n");
		return ifail;
	}

#ifdef __cplusplus
}
#endif


/**
* 核对数据集
*/
bool checkDateset(tag_t itemRev) {
	logical
		flag = false;

	char
		* targetType = NULL,
		* datasetName = NULL;

	tag_t
		* EDAPlacement_tag_list = NULLTAG,
		* tag_list = NULLTAG;
	int
		EDAPlacementCount = 0,
		tagCount = 0;

	TC_write_syslog("\n 开始获取EDAHasDerivedDataset下面的数组\n");
	ITKCALL(AOM_ask_value_tags(itemRev, "EDAHasDerivedDataset", &EDAPlacementCount, &EDAPlacement_tag_list));
	for (size_t i = 0; i < EDAPlacementCount; i++) {
		ITKCALL(WSOM_ask_object_type2(EDAPlacement_tag_list[i], &targetType));
		TC_write_syslog("\n EDAHasDerivedDataset属性下的targetType == %s\n", targetType);
		if (tc_strcmp(targetType, "D9_Placement") == 0) {
			flag = true;
		}
	}

	if (flag) {
		goto CLEANUP;
	}

	TC_write_syslog("\n 开始获取IMAN_specification下面的数组\n");
	ITKCALL(AOM_ask_value_tags(itemRev, "IMAN_specification", &tagCount, &tag_list));
	for (size_t j = 0; j < tagCount; j++) {
		ITKCALL(WSOM_ask_object_type2(tag_list[j], &targetType));
		TC_write_syslog("\n IMAN_specification属性下的targetType == %s\n", targetType);
		if (tc_strcmp(targetType, "MSExcel") == 0 | tc_strcmp(targetType, "MSExcelX") == 0) {
			ITKCALL(AOM_ask_value_string(tag_list[j], "object_name", &datasetName)); // 获取数据名称
			TC_write_syslog("\n Excel数据集名称为 == %s\n", datasetName);
			if (tc_strstr(datasetName, "Location")) {
				flag = true;
			}
		}
	}

CLEANUP:
	POM_AM__set_application_bypass(false);
	TC_write_syslog("\n start checkDateset CLEANUP\n");
	DOFREE(EDAPlacement_tag_list);
	DOFREE(targetType);
	DOFREE(tag_list);
	DOFREE(datasetName);
	TC_write_syslog("\n ending checkDateset CLEANUP over\n");
	return flag;
}


/**
* 获取专案文件夹
*/
tag_t getProjectFolder(char* ids) {
	char** qry_entries = (char**)MEM_alloc(sizeof(char**));
	char** qry_values = (char**)MEM_alloc(sizeof(char**));
	qry_entries[0] = (char*)MEM_alloc(sizeof(char*) * 200);
	qry_values[0] = (char*)MEM_alloc(sizeof(char*) * 200);

	tc_strcpy(qry_entries[0], "SPAS ID");
	tc_strcpy(qry_values[0], ids);

	tag_t
		* queryResults = NULLTAG;
	tag_t
		projectFolder = NULLTAG;

	int
		resultLength = 0;

	ITKCALL(query("__D9_Find_Project_Folder", qry_entries, qry_values, 1, &queryResults, &resultLength));
	TC_write_syslog(" -- >> project ids : %s queryresult : %d \n", ids, resultLength);

	if (resultLength > 0) {
		projectFolder = queryResults[0];
	}
	return projectFolder;
}


/**
* 获取文件夹路径
*/
void getFolderPath(tag_t current, map<string, tag_t>& map, string path, bool flag, string dept) {
	char
		* targetType = NULL,
		* objectName = NULL;
	int
		count = 0;
	tag_t
		* contents = NULLTAG;

	ITKCALL(AOM_ask_value_string(current, "object_name", &objectName)); // 获取文件夹名称
	if (tc_strcmp(U2G(objectName), G2U(dept.c_str())) == 0) {
		flag = true;
	}

	ITKCALL(WSOM_ask_object_type2(current, &targetType));

	logical
		isFolder = false;

	GTCTYPE_is_type_of(current, "Folder", &isFolder);
	if (!isFolder) {
		return;
	}

	path = path + (string)U2G(objectName) + "/";
	AOM_ask_value_tags(current, "contents", &count, &contents);
	if (count == 0 || !checkFolder(count, contents)) {
		if (map.count(path) == 0 && flag) {
			map.insert(pair<string, tag_t>(path.substr(0, path.length() - 1), current));
		}
		return;
	}

	for (size_t i = 0; i < count; i++) {
		getFolderPath(contents[i], map, path, flag, dept);
	}

CLEANUP:
	DOFREE(objectName);
	DOFREE(targetType);
	DOFREE(contents);
}



bool checkFolder(int count, tag_t* contents) {
	for (size_t i = 0; i < count; i++) {
		logical
			isFolder = false;
		GTCTYPE_is_type_of(contents[i], "Folder", &isFolder);
		if (isFolder) {
			return true;
		}
	}

	return false;
}


/**
* 获取PCBA对象版本
*/
int getItemRevList(tag_t folder, vector<string> tagTypeVec, vector<tag_t>& list) {
	int rcode = ITK_ok;
	ITKCALL(AOM_refresh(folder, false)); // 刷新文件夹

	tag_t
		* contents = NULLTAG;

	int
		count = 0;
	AOM_ask_value_tags(folder, "contents", &count, &contents);
	for (size_t i = 0; i < count; i++) {
		if (checkItemType(tagTypeVec, contents[i]) && isBom(contents[i])) { // 类型符合要求，并且含有BOM结构
			list.push_back(contents[i]);
		}
	}

CLEANUP:
	DOFREE(contents);
	return rcode;
}

boolean compareTag(const tag_t& firstTag, const tag_t& secondTag) {
	char
		* fristItemId = NULL,
		* secondItemId = NULL;

	ITKCALL(AOM_ask_value_string(firstTag, "item_id", &fristItemId));
	ITKCALL(AOM_ask_value_string(secondTag, "item_id", &secondItemId));

	string str1 = fristItemId;
	string str2 = secondItemId;

CLEANUP:
	DOFREE(fristItemId);
	DOFREE(secondItemId);
	return str1 < str2;
}

/*
* 过滤EE对象版本记录，图号相同的只获取最大版本的对象
*/
vector<tag_t> filterEEList(vector<tag_t>& EEList) {
	vector<tag_t>
		filterVec,
		tempVec;

	tag_t
		item = NULLTAG,
		lastItemRev = NULLTAG;
	int
		count = 0;

	tag_t
		* revList = NULLTAG;

	char
		* sourceUid = NULL,
		* destUid = NULL;

	
	tempVec.assign(EEList.begin(), EEList.end()); // 数组的复制操作

	TC_write_syslog("所有的EE或PSU的PCBA对象信息\n");
	for_each(EEList.begin(), EEList.end(), [](tag_t& t) {		
		TC_write_syslog("itemId == %s, version=%s\n", getPropValue(t, "item_id").c_str(), getPropValue(t, "item_revision_id").c_str());
	});
	TC_write_syslog("=========================\n");


	std::sort(tempVec.begin(), tempVec.end(), compareTag);
	tempVec.erase(std::unique(tempVec.begin(), tempVec.end(), [](const tag_t& firstTag, const tag_t& secondTag) {
		string firstStr = getPropValue(firstTag, "item_id");
		string secondStr = getPropValue(secondTag, "item_id");
		return firstStr == secondStr;
		}), tempVec.end());


	for (size_t i = 0; i < tempVec.size(); i++) {
		tag_t temp_tag = tempVec[i];

		TC_write_syslog("去重后的零组件ID和版本信息\n");
		TC_write_syslog("itemId == %s, version=%s\n", getPropValue(temp_tag, "item_id").c_str(), getPropValue(temp_tag, "item_revision_id").c_str());

		ITKCALL(ITEM_ask_item_of_rev(temp_tag, &item)); // 通过对象版本获取对象
		ITKCALL(AOM_ask_value_tags(item, "revision_list", &count, &revList)); // 获取所有的版本
		if (count > 0) {
			TC_write_syslog("当前零组件所存在所有版本信息\n");
			for (int j = count - 1; j >= 0; j--) {
				tag_t tag = revList[j];
				TC_write_syslog("itemId == %s, version=%s\n", getPropValue(tag, "item_id").c_str(), getPropValue(tag, "item_revision_id").c_str());

				auto flag = find_if(EEList.begin(), EEList.end(), [&](tag_t& itemRev) {

					ITK__convert_tag_to_uid(itemRev, &sourceUid); // 获取uid
					ITK__convert_tag_to_uid(tag, &destUid); // 获取uid
					return tc_strcmp(sourceUid, destUid) == 0;
				});

				bool found = flag != EEList.end();

				if (found) {
					filterVec.push_back(tag);
					break;
				}
			}

			TC_write_syslog("匹配同一图号归档最大版本信息如下:\n");			
			for_each(filterVec.begin(), filterVec.end(), [](tag_t& t) {
				TC_write_syslog("itemId == %s, version=%s\n", getPropValue(t, "item_id").c_str(), getPropValue(t, "item_revision_id").c_str());
			});
			TC_write_syslog("=========================\n");

		}
	}
CLEAUP:
	DOFREE(sourceUid);
	DOFREE(destUid);
	return filterVec;
}


int getEERevInfo(vector<tag_t> EEList, string& EERevUid) {
	int rcode = ITK_ok;
	char
		* uid = NULL;

	for (size_t i = 0; i < EEList.size(); i++) {
		ITK__convert_tag_to_uid(EEList[i], &uid);
		EERevUid += (string)uid + ";";
	}

	EERevUid = EERevUid.substr(0, EERevUid.length() - 1);
CLEANUP:
	DOFREE(uid);
	return rcode;
}
