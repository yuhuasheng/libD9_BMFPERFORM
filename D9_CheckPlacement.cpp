#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <dispatcher/dispatcher_itk.h>

// logical checkTagType(vector<string>  tagTypeVec, tag_t  rev_tag);
//logical checkStructure(tag_t itemRev, int structCount, tag_t *struct_tag_list);

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_CheckPlacement(EPM_action_message_t msg)
	{
		int ifail = ITK_ok;
		char* task_uid = NULL;
		tag_t trans_rqst = NULLTAG;
		tag_t root_task = NULLTAG;
		tag_t *attach_object_list = NULLTAG;
		int attach_object_num = 0;
		//需要检查的对象类型
		vector<string> tagTypeVec;
		int tagCount = 0;
		tag_t *tag_list = NULLTAG;
		tag_t Layout_PCBARevision = NULLTAG;
		tag_t EE_PCBARevision = NULLTAG;
		int EDAPlacementCount = 0;
		tag_t *EDAPlacement_tag_list = NULLTAG;
		int structCount = 0;
		tag_t *struct_tag_list = NULLTAG;
		char *targetType = NULL;
		logical updateEESchematicFlag = false; // 作为是否需要触发Dispatcher服务的标识
		logical excelFlag = false; // 作为对象版本的规范关系下是否含有Excel数据集的判断标识
		char** request_args = (char**)MEM_alloc(sizeof(char **));
		request_args[0] = (char *)MEM_alloc(sizeof(char[200]));
		request_args[1] = (char *)MEM_alloc(sizeof(char[200]));
		char *taskname = NULL;
		char *datasetName = NULL;

		printf("\n enter D9_CheckPlacement.....!\n");
		TC_write_syslog("\n enter D9_CheckPlacement.....!\n");

		tag_t current_task = msg.task;
		//获取 task 的uid
		ITK__convert_tag_to_uid(msg.task, &task_uid);

		// 获取首选项
		getPreferenceByName("D9_Allow_TandBInfo_Item", tagTypeVec);

		//获取根任务
		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskname));

		printf("\n 开始处理流程：%s\n", taskname);
		TC_write_syslog("\n 开始处理流程：%s\n", taskname);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (int i = 0; i < attach_object_num; i++)
		{
			if (checkItemType(tagTypeVec, attach_object_list[i])) // 判断目标对象文件夹下的对象版本类型是否和首选项D9_Allow_TandBInfo_Item一致
			{
				//printf("\n 目标文件夹的对象版本含有LayOut对象版本 \n");
				Layout_PCBARevision = attach_object_list[i];
				break;
			}
		}

		if (Layout_PCBARevision != NULL)
		{
			ITKCALL(AOM_ask_value_tags(Layout_PCBARevision, "EDAHasDerivedDataset", &EDAPlacementCount, &EDAPlacement_tag_list));
			if (EDAPlacementCount > 0)
			{
				printf("\n *************************************** \n");				
				printf("\n 存在属性为EDAHasDerivedDataset的遍历方式\n");
				TC_write_syslog("\n 存在属性为EDAHasDerivedDataset的遍历方式\n");
				for (int j = 0; j < EDAPlacementCount; j++)
				{
					ITKCALL(WSOM_ask_object_type2(EDAPlacement_tag_list[j], &targetType));
					printf("\n EDAHasDerivedDataset属性下的targetType == %s\n", targetType);
					TC_write_syslog("\n EDAHasDerivedDataset属性下的targetType == %s\n", targetType);
					if (tc_strcmp(targetType, "D9_Placement") == 0)
					{
						printf("\n 开始获取IMAN_specification下面的数组\n");
						TC_write_syslog("\n 开始获取IMAN_specification下面的数组\n");						
						ITKCALL(AOM_ask_value_tags(Layout_PCBARevision, "IMAN_specification", &tagCount, &tag_list));
						for (int m = 0; m < tagCount; m++)
						{
							ITKCALL(WSOM_ask_object_type2(tag_list[m], &targetType));
							printf("\n IMAN_specification属性下的targetType == %s\n", targetType);
							TC_write_syslog("IMAN_specification属性下的targetType == %s\n", targetType);
							if (checkItemType(tagTypeVec, tag_list[m]))
							{
								TC_write_syslog("\n 目标文件夹的Layout对象版本的属性IMAN_specification含有EE对象版本\n");
								printf("\n 目标文件夹的Layout对象版本的属性IMAN_specification含有EE对象版本\n");
								EE_PCBARevision = tag_list[m];
								if (checkStructure(EE_PCBARevision))
								{
									updateEESchematicFlag = true;
								}
							}
						}
					}
				}
				printf("\n *************************************** \n");
			}
			else
			{
				printf("\n *************************************** \n");
				printf("\n 不存在属性为EDAHasDerivedDataset的遍历方式\n");
				TC_write_syslog("\n 不存在属性为EDAHasDerivedDataset的遍历方式\n");
				printf("\n *************************************** \n");

				printf("\n 开始获取IMAN_specification下面的数组\n");
				TC_write_syslog("\n 开始获取IMAN_specification下面的数组\n");
				ITKCALL(AOM_ask_value_tags(Layout_PCBARevision, "IMAN_specification", &tagCount, &tag_list));
				for (int k = 0; k < tagCount; k++)
				{
					ITKCALL(WSOM_ask_object_type2(tag_list[k], &targetType));
					printf("\n IMAN_specification属性下的targetType == %s\n", targetType);
					TC_write_syslog("\n IMAN_specification属性下的targetType == %s\n", targetType);
					if (tc_strcmp(targetType, "MSExcel") == 0 | tc_strcmp(targetType, "MSExcelX") == 0) // 判断数据集类型是否为Excel
					{
						ITKCALL(AOM_ask_value_string(tag_list[k], "object_name", &datasetName)); // 获取数据名称
						printf("\n Excel数据集名称为 == %s\n", datasetName);
						TC_write_syslog("\n Excel数据集名称为 == %s\n", datasetName);
						if (tc_strstr(datasetName, "Location")) // 判断数据集名称是否包含Location字样
						{
							excelFlag = true; // 作为数据集是否为Excel的标识
							break;
						}

					}
				}

				if (excelFlag)
				{
					for (int r = 0; r < tagCount; r++)
					{
						if (checkItemType(tagTypeVec, tag_list[r]))
						{
							printf("\n 目标文件夹的Layout对象版本的属性IMAN_specification含有EE对象版本\n");
							TC_write_syslog("\n 目标文件夹的Layout对象版本的属性IMAN_specification含有EE对象版本\n");
							EE_PCBARevision = tag_list[r];
							if (checkStructure(EE_PCBARevision))
							{
								updateEESchematicFlag = true;
							}
						}
					}
				}
				printf("\n *************************************** \n");
			}
		}

		if (updateEESchematicFlag)
		{
			printf("\n task uid %s\n", task_uid);
			TC_write_syslog("\n task uid %s\n", task_uid);

			char ProviderName[128] = "FOXCONN";//Provider名称
			char ServiceName[128] = "checkplacement";//服务名称

//			char** request_args = (char**)MEM_alloc(sizeof(char **));
//			request_args[0] = (char *)MEM_alloc(sizeof(char[200]));
			strcat(request_args[0], "task_uid=");
			strcat(request_args[0], task_uid);
			strcat(request_args[1], "process_name=");
			strcat(request_args[1], taskname);

			printf("\n call dispatcher start .....\n");
			TC_write_syslog("\n call dispatcher start .....\n");
			//call sipather 服务
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
				&Layout_PCBARevision,// the array of primary objects  请求对象
				NULL,//the array of related secondary objects
				2, //the number of request arguments
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
		DOFREE(EDAPlacement_tag_list);
		DOFREE(tag_list);
		DOFREE(targetType);
		DOFREE(datasetName);
		DOFREE(struct_tag_list);
		printf("\n ending CLEANUP over\n");
		return ifail;
	}
#ifdef __cplusplus
}
#endif

/**
 判断是否含有BOM视图
*/
//logical checkStructure(tag_t itemRev, int structCount, tag_t *struct_tag_list)
//{
//	//	int structCount = 0;
//	//	tag_t *struct_tag_list = NULLTAG;
//	ITKCALL(AOM_ask_value_tags(itemRev, "structure_revisions", &structCount, &struct_tag_list));
//	if (structCount > 0)
//	{
//		return true;
//	}
//	return false;
//}