
/*=================================================================================
* @file					epm_register_handler.h
* @brief				itk user exits function declation, to register custom handlers
* @date					2009/2/13
* @author				Ray Li
* @history
* ===================================================================================
* Date					Name				Description
* 13-Feb-2009			Ray				created
*===================================================================================*/

#ifndef EPM_REGISTER_HANDLER_CUSTOM
#define EPM_REGISTER_HANDLER_CUSTOM



#ifdef __cplusplus
extern "C" {
#endif
	extern DLLAPI int CUST_init_gs_shell_module(int *, va_list);
	extern DLLAPI int USERSERVICE_custom_register_methods();
	extern DLLAPI int CUST_init_module(int *, va_list);

	//user service
	//extern DLLAPI int USERSERVICE_custom_register_methods();
	//int NHL_import_item( void *retValType );
	//int NHL_import_bom( void *retValType );
	//int ORIGIN_set_bypass( void *retValType );
	//int ORIGIN_close_bypass( void *retValType );
	//int G5_ebom_to_pbom (void * returnValueType);
	//int grgb_get_classfication_value(void * returnValueType);


#ifdef __cplusplus
}
#endif

#endif 
