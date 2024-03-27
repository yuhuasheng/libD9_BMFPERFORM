#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include "search.h"
#include <direct.h>
#include <iostream>
#include <codecvt>


extern "C" int POM_AM__set_application_bypass(logical bypass);
tag_t getTemplateFile(const char* templateItemId, const char* itemId);



#ifdef __cplusplus
extern "C" {
#endif

    int D9_CreateItemPostAction(METHOD_message_t* msg, va_list args) {
        TC_write_syslog("\n================= D9_CreateItemPostAction start ==================\n");
        int ifail = ITK_ok;
        POM_AM__set_application_bypass(true);

        va_list item_args;
        va_copy(item_args, args);    

        tag_t
            templateDataset = NULLTAG,
            relType = NULLTAG,
            relation = NULLTAG,
            tItem = NULLTAG,
            tItemRev = NULLTAG;

        tItem = va_arg(item_args, tag_t);

        char 
            *itemId = NULL,
            *objectType = NULL,
            *version = NULL;
            
        const char*
            templateItemId = NULL;

        ITKCALL(WSOM_ask_object_type2(tItem, &objectType));
        TC_write_syslog("objectType == %s\n", objectType);

        ITKCALL(AOM_ask_value_string(tItem, "item_id", &itemId));
        TC_write_syslog("item_id == %s\n", itemId);

        ITKCALL(ITEM_ask_latest_rev(tItem, &tItemRev));
        ITKCALL(AOM_ask_value_string(tItemRev, "item_revision_id", &version));
        TC_write_syslog("version == %s\n", version);

        vector<string> vec;
        getPreferenceByName("D9_Item_Create_PostAction_Excel_Template", vec);

        templateItemId = vec[0].c_str();

        TC_write_syslog("templateItemId == %s\n", templateItemId);
        templateDataset = getTemplateFile(templateItemId, itemId); // 获取Excel模板文件

        boolean flag = checkExistItem(tItemRev, templateDataset, "IMAN_specification"); // 判断是否挂载
        if (!flag) {
            ITKCALL(AOM_lock(tItemRev)) //锁定设计对象
            ITKCALL(GRM_find_relation_type("IMAN_specification", &relType)); //返回指定关系类型名称物件的关系类型
            TC_write_syslog("create relation ... \n");

            ITKCALL(GRM_create_relation(tItemRev, templateDataset, relType, NULL, &relation)); //添加关联关系
            ITKCALL(GRM_save_relation(relation));
            ITKCALL(AOM_save_with_extensions(tItemRev)); //保存设计对象的改变
            ITKCALL(AOM_unlock(tItemRev)) //解锁设计对象
            TC_write_syslog("relation finish ... \n");
        }



    CLEANUP:
        POM_AM__set_application_bypass(false);
        DOFREE(objectType); //释放对象内存
        DOFREE(itemId);
        DOFREE(version);        
        return ifail;
    }

#ifdef __cplusplus
}
#endif





tag_t getTemplateFile(const char* templateItemId, const char* itemId) {



    char** qry_entries = (char**)MEM_alloc(sizeof(char**));
    char** qry_values = (char**)MEM_alloc(sizeof(char**));
    qry_entries[0] = (char*)MEM_alloc(sizeof(char*) * 200);
    qry_values[0] = (char*)MEM_alloc(sizeof(char*) * 200);



    tc_strcpy(qry_entries[0], ID);
    tc_strcpy(qry_values[0], templateItemId);



    char
        * targetType = NULL,
        * datasetName = NULL;



    tag_t
        * queryResults = NULLTAG,
        * tag_list = NULLTAG;



    tag_t
        templateItem = NULLTAG,
        templateItemRev = NULLTAG,
        templateDataset = NULLTAG,
        newTemplateDataset = NULLTAG;
    int
        resultLength = 0,
        tagCount = 0;





    ITKCALL(query(D9_ItemID, qry_entries, qry_values, 1, &queryResults, &resultLength));
    if (resultLength > 0) {
        templateItem = queryResults[0];
        ITKCALL(ITEM_ask_latest_rev(templateItem, &templateItemRev));



        ITKCALL(AOM_ask_value_tags(templateItemRev, "IMAN_specification", &tagCount, &tag_list));



        for (int i = 0; i < tagCount; i++) {
            ITKCALL(WSOM_ask_object_type2(tag_list[i], &targetType));
            if (tc_strcmp(targetType, "MSExcel") == 0 | tc_strcmp(targetType, "MSExcelX") == 0) {
                ITKCALL(AOM_ask_value_string(tag_list[i], "object_name", &datasetName)); // 获取数据名称
                TC_write_syslog("\nExcel数据集名称为 == %s\n", datasetName);



                templateDataset = tag_list[i];
                break;
            }
        }
    }



    if (templateDataset != NULL) {
        ITKCALL(AE_copy_dataset_with_id(templateDataset, itemId, NULL, NULL, &newTemplateDataset));
        ITKCALL(AOM_ask_value_string(newTemplateDataset, "object_name", &datasetName));
        TC_write_syslog("copy datasetName == %s\n", datasetName);
    }
CLEANUP:
    DOFREE(tag_list);
    DOFREE(targetType);
    DOFREE(datasetName);



    return newTemplateDataset;
}