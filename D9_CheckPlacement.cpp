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
		//��Ҫ���Ķ�������
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
		logical updateEESchematicFlag = false; // ��Ϊ�Ƿ���Ҫ����Dispatcher����ı�ʶ
		logical excelFlag = false; // ��Ϊ����汾�Ĺ淶��ϵ���Ƿ���Excel���ݼ����жϱ�ʶ
		char** request_args = (char**)MEM_alloc(sizeof(char **));
		request_args[0] = (char *)MEM_alloc(sizeof(char[200]));
		request_args[1] = (char *)MEM_alloc(sizeof(char[200]));
		char *taskname = NULL;
		char *datasetName = NULL;

		printf("\n enter D9_CheckPlacement.....!\n");
		TC_write_syslog("\n enter D9_CheckPlacement.....!\n");

		tag_t current_task = msg.task;
		//��ȡ task ��uid
		ITK__convert_tag_to_uid(msg.task, &task_uid);

		// ��ȡ��ѡ��
		getPreferenceByName("D9_Allow_TandBInfo_Item", tagTypeVec);

		//��ȡ������
		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskname));

		printf("\n ��ʼ�������̣�%s\n", taskname);
		TC_write_syslog("\n ��ʼ�������̣�%s\n", taskname);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (int i = 0; i < attach_object_num; i++)
		{
			if (checkItemType(tagTypeVec, attach_object_list[i])) // �ж�Ŀ������ļ����µĶ���汾�����Ƿ����ѡ��D9_Allow_TandBInfo_Itemһ��
			{
				//printf("\n Ŀ���ļ��еĶ���汾����LayOut����汾 \n");
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
				printf("\n ��������ΪEDAHasDerivedDataset�ı�����ʽ\n");
				TC_write_syslog("\n ��������ΪEDAHasDerivedDataset�ı�����ʽ\n");
				for (int j = 0; j < EDAPlacementCount; j++)
				{
					ITKCALL(WSOM_ask_object_type2(EDAPlacement_tag_list[j], &targetType));
					printf("\n EDAHasDerivedDataset�����µ�targetType == %s\n", targetType);
					TC_write_syslog("\n EDAHasDerivedDataset�����µ�targetType == %s\n", targetType);
					if (tc_strcmp(targetType, "D9_Placement") == 0)
					{
						printf("\n ��ʼ��ȡIMAN_specification���������\n");
						TC_write_syslog("\n ��ʼ��ȡIMAN_specification���������\n");						
						ITKCALL(AOM_ask_value_tags(Layout_PCBARevision, "IMAN_specification", &tagCount, &tag_list));
						for (int m = 0; m < tagCount; m++)
						{
							ITKCALL(WSOM_ask_object_type2(tag_list[m], &targetType));
							printf("\n IMAN_specification�����µ�targetType == %s\n", targetType);
							TC_write_syslog("IMAN_specification�����µ�targetType == %s\n", targetType);
							if (checkItemType(tagTypeVec, tag_list[m]))
							{
								TC_write_syslog("\n Ŀ���ļ��е�Layout����汾������IMAN_specification����EE����汾\n");
								printf("\n Ŀ���ļ��е�Layout����汾������IMAN_specification����EE����汾\n");
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
				printf("\n ����������ΪEDAHasDerivedDataset�ı�����ʽ\n");
				TC_write_syslog("\n ����������ΪEDAHasDerivedDataset�ı�����ʽ\n");
				printf("\n *************************************** \n");

				printf("\n ��ʼ��ȡIMAN_specification���������\n");
				TC_write_syslog("\n ��ʼ��ȡIMAN_specification���������\n");
				ITKCALL(AOM_ask_value_tags(Layout_PCBARevision, "IMAN_specification", &tagCount, &tag_list));
				for (int k = 0; k < tagCount; k++)
				{
					ITKCALL(WSOM_ask_object_type2(tag_list[k], &targetType));
					printf("\n IMAN_specification�����µ�targetType == %s\n", targetType);
					TC_write_syslog("\n IMAN_specification�����µ�targetType == %s\n", targetType);
					if (tc_strcmp(targetType, "MSExcel") == 0 | tc_strcmp(targetType, "MSExcelX") == 0) // �ж����ݼ������Ƿ�ΪExcel
					{
						ITKCALL(AOM_ask_value_string(tag_list[k], "object_name", &datasetName)); // ��ȡ��������
						printf("\n Excel���ݼ�����Ϊ == %s\n", datasetName);
						TC_write_syslog("\n Excel���ݼ�����Ϊ == %s\n", datasetName);
						if (tc_strstr(datasetName, "Location")) // �ж����ݼ������Ƿ����Location����
						{
							excelFlag = true; // ��Ϊ���ݼ��Ƿ�ΪExcel�ı�ʶ
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
							printf("\n Ŀ���ļ��е�Layout����汾������IMAN_specification����EE����汾\n");
							TC_write_syslog("\n Ŀ���ļ��е�Layout����汾������IMAN_specification����EE����汾\n");
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

			char ProviderName[128] = "FOXCONN";//Provider����
			char ServiceName[128] = "checkplacement";//��������

//			char** request_args = (char**)MEM_alloc(sizeof(char **));
//			request_args[0] = (char *)MEM_alloc(sizeof(char[200]));
			strcat(request_args[0], "task_uid=");
			strcat(request_args[0], task_uid);
			strcat(request_args[1], "process_name=");
			strcat(request_args[1], taskname);

			printf("\n call dispatcher start .....\n");
			TC_write_syslog("\n call dispatcher start .....\n");
			//call sipather ����
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
				&Layout_PCBARevision,// the array of primary objects  �������
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
 �ж��Ƿ���BOM��ͼ
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