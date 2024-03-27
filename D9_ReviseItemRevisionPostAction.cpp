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
		POM_AM__set_application_bypass(true); //������·
		// �����������Ϣ
		EMH_clear_errors();
		va_list item_args;
		va_copy(item_args, args);
		tag_t tItem_rev = NULLTAG; //����汾
		tag_t tItem = NULLTAG; //����		
		int count; //�汾��
		int status_num = 0;
		tag_t *rev_list = NULLTAG; //���еİ汾����
		tag_t last_rev = NULLTAG; //���汾����
		tItem_rev = va_arg(item_args, tag_t);
		char *itemId = NULL; //�����ID
		char *newversion = NULL; //�汾��

		ITKCALL(ITEM_ask_item_of_rev(tItem_rev, &tItem));  // ����汾��ȡ����
		ITKCALL(AOM_ask_value_string(tItem, "item_id", &itemId)); //����ID
		printf("item_id == %s\n", itemId);

		ITKCALL(ITEM_ask_latest_rev(tItem, &last_rev));
		ITKCALL(AOM_ask_value_string(last_rev, "item_revision_id", &newversion));
		printf("revise rule rev item_revision_id == %s\n", newversion);	
		
		ITKCALL(ITEM_list_all_revs(tItem, &count, &rev_list)); // ��ȡ��������ж���汾
		printf("count == %d\n", count);

		vector<string> vec = getReviseBeforeItemRevList(rev_list, count); // ��ȡ����ǰ����汾�ż���		

		//�˶԰汾�������
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
			printf("\n ����������Ҫ��, �������� \n");

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

// �˶԰汾�������
bool checkVerRule(vector<string> verVec, char *version)
{
	printf("\n ****checkVerRule start**** \n");
	std::tr1::regex regletter("^[a-zA-Z]+$"); //�����Ƿ�����ĸ
	std::tr1::regex regnumber("^[0-9]+$"); //ƥ���Ƿ�������
	string letterFlag;
	string numberFlag;
	bool flag = true;
	for (int i = 0; i < verVec.size(); i++)
	{
		printf("rev item_revision_id == %s\n", verVec[i]);
		if (regex_matchstr(verVec[i], regletter) == 1) { //�жϰ汾�����Ƿ�����ĸ�� 	
			letterFlag = "������ĸ��";
			break;
		}
		//else if (regex_matchstr(verVec[i], regnumber)) { //�жϰ汾�����Ƿ������ְ�
		//	numberFlag = "�������ְ�";
		//}

	}

//	string errstring = "";
	if ((regex_matchstr(version, regnumber) == 1) && tc_strcmp(letterFlag.c_str(), "������ĸ��") == 0) //��ǰѡ�е��������棬������ʷ�Ķ���汾������ĸ�棬��Ҫ���б�����ʾ
	{
//		errstring = "��ǰֻ��ѡ����ĸ��������, лл";
		flag = false;
	}
	printf("\n ****checkVerRule end**** \n");
	return flag;
}