#include "ykqtutil.h"
#include <QString>
#include <QDir>

YKQTUtil::StringTokenizer::StringTokenizer( void )
	: _token( NULL )
	, _curTokenIndex( 0 )
{
	memset( _strBuf, 0, MAX_LINE_LENGTH );
	memset( _srcStr, 0, MAX_LINE_LENGTH );
	memset( _seps, 0, 10 );
}

YKQTUtil::StringTokenizer::~StringTokenizer(void)
{
}

void YKQTUtil::StringTokenizer::SetString( char * str, char * seps )
{
	memset( _strBuf, 0, MAX_LINE_LENGTH );
	memset( _srcStr, 0, MAX_LINE_LENGTH );
	memset( _seps, 0, 10 );

	strncpy( _strBuf, str, strlen( str ) );
	strncpy( _srcStr, str, strlen( str ) );
	strncpy( _seps, seps, strlen( seps ) );

	_token = NULL;
	_curTokenIndex = 0;
}

void YKQTUtil::StringTokenizer::SetString(QString& qstr, QString& qseps)
{
	const char* str = qstr.toStdString().c_str();
	const char* seps = qseps.toStdString().c_str();
	SetString((char*)str, (char*)seps);
}

char * YKQTUtil::StringTokenizer::GetNextToken()
{
	if( strlen( _strBuf ) == 0 )
	{
		return NULL;
	}

	if( _token == NULL )
	{
		_token = strtok( _strBuf, _seps );
	}
	else
	{
		_token = strtok( NULL, _seps );
	}

	_curTokenIndex++;

	return _token;
}

char * YKQTUtil::StringTokenizer::GetToken( int index )
{
	//assert( _curTokenIndex - 1 < index );

	char * token = NULL;

	for( int i = _curTokenIndex - 1; i < index; i++ )
	{
		token = GetNextToken();
	}

	if (token == NULL)
		token = "";

	return token;
}

char * YKQTUtil::StringTokenizer::GetRemainStr()
{
	char * token = GetNextToken();

	if (token == NULL)
		return "";
	else
		return _srcStr + (token - _strBuf);
}

bool YKQTUtil::IsThereDirectory(QString& path)
{
		//CFileFind   find;
		//CString     fupath      =   "";
		//BOOL        is_found    =   FALSE;

		////fupath.Format( "%s\\%s", path, name );
		//fupath.Format( "%s", path);
		////TRACE( "%s\n", fupath ); 

		//if ( is_found = find.FindFile(fupath, 0) )  //if there are some files
		//{
		//	while( is_found )
		//	{
		//		is_found    =   find.FindNextFile(); 

		//		if ( find.IsDots() )
		//			continue; //continue the loop

		//		if ( find.IsDirectory() ) //don't search the subDirectory
		//		{
		//			find.Close();
		//			return true;
		//			continue;//continue the loop
		//		}
		//		else
		//		{
		//			//CString* currentFilePath = new CString((LPCTSTR)find.GetFileName());
		//			////Here, make a code for file accessing
		//			//fileLists.pushback(currentFilePath);
		//		}
		//	}
		//}		
		//find.Close();
		return false;
}

bool YKQTUtil::IsThereFile(QString& fullPath)
{
	//{
	//	CFileFind   find;
	//	CString     fupath      =   "";
	//	BOOL        is_found    =   FALSE;

	//	//fupath.Format( "%s\\%s", path, name );
	//	fupath.Format( "%s", fullPath);
	//	//TRACE( "%s\n", fupath ); 

	//	if ( is_found = find.FindFile(fupath, 0) )  //if there are some files
	//	{
	//		while( is_found )
	//		{
	//			is_found    =   find.FindNextFile(); 

	//			if ( find.IsDots() )
	//				continue; //continue the loop

	//			if ( find.IsDirectory() ) //don't search the subDirectory
	//			{
	//				//find.Close();
	//				//return true;
	//				continue;//continue the loop
	//			}
	//			else
	//			{
	//				find.Close();
	//				return true;
	//				//CString* currentFilePath = new CString((LPCTSTR)find.GetFileName());
	//				////Here, make a code for file accessing
	//				//fileLists.pushback(currentFilePath);
	//			}
	//		}
	//	}		
	//	find.Close();
	//	return false;
	//}
	return true;
}

void YKQTUtil::MakeDirectory(QString& full_path)
{
	//char temp[256], *sp;
	//strcpy(temp, full_path);    // ��ι��ڿ��� ����
	//sp = temp;                  // �����͸� ���ڿ� ó������

	//while((sp = strchr(sp, '\\'))) {    // ���丮 �����ڸ� ã������
	//	if(sp > temp && *(sp - 1) != ':') { // ��Ʈ���丮�� �ƴϸ�
	//		*sp = '\0';         // ��� ���ڿ� ������ ����
	//		//mkdir(temp, S_IFDIR);
	//		CreateDirectory(temp, NULL);
	//		// ���丮�� ����� (�������� ���� ��)
	//		*sp = '\\';         // ���ڿ��� ������� ����
	//	}
	//	sp++;                   // �����͸� ���� ���ڷ� �̵�
	//}

}

void YKQTUtil::MakeDirectory(char *full_path)
{


}

//
//void Utilities::MakeDirectory(CString strFullPath)
//{
//	MakeDirectory(strFullPath.GetBuffer(0));
//}
//

//3�� ������ �׽�Ʈ �ʿ�
QString YKQTUtil::GetFileNameFromPath (QString& full_path)
{
	/*int tmpIdx = 0;
	tmpIdx = GetLastCharIndexFromPath("\\", full_path);    

	int len = full_path.GetLength();

	if (tmpIdx < 0)
	{
        return full_path;
	}
	else
	{
		return full_path.Right(len - tmpIdx-1);
	}	*/
	return "";
}

//extension�� ������ �����̸� �ޱ�
QString YKQTUtil::GetFileNameWoExtFromPath (QString& full_path)
{
	//int tmpIdx = 0;
	//tmpIdx = GetLastCharIndexFromPath("\\", full_path);    

	//int len = full_path.GetLength();

	//CString tmpFileNameWExt; //extension�� ������ ���ϸ�

	//if (tmpIdx < 0) //������ ���� ��
	//{
	//	tmpFileNameWExt = full_path;
	//}
	//else
	//{
	//	tmpFileNameWExt = full_path.Right(len - tmpIdx-1);
	//}	


	//CString tmpResult;

	//tmpIdx = 0;
	//tmpIdx = GetLastCharIndexFromPath(".", tmpFileNameWExt);

	////int len = tmpFileNameWExt.GetLength();

	//if (tmpIdx < 0)
	//{
	//	tmpResult = tmpFileNameWExt;
	//}
	//else
	//{
	//	tmpResult = tmpFileNameWExt.Left(tmpIdx);
	//	//return full_path.Right(len - tmpIdx-1);
	//}	


	//return tmpResult;
	return "";
}



QString YKQTUtil::GetFoldernameFromPath (QString& full_path)
{
	/*int tmpIdx = 0;

	tmpIdx = GetLastCharIndexFromPath("\\", full_path);

     if (tmpIdx < 0)
	 {
		 return "";			 
	 }
	 else
	 {
		 return full_path.Left(tmpIdx);
	 }*/
	return "";
}


QString YKQTUtil::GetExtnameFromPath (QString& full_path)
{
	/*int tmpIdx = 0;

	tmpIdx = GetLastCharIndexFromPath(".", full_path);

	int len = full_path.GetLength();

	if (tmpIdx < 0)
	{
		return "";
	}
	else
	{
		return full_path.Right(len - tmpIdx-1);
	}	*/
	return "";
}



QString YKQTUtil::GetEndfixPathName (QString& full_path, QString& endfix)
{

	//QString fileNameWoExt = GetFileNameWoExtFromPath(full_path);
	//QString folderName = GetFoldernameFromPath(full_path);
	//QString extName = GetExtnameFromPath(full_path);
	//

	//QString tmpResult = folderName + "\\" + fileNameWoExt + endfix + "." + extName; 

	///*	strImg2FullPath = strFolderPath + "\\"+strImg2Name;

	//CString strMarkerArchiveFilePath = */

	//return tmpResult;
	return "";
}


//0 based index
int YKQTUtil::GetLastCharIndexFromPath(QString& strChar, QString& searchStr)
{
	/*int i;

	if (strChar.GetLength() != 1)
	{
		return -1;
	}

	int len = searchStr.GetLength();
	for (i =  0 ; i< len ; i++)
	{
		CString test = searchStr.Mid(len-i-1, 1);
		if (test == strChar)
		{
            return (len-i	-1);
		}		
	}	
	return -1;		*/		
	return 0;

}					


void YKQTUtil::Char2Hex(unsigned char ch, char* szHex)
{
	static unsigned char saucHex[] = "0123456789ABCDEF";
	szHex[0] = saucHex[ch >> 4];
	szHex[1] = saucHex[ch&0xF];
	szHex[2] = 0;
}

QString YKQTUtil::GetSecondStrByTab(QString& fullStr)
{
	/*StringTokenizer token;
	token.SetString(fullStr, "	");

	QString strRes;
	strRes = token.GetToken(1);

	return strRes;*/
	return "";
}