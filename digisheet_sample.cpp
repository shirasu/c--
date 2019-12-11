#include <stdio.h>
#include <winsock2.h>
#include <string>

//	設定値

//	Proxy
const std::string strProxyAddress = "XXX.XXX.XXX.XXX";
const int nProxyPort = 8080;

//	接続ユーザー情報
//	派遣元 CD
const std::string strBaseCD = "XXXX";
//	スタッフID
const std::string strStaffID = "XXXXXXX";
//	パスワード
const std::string strPassword = "XXXXXXXXX";

//	POSTして、recv情報を取得
std::string getPostData( SOCKET sock, const std::string& strPostData )
{
	std::string strRecvData;

	// HTTPリクエスト送信
	int n = send(sock, strPostData.c_str(), strPostData.size(), 0);
	if (n < 0) {
		printf("send : %d\n", WSAGetLastError());
		return strRecvData;
	}
	
	// サーバからのHTTPメッセージ受信
	char buf[1024];
	while (n > 0) {
		memset(buf, 0, sizeof(buf));
		n = recv(sock, buf, sizeof(buf), 0);
		if (n < 0) {
			printf("recv : %d\n", WSAGetLastError());
			return strRecvData;
		}
		// 受信結果を表示
		//fwrite(buf, n, 1, stdout);

		//	受信データに追加
		strRecvData.append(buf,n);
	}

	//	データを返す
	return strRecvData;
}

//	ソケットを作成する
//		成功:nError==0
int makeSocket(
		SOCKET*				pSock,				//	ソケット
		const std::string&	strServerAddress,	//	接続先アドレス
		int					nServerPort			//	接続先ポート
)
{

	struct sockaddr_in server;
	unsigned int **addrptr;

	*pSock = socket(AF_INET, SOCK_STREAM, 0);
	if (*pSock == INVALID_SOCKET) {
		printf("socket : %d\n", WSAGetLastError());
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(nServerPort); // 接続ポート

	server.sin_addr.S_un.S_addr = inet_addr(strServerAddress.c_str());
	if (server.sin_addr.S_un.S_addr == 0xffffffff) {
		struct hostent *host;

		host = gethostbyname(strServerAddress.c_str());	//	接続先アドレス

		if (host == NULL) {
			if (WSAGetLastError() == WSAHOST_NOT_FOUND) {
				printf("host not found : %s\n", strServerAddress.c_str());
			}
			return 1;
		}

		addrptr = (unsigned int **)host->h_addr_list;

		while (*addrptr != NULL) {
			server.sin_addr.S_un.S_addr = *(*addrptr);

			// connect()が成功したらloopを抜けます
			if (connect(*pSock,
					(struct sockaddr *)&server,
					sizeof(server)) == 0) {
				break;
			}

			addrptr++;
			// connectが失敗したら次のアドレスで試します
		}

		// connectが全て失敗した場合
		if (*addrptr == NULL) {
			printf("connect : %d\n", WSAGetLastError());
			return 1;
		}
	}else{
		if(connect(*pSock,(struct sockaddr *)&server,sizeof(server)) != 0){
			printf("connect : %d\n", WSAGetLastError());
			return 1;
		}
	}

	//	成功
	return 0;

}

int main(int argc, char *argv[])
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
		printf("WSAStartup failed\n");
		return 1;
	}

	std::string strCookie;	//	クッキー

	//	ログイン、クッキー取得
	{
		SOCKET sock;

		//	ソケット作成
		if( makeSocket( &sock, strProxyAddress,	nProxyPort ) ){
			printf("接続できませんでした\n");
			return 1;	
		}

		//	リクエストを投げる
		std::string strPostData;
		strPostData += "POST http://www.xxxxxxxx.com/servlet/d HTTP/1.1\r\n";
		strPostData += "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/vnd.ms-excel, application/msword, */*\r\n";
		strPostData += "Referer: http://www.xxxxxxxx.com/staffLogin.html\r\n";
		strPostData += "Accept-Language: ja\r\n";
		strPostData += "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727; Lunascape 3.0.4)\r\n";
		strPostData += "Content-Type: application/x-www-form-urlencoded\r\n";
		strPostData += "Accept-Encoding: gzip, deflate\r\n";
		strPostData += "Host: www.xxxxxxxx.com\r\n";
		strPostData += "Content-Length: 63\r\n";
		strPostData += "Proxy-Connection: Keep-Alive\r\n";
		strPostData += "Pragma: no-cache\r\n";
		//strPostData += "Cookie: JServSessionIdroot=sozb41beo1\r\n";
		strPostData += "\r\n";
		strPostData += "HC=XXXX&UI=XXXXXXX&Pw=XXXXXXX&loginButton=login&SI=&CI=0&Typ=1\r\n";
		
		//	データ送信、受信
		std::string strRecvData = getPostData( sock, strPostData );

		printf("******\n");
		printf("%s",strRecvData.c_str());

		//クッキーを取得
		unsigned int nPos = strRecvData.rfind("Set-Cookie: ");
		strCookie.append( strRecvData,nPos+31,10);
		printf("strBuff=%s\n",strCookie.c_str());

		shutdown(sock,0);
		closesocket(sock);
	}

	//	情報送信
	{
		SOCKET sock;

		//	ソケット作成
		if( makeSocket( &sock, strProxyAddress,	nProxyPort ) ){
			printf("接続できませんでした\n");
			return 1;	
		}

		//	リクエストを投げる
		std::string strPostData;

		//	データ送信
		strPostData += "POST http://www.xxxxxxxx.com/servlet/d HTTP/1.1\r\n";
		strPostData += "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/vnd.ms-excel, application/msword, */*\r\n";
		strPostData += "Referer: http://www.xxxxxxxx.com/servlet/d\r\n";
		strPostData += "Accept-Language: ja\r\n";
		strPostData += "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727; Lunascape 3.0.4)\r\n";
		strPostData += "Content-Type: application/x-www-form-urlencoded\r\n";
		strPostData += "Accept-Encoding: gzip, deflate\r\n";
		strPostData += "Host: www.xxxxxxxx.com\r\n";
		strPostData += "Content-Length: 543\r\n";
		strPostData += "Proxy-Connection: Keep-Alive\r\n";
		strPostData += "Pragma: no-cache\r\n";
		//Cookie: JServSessionIdroot=bpiyfvbra1
		strPostData += "\r\n";
		strPostData += "HC=7008&Cl=null&SI=";
		strPostData += strCookie;				//　クッキー
		strPostData += "&UI=XXXXXXX";			//	社員番号
		strPostData += "&CI=101";
		strPostData += "&SubCommandID=2";
		strPostData += "&Typ=1";
		strPostData += "&Cr=0000XXXXXXX001";	//	社員番号＋？
		strPostData += "&Y=2006";				//	年
		strPostData += "&M=11";					//	月
		strPostData += "&Up=2";
		strPostData += "&PrevCommandID=101";
		strPostData += "&D=6";					//	日
		strPostData += "&Years=2006";
		strPostData += "&PrevSubCommandID=";
		strPostData += "&YearStart=2006";
		strPostData += "&MonthStart=10";
		strPostData += "&DayStart=1";
		strPostData += "&YearEnd=2006";
		strPostData += "&MonthEnd=10";
		strPostData += "&DayEnd=30";
		strPostData += "&DayEndSelect=2760";
		strPostData += "&MoneyInputFlag=0";
		strPostData += "&ProcInputFlag=0";
		strPostData += "&CalculateFlag=80";
		strPostData += "&AttendSelect=A0";
		strPostData += "&SaveAttendSelect=A0";
		strPostData += "&HourStart=9";		//始業時間
		strPostData += "&MinuteStart=0";	//始業時間
		strPostData += "&HourRest=1";		//休憩時間
		strPostData += "&MinuteRest=0";		//休憩時間
		strPostData += "&HourEnd=21";		//終了時間
		strPostData += "&MinuteEnd=0";		//終了時間
		strPostData += "&OverHourRest=0";
		strPostData += "&OverMinuteRest=0";
		strPostData += "&NightHourRest=0";
		strPostData += "&NightMinuteRest=0";
		strPostData += "&ContentSelect=0000000000";
		strPostData += "&ProcTimeHour=0";
		strPostData += "&ProcTimeMinute=0";
		strPostData += "&CommentInput=\r\n";

		//	データ送信、受信
		std::string strRecvData = getPostData( sock, strPostData );

		printf("******\n");
		printf("%s",strRecvData.c_str());

		shutdown(sock,0);
		closesocket(sock);
	}
	
	WSACleanup();
	
	return 0;
}
