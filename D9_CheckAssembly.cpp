#include "extension.h"
#include "errorMsg.h"
#include "util.h"
int checkChildBOMLines(tag_t bomLine, char *login_user_id, vector<string> tagAttrVec, vector<string>tagTypeVec, vector<string>tagUserGroupVec, vector<string>tagAttrUserGroupVec, char **error_msg);
int checkUserGroupAttr(tag_t rev_tag, vector<string> tagUserGroupVec, vector<string> tagAttrUserGroupVec, char **error_msg);
int  checkOwnerAndStatus(tag_t rev_tag, char *login_user_id, vector<string> tagAttrVec, char **error_msg);
extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_CheckAssembly(EPM_rule_message_t msg)
	{
		EPM_decision_t decision = EPM_go;
		int	ifail = ITK_ok,attach_object_num, i;
		char *error_msg = NULL,*login_user_id = NULL, *owning_user_id = NULL, *object_string = NULL;
		tag_t root_task = NULLTAG,window = NULLTAG, top_bomline = NULLTAG, owning_user_tag = NULLTAG;
		tag_t *attach_object_list = NULL;	
		tag_t *status_list = NULL;
		//��Ҫ���Ķ�������
		vector<string> tagTypeVec;

		//��Ҫ�����û�Ⱥ��
		vector<string> tagUserGroupVec;

		//��Ҫ���Ķ�������
		vector<string> tagAttrVec;

		//��Ҫ�����û�Ⱥ���������
		vector<string> tagAttrUserGroupVec;

		int		bvrs_count = 0;
		tag_t	*bvrs_tag = NULL;
		printf("\n=================     D9_CheckAssembly start      ======================\n");
		TC_write_syslog("\n=================   D9_CheckAssembly   start   ==================\n");

		error_msg = (char*)MEM_alloc(sizeof(char) * 9000);
		memset(error_msg, '\0', 9000);
		tc_strcpy(error_msg, "");

		getPreferenceByName("D9_Item_Check_Type", tagTypeVec);
		getPreferenceByName("D9_Item_Check_Attr", tagAttrVec);

		getPreferenceByName("D9_Item_Check_User_Group", tagUserGroupVec);//��Ҫ������Ե��û�Ⱥ��

		getPreferenceByName("D9_Item_Check_Attr_By_User_Group", tagAttrUserGroupVec);//by user group ��������

		//��ȡ��½�߹���
		ITKCALL(POM_get_user_id(&login_user_id));
		printf("\n login_user_id %s \n",login_user_id);

		//��ȡ����Ŀ�����
		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));
		for (i = 0; i < attach_object_num; i++)
		{
			//�ж��ǲ�����Ҫ���Ķ�������
			if (checkItemType(tagTypeVec, attach_object_list[i]))
			{ 
			      //���Ҷ���������
				  ITKCALL(AOM_ask_value_tag(attach_object_list[i], "owning_user", &owning_user_tag));
				  ITKCALL(POM_ask_user_id(owning_user_tag, &owning_user_id));

				  //����item name
				  ITKCALL(AOM_ask_value_string(attach_object_list[i], "object_name", &object_string));
				  printf("Ŀ�����������:%s \n", owning_user_id);
				  TC_write_syslog("Ŀ�����������:%s \n", owning_user_id);

				  ITKCALL(checkOwnerAndStatus(attach_object_list[i], login_user_id, tagAttrVec, &error_msg));		
				 
				  ITKCALL(checkUserGroupAttr(attach_object_list[i], tagUserGroupVec, tagAttrUserGroupVec, &error_msg));

				  ITKCALL(ITEM_rev_list_all_bom_view_revs(attach_object_list[i], &bvrs_count, &bvrs_tag));
				  if (bvrs_count == 0) {				
					continue;
				  }
				  ITKCALL(BOM_create_window(&window));
				  ITKCALL(BOM_set_window_top_line_bvr(window, bvrs_tag[0], &top_bomline));
				  //����ǰ��½�û�Ϊ������汾�����ߣ������Ӽ��������ߺ�״̬
				  ITKCALL(checkChildBOMLines(top_bomline, login_user_id,tagAttrVec,tagTypeVec, tagUserGroupVec, tagAttrUserGroupVec,&error_msg));
				  ITKCALL(BOM_close_window(window));
			}
		}


		if (tc_strcmp(error_msg, "") != 0)
		{	
			decision = EPM_nogo;
			EMH_store_error_s1(EMH_severity_user_error, VER_RULE_ERROR, error_msg);
			ifail = VER_RULE_ERROR;
			goto CLEANUP;
		}

	    CLEANUP:
		DOFREE(login_user_id);
		DOFREE(attach_object_list);
		printf("\n=================     D9_CheckAssembly end      ======================\n");
		TC_write_syslog("\n=================   D9_CheckAssembly   end   ==================\n");
		return decision;
	}


#ifdef __cplusplus
}
#endif


//����BOM line ���ÿ��bom�м�¼
int checkChildBOMLines(tag_t bomLine, char *login_user_id, vector<string> tagAttrVec , vector<string>tagTypeVec, vector<string>tagUserGroupVec, vector<string>tagAttrUserGroupVec, char **error_msg)
{
	int		rcode = ITK_ok;
	int		child_count = 0;
	tag_t	*child_lines = NULL;
	int		attribute;
	tag_t	rev_tag = NULLTAG;
	//����BOM��
	ITKCALL(BOM_line_ask_child_lines(bomLine, &child_count, &child_lines));
	for (int i = 0; i < child_count; i++)
	{
		//ITKCALL(BOM_line_look_up_attribute(bomAttr_lineItemRevTag, &attribute));-----------------20221222
		//ITKCALL(BOM_line_ask_attribute_tag(child_lines[i], attribute, &rev_tag));------------------20221222
		ITKCALL(AOM_ask_value_tag(child_lines[i], bomAttr_lineItemRevTag, &rev_tag));
		
		//�����������ߺ�״̬
		if(checkItemType(tagTypeVec, rev_tag)) {
			ITKCALL(checkOwnerAndStatus(rev_tag, login_user_id, tagAttrVec,error_msg));
			ITKCALL(checkUserGroupAttr(rev_tag, tagUserGroupVec,tagAttrUserGroupVec, error_msg));
			ITKCALL(checkChildBOMLines(child_lines[i], login_user_id, tagAttrVec, tagTypeVec, tagUserGroupVec,tagAttrUserGroupVec,error_msg));
		}
	}
	return rcode;
}



//�����������ߺ�״̬
int checkOwnerAndStatus(tag_t rev_tag, char *login_user_id, vector<string> tagAttrVec,char **error_msg)
{
	int		rcode = ITK_ok;
	tag_t	owning_user_tag = NULLTAG;
	char	*owning_user_id = NULL;
	int		status_num = 0;
	tag_t	*status_list = NULL;
	char	*object_string = NULL, *object_id=NULL;
	char	tempStr[256];

	//��ȡ����������
	ITKCALL(AOM_ask_value_tag(rev_tag, "owning_user", &owning_user_tag));
	ITKCALL(POM_ask_user_id(owning_user_tag, &owning_user_id));

    //item name
	ITKCALL(AOM_ask_value_string(rev_tag, "object_name", &object_string));
	ITKCALL(AOM_ask_value_string(rev_tag, "item_id", &object_id));
	
	ITKCALL(AOM_ask_value_tags(rev_tag, "release_status_list", &status_num, &status_list));
	if (tc_strcmp(owning_user_id, login_user_id) != 0 && status_num<=0)
	{
		memset(tempStr, '\0', 256);
		sprintf(tempStr, G2B5("����:%s/%s,�ǵ�ǰ��½�û�����,����������:%s,��δ����,�뷢��!\n"), object_id, object_string, owning_user_id);
		TC_write_syslog(G2B5("����:%s/%s,�ǵ�ǰ��½�û�����,����������:%s,��δ����,�뷢��!\n"), object_id, object_string, owning_user_id);
		if(strlen(*error_msg)<8000){
	    	tc_strcat(*error_msg, tempStr);
		}
	}
	if (status_num<=0) {
	    //����������
		if (strlen(*error_msg) < 8000) {
			checkTagAttr(tagAttrVec, rev_tag, error_msg);
		}
	}

	return rcode;
}



//��� user group attr
int checkUserGroupAttr(tag_t rev_tag, vector<string> tagUserGroupVec, vector<string> tagAttrUserGroupVec, char **error_msg)
{
	tag_t	*status_list = NULL;
	int		status_num = 0;
	int		rcode = ITK_ok, isFind=0;
	tag_t	owning_group_tag = NULLTAG;
	char	*user_group, *object_id = NULL;
	ITKCALL(AOM_ask_value_string(rev_tag, "item_id", &object_id));
	ITKCALL(AOM_ask_value_tag(rev_tag, "owning_group", &owning_group_tag));
	ITKCALL(AOM_ask_value_string(owning_group_tag, "full_name", &user_group));
	printf("=====%s user group1:%s\n", object_id, user_group);

	for (int i = 0; i < tagUserGroupVec.size(); i++)
	{
		printf("===== user group2:%s\n", tagUserGroupVec[i].c_str());
		char  c[120];
		strcpy(c, tagUserGroupVec[i].c_str());
		if (endWith(c, user_group)) {
			isFind = 1;
		}
	}

	//����������
	if (isFind==1) {
		if (strlen(*error_msg) < 8000) {
			
				ITKCALL(AOM_ask_value_tags(rev_tag, "release_status_list", &status_num, &status_list));
				if (status_num<=0) {
		      	checkStrAttr(tagAttrUserGroupVec, rev_tag, error_msg);
			
				}
		}
	}

	return rcode;
}

 


