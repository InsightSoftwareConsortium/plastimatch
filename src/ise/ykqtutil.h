#pragma once

#define MAX_LINE_LENGTH 1024
#define round(dTemp) (long(dTemp+(dTemp>0? .5:-.5)))

//#include <afx.h>
//#include <afxwin.h>
//#include <vector>
//#include <assert.h>

class QString;

//using namespace std;

namespace YKQTUtil
{
	class StringTokenizer
	{
	public:
		StringTokenizer( void );
		~StringTokenizer(void);

		void SetString( char * str, char * seps );
		void SetString(QString& qstr, QString& seps);

		char * GetNextToken();
		char * GetToken( int index );
		char * GetRemainStr();

	private:
		char _srcStr[MAX_LINE_LENGTH];
		char _strBuf[MAX_LINE_LENGTH];
		char _seps[10];
		char * _token;

		int _curTokenIndex;
	};

	bool IsThereDirectory(QString& path);
	bool IsThereFile(QString& fullPath);
	void MakeDirectory(char *full_path);
	void MakeDirectory(QString& strFullPath);

	QString GetFileNameFromPath (QString& full_path);
	QString GetFileNameWoExtFromPath (QString& full_path);
	QString GetFoldernameFromPath (QString& full_path);

	QString GetExtnameFromPath (QString& full_path);	

	//���ϸ� ��(Ȯ���� ��)�� end fix�� ���� path�� ��ȯ
	QString GetEndfixPathName (QString& full_path, QString& endfix);
	int GetLastCharIndexFromPath(QString& strChar, QString& searchStr);
	void Char2Hex(unsigned char ch, char* szHex);
	QString GetSecondStrByTab(QString& fullStr); //option file��� ������ ���е� str�� �ι�°(������) ��� ��ȯ
};