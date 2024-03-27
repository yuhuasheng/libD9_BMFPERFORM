#include "extension.h"
#include "util.h"
#include "errorMsg.h"

int checkStpItemRev(tag_t StpItemRev, char* task_uid, char* task_name);
extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_StpToJT(EPM_action_message_t msg)
	{

		int ifail = ITK_ok,
			attach_object_num = 0;

		tag_t
			root_task = NULL_TAG,
			current_task = NULL_TAG,
			target_obj = NULL_TAG,
			StpItemRev = NULL_TAG;

		tag_t
			* attach_object_list = NULLTAG;

		char
			* task_name = NULL,
			* itemId = NULL,
			* version = NULL,
			* task_uid = NULL;

		vector<string> tagTypeVec;
		
		TC_write_syslog("\n=================D9_StpToJT   start==================\n");

		POM_AM__set_application_bypass(true); // 开启旁路

		current_task = msg.task;
		ITK__convert_tag_to_uid(msg.task, &task_uid); // 获取当前任务的uid

		getPreferenceByName("D9_Item_StepToJT_Type", tagTypeVec); // 获取首选项

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // 获取流程根任务
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &task_name)); // 获取流程任务名称
		
		TC_write_syslog("\n 开始处理流程：%s\n", task_name);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (int i = 0; i < attach_object_num; i++)
		{			
			target_obj = attach_object_list[i];

			if (checkItemType(tagTypeVec, attach_object_list[i])) { // 判断目标文件夹下面的对象版本类型和首选项是否一致
				StpItemRev = target_obj;
				break;
			}			
		}

		if (StpItemRev != NULLTAG)
		{
			ITKCALL((AOM_ask_value_string(StpItemRev, "item_id", &itemId))); // 获取零组件ID			
			TC_write_syslog("itemId == %s\n", itemId);

			ITKCALL((AOM_ask_value_string(StpItemRev, "item_revision_id", &version))); // 获取版本号			
			TC_write_syslog("version == %s\n", version);

			checkStpItemRev(StpItemRev, task_uid, task_name);
		}

	CLEANUP:
		DOFREE(task_name);
		DOFREE(attach_object_list);
		DOFREE(itemId);
		DOFREE(version);
		return ifail;

	}
#ifdef __cplusplus
}
#endif

int checkStpItemRev(tag_t StpItemRev, char* task_uid, char* task_name) {
	int
		tagCount = 0,
		rcode = ITK_ok;

	tag_t
		spec_object = NULLTAG,
		dataset = NULLTAG,
		trans_rqst = NULLTAG;

	tag_t
		* tagList = NULLTAG;

	char
		* targetType = NULL,
		* objectName = NULL;

	logical
		StpToJTFlag = false;

	char**
		request_args = (char**)MEM_alloc(sizeof(char**));
	request_args[0] = (char*)MEM_alloc(sizeof(char[200]));
	request_args[1] = (char*)MEM_alloc(sizeof(char[200]));
	
	TC_write_syslog("******** Start Check StpItemRev ******** \n");

	ITKCALL(AOM_ask_value_tags(StpItemRev, "IMAN_specification", &tagCount, &tagList));
	if (tagCount <= 0) {		
		TC_write_syslog("IMAN_specification is Empty \n");
		goto CLEANUP;
	}

	for (int i = 0; i < tagCount; i++) {
		logical
			isDataset = false;
		ITKCALL(WSOM_ask_object_type2(tagList[i], &targetType));		
		TC_write_syslog("\n IMAN_specification属性下的targetType == %s\n", targetType);

		if (tc_strcmp(targetType, "D9_STEP") != 0) {
			continue;
		}	

		spec_object = tagList[i];

		ITKCALL((AOM_ask_value_string(spec_object, "object_name", &objectName))); // 获取数据集名称		
		TC_write_syslog("objectName == %s\n", objectName);

		if (endWith(objectName, ".stp") > 0) {
			TC_write_syslog("\n 目标文件夹下对象版本的属性IMAN_specification含有Stp数据集对象\n");			
			StpToJTFlag = true;
			break;
		}
	}

	if (StpToJTFlag) {		
		TC_write_syslog("\n task_uid %s\n", task_uid);

		char ProviderName[128] = "FOXCONN";//Provider名称
		char ServiceName[128] = "stptojt";//服务名称

		tc_strcat(request_args[0], "task_uid=");
		tc_strcat(request_args[0], task_uid);
		tc_strcat(request_args[1], "process_name=");
		tc_strcat(request_args[1], task_name);
		
		TC_write_syslog("\n call dispatcher start .....\n");
		//call dispatcher 服务
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
			1,// the length of the primary and secondary object arrays
			&StpItemRev,// the array of primary objects  请求对象
			NULL,//the array of related secondary objects
			2, //the number of request arguments
			(const char**)request_args,//the array of request arguments in the format of
			NULL, // a string for use by the application creating the request for use in defining a type for the request (i.e. SYSTEM, THINCLIENT, etc.)
			0, //the number of datafiles you would like to upload
			0,
			0,
			&trans_rqst));		
		TC_write_syslog("\n call dispatcher end .....\n");
	}


CLEANUP:
	DOFREE(tagList);
	DOFREE(targetType);
	DOFREE(objectName);
	return rcode;
}