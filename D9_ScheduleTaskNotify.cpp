#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <curl/curl.h>
#include <dispatcher/dispatcher_itk.h>

#define CURL_STATICLIB
extern "C" int POM_AM__set_application_bypass(logical bypass);

#ifdef __cplusplus
extern "C" {
#endif

	void sendGetHttp(char* url);

	int D9_ScheduleTaskNotify(EPM_action_message_t msg)
	{
		char
			* task_uid = NULL,
			* taskname = NULL,
			* scheduleName = NULL,
			* endDate = NULL,
			* taskId = NULL,
			* scheduleId = NULL,
			* actionItemId = NULL,
			char_cmd[200] = "",
			char_status_cms[200] = "";

		tag_t
			current_task = NULLTAG,
			root_task = NULLTAG,
			target_obj = NULLTAG,
			scheduleTask = NULLTAG,
			trans_rqst = NULLTAG,
			schedule = NULLTAG,
			*schedule_task_list = NULLTAG;

		date_t dueDate;
		int
			schedule_task_num = 0,
			ifail = ITK_ok;

		printf("\n=================     D9_ScheduleTaskNotify start      ======================\n");
		TC_write_syslog("\n=================   D9_ScheduleTaskNotify start   ==================\n");

		current_task = msg.task;
		//获取根任务
		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskname));

		printf("\n 开始处理流程：%s\n", taskname);
		TC_write_syslog("\n 开始处理流程：%s\n", taskname);

		ITKCALL(AOM_ask_value_tags(root_task, "project_task_attachments", &schedule_task_num, &schedule_task_list));

		TC_write_syslog("\n schedule_task_num：%d\n", schedule_task_num);

		for (size_t i = 0; i < schedule_task_num; i++) {
			logical
				isScheduleTask = false;
			target_obj = schedule_task_list[i];

			GTCTYPE_is_type_of(target_obj, "ScheduleTask", &isScheduleTask);
			if (isScheduleTask)
			{
				printf("\n schedule_task\n");
				scheduleTask = target_obj;
				break;
			}
		}


		if (scheduleTask != NULL) {
			string url = "";
			string taskstatusUrl = "";
			vector<string>* urlVec = new vector<string>();
			getPreferenceByName("D9_SpringCloud_URL", *urlVec);
			if ((*urlVec).size() > 0) {
				url = (*urlVec)[0] + "/tc-integrate/meet/sendMeetTaskEmail";
				TC_write_syslog("url == %s\n", url.c_str());
				taskstatusUrl = (*urlVec)[0] + "/tc-integrate/meet/sendTaskStatusToTCFR";
				TC_write_syslog("taskstatusUrl == %s\n", taskstatusUrl.c_str());
			}
			//url = "http://localhost:8354/meet/sendMeetTaskEmail";
			ITKCALL(AOM_ask_value_tag(scheduleTask, "schedule_tag", &schedule));
			ITKCALL(AOM_ask_value_string(scheduleTask, "object_desc", &actionItemId));
			TC_write_syslog("actionItemId == %s\n", actionItemId);
			if (schedule != NULLTAG) {
				ITKCALL(AOM_ask_value_date(schedule, "finish_date", &dueDate));
				endDate = (char*)MEM_alloc(sizeof(char) * 4096);
				formatDate(endDate, "%Y/%m/%d", dueDate);
				TC_write_syslog("endDate == %s\n", endDate);

				ITK__convert_tag_to_uid(schedule, &scheduleId);
				TC_write_syslog("scheduleId == %s\n", scheduleId);

				ITK__convert_tag_to_uid(scheduleTask, &taskId);
				TC_write_syslog("taskId == %s\n", taskId);

				//sprintf(char_cmd, "curl   --connect-time 10  --max-time 10 --url \"%s?endDate=%s&taskId=%s&scheduleId=%s\"", url.data(), endDate, taskId, scheduleId);
				sprintf(char_cmd, "%s?endDate=%s&taskId=%s&scheduleId=%s", url.data(), endDate, taskId, scheduleId);
				TC_write_syslog("char_cmd ::: %s\n ", char_cmd);

				sprintf(char_status_cms, "%s?taskId=%s&actionItemId=%s&state=%s", taskstatusUrl.data(), taskId, actionItemId, "1");
				TC_write_syslog("char_status_cms ::: %s\n ", char_status_cms);

				sendGetHttp(char_cmd);	// 发送任务完成提醒主责人邮件

				sendGetHttp(char_status_cms); // 任务完成，将任务状态回传给TCFR
			}
		}

	CLEANUP:
		POM_AM__set_application_bypass(false);
		printf("\n start CLEANUP\n");
		DOFREE(taskname);
		DOFREE(schedule_task_list);
		DOFREE(actionItemId);
		DOFREE(scheduleId);
		DOFREE(taskId);		
		printf("\n ending CLEANUP over\n");
		TC_write_syslog("\n=================   D9_ScheduleTaskNotify   end   ==================\n");
		return ifail;
	}

	void sendGetHttp(char* url) {
		CURL* curl;
		CURLcode res;
		curl = curl_easy_init();
		TC_write_syslog("http curl :: %d\n", curl);
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}
		TC_write_syslog("http get status :: %d\n", res);
	}
#ifdef __cplusplus
}
#endif
