#include "framework.h"
#include "sockettest.h"

// ������ ���� �ý����� ����ϱ� ���� initialize�� �� �� ���Դϴ�.
WSADATA wsaData;
// Ŭ���̾�Ʈ ����
SOCKET hClntSock[MAX];
// ���� ��ũ���� ���� �����ϱ� ���� ����ü ����, Ŭ���̾�Ʈ�� �ּҰ��� �����ϱ� ���� ����ü ����
SOCKADDR_IN servAddr, clntAddr[MAX];
fd_set set, cpset;
int fd_num, chk_conn;
WCHAR buf[MAX] = {};
WCHAR buf2[MAX] = {};

bool servRunning;			// ���� ���� ���θ� ��Ÿ���ϴ�.
SOCKET hServSock;				// ���� ���� ����ü

void app_print(HWND hWnd, HDC hdc, const wchar_t* str);
int t_y;

void WSAStartup_error(HWND hWnd, int code);
void bind_error(HWND hWnd, int code);

/*!
* @breif		������ ���۽ÿ� ����Ǵ� ������
* @details		�ش� �����尡 ������ ������ �ϸ� ������ ����Ѵ�.
* @param		LPVOID	param	hWnd�� �� ������ �ڵ鰪�� ���� ����ȴ�.
* @return		���ϰ��� �ƹ� �ǹ̸� ���� �ʴ´�.
* @retval		1 ��������
* @bug
* @warning
*/


DWORD WINAPI runServ(LPVOID Param)
{
	// ������ Param ���� hWnd ������ ����
	HWND hWnd = (HWND)Param;
	int portNumber = 10000;
	int szClntAddr;
	timeval timeout;
	int i, j;
	t_y = 0;

	// DC �� �Ҵ�
	HDC hdc = GetDC(hWnd);

	// ��¥ ��� ǥ�ø� �����ϰ� ����
	SetBkMode(hdc, TRANSPARENT);
	// ������ ������ �ٽ� �׸�
	InvalidateRect(hWnd, NULL, TRUE);

	// ���� ���α׷� ���ۺ�
	// DLL�ʱ�ȭ �� ���ø����̼� �䱸���� ���� Ȯ��
	if ((i = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		// ����� ���� ������ ���
		WSAStartup_error(hWnd,i);

		// ������ ����
		servRunning = false;
		ExitThread(0);
	}

	// ���� ���� �Ҵ�
	// ��ȯ���� ���� ��ũ����
	hServSock = socket(PF_INET, SOCK_STREAM, 0);

	// ���� �Ҵ翡 ������ ���
	if (hServSock == INVALID_SOCKET)
	{
		// �����޽��� ���
		MessageBox(hWnd, L"socket() error!", L"����", NULL);
		// ���� ��� ����
		WSACleanup();

		// ������ ����
		WSACleanup();
		servRunning = false;
		ExitThread(0);
	}

	// ����� �غ�Ǿ����� �˸��� �ڵ�
	app_print(hWnd, hdc, L"Successfully created socket.");

	// servAddr ����ü�� �޸� �ʱ�ȭ
	memset(&servAddr, 0x00, sizeof(servAddr));
	// IPv4 ���ͳ� �������� �ּ� ü�踦 ���
	servAddr.sin_family = AF_INET;
	// �ּҰ��� 0���� �ʱ�ȭ
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// ��Ʈ��ȣ�� portNumber ������ �ʱ�ȭ
	servAddr.sin_port = htons(portNumber);


	// ���� �ּҸ� ���ϰ� ����
	if ((i = bind(hServSock, (sockaddr*)&servAddr, sizeof(servAddr))) == SOCKET_ERROR)
	{
		// �����޽��� ���
		bind_error(hWnd, i);

		// ������ ����
		closesocket(hServSock);
		WSACleanup();
		servRunning = false;
		ExitThread(0);
	}
	else {
		// ���� �޽��� ���
		app_print(hWnd, hdc, L"Bind succeed.");
	}

	// ��Ʈ��ũ �Է��� ����
	// ����ڰ� ���� �� ���� ���
	listen(hServSock, 10);

	app_print(hWnd, hdc, L"Listening...");

	// file descriptor�� ������ ���� �迭 ����ü
	// FD_ZERO�� fd_set�� �ʱ�ȭ, FD_SET���� fd_set�� ���� ������ ����
	FD_ZERO(&set);
	FD_SET(hServSock, &set);

	app_print(hWnd, hdc, L"Server is running...");
	
	//Ÿ�Ӿƿ� ����ü ��� ����(��, �и���)
	timeout.tv_sec = 5;
	timeout.tv_usec = 5000;

	// ���� �ݺ� ����
	while (1)
	{
		//���� ���� ���� �ƴϸ� while�� Ż��, ������ ������ ����
		if (servRunning == false)
			break;

		//select���� ���� fd_set ������ �ѼյǱ� ������ ���� set�� �����ؼ� ���
		cpset = set;
		
		//select���� file descriptor�� ������ ����, ����� fd_set�� ���� ���� �ϳ��� �ִ� ����
		//�μ��� ���ʷ� (����, �Է� ������ ��Ʈ, ��� ������ ��Ʈ, ���� ������ ��Ʈ, Ÿ�Ӿƿ�)
		fd_num = select(set.fd_count, &cpset, 0, 0, &timeout);

		//���� �߻� �� while�� Ż��, ������ ������ ����
		if (fd_num == -1)
			break;
		//��ȭ���� ���� ��� ó������ �ʰ� �ݺ�
		else if (fd_num == 0)
			continue;

		//set�� file descriptor ������ŭ �ݺ�
		for (i = 0; i < set.fd_count; i++)
		{
			//��� ���Ͽ��� ��ȭ�� �߻��ߴ��� �Ǵ�
			if (FD_ISSET(set.fd_array[i], &cpset))
			{
				//�ش� ������ ���� �����̶�� �ű� ���� ��ȣ�� �� ��
				if (set.fd_array[i] == hServSock)
				{
					//Ŭ���̾�Ʈ �ּ� ũ�⸦ ���ϰ� accept, ����� ������ Ŭ���̾�Ʈ ���� �迭�� �Ҵ�
					szClntAddr = sizeof(clntAddr);
					hClntSock[i] = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);

					//�ش� ������ fd_set�� �Ҵ��ϰ� fd_count���� �۴ٸ� i�� ����
					FD_SET(hClntSock[i], &set);
					if (set.fd_count < i)
						set.fd_count = i;

					//���ῡ �����ߴٸ� ���� ���� �޽��� ���
					if (hClntSock[i] != -1) {
						wsprintf(buf, L"Connected Client: %d", hClntSock[i]);
						app_print(hWnd, hdc, buf);
						for (j = 0; j < set.fd_count; j++)
							send(set.fd_array[j], (char*)buf, MAX, 0);
					}
				}
				//�ش� ������ ���� ������ �ƴ� ���
				else
				{
					//���� Ȯ���� ���� recv �Լ� ��ȯ
					chk_conn = recv(set.fd_array[i], (char*)buf, MAX, 0);
					//���� ���� 0�̸� ���� ����, 1�̸� ������ �������� ��Ÿ��
					if (chk_conn <= 0)
					{
						//���� ���� �޽��� ����
						wsprintf(buf, L"Disconnected Client: %d", set.fd_array[i]);
						
						//�ش� ������ set���� ã�� ����
						closesocket(set.fd_array[i]);
						//����� ������ �ִ� fd_set �� �迭 ���Ҹ� 0���� �ʱ�ȭ
						FD_CLR(set.fd_array[i], &set);
						
						//���� ���� �޽��� ���
						app_print(hWnd, hdc, buf);
						for (j = 0; j < set.fd_count; j++)
							send(set.fd_array[j], (char*)buf, MAX, 0);
					}
					//recv �Լ��� ���� �۵��� ���(��ȯ�� 1�� ���)
					else
					{
						//buf2���� �ش� ���� ��ȣ�� ����
						wsprintf(buf2, L"%d         : ", set.fd_array[i]);
						//buf2 �ڿ� recv�� �޽����� ����Ǿ� �ִ� buf�� ����
						wcscat(buf2, buf);
						//buf2�� ���
						//846		: hello �� ���� ���·� ���
						app_print(hWnd, hdc, buf2);
						for (j = 0; j < set.fd_count; j++)
							send(set.fd_array[j], (char*)buf2, MAX, 0);
					}
				}
			}
		}
	}

	// ���� ����
	app_print(hWnd, hdc, L"Server Terminated.");
	ReleaseDC(hWnd, hdc);

	// ������ ����
	closesocket(hServSock);
	WSACleanup();
	ExitThread(0);
	return 0;
}



/*!
* @breif		WSAStartup �Լ��� �����޽����� ó���Ѵ�.
* @details		WSAStartup �Լ��� ��ȯ�ϴ� �������� ���� �����޽����� ǥ���Ѵ�.
* @details		reference : https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
* @param		HWND	hWnd		�����쿡 ���� �ڵ鰪
* @param		int		code		WSAStartup�� ��ȯ�ϴ� �ڵ尪
* @return		void
*/
void WSAStartup_error(HWND hWnd, int code)
{
	switch (code)
	{
	case WSASYSNOTREADY:
		// The underlying network subsystem is not ready for network communication.
		MessageBox(hWnd, L"winsock error!", L"���α׷��� ����� ���� �غ� ���� �ʾҽ��ϴ�.", NULL);
		break;
	case WSAVERNOTSUPPORTED:
		// The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.
		MessageBox(hWnd, L"winsock error!", L"windows ���� ���� ������ �������� �ʽ��ϴ�.", NULL);
		break;
	case WSAEINPROGRESS:
		// A blocking Windows Sockets 1.1 operation is in progress.
		MessageBox(hWnd, L"winsock error!", L"���� �۾��� ���ܵǾ����ϴ�.", NULL);
		break;
	case WSAEPROCLIM:
		// A limit on the number of tasks supported by the Windows Sockets implementation has been reached.
		MessageBox(hWnd, L"winsock error!", L"������ ������ �ʰ��Ǿ����ϴ�.", NULL);
		break;
	case WSAEFAULT:
		// The lpWSAData parameter is not a valid pointer.
		MessageBox(hWnd, L"winsock error!", L"�߸��� ȣ���� ����Ǿ����ϴ�.", NULL);
		break;
	default:
		MessageBox(hWnd, L"winsock error!", L"����", NULL);
		break;
	}
}

/*!
* @breif		bind �Լ��� �����޽����� ó���Ѵ�.
* @details		bind �Լ��� ��ȯ�ϴ� �������� ���� �����޽����� ǥ���Ѵ�.
* @details		reference : https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-bind
* @param		HWND	hWnd		�����쿡 ���� �ڵ鰪
* @param		int		code		WSAStartup�� ��ȯ�ϴ� �ڵ尪
* @return		void
*/
void bind_error(HWND hWnd, int code)
{
	switch (code)
	{
	case WSANOTINITIALISED:
		MessageBox(hWnd, L"bind error!", L"WSAStartup �Լ��� ȣ����� �ʾҽ��ϴ�.", NULL);
		break;	
	case WSAENETDOWN:
		MessageBox(hWnd, L"bind error!", L"��Ʈ��ũ �����ý����� ������ ���� �߽��ϴ�.", NULL);
		break;
	case WSAEACCES:
		MessageBox(hWnd, L"bind error!", L"�׼����������� ���ܵǾ����ϴ�.", NULL);
		break;
	case WSAEADDRINUSE:
		MessageBox(hWnd, L"bind error!", L"�ش� �ּ� �Ǵ� ��Ʈ�� �̹� ����� �Դϴ�.", NULL);
		break;
	case WSAEADDRNOTAVAIL:
		MessageBox(hWnd, L"bind error!", L"��ȿ���� ���� �ּ��Դϴ�.", NULL);
		break;
	case WSAEFAULT:
		MessageBox(hWnd, L"bind error!", L"�߸��� ������ ������ �߻��߽��ϴ�.", NULL);
		break;
	case WSAEINPROGRESS:
		MessageBox(hWnd, L"bind error!", L"���� �۾��� ���ܵǾ����ϴ�.", NULL);
		break;
	case WSAEINVAL:
		MessageBox(hWnd, L"bind error!", L"�߸��� �μ��� ���޵Ǿ����ϴ�.", NULL);
		break;
	case WSAENOBUFS:
		MessageBox(hWnd, L"bind error!", L"�޸𸮰� �ʰ��Ǿ����ϴ�.", NULL);
		break;
	case WSAENOTSOCK:
		MessageBox(hWnd, L"bind error!", L"������ �ƴմϴ�.", NULL);
		break;
	default:
		MessageBox(hWnd, L"bind error!", L"����", NULL);
		break;
	}
}

/*!
* @breif		�޽����� ����մϴ�.
* @details		���� ���¿� Ŭ���̾�Ʈ �޽����� ����մϴ�.
* @param		HDC		hdc		�����쿡 ���� �ڵ鰪
* @param		const	WHCAR*	str		����� ���ڿ�
* @return		void
*/

void app_print(HWND hWnd, HDC hdc, const wchar_t* str)
{
	TextOut(hdc, 10, 10+t_y, str, lstrlenW(str));
	t_y += 20;
	InvalidateRect(hWnd, NULL, 0);
}