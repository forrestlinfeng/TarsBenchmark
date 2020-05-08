
/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#ifndef __LICOTE_PRIVATE_H__
#define __LICOTE_PRIVATE_H__


/***************************************************************
* �궨��
***************************************************************/
#define LICOTE_MAX_OPTIONS			(128)	/* Licote֧�ֵ����ѡ����� */
#define LICOTE_HASH_SIZE			(10 + 26 + 26 + 1)/* HASH���С */ 
#define LICOTE_USAGE_LEN			(128)	/* �÷���󳤶� */
#define LICOTE_USAGE_SIZE			(20)	/* ÿ�����ѡ����� */
#define LICOTE_MAX_KEY_LEN			(32)	/* ��ѡ������ַ����� */
#define LICOTE_BOOL_TRUE			"1"		/* Licoteѡ���Ƿ���� */

/* Licote�汾��Ϣ */
#define __LICOTE_VERSION			"1.3.4"
#define __LICOTE_RELEASE			"2013/11/11"

/* Licote���������ѡ�� */
#define LICOTE_LIXOPT_PWD			"--licote-pwd"
#define LICOTE_WINOPT_PWD			"/licote-pwd"

/* LICOTEѡ���� */
#define	FLAGS_ONOFF					(1 << 0)
#define FLAGS_OPTIONAL				(1 << 1)
#define FLAGS_HOOK					(1 << 2)
#define FLAGS_HELP					(1 << 3)
#define FLAGS_HIDE					(1 << 4)
#define FLAGS_CLASS					(1 << 5)
#define FLAGS_DEPEND				(1 << 6)
#define FLAGS_PASSWD				(1 << 7)
#define FLAGS_APPEND				(1 << 8)
#define FLAGS_VERSION				(1 << 9)
/* ^.^ */
#define FLAGS_EXIST					(1 << 10)	// ��ǻ���û�����
#define FLAGS_NONE					(1 << 11)	// ��Ƿ�ѡ��ڵ�

enum{
	LINUX = '-',					/* Linux��� */
	WINDOWS = '/'					/* Windows��� */
};
/* �������� */
enum{
	OPT_PWD_EXIST = (1 << 0),		/* �����������ص�ѡ�� */
	OPT_PWD_CORRECT = (1 << 1)		/* �û���������ȷ������ */
};
/* �ַ�������: ��g_LicoteF[]һһ��Ӧ */
enum{
	TIPS = 0,
	DEPICT,
	USAGE,
	HIDE,
	OPTION,
	EXAMPLE,
	ERROR_OPTION,
	MISS_OPTION,
	MISS_VALUE,
	ERROR_USAGE,
	ERROR_ARGC,
	ERROR_PWD
};


/** ��ʽ���ú������ */
#define __licote_call
#define __no_call


/***************************************************************
* Licote���Ӻ���ԭ��
***************************************************************/
/** 
 * @param opt	: ƥ��ɹ���option����
 * @param argc	: �����б����
 * @param argv	: ����ָ������
 * @return	: >0: �����Ѿ�������ɵĲ�������, 
 *			  0 : ��ʾֻ�����˵�ǰoption,��û�д����κ�ֵ
 * @brief	: Licote���Ӻ���ԭ��
 */
typedef int (*licote_hook_t)(const char* opt, 
							 int argc, 
							 char** argv);
/***************************************************************
* ˽�нӿ�
***************************************************************/
#ifdef __cplusplus
	extern "C" {
#endif  
/**
 * @brief licote���캯��
*/
__no_call void 
__licote_core_init(void);

/**
 * @brief licote��������
*/
__no_call void
licote_option_exit(void)__attribute__((destructor));

/**
 * @brief ע��һ��Licoteѡ��
*/
__no_call void
licote_option_add(const char* opt, 
				  const char* flags, 
				  const char* info);

/**
 * @brief ע��ѡ��ı���
*/
__no_call void
licote_option_alias(const char* o,
					const char* a);
	
/**
 * @brief ע��ѡ��Ĺ��Ӻ���
*/
__no_call void
licote_option_hook(const char* patt, 
				   licote_hook_t hook);

#ifdef __cplusplus
}
#endif
#endif /* __LICOTE_PRIVATE_H__ */


