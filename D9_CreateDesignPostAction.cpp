#include "extension.h"
#include "util.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif
	int design_create_post_action(METHOD_message_t* msg, va_list args) {
		int ifail = ITK_ok;
		printf("\ndesign_create_post_action commplete\n");
		TC_write_syslog("\ndesign_create_post_action \n");
		POM_AM__set_application_bypass(true);
		va_list item_args;
		va_copy(item_args, args);
		tag_t tItem = NULLTAG;
		tag_t tItem_rev = NULLTAG;
		tItem = va_arg(item_args, tag_t);
		char *itemId = NULL; //零组件ID
		const char *templateItemId; //模板零组件ID
		char *codeNR = NULL; //编码规则
		char *objectType = NULL; //对象类型
		char *templateName = NULL; //模板名称
		char *version = NULL; //对象版本
		const char *uid = NULL;
		tag_t templateItemTag = NULLTAG; //数据集模板对象
		tag_t templateItemRevTag = NULLTAG; //数据集模板对象版本
		tag_t relType = NULLTAG;
		tag_t relation = NULLTAG;
		int tagCount = 0;
		tag_t *tag_list = NULLTAG;
		tag_t newDatasetName = NULLTAG;
		char  *datasetName = NULL;
		char  *datasetId = NULL;

		ITKCALL(WSOM_ask_object_type2(tItem, &objectType));
		TC_write_syslog("objectType == %s\n", objectType);
		ITKCALL(AOM_ask_value_string(tItem, "item_id", &itemId));
		TC_write_syslog("item_id == %s\n", itemId);
		ITKCALL(AOM_ask_value_string(tItem, "fx8_CodeNR", &codeNR));
		printf("fx8_CodeNR == %s\n", codeNR);
		ITKCALL(AOM_ask_value_string(tItem, "fx8_DateMB", &templateName));
		printf("fx8_DateMB == %s\n", templateName);
		ITKCALL(ITEM_ask_latest_rev(tItem, &tItem_rev));
		ITKCALL(AOM_ask_value_string(tItem_rev, "item_revision_id", &version));
		printf("rev objName == %s\n", version);

		vector<string> vec;
		//获取首选项
		getPreferenceByName("Fx8_3D_Template", vec);
		TC_write_syslog("Fx8_3D_Template size == %d\n", vec.size());
		// 获取模板数据集零组件ID
		string result = getTemplateId(vec, templateName);
		uid = result.c_str();
		//获取零组件对象
		ITK__convert_uid_to_tag(uid, &templateItemTag);
		ITKCALL(ITEM_ask_latest_rev(templateItemTag, &templateItemRevTag));		
		ITKCALL(AOM_ask_value_string(templateItemRevTag, "item_id", &itemId));
		printf("item_id == %s\n", itemId);

		ITKCALL(AOM_ask_value_tags(templateItemRevTag, "IMAN_specification", &tagCount, &tag_list));
		for (int j = 0; j < tagCount; j++)
		{
			ITKCALL(AOM_ask_value_string(tag_list[j], "object_name", &datasetName));
			printf("original datasetName == %s\n", datasetName);
			//			ITKCALL(WSOM_copy(tag_list[j], itemId, &newDatasetName));
			ITKCALL(AE_copy_dataset_with_id(tag_list[j], itemId, NULL, NULL, &newDatasetName));
			ITKCALL(AOM_ask_value_string(newDatasetName, "object_name", &datasetName));
			printf("copy datasetName == %s\n", datasetName);

			ITKCALL(AOM_lock(tItem_rev)) //锁定设计对象
			ITKCALL(GRM_find_relation_type("IMAN_specification", &relType)); //返回指定关系类型名称物件的关系类型
			TC_write_syslog("create relation ... \n");
			//添加关联关系
			ITKCALL(GRM_create_relation(tItem_rev, newDatasetName, relType, NULL, &relation));
			ITKCALL(GRM_save_relation(relation));
			ITKCALL(AOM_save_with_extensions(tItem_rev)); //保存设计对象的改变
			ITKCALL(AOM_unlock(tItem_rev)) //解锁设计对象
			TC_write_syslog("relation finish ... \n");
		}
		POM_AM__set_application_bypass(false);
		DOFREE(objectType); //释放对象内存
		DOFREE(itemId);
		DOFREE(codeNR);
		DOFREE(templateName);
		DOFREE(version);
		DOFREE(tag_list);
		DOFREE(datasetName);

		return ifail;
	}

#ifdef __cplusplus
}
#endif