#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <dispatcher/dispatcher_itk.h>
#include "preference.h"

// logical checkTagType(vector<string>  tagTypeVec, tag_t  rev_tag);
//logical checkStructure(tag_t itemRev, int structCount, tag_t *struct_tag_list);

extern "C" int POM_AM__set_application_bypass(logical bypass);
int getItemRevs(vector<string> tagTypeMappVec, vector<string>& tagTypeVec);

#ifdef __cplusplus
extern "C" {
#endif

	int D9_SaveDCNChange(EPM_action_message_t msg)
	{			 
		char
			*task_uid = NULL,
			*taskname = NULL;
		char
			** request_args = (char**)MEM_alloc(sizeof(char **));
		request_args[0] = (char *)MEM_alloc(sizeof(char[200]));		

		tag_t
			current_task = NULLTAG,
			root_task = NULLTAG,
			target_obj = NULLTAG,
			MNT_DCN_Revision = NULLTAG,
			trans_rqst = NULLTAG;

		tag_t 
			*attach_object_list = NULLTAG;

		vector<string> 
			tagTypeMappVec,
			tagTypeVec;

		tagTypeMappVec.clear();
		tagTypeVec.clear();

		int
			attach_object_num = 0,
			ifail = ITK_ok;

		TC_write_syslog("\n enter D9_SaveDCNChange.....!\n");

		getPreferenceByName(D9_ALLOW_TOSAP_ITEM, tagTypeMappVec); // 获取允许执行抛转Sap的对象版本类型首选项

		getItemRevs(tagTypeMappVec, tagTypeVec); // 获取只包含对象类型的集合

		current_task = msg.task;
		//获取 task 的uid
		ITK__convert_tag_to_uid(msg.task, &task_uid);
		
		//获取根任务
		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskname));

		TC_write_syslog("\n 开始处理流程：%s\n", taskname);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));
		for (size_t i = 0; i < attach_object_num; i++)
		{
			if (checkItemType(tagTypeVec, attach_object_list[i])) { // 判断目标对象文件夹下的对象版本类型是否和首选项D9_Allow_ToSap_ITEM一致
				MNT_DCN_Revision = attach_object_list[i];
				break;
			}
		}

		if (MNT_DCN_Revision != NULLTAG)
		{
			printf("\n task uid %s\n", task_uid);
			TC_write_syslog("\n task uid %s\n", task_uid);

			char ProviderName[128] = "FOXCONN";//Provider名称
			char ServiceName[128] = "savedcnchange";//服务名称

			strcat(request_args[0], "task_uid=");
			strcat(request_args[0], task_uid);
			
			printf("\n call dispatcher start .....\n");
			TC_write_syslog("\n call dispatcher start .....\n");

			// call dispatcher 服务
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
				&MNT_DCN_Revision,// the array of primary objects  请求对象
				NULL,//the array of related secondary objects
				1, //the number of request arguments
				(const char **)request_args,//the array of request arguments in the format of
				NULL, // a string for use by the application creating the request for use in defining a type for the request (i.e. SYSTEM, THINCLIENT, etc.)
				0, //the number of datafiles you would like to upload
				0,
				0,
				&trans_rqst));
			printf("\n call dispatcher end .....\n");
			TC_write_syslog("\n call dispatcher end .....\n");
		}

	CLEANUP:
		printf("\n start handler CLEANUP\n");
		DOFREE(task_uid);
		DOFREE(taskname);
		DOFREE(request_args);
		DOFREE(attach_object_list);
		printf("\n ending CLEANUP over\n");
		return ifail;
	}
#ifdef __cplusplus
}
#endif

int getItemRevs(vector<string> tagTypeMappVec, vector<string> &tagTypeVec) {
	int rcode = ITK_ok;

	for (size_t i = 0; i < tagTypeMappVec.size(); i++) {
		tagTypeVec.push_back(split(tagTypeMappVec[i], "=")[0]);
	}

CLEANUP:
	return rcode;
}
