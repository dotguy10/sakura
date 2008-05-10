/*!	@file
	@brief PPA Library Handler

	PPA.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author YAZAKI
	@date 2002�N1��26��
*/
/*
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, YAZAKI, aroka, genta, Moca
	Copyright (C) 2003, Moca, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "stdafx.h"
#include "CPPA.h"
#include "view/CEditView.h"
#include "Funccode.h"
#include "debug/Debug.h"
#include "CMacro.h"
#include "macro/CSMacroMgr.h"// 2002/2/10 aroka
#include "CShareData.h"
#include "CControlTray.h"
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "OleTypes.h"

#define NEVER_USED_PARAM(p) ((void)p)


// 2003.06.01 Moca
#define omGet (0)
#define omSet (1)

CNativeA		CPPA::m_cMemRet;
CNativeA		CPPA::m_cMemDebug;
CEditView*		CPPA::m_pcEditView = NULL;
DLLSHAREDATA*	CPPA::m_pShareData = NULL;
bool			CPPA::m_bError = false;


CPPA::CPPA()
{
}

CPPA::~CPPA()
{
	//	Apr. 15, 2002 genta cleanup�����ǉ�
	if( IsAvailable()){
		DeinitDll();
	}
}


//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
void CPPA::Execute(CEditView* pcEditView )
{
	m_pcEditView = pcEditView;
	m_pShareData = CShareData::getInstance()->GetShareData();
	m_bError = false;			//	2003.06.01 Moca
	m_cMemDebug.SetString("");	//	2003.06.01 Moca
	m_fnExecute();
}

LPCTSTR CPPA::GetDllName( LPCTSTR str )
{
	return _T("PPA.DLL");
}

/*!
	DLL�̏�����

	�֐��̃A�h���X���擾���ă����o�ɕۊǂ���D

	@retval 0 ����
	@retval 1 �A�h���X�擾�Ɏ��s
*/
int CPPA::InitDll()
{
	/* PPA.DLL�������Ă���֐������� */

	//	Apr. 15, 2002 genta const��t����
	//	�A�h���X�̓���ꏊ�̓I�u�W�F�N�g�Ɉˑ�����̂�
	//	static�z��ɂ͂ł��Ȃ��B
	const ImportTable table[] = 
	{
		{ &m_fnExecute,		"Execute" },
		{ &m_fnSetDeclare,	"SetDeclare" },
		{ &m_fnSetSource,	"SetSource" },
		{ &m_fnSetDefProc,	"SetDefProc" },
		{ &m_fnSetDefine,	"SetDefine" },
		{ &m_fnSetIntFunc,	"SetIntFunc" },
		{ &m_fnSetStrFunc,	"SetStrFunc" },
		{ &m_fnSetProc,		"SetProc" },
		{ &m_fnSetErrProc,	"SetErrProc" },
		{ &m_fnAbort,		"ppaAbort" },
		{ &m_fnGetVersion,	"GetVersion" },
		{ &m_fnDeleteVar,	"DeleteVar" },
		{ &m_fnGetArgInt,	"GetArgInt" },
		{ &m_fnGetArgStr,	"GetArgStr" },
		{ &m_fnGetArgBStr,	"GetArgBStr" },
		{ &m_fnGetIntVar,	"GetIntVar" },
		{ &m_fnGetStrVar,	"GetStrVar" },
		{ &m_fnGetBStrVar,	"GetBStrVar" },
		{ &m_fnSetIntVar,	"SetIntVar" },
		{ &m_fnSetStrVar,	"SetStrVar" },
		{ &m_fnAddIntObj,	"AddIntObj" },
		{ &m_fnAddStrObj,	"AddStrObj" },
		{ &m_fnAddIntVar,	"AddIntVar" },
		{ &m_fnAddStrVar,	"AddStrVar" },
		{ &m_fnSetIntObj,	"SetIntObj" },
		{ &m_fnSetStrObj,	"SetStrObj" },

#if PPADLL_VER >= 120
		{ &m_fnAddRealVar,	"AddRealVar" },
		{ &m_fnSetRealObj,	"SetRealObj" },
		{ &m_fnAddRealObj,	"AddRealObj" },
		{ &m_fnGetRealVar,	"GetRealVar" },
		{ &m_fnSetRealVar,	"SetRealVar" },
		{ &m_fnSetRealFunc,	"SetRealFunc" },
		{ &m_fnGetArgReal,	"GetArgReal" },
#endif

#if PPADLL_VER >= 123
		{ &m_fnIsRunning, "IsRunning" },
		{ &m_fnSetFinishProc, "SetFinishProc"}, // 2003.06.23 Moca
#endif

		{ NULL, 0 }
	};

	//	Apr. 15, 2002 genta
	//	CDllHandler�̋��ʊ֐�������
	if( ! RegisterEntries(table) )
		return 1;

	SetIntFunc(CPPA::stdIntFunc);	// 2003.02.24 Moca
	SetStrFunc(CPPA::stdStrFunc);
	SetProc(CPPA::stdProc);

	// 2003.06.01 Moca �G���[���b�Z�[�W��ǉ�
	SetErrProc(CPPA::stdError);
	SetStrObj(CPPA::stdStrObj);	// UserErrorMes�p
#if PPADLL_VER >= 123
	SetFinishProc(CPPA::stdFinishProc);
#endif

	SetDefine( "sakura-editor" );	// 2003.06.01 Moca SAKURA�G�f�B�^�p�Ǝ��֐�������
	AddStrObj( "UserErrorMes", "", FALSE, 2 ); // 2003.06.01 �f�o�b�O�p������ϐ���p��

	int i;
	
	//	Jun. 16, 2003 genta �ꎞ��ƃG���A
	char buf[1024];
	// �R�}���h�ɒu���������Ȃ��֐� �� PPA�����ł͎g���Ȃ��B�B�B
	for (i=0; CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; i++) {
		//	2003.06.08 Moca �������[���[�N�̏C��
		//	2003.06.16 genta �o�b�t�@���O����^����悤��
		//	�֐��o�^�p��������쐬����
		GetDeclarations( CSMacroMgr::m_MacroFuncInfoNotCommandArr[i], buf );
		SetDefProc( buf );
	}

	// �R�}���h�ɒu����������֐� �� PPA�����ł��g����B
	for (i=0; CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName != NULL; i++) {
		//	2003.06.08 Moca �������[���[�N�̏C��
		//	2003.06.16 genta �o�b�t�@���O����^����悤��
		//	�֐��o�^�p��������쐬����
		GetDeclarations( CSMacroMgr::m_MacroFuncInfoArr[i], buf );
		SetDefProc( buf );
	}
	return 0; 
}

/*! PPA�Ɋ֐���o�^���邽�߂̕�������쐬����

	@param cMacroFuncInfo [in]	�}�N���f�[�^
	@param szBuffer [out]		�������������������o�b�t�@�ւ̃|�C���^

	@note �o�b�t�@�T�C�Y�� 9 + 3 + ���\�b�h���̒��� + 13 * 4 + 9 + 5 �͍Œ�K�v

	@date 2003.06.01 Moca
				�X�^�e�B�b�N�����o�ɕύX
				cMacroFuncInfo.m_pszData�����������Ȃ��悤�ɕύX

	@date 2003.06.16 genta ���ʂ�new/delete������邽�߃o�b�t�@���O����^����悤��
*/
char* CPPA::GetDeclarations( const MacroFuncInfo& cMacroFuncInfo, char* szBuffer )
{
	char szType[20];			//	procedure/function�p�o�b�t�@
	char szReturn[20];			//	�߂�l�^�p�o�b�t�@
	if (cMacroFuncInfo.m_varResult == VT_EMPTY){
		strcpy( szType, "procedure" );
		szReturn[0] = '\0';
	}
	else {
		strcpy( szType, "function" );
		if (cMacroFuncInfo.m_varResult == VT_BSTR){
			strcpy( szReturn, ": string" );
		}
		else if ( cMacroFuncInfo.m_varResult == VT_I4 ){
			strcpy( szReturn, ": Integer" );
		}
		else {
			szReturn[0] = '\0';
		}
	}
	
	char szArguments[4][20];	//	�����p�o�b�t�@
	int i;
	for (i=0; i<4; i++){
		if ( cMacroFuncInfo.m_varArguments[i] == VT_EMPTY ){
			break;
		}
		if ( cMacroFuncInfo.m_varArguments[i] == VT_BSTR ){
			strcpy( szArguments[i], "s0: string" );
			szArguments[i][1] = '0' + i;
		}
		else if ( cMacroFuncInfo.m_varArguments[i] == VT_I4 ){
			strcpy( szArguments[i], "i0: Integer" );
			szArguments[i][1] = '0' + i;
		}
		else {
			strcpy( szArguments[i], "u0: Unknown" );
		}
	}
	if ( i > 0 ){	//	�������������Ƃ�
		int j;
		char szArgument[80];
		// 2002.12.06 Moca �����s�������Cstrcat��VC6Pro�ł��܂������Ȃ��������߁Cstrcpy�ɂ��Ă݂��瓮����
		strcpy( szArgument, szArguments[0] );
		for ( j=1; j<i; j++){
			strcat( szArgument, "; " );
			strcat( szArgument, szArguments[j] );
		}
		auto_sprintf( szBuffer, "%hs S_%ls(%hs)%hs; index %d;",
			szType,
			cMacroFuncInfo.m_pszFuncName,
			szArgument,
			szReturn,
			cMacroFuncInfo.m_nFuncID
		);
	}
	else {
		auto_sprintf( szBuffer, "%hs S_%ts%hs; index %d;",
			szType,
			cMacroFuncInfo.m_pszFuncName,
			szReturn,
			cMacroFuncInfo.m_nFuncID
		);
	}
	//	Jun. 01, 2003 Moca / Jun. 16, 2003 genta
	return szBuffer;
}

/*!	
*/
int CPPA::DeinitDll( void )
{
	// Jun. 01, 2003 Moca m_pszData���g��Ȃ��Ȃ������߁C
	//	CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData�̌�n�����폜

	return 0;
}


/*! ���[�U�[��`������^�I�u�W�F�N�g
	���݂́A�f�o�b�O�p�������ݒ肷��ׂ̂�
*/
void __stdcall CPPA::stdStrObj(const char* ObjName, int Index, BYTE GS_Mode, int* Err_CD, char** Value)
{
	NEVER_USED_PARAM(ObjName);
	*Err_CD = 0;
	switch(Index){
	case 2:
		switch(GS_Mode){
		case omGet:
//			::MessageBoxA( m_pcEditView->m_hWnd, m_cMemDebug.GetPtr(), "GetStrObj", MB_OK );
			*Value = m_cMemDebug.GetStringPtr();
			break;
		case omSet:
//			::MessageBoxA( m_pcEditView->m_hWnd, *Value, "SetStrObj", MB_OK );
			m_cMemDebug.SetString(*Value);
			break;
		}
		break;
	default:
		*Err_CD = -1;
	}
}


/*! ���[�U�[��`�֐��̃G���[���b�Z�[�W�̍쐬

	stdProc, stdIntFunc, stdStrFunc ���G���[�R�[�h��Ԃ����ꍇ�APPA����Ăяo�����B
	�ُ�I��/�s���������̃G���[���b�Z�[�W��Ǝ��Ɏw�肷��B
	@author Moca
	@param Err_CD IN  0�ȊO�e�R�[���o�b�N�֐����ݒ肵���l
			 1�ȏ� FuncID + 1
			 0     PPA�̃G���[
			-1�ȉ� ���̑����[�U��`�G���[
	@param Err_Mes IN �G���[���b�Z�[�W

	@date 2003.06.01 Moca
*/
void __stdcall CPPA::stdError( int Err_CD, const char* Err_Mes )
{
	if( false != m_bError ){
		return;
	}
	m_bError = true; // �֐����Ŋ֐����Ăԏꍇ���A2��\�������̂�h��

	char szMes[2048]; // 2048����Α���邩��
	const char* pszErr;
	pszErr = szMes;
	if( 0 < Err_CD ){
		int i, FuncID;
		FuncID = Err_CD - 1;
		for( i = 0; CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_nFuncID != -1; i++ ){
			if( CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_nFuncID == FuncID ){
				break;
			}
		}
		if( CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_nFuncID != -1 ){
			char szFuncDec[1024];
			GetDeclarations( CSMacroMgr::m_MacroFuncInfoNotCommandArr[i], szFuncDec );
			auto_sprintf( szMes, "�֐��̎��s�G���[\n%hs", szFuncDec );
		}else{
			auto_sprintf( szMes, "�s���Ȋ֐��̎��s�G���[(�o�O�ł�)\nFunc_ID=%d", FuncID );
		}
	}else{
		switch( Err_CD ){
		case 0:
			if( 0 == lstrlenA( Err_Mes ) ){
				pszErr = "�ڍוs���̃G���[";
			}else{
				pszErr = Err_Mes;
			}
			break;
		default:
			auto_sprintf( szMes, "����`�̃G���[\nError_CD=%d\n%hs", Err_CD, Err_Mes );
		}
	}
	if( 0 == m_cMemDebug.GetStringLength() ){
		::MessageBoxA( m_pcEditView->GetHwnd(), pszErr, "PPA���s�G���[", MB_OK );
	}else{
		char* p = new char [ lstrlenA(pszErr) + m_cMemDebug.GetStringLength() + 2 ];
		strcpy( p, pszErr );
		strcat( p, "\n" );
		strcat( p, m_cMemDebug.GetStringPtr() );
		::MessageBoxA( m_pcEditView->GetHwnd(), p, "PPA���s�G���[", MB_OK );
		delete [] p;
	}
}



//----------------------------------------------------------------------
void __stdcall CPPA::stdProc(
	const char*		FuncName,
	const int		_Index,
	const char*		Argument[],
	const int		ArgSize,
	int*			Err_CD
)
{
	NEVER_USED_PARAM(FuncName);
	EFunctionCode Index=(EFunctionCode)_Index;

	*Err_CD = 0;

	//Argument��wchar_t[]�ɕϊ� -> tmpArguments
	WCHAR** tmpArguments2=new WCHAR*[ArgSize];
	for(int i=0;i<ArgSize;i++){
		if(Argument[i]){
			tmpArguments2[i]=mbstowcs_new(Argument[i]);
		}
		else{
			tmpArguments2[i]=NULL;
		}
	}
	const WCHAR** tmpArguments=(const WCHAR**)tmpArguments2;

	//����
	CMacro::HandleCommand( m_pcEditView, Index, tmpArguments, ArgSize );

	//tmpArguments�����
	for(int i=0;i<ArgSize;i++){
		if(tmpArguments2[i]){
			WCHAR* p=const_cast<WCHAR*>(tmpArguments2[i]);
			delete[] p;
		}
	}
	delete[] tmpArguments2;
}

//----------------------------------------------------------------------
/*!
	�����l��Ԃ��֐�����������

	PPA����Ăт������
	@author Moca
	@date 2003.02.24 Moca
*/
void __stdcall CPPA::stdIntFunc(
	const char* FuncName, const int Index,
	const char* Argument[], const int ArgSize, int* Err_CD,
	int* ResultValue)
{
	NEVER_USED_PARAM(FuncName);
	VARIANT Ret;
	::VariantInit(&Ret);

	*ResultValue = 0;
	*Err_CD = 0;
	if( false != CallHandleFunction(Index, Argument, ArgSize, &Ret) ){
		switch( Ret.vt ){
		case VT_I4:
			*ResultValue = Ret.lVal;
			break;
		case VT_INT:
			*ResultValue = Ret.intVal;
			break;
		case VT_UINT:
			*ResultValue = Ret.uintVal;
			break;
		default:
			*Err_CD = -2; // 2003.06.01 Moca �l�ύX
		}
		::VariantClear(&Ret);
		return;
	}
	*Err_CD = Index + 1; // 2003.06.01 Moca
	::VariantClear(&Ret);
	return;
}

//----------------------------------------------------------------------
/*!
	�������Ԃ��֐�����������

	PPA����Ăт������
	@date 2003.02.24 Moca CallHandleFunction�Ή�
*/
void __stdcall CPPA::stdStrFunc(
	const char* FuncName, const int Index,
	const char* Argument[], const int ArgSize, int* Err_CD,
	char** ResultValue)
{
	NEVER_USED_PARAM(FuncName);

	VARIANT Ret;
	::VariantInit(&Ret);
	*Err_CD = 0;
	if( false != CallHandleFunction(Index, Argument, ArgSize, &Ret) ){
		if(VT_BSTR == Ret.vt){
			int len;
			char* buf;
			Wrap(&Ret.bstrVal)->Get(&buf,&len);
			m_cMemRet.SetString(buf,len); // Mar. 9, 2003 genta
			delete[] buf;
			*ResultValue = m_cMemRet.GetStringPtr();
			::VariantClear(&Ret);
			return;
		}
	}
	::VariantClear(&Ret);
	*Err_CD = Index + 1;
	*ResultValue = "";
	return;
}

/*!
	�����^�ϊ�

	������ŗ^����ꂽ������VARIANT/BSTR�ɕϊ�����CMacro::HandleFunction()���Ăт���
	@author Moca
*/
bool CPPA::CallHandleFunction(
	const int Index, const char* Arg[],
	int ArgSize, VARIANT* Result )
{
	int i, ArgCnt;
	VARIANT vtArg[4];
	const MacroFuncInfo* mfi;
	bool Ret;

	mfi = CSMacroMgr::GetFuncInfoByID(Index);
	for( i=0; i<4; i++ ){
		::VariantInit( &vtArg[i] );
	}
	for(i=0, ArgCnt=0; i<4 && i<ArgSize; i++ ){
		if(VT_EMPTY == mfi->m_varArguments[i]){
			break;
		}

		switch( mfi->m_varArguments[i] ){
		case VT_I4:
		{
			vtArg[i].vt = VT_I4;
			vtArg[i].lVal = atoi( Arg[i] );
			break;
		}
		case VT_BSTR:
		{
			SysString S(Arg[i],lstrlenA(Arg[i]));
			Wrap(&vtArg[i])->Receive(S);
			break;
		}
		default:
			for( i=0; i<4; i++ ){
				::VariantClear( &vtArg[i] );
			}
			return false;
		}
		ArgCnt++;
	}

	if(Index >= F_FUNCTION_FIRST)
	{
		Ret = CMacro::HandleFunction(m_pcEditView, Index, vtArg, ArgCnt, *Result);
		for( i=0; i<4; i++ ){
			::VariantClear( &vtArg[i] );
		}
		return Ret;
	}else{
		for( i=0; i<4; i++ ){
			::VariantClear( &vtArg[i] );
		}
		return false;
	}
}


#if PPADLL_VER >= 123

/*!
	PPA�}�N���̎��s�I�����ɌĂ΂��
	
	@date 2003.06.01 Moca
*/
void __stdcall CPPA::stdFinishProc()
{
	m_cMemRet.SetString("");
	m_cMemDebug.SetString("");
	m_pShareData = NULL;
	m_pcEditView = NULL;
	m_bError = false;
}

#endif

