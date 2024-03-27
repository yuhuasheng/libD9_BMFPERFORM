/*!
* @addtogroup common
* \file error_handling.h
* \brief ��������
* \date 2008/6/10
* \author Ray Li
*/

#ifndef SIMPLE_ERR_H_INCLUDED
#define SIMPLE_ERR_H_INCLUDED


#include <pom\pom\pom_errors.h>
//#include <itk/mem.h>                -----------20221222
//#include <tc\iman.h>                -----------20221222
#include <tc\emh.h>
//#include <tc\emh_const.h>
//#include <tc\tc.h>                -----------20221222
#include <pom\pom\pom.h>

#include <stdio.h>
#include <time.h>

#define BUFSIZE				512

//��ѡ���
#define PREF_NHL_ftp_info "NHL_ftp_info"
//#define PREF_TCE_MID_CLS		"Haier_hrTceMidCls"
//#define PREF_Haier_PlanPP_Prefix	"Haier_PlanPP_Prefix"
//#define PREF_Haier_hrModelPBCS		"Haier_hrModelPBCS"
//#define PREF_Haier_hrModelTCE		"Haier_hrModelTCE"
//#define PREF_Haier_hrArchTCE		"Haier_hrArchTCE"
////#define	PREF_TCE_ORACLE_SID		"TCE_ORACLE_SID"
////#define PREF_TCE_ORACLE_USER	"TCE_ORACLE_USER"
////#define PREF_TCE_MID_CLS_TABLE	"TCE_MID_CLS_TABLE"
//#define PREF_QH_RUN_PRO         "Haier_PlanProj_Runtime"
//#define PREF_KF_RUN_PRO         "Haier_DevProj_Runtime"
//#define PREF_XH_RUN_PRO         "Haier_Model_Runtime"
//#define PREF_Haier_send_proj_mail		"Haier_send_proj_mail"
//#define PREF_Haier_TCE_Finish_Progress	"Haier_TCE_Finish_Progress"
//#define PREF_W2_Revise_clear_form "W2_Revise_clear_form"


////��������ͺ�
//#define ITEM_hrModelFamCls			"������ѡ���"
//#define ITEM_hrArchetecture			"�ܹ�"
//#define ITEM_hrProdPlanProj			"����Ŀ"
//#define ITEM_hrModel				"�ͺ�"
//#define ITEM_hrArchNode				"�ܹ��ڵ�"
//#define ITEM_hrDevProj				"������Ŀ"
//#define ITEM_hrModuleInt			"�ӿڱ�׼"
//#define ITEM_hrModuleSpec			"ģ������"


//����
//#define FORM_MID_CLS			"�������뵥"

//���Ժ�

#define PROP_g5code		"g5Code"
#define PROP_g5SAP	"g5SAP"
#define PROP_g5codeRevision	"g5codeRevision"
//#define PROP_hrMidClsName		"hrMidClsName"
//#define PROP_hrGroup			"hrGroup"
//#define PROP_MODEL_hrPFName			"hrPFName"
//#define PROP_hrPFName			"hrPF"
//#define PROP_hrPLName			"hrPLName"
//#define PROP_hrArchCode			"hrArchCode"
//#define PROP_hrArchName			"hrArchName"
//#define PROP_hrMidClsName		"hrMidClsName"
//#define PROP_hrProdFam			"hrProdFam"
//#define PROP_hrProdLine			"hrProdLine"
//#define PROP_hrProjCode			"hrProjCode"
//#define PROP_hrProdPlanProjCode	"hrProdPlanProjCode"
//#define PROP_hrModelFamCls		"hrModelFamCls"
//#define PROP_hrNeedComp			"hrNeedComp"
//#define PROP_flag_value			"N"
//#define PROP_hrKeyStatus		"hrKeyStatus"
//#define PROP_hrIsModule			"hrIsModule"
//#define PROP_hrNumInterface		"hrNumInterface"
//#define PROP_view				"view"
//#define PROP_hrKeyStatus		"hrKeyStatus"
//#define PROP_hrLagReason		"hrLagReason"
//#define PROP_hrProjDesc			"hrProjDesc"
//#define PROP_hrProjManager		"hrProjManager"
//#define PROP_hrProjTargetType	"hrProjTargetType"
//#define PROP_hrProjType			"hrProjType"
//#define PROP_hrProjCode			"hrProjCode"
//#define PROP_hrRelated			"hrRelated"
//#define PROP_hrResponPerson		"hrResponPerson"
//#define PROP_hrProdDevProjCode	"hrProdDevProjCode"
//#define PROP_hrTargetLauchYear	"hrTargetLauchYear"
//#define PROP_hrTargetLauchMonth	"hrTargetLauchMonth"
//#define PROP_hrKeyStatus		"hrKeyStatus"
//#define PROP_hrLagReason		"hrLagReason"
//#define PROP_hrResponPerson		"hrResponPerson"
//#define PROP_hrProjManager		"hrProjManager"
//#define PROP_hrProdCode			"hrProdCode"
//#define PROP_release_status_list		"release_status_list"
//#define PROP_creation_date		"creation_date"

//����
//#define SPLIT					"|"
//#define TITLE_PLC				"PLC״̬"
//�����
#define ICS_ROOT_ID				"ICM"

//sos��
//#define	SOS_TYPE				"StoredOptionSet"

//��ѯ�������ƺ�
#define QRY_ITEM_NAME		"object_name"
#define QRY_search_wuliao	"Origin_Search_Material"
//#define QRY_hrMidClsName		"object_name"
//#define QRY_hrGroup				"hrGroup"
//#define QRY_hrPFName			"hrPFName"
//#define QRY_hrPLName			"hrPLName"
//#define QRY_ITEM_TYPE			"object_type"
//#define	QRY_Haier_hrProjDev		"Haier-hrDevProjRev"
//#define QRY_Haier_hrModelRev	"Haier-hrModelRev"

//��ѯ��Ŀ
#define QRY_g5codeRevision	"g5codeRevision"

//������
#define ARGUMENT_CHECK			"check"
#define ARGUMENT_QRY_MF_NAME	"qry_mf_name"
#define ARGUMENT_QRY_MODEL_NAME	"qry_model_name"
#define ARGUMENT_compe_percent	"compe_percent"

//���ݿ���غ�
#define TCE_ORACLE_SID			"tc"
#define TCE_ORACLE_USER			"infodba"
#define TCE_ORACLE_PASSWORD		"infodba"
#define TCE_CLASS_FIELD			"class"

//״̬��
#define STATUS_ProdPlanReleased	"ProdPlanReleased"
#define STATUS_G5XPL	"С����"
#define STATUS_G5Released	"����"
#define STATUS_G5PreRelease	"Ԥ����"
#define STATUS_G5TestRe	"ͼֽԤ����"
#define STATUS_G5TempRelease "��ʱ����"

//����
#define HAIER_MID_FOLDER				"Haier����Ŀ¼"


//�汾����
#define RULE_Any_Status_NO_working	"Any Status; No Working"//"Any Status; No Working"



#define HANDLER_ARGUMENT_ERROR					EMH_USER_error_base + 1
#define WORKFLOW_NODE_IS_NOT_VALID				EMH_USER_error_base + 2
#define HANDLER_PLACED_INVALID					EMH_USER_error_base + 3

//user errors define
//<error id="100">δ�ҵ���������</error>
#define ERROR_WULIAO_NOT_FOUND (EMH_USER_error_base + 100)
//<error id="101">δ����bomview</error>
#define ERROR_NO_FIND_VIEW (EMH_USER_error_base + 101)
//<error id="102">%1$ ��ѯ���������</error>
#define ERROR_TOP_HAS_MULTI_MATERIALS (EMH_USER_error_base + 102)
////<error id="100">��վ���ļ���ȱ�� %1$ ���ã������������������</error>
//#define ERROR_PREFERENCE_NOT_FOUND (EMH_USER_error_base + 100)
//
////<error id="101">���� %1$ ���Բ�ӦΪ��ֵ</error>
//#define ERROR_FORM_PROPERTY_VALUE_IS_NULL (EMH_USER_error_base + 101)
//
////<error id="102">�ܹ� %1$ �Ѵ���</error>
//#define ERROR_ARCHITEM_ALREADY_EXIST (EMH_USER_error_base + 102)
//
////<error id="103">����item %1$ ʱʧ�� </error>
//#define ERROR_CREATE_ITEM_FAILED (EMH_USER_error_base + 103)
//
////<error id="104">����Ŀ����δ���ҵ� %1$ ���͵ı� </error>
//#define ERROR_MID_FORM_NOT_FOUND (EMH_USER_error_base + 104)
//
////<error id="105">δ���ҵ� %1$ ���̲��� </error>
//#define ERROR_NOT_FOUND_ARGUMENT (EMH_USER_error_base + 105)
//
////<error id="106">����Ŀ����δ���ҵ� %1$ ���͵�ItemRevision </error>
//#define ERROR_MID_ITEMREV_NOT_FOUND (EMH_USER_error_base + 106)
//
////<error id="107">���̲��� %1$ δ������ȷ������ </error>
//#define ERROR_ARGUMENT_WRONG_TYPE (EMH_USER_error_base + 107)
//
////<error id="108">δ�ҵ��ļ��� %1$  </error>
//#define ERROR_NOT_FOUND_FOLDER (EMH_USER_error_base + 108)
//
////<error id="110">����TCE���ݿ�ʧ�� </error>
//#define ERROR_CONNECT_TCE_DB_FAILED (EMH_USER_error_base + 110)
//
////<error id="111">д��TCE���ݱ� %1$ʱʧ�� </error>
//#define ERROR_CALL_SQL (EMH_USER_error_base + 111)
//
////<error id="112">��ѡ�� %1$ δ������ȷ��ֵ </error>
//#define ERROR_PREF_VALUE (EMH_USER_error_base + 112)
//
////<error id="113">δ�ҵ����� %1$  </error>
//#define ERROR_NOT_FOUND_PROP (EMH_USER_error_base + 113)
//
//<error id="114">���� %1$ ֵδ������ȷ </error>
#define ERROR_FOUND_WRONG_VALUE (EMH_USER_error_base + 114)
//
////<error id="115">%1$ itemrev sos������ </error>
//#define ERROR_SOS_NOT_COMPLETE (EMH_USER_error_base + 115)
//
////<error id="116">ģ�� %1$ δ����ʵ�� </error>
//#define ERROR_NOT_FOUND_INSTANCE_PART (EMH_USER_error_base + 116)
//
//<error id="120">δ�ҵ���ѯ %1$ </error>
#define ERROR_QRY_NOT_FOUND (EMH_USER_error_base + 120)
//
////<error id="121">δ�ҵ�ƥ��� %1$</error>
//#define ERROR_QRY_NOT_FOUND_OBJECT (EMH_USER_error_base + 121)
//
////<error id="122">�ҵ���� %1$</error>
//#define ERROR_QRY_FOUND_MULTI_OBJECT (EMH_USER_error_base + 122)
//
////<error id="123">δ�ҵ�����Ϊ %1$ �ķ���</error>
//#define ERROR_ICS_NOT_FOUND (EMH_USER_error_base + 123)
//
////<error id="124">���� %1$ �Ѵ���</error>
//#define ERROR_ICS_ALREADY_EXIST (EMH_USER_error_base + 124)
//
////<error id="125">�������� %1$ ʱʧ��</error>
//#define ERROR_ICS_CREATE_FAILED (EMH_USER_error_base + 125)
//
////<error id="130">SOS %1$ ��ֵС��4λ,��ȷ��</error>
//#define ERROR_SOS_VALUE_LESS_4 (EMH_USER_error_base + 130)
//
////<error id="131">%1$ ItemRevδ�������SOS����,��ȷ��</error>
//#define ERROR_NO_SOS_EXIST (EMH_USER_error_base + 131)
//
////<error id="132">δ�ҵ�����IDΪ %1$ �������ֵ�,��ȷ��</error>
//#define ERROR_NO_DIC_ATTR_EXIST (EMH_USER_error_base + 132)
//
////<error id="133">δ�ҵ�IDΪ %1$ �ķ���,��ȷ��</error>
//#define ERROR_ICS_NOT_FOUND_CLASS (EMH_USER_error_base + 133)
//
////<error id="134">��ӷ������� %1$ ʱʧ��</error>
//#define ERROR_ICS_ADD_ATTRS_FAILED (EMH_USER_error_base + 134)
//
////<error id="135">����[����¶���]ȡֵΪN���ͺ�ռ���ͺŵı���С�ڲ���-compe_percent���õ�ֵ������޷�ִ�м�龺Ʒ��Ϣ</error>
//#define ERROR_COMP_LESS_THAN_ARGUMENT (EMH_USER_error_base + 135)
//
////<error id="136">�ܹ��ڵ� %1$ δ���κμܹ�ʹ��</error>
//#define ERROR_ARCHNODE_NOT_FOUND_PARENT_ARCH (EMH_USER_error_base + 136)
//
////<error id="137">δ�ҵ� %1$, ��ȷ��</error>
//#define ERROR_NOT_FOUND_ITEMREV (EMH_USER_error_base + 137)
//
////<error id="138">�ܹ��ڵ� %1$ �Ľӿڸ���ֵ����ͳ�ƵĽӿڱ�׼����һ��</error>
//#define ERROR_INTERFACE_NOT_COMPLETE (EMH_USER_error_base + 138)
//
////<error id="139">�ڶ��� %1$ ��δ�ҵ�ģ��˵����, ��ȷ��</error>
//#define ERROR_NOT_FOUND_SPEC (EMH_USER_error_base + 139)
//
////<error id="140">handler����ָ��status����</error>
//#define ERROR_HANDLER_ARG_COUNT (EMH_USER_error_base + 140)
//
////<error id="141">�����б�����%1$���Ͷ���</error>
//#define ERROR_HANDLER_ARG_LIST (EMH_USER_error_base + 141)
//
////<error id="142">������ %1$ ��дȨ��,��ȷ��</error>
//#define ERROR_NO_WRITE_PRIVILEGE (EMH_USER_error_base + 142)
//
////<error id="143">����������������߲�ͬ</error>
//#define ERROR_OWNER_AND_LOGIN_USER_NOT_MET (EMH_USER_error_base + 143)
//<error id="144">����%s�Ѵ���</error>
#define ERROR_ITEM_ALREADY_EXIST (EMH_USER_error_base + 144)
//<error id="145">����%s�Ѵ���</error>
#define ERROR_ITEM_NO_FIND (EMH_USER_error_base + 145)
//<error id="146">����%s�Ѵ���</error>
#define ERROR_REV_ALREADY_EXIST (EMH_USER_error_base + 146)
//<error id="147">δ�ҵ� %1$, ��ȷ��</error>
#define ERROR_NOT_FOUND_INFO_FILE (EMH_USER_error_base + 147)

#define ERROR_NOT_FOUND_INFO_DATASET (EMH_USER_error_base + 148)

#define DOFREE(obj)								\
{												\
	if(obj)										\
	{											\
		MEM_free(obj);							\
		obj = NULL;								\
	}											\
}



#define ECHO(X)      printf X;
#define SYS_LOG(X)   IMAN_write_syslog X;
#define LOG_ECHO(X)  printf X; IMAN_write_syslog X;



/*!
* \def CALL(x)
* ��ӡ������Ϣ
*/
#define CALL(x) {               \
	int stat;                     \
	char *err_string;             \
	if( (stat = (x)) != ITK_ok)   \
	{                             \
	EMH_ask_error_text (stat, &err_string);              \
	LOG_ECHO( ("ERROR: %d ERROR MSG: %s.\n",stat, err_string) )	\
	LOG_ECHO( ("Function: %s FILE: %s LINE: %d\n", #x, __FILE__, __LINE__ ) )	\
	MEM_free (err_string);   \
	return (stat);          \
	}                         \
}

/*=============================================================================*
 * MACRO:	 DEBUGSAFECALL
 * PURPOSE : do safe call of function
 *           if function's return is not normal ITK_ok, get the error info
 *           if debugFlag is open, it print log message into log file
 * INPUT:
 *     x:       Function address
 *============================================================================*/
#define DEBUGSAFECALL(x)                       																\
{                                         																	\
	printf("[debug]Doing: %s FILE: %s LINE: %d", #x, __FILE__, __LINE__);					\
}

 /*!
 * \def DO(x)
 * ��ӡ������Ϣ
 */
#define DO(x) {             \
	int stat;                     \
	char *err_string;             \
	if( (stat = (x)) != POM_ok)   \
	{                             \
	EMH_ask_error_text (stat, &err_string);              \
	printf ("ERROR: %d ERROR MSG: %s.\n", stat, err_string);        \
	printf ("Function: %s FILE: %s LINE: %d\n",#x, __FILE__, __LINE__);             \
	MEM_free (err_string);   \
	}                         \
}

 /*!
 * \def CALLRNULL(x)
 * ��ӡ������Ϣ
 */
#define CALLRNULL(x) {              \
	int stat;                     \
	char *err_string;             \
	if( (stat = (x)) != ITK_ok)   \
	{                             \
	EMH_ask_error_text (stat, &err_string);              \
	printf ("ERROR: %d ERROR MSG: %s.\n", stat, err_string);        \
	printf ("Function: %s FILE: %s LINE: %d\n",#x, __FILE__, __LINE__);             \
	MEM_free (err_string);   \
	return ((char *)NULL);  \
	}                         \
}

 /*!
 * \def CALL2(x)
 * ��ӡ������Ϣ
 */
#define CALL2(x) { \
	int stat, n_ifails, *serverities, *ifails, err_count;  \
	char *err_string, **texts;                             \
	if( (stat = (x)) != ITK_ok)                            \
	{                                                      \
	printf ("Function: %s FILE: %s LINE: %d\n",#x, __FILE__, __LINE__);             \
	EMH_ask_errors( &n_ifails, (const int**)(&serverities), (const int**)(&ifails), (const char***)(&texts)  );\
	for( err_count=0; err_count<n_ifails; err_count++ ) \
		{                                                   \
		printf( "ERROR: %d ERROR MSG: %s.\n", ifails[i], texts[i] );              \
		}                                                   \
		MEM_free( serverities );                            \
		MEM_free( ifails );                                 \
		MEM_free( texts );                                  \
		return (stat);                                      \
	}                                                       \
}                                                           \

#define CHECK_FILE(x,ret) { \
	FILE *stream = NULL;	\
	if( (stream = fopen(x,"rb")) == NULL ) {	\
	printf( "%s doesn't exists, please check!\n", x );		\
	IMAN_write_syslog( "%s doesn't exists, please check!\n", x)	;	\
	ret = -1;	\
	}	\
	else {	\
	fclose(stream);	\
	stream = NULL;	\
	ret = 0;	\
	}	\
}	\


#define CHECK_FILE_NOT_EXISTS(x,ret) { \
	FILE *stream = NULL;	\
	if( (stream = fopen(x,"rb")) != NULL ) {	\
	fclose(stream);	\
	stream = NULL;	\
	printf( "%s exists, please check!\n", x );		\
	IMAN_write_syslog( "%s exists, please check!\n", x)	;	\
	ret = -1;	\
	}	\
	else {	\
	ret = 0;	\
	}	\
}	\






#endif  
