#include "extension.h"
#include "util.h"
#include <tc/folder.h>

extern "C" int POM_AM__set_application_bypass(logical bypass);
int recurseBOMLine(tag_t bomLine);
string getAltGroupByRule(int findNum);

#ifdef __cplusplus
extern "C" {
#endif

	/*
	* 此handler暂时不用，修改为dispatcher统一设置替代料群组
	*/
	int D9_Save_MNT_L10_AltGroup(EPM_action_message_t msg)
	{
		int ifail = ITK_ok,
			attach_object_num = 0;

		tag_t
			root_task = NULL_TAG,
			target_obj = NULL_TAG,
			tItem = NULL_TAG,
			finishPartItemRev = NULL_TAG,
			window = NULL_TAG,
			topLine = NULL_TAG;

		tag_t
			* attach_object_list = NULLTAG,
			* struct_tag_list = NULLTAG;

		char
			* taskName = NULL,
			* itemId = NULL,
			* version = NULL;

		TC_write_syslog("\n=================   D9_Save_MNT_L10_AltGroup  start   ==================\n");
		POM_AM__set_application_bypass(true); // 开启旁路

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // 获取流程根任务
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskName));
		
		TC_write_syslog("\n 开始处理流程：%s\n", taskName);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));
		for (size_t i = 0; i < attach_object_num; i++)
		{
			logical
				isFinishPartRev = false;
			target_obj = attach_object_list[i];

			GTCTYPE_is_type_of(target_obj, "D9_FinishedPartRevision", &isFinishPartRev);
			

			if (isFinishPartRev)
			{
				ITKCALL(AOM_ask_value_string(target_obj, "item_id", &itemId));
				ITKCALL((AOM_ask_value_string(target_obj, "item_revision_id", &version)));

				if (!startWith(itemId, "8"))
				{
					continue;					
				}

				TC_write_syslog("itemId == %s\n", itemId);
				TC_write_syslog("version == %s\n", version);

				try
				{
					if (!isBom(target_obj))
					{
						continue;
					}

					ITKCALL(ITEM_ask_item_of_rev(target_obj, &tItem));

					ITKCALL(BOM_create_window(&window)); // 创建BOMWindow

					ITKCALL(BOM_set_window_top_line(window, tItem, target_obj, NULLTAG, &topLine)); // 获取顶层BOMLine
					ITKCALL(recurseBOMLine(topLine)); // 递归遍历所有的BOMLine
				}
				catch (const std::exception& e) {
					TC_write_syslog("error == %s\n", e.what());
				}

				ITKCALL(BOM_save_window(window)); // 保存BOMIWindow
				ITKCALL(BOM_close_window(window)); // 关闭BOMWindow
				
			}
		}


	


	CLEANUP:
		TC_write_syslog("\n start handler CLEANUP\n");
		DOFREE(taskName);
		DOFREE(attach_object_list);	
		DOFREE(itemId);
		DOFREE(version);		
		TC_write_syslog("\n ending CLEANUP over\n");
		POM_AM__set_application_bypass(false); // 关闭旁路
		TC_write_syslog("\n=================   D9_Check_MNT_DCN  end   ==================\n");
		return ifail;
	}


#ifdef __cplusplus
}
#endif


/*
* 递归遍历BOMLine
*/
int recurseBOMLine(tag_t bomLine) {
	int 
		ifail = ITK_ok,
		child_count = 0,
		sub_child_count = 0;

	tag_t
		* children = NULLTAG,
		* sub_children = NULLTAG;

	char
		* findNum = NULL,
		* item_id = NULL,
		* version = NULL;


	ITKCALL(BOM_line_ask_child_lines(bomLine, &child_count, &children));
	for (int i = 0; i < child_count; i++) {

		logical
			isSubstitute = false,
			hasSubstitutes = false;			

		ITKCALL(BOM_line_ask_is_substitute(children[i], &isSubstitute)); // 判断是否为替代料

		if (isSubstitute)
		{
			continue;
		}

		ITKCALL(BOM_line_ask_has_substitutes(children[i], &hasSubstitutes)); // 判断是否含有替代料

		if (hasSubstitutes)
		{
 			ITKCALL((AOM_ask_value_string(children[i], "bl_sequence_no", &findNum)));
			ITKCALL((AOM_ask_value_string(children[i], "bl_item_item_id", &item_id)));
			ITKCALL((AOM_ask_value_string(children[i], "bl_rev_item_revision_id", &version)));

			TC_write_syslog("\n item_id = %s, version = %s, findNum = %s\n", item_id, version, findNum);

			if (findNum == NULL)
			{
				continue;
			}

			string altGroup = getAltGroupByRule(atoi(findNum));

			TC_write_syslog("altGroup == %s\n", altGroup);

			ITKCALL(AOM_set_value_string(children[i], "bl_occ_d9_AltGroup", altGroup.c_str())); //设置替代料群主编号属性 
//			updateProp(children[i], "bl_occ_d9_AltGroup", altGroup.c_str()); // 保存替代料群主编号属性

			ITKCALL(BOM_line_ask_child_lines(children[i], &sub_child_count, &sub_children)); // 判断替代料下是否含有BOM

			for (size_t j = 0; j < sub_child_count; j++)
			{
				ITKCALL(recurseBOMLine(sub_children[j]))
			}
		}

		ITKCALL(recurseBOMLine(children[i]))
	}

CLEANUP:
	DOFREE(children);
	DOFREE(findNum);
	DOFREE(item_id);
	DOFREE(version);
	DOFREE(sub_children);
	return ifail;
}


/*
* 根据查找编号获取物料群主编号
*/
string getAltGroupByRule(int findNum) {
	string altGroup = "";
	if (findNum == NULL)
	{
		return "";
	}
	else if (findNum >= 10 && findNum < 1000)
	{
		altGroup = to_string(findNum);
		altGroup.pop_back();
	}
	else if (findNum >= 1000 && findNum <= 3599)
	{
		string str = to_string(findNum);
		int first = atoi(str.substr(0, 2).c_str());
		first += 55;

		string letter = "";
		letter.push_back(char(first));


		if (strcmp(letter.c_str(), "I") == 0 || strcmp(letter.c_str(), "O") == 0)
		{
			first++;
			letter.push_back(char(first));
		}

		str = str.substr(2, str.length() - 1);

		str.pop_back();
		altGroup = letter + str;
	}

	return altGroup;
}