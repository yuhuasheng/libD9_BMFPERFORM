#include "extension.h"
#include "util.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_SubmitToRPA(EPM_action_message_t msg)
	{
		int
			ifail = ITK_ok,
			attach_object_num = 0;

		tag_t
			root_task = NULL_TAG;

		tag_t
			* attach_object_list = NULLTAG;

		char
			* taskName = NULL,
			* objectId = NULL;

		printf("\n=================     D9_SubmitToRPA start      ======================\n");
		TC_write_syslog("\n=================   D9_SubmitToRPA   start   ==================\n");

		POM_AM__set_application_bypass(true); // ������·

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // ��ȡ���̸�����
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskName)); // ��ȡ������������

		printf("\n ��ʼ�������̣�%s\n", taskName);
		TC_write_syslog("\n ��ʼ�������̣�%s\n", taskName);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));
		string uidList = "[";
		for (int i = 0; i < attach_object_num; i++)
		{
			logical
				isIRRev = false;

			ITKCALL(GTCTYPE_is_type_of(attach_object_list[i], "D9_IR_DELLRevision", &isIRRev));

			if (!isIRRev) {
				continue;
			}

			ITK__convert_tag_to_uid(attach_object_list[i], &objectId);
			printf("objectId == %s\n", objectId);
			TC_write_syslog("objectId == %s\n", objectId);
			if (uidList.length() > 1 ) {
				uidList = uidList + "," ;
			}
			uidList = uidList + "\"" + objectId + "\"";
		}
		uidList = uidList + "]";
		printf("urilist�� %s\n", uidList);
		TC_write_syslog("urilist�� %s\n", uidList);
		string url = "";
		vector<string>* urlVec = new vector<string>();
		getPreferenceByName("D9_SpringCloud_URL", *urlVec);
		if ((*urlVec).size() > 0) {
			url = (*urlVec)[0] + "/tc-service/issueManagement/submitToRpa";
			TC_write_syslog("url == %s\n", url.c_str());
		}
		string res = "";
		httpPost(url, uidList, res, 10);
		printf("�ӿڷ���ֵ�飺 %s\n", res);
		TC_write_syslog("�ӿڷ���ֵ�飺 %s\n", res);



	CLEANUP:
		POM_AM__set_application_bypass(false); // �ر���·
		DOFREE(taskName);
		DOFREE(attach_object_list);
		DOFREE(objectId);

		printf("\n=================     D9_SubmitToRPA end      ======================\n");
		TC_write_syslog("\n=================   D9_SubmitToRPA   end   ==================\n");
		return ifail;
	}
#ifdef __cplusplus
}
#endif
