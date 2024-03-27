#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <time.h>

bool checkVerRule(vector<string> verVec, char *version);
#ifdef __cplusplus
extern "C" {
#endif
	int itemrev_revise_post_action(METHOD_message_t* msg, va_list args) {
		printf("\n=================itemrev_revise_post_action start======================\n");
		TC_write_syslog("\n=================itemrev_revise_post_action start==================\n");
		int ifail = ITK_ok;
		POM_AM__set_application_bypass(true); //开启旁路
		// 先清除错误消息
		EMH_clear_errors();
		va_list item_args;
		va_copy(item_args, args);
		tag_t tItem_rev = NULLTAG; //对象版本
		tag_t tItem = NULLTAG; //对象		
		int count; //版本数
		int status_num = 0;
		tag_t *rev_list = NULLTAG; //所有的版本对象
		tag_t last_rev = NULLTAG; //最大版本对象
		tItem_rev = va_arg(item_args, tag_t);
		char *itemId = NULL; //零组件ID
		char *newversion = NULL; //版本号

		ITKCALL(ITEM_ask_item_of_rev(tItem_rev, &tItem));  // 对象版本获取对象
		ITKCALL(AOM_ask_value_string(tItem, "item_id", &itemId)); //对象ID
		printf("item_id == %s\n", itemId);

		ITKCALL(ITEM_ask_latest_rev(tItem, &last_rev));
		ITKCALL(AOM_ask_value_string(last_rev, "item_revision_id", &newversion));
		printf("revise rule rev item_revision_id == %s\n", newversion);	
		
		ITKCALL(ITEM_list_all_revs(tItem, &count, &rev_list)); // 获取对象的所有对象版本
		printf("count == %d\n", count);

		vector<string> vec = getReviseBeforeItemRevList(rev_list, count); // 获取升版前对象版本号集合		

		//核对版本升版规则
		bool result = checkVerRule(vec, newversion); 
		if (result == 0)
		{
//			printf("result == %s\n", result.c_str());
			ifail = REVISE_RULE_ERROR;
			EMH_store_error(EMH_severity_error, ifail);
//			EMH_store_error_s1(EMH_severity_error, ifail, G2U(result.c_str()));
			goto CLEANUP;			
		}
		else
		{		
			printf("\n 升版规则符合要求, 可以升版 \n");

		}
	CLEANUP:
		POM_AM__set_application_bypass(false);
		DOFREE(itemId);
		DOFREE(newversion);
		DOFREE(rev_list);
		printf("\n=================itemrev_revise_post_action end======================\n");
		TC_write_syslog("\n=================itemrev_revise_post_action end==================\n");
		return ifail;
	}


	

#ifdef __cplusplus
}
#endif

// 核对版本升版规则
bool checkVerRule(vector<string> verVec, char *version)
{
	printf("\n ****checkVerRule start**** \n");
	std::tr1::regex regletter("^[a-zA-Z]+$"); //配置是否是字母
	std::tr1::regex regnumber("^[0-9]+$"); //匹配是否是数字
	string letterFlag;
	string numberFlag;
	bool flag = true;
	for (int i = 0; i < verVec.size(); i++)
	{
		printf("rev item_revision_id == %s\n", verVec[i]);
		if (regex_matchstr(verVec[i], regletter) == 1) { //判断版本里面是否含有字母版 	
			letterFlag = "含有字母版";
			break;
		}
		//else if (regex_matchstr(verVec[i], regnumber)) { //判断版本里面是否含有数字版
		//	numberFlag = "含有数字版";
		//}

	}

//	string errstring = "";
	if ((regex_matchstr(version, regnumber) == 1) && tc_strcmp(letterFlag.c_str(), "含有字母版") == 0) //当前选中的数字升版，但是历史的对象版本中有字母版，需要进行报错提示
	{
//		errstring = "当前只能选择字母进行升版, 谢谢";
		flag = false;
	}
	printf("\n ****checkVerRule end**** \n");
	return flag;
}