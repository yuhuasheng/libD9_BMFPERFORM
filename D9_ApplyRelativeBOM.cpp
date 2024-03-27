#include "extension.h"
#include "util.h"

int handlerSolutionItemRev(tag_t DCNItemRev);
boolean isExistItemRev(tag_t BOMApplyItemRev, tag_t item_rev);
extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_ApplyRelativeBOM(EPM_action_message_t msg)
	{
		int
			ifail = ITK_ok,
			attach_object_num = 0;
			
		tag_t
			root_task = NULL_TAG,
			target_obj = NULL_TAG,			
			DCNItemRev = NULL_TAG;

		tag_t
			*attach_object_list = NULLTAG;

		char
			*taskName = NULL,			
			*DCNItemId = NULL,
			*DCNVersion = NULL;

		printf("\n=================     D9_ApplyRelativeBOM start      ======================\n");
		TC_write_syslog("\n=================   D9_ApplyRelativeBOM   start   ==================\n");

		POM_AM__set_application_bypass(true); // 开启旁路

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // 获取流程根任务
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskName)); // 获取流程任务名称

		printf("\n 开始处理流程：%s\n", taskName);
		TC_write_syslog("\n 开始处理流程：%s\n", taskName);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (int i = 0; i < attach_object_num; i++)
		{
			logical
				isMNTDCNRev = false,
				isPRTDCNRev = false;
			target_obj = attach_object_list[i];

			GTCTYPE_is_type_of(target_obj, "D9_MNT_DCNRevision", &isMNTDCNRev); // 判断是否为MNT DCN对象版本
			GTCTYPE_is_type_of(target_obj, "D9_PRT_DCNRevision", &isPRTDCNRev); // 判断是否为PRT DCN对象版本

			if (!isMNTDCNRev && !isPRTDCNRev) {
				continue;
			}

			DCNItemRev = target_obj;
			break;
		}

		if (DCNItemRev != NULLTAG)
		{
			ITKCALL((AOM_ask_value_string(DCNItemRev, "item_id", &DCNItemId))); // 获取DCN零组件ID
			printf("DCNItemId == %s\n", DCNItemId);
			TC_write_syslog("DCNItemId == %s\n", DCNItemId);

			ITKCALL((AOM_ask_value_string(DCNItemRev, "item_revision_id", &DCNVersion))); // 获取DCN版本号
			printf("DCNVersion == %s\n", DCNVersion);
			TC_write_syslog("DCNVersion == %s\n", DCNVersion);

			handlerSolutionItemRev(DCNItemRev); // 处理解决方案项对象版本 
		}

	CLEANUP:
		POM_AM__set_application_bypass(false); // 关闭旁路
		DOFREE(taskName);
		DOFREE(attach_object_list);
		DOFREE(DCNItemId);
		DOFREE(DCNVersion);

		printf("\n=================     D9_ApplyRelativeBOM end      ======================\n");
		TC_write_syslog("\n=================   D9_ApplyRelativeBOM   end   ==================\n");
		return ifail;
	}
#ifdef __cplusplus
}
#endif

/**
 处理解决方案项对象版本
**/
int handlerSolutionItemRev(tag_t DCNItemRev)
{
	int		
		n_referencers = 0,
		*levels = NULL,		
		solution_item_num = 0,
		rev_count = 0,
		rcode = ITK_ok;

	tag_t
		tItem = NULLTAG,
		partItemRev = NULLTAG,
		BOMApplyItemRev = NULLTAG,
		EBOMRelType = NULLTAG,
		relation = NULLTAG;

	tag_t		
		*referencers = NULLTAG,
		*solution_item_list = NULLTAG,
		*rev_list = NULLTAG;

	char
		**relations = NULL,
		*BOMApplyItemId = NULL,
		*BOMApplyVersion = NULL;

	logical
		isRev = true;

	printf("******** start handler solutionItemRev ******** \n");
	TC_write_syslog("******** start handler solutionItemRev ******** \n");

	ITKCALL(AOM_ask_value_tags(DCNItemRev, "CMHasSolutionItem", &solution_item_num, &solution_item_list));

	if (solution_item_num <= 0)
	{
		printf("CMHasSolutionItem is Empty \n");
		TC_write_syslog("CMHasSolutionItem is Empty \n");
		goto CLEANUP;
	}
	for (int i = 0; i < solution_item_num; i++)
	{
		ITKCALL(GTCTYPE_is_type_of(solution_item_list[i], "ItemRevision", &isRev)); // 判断是否为对象版本
		if (!isRev)
		{
			continue;
		}

		partItemRev = solution_item_list[i];
		ITKCALL(ITEM_ask_item_of_rev(partItemRev, &tItem));  // 对象版本获取对象
		ITKCALL(ITEM_list_all_revs(tItem, &rev_count, &rev_list)); // 获取所有的对象版本集合

		for (int j = 0; j < rev_count - 1; j++)
		{
			ITKCALL(WSOM_where_referenced2(rev_list[j], 1, &n_referencers, &levels, &referencers, &relations));
			for (int k = 0; k < n_referencers; k++)
			{
				
				printf("查找第%d个数据，关系 == %s\n", k, relations[k]);
				TC_write_syslog("查找第%d个数据，关系 == %s\n", k, relations[k]);

				if (strcmp(relations[k], "D9_BOMReq_PCBA_REL") == 0)
				{
					BOMApplyItemRev = referencers[k]; // BOM申请单对象版本

					ITKCALL((AOM_ask_value_string(BOMApplyItemRev, "item_id", &BOMApplyItemId))); // 获取零组件ID
					printf("BOMApplyItemId == %s\n", BOMApplyItemId);
					TC_write_syslog("BOMApplyItemId == %s\n", BOMApplyItemId);

					ITKCALL((AOM_ask_value_string(BOMApplyItemRev, "item_revision_id", &BOMApplyVersion))); // 获取版本号
					printf("BOMApplyVersion == %s\n", BOMApplyVersion);
					TC_write_syslog("BOMApplyVersion == %s\n", BOMApplyVersion);

					printf("========================= \n");
					if (!isExistItemRev(BOMApplyItemRev, partItemRev)) // 判断PCBA EBOM建立信息伪文件夹是否已经关联此对象版本
					{
						printf("********* start add EBOM顶层料号对象 ********** \n");
						TC_write_syslog("********* start add EBOM顶层料号对象 ********** \n");
						ITKCALL(AOM_lock(BOMApplyItemRev)) //锁定BOM申请单对象版本
							ITKCALL(GRM_find_relation_type("D9_BOMReq_EBOM_REL", &EBOMRelType)); //返回指定关系类型名称物件的关系类型
						printf("******** create relation ******** \n");
						TC_write_syslog("******** create relation ******** \n");

						ITKCALL(GRM_create_relation(BOMApplyItemRev, partItemRev, EBOMRelType, NULL, &relation)); // BOM申请单对象添加(解决方案项下面的对象版本)到PCBA EBOM建立信息伪文件夹
						ITKCALL(GRM_save_relation(relation));
						ITKCALL(AOM_save_with_extensions(BOMApplyItemRev)); //保存BOM申请单对象版本的改变
						ITKCALL(AOM_unlock(BOMApplyItemRev)) //解锁BOM申请单对象版本
						printf("******** relation finish ******** \n");
						TC_write_syslog("******** relation finish ******** \n");
						printf("********* add finish EBOM顶层料号对象 ********** \n");
						TC_write_syslog("********* add finish EBOM顶层料号对象 ********** \n");
					}

					printf("========================= \n");
					if (!isExistItemRev(BOMApplyItemRev, DCNItemRev)) // 判断PCBA EBOM建立信息伪文件夹是否已经关联此对象版本
					{
						printf("********* start add DCN对象版本 ********** \n");
						TC_write_syslog("********* start add DCN对象版本 ********** \n");
						ITKCALL(AOM_lock(BOMApplyItemRev)) //锁定BOM申请单对象版本
						ITKCALL(GRM_find_relation_type("D9_BOMReq_EBOM_REL", &EBOMRelType)); //返回指定关系类型名称物件的关系类型
						printf("******** create relation ******** \n");
						TC_write_syslog("******** create relation ******** \n");

						ITKCALL(GRM_create_relation(BOMApplyItemRev, DCNItemRev, EBOMRelType, NULL, &relation)); // BOM申请单对象添加(DCN对象版本)到PCBA EBOM建立信息伪文件夹
						ITKCALL(GRM_save_relation(relation));
						ITKCALL(AOM_save_with_extensions(BOMApplyItemRev)); //保存BOM申请单对象版本的改变
						ITKCALL(AOM_unlock(BOMApplyItemRev)) //解锁BOM申请单对象版本
						printf("******** relation finish ******** \n");
						TC_write_syslog("******** relation finish ******** \n");
						printf("********* add finish DCN对象版本 ********** \n");
						TC_write_syslog("********* add finish DCN对象版本 ********** \n");
					}
				}							
			}
		}			
	}

	CLEANUP:
		DOFREE(solution_item_list);		
		DOFREE(rev_list);		
		DOFREE(levels);
		DOFREE(referencers);
		DOFREE(relations);
		DOFREE(BOMApplyItemId);
		DOFREE(BOMApplyVersion);
		printf("******** handler solutionItemRev finish ******** \n");
		TC_write_syslog("******** handler solutionItemRev finish ******** \n");
	return rcode;
}

boolean isExistItemRev(tag_t BOMApplyItemRev, tag_t item_rev)
{
	int
		count = 0;
	tag_t
		*ebomrel_item_list = NULLTAG;
	char
		*itemRevUid = NULL;

	ITKCALL(AOM_ask_value_tags(BOMApplyItemRev, "D9_BOMReq_EBOM_REL", &count, &ebomrel_item_list));
	ITK__convert_tag_to_uid(item_rev, &itemRevUid);
	for (size_t i = 0; i < count; i++)
	{
		tag_t ebomrelItemRev = ebomrel_item_list[i];
		char *compareUID = NULL;
		ITK__convert_tag_to_uid(ebomrelItemRev, &compareUID);
		if (strcmp(itemRevUid, compareUID) == 0)
		{
			return true;
		}
	}
	return false;
}