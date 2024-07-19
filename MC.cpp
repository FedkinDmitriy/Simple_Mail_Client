#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class Socket
{
private:

	SOCKET socket_{};
	sockaddr_in servInfo_;

	in_addr convert_IP(string SERVER_IP)
	{
		in_addr ip_to_num;
		int ES{};
		ES = inet_pton(AF_INET, SERVER_IP.data(), &ip_to_num);
		if (ES <= 0)
		{
			cerr << "Error in IP translation to special numeric format" << endl;
		}
		else
		{
			return ip_to_num;
		}
	}

public:

	Socket(const string &ip, const int port)
	{
		socket_ = socket(AF_INET, SOCK_STREAM, 0);
		if (socket_ == INVALID_SOCKET)
		{
			cerr << "Error initializing socket " << WSAGetLastError() << endl;
		}

		in_addr ip_to_num = convert_IP(ip);		
		ZeroMemory(&servInfo_, sizeof(servInfo_));
		servInfo_.sin_family = AF_INET;
		servInfo_.sin_addr = ip_to_num;
		servInfo_.sin_port = htons(port);
	}
	~Socket()
	{
		closesocket(socket_);
	};

	Socket(const Socket &s) = delete;
	Socket(const Socket &&s) = delete;
	Socket& operator=(const Socket &s) = delete;
	Socket& operator=(const Socket &&s) = delete;

	int plug() const
	{
		int value{};
		value = connect(socket_, (sockaddr*)&servInfo_, sizeof(servInfo_));
		if (value != 0)
		{
			cerr << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;			
		}
		return value;
	}

	bool post(string msg) const
	{
		int bytes{};
		string m{msg};
		m += "\r\n";
		bytes = send(socket_, m.data(), m.size(), 0);
		if (bytes == SOCKET_ERROR)
		{
			return false;
		}
		return true;
	}

	string get(vector<char> &BUFF)
	{
		int bytes{};
		bytes = recv(socket_, BUFF.data(), BUFF.size(), 0);
		if (bytes == SOCKET_ERROR)
		{
			cerr << "Recieved from Server is FAILED. Error # " << WSAGetLastError() << endl;
			return string{};
		}
		else
		{
			string response{ BUFF.begin(), BUFF.begin() + bytes };
			return response;
		}
	}
};

bool write(string str)
{
	ofstream file;
	file.open("test.txt", ios::app);
	if (!file.is_open())
	{
		cerr << "Error opening file" << endl;
		return false;
	}
	file << str;
	file.close();
	return true;
}

int main()
{
	WSADATA wsData;
	if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
	{
		cout << "WinSock init failed: " << WSAGetLastError() << endl;
		return 1;
	}

	vector<char> buffer(2048);
	string msg;	
	string temp;

	bool quit = false;
	while(!quit)
	{
		Socket smtp("172.20.10.8", 25);
		Socket pop3("172.20.10.8", 110);
		int x{};
		cout << "\nHello, stanger, what do you want ?\n1 - send mail\n2 - check mail\n3 - personal_task\n4 - exit\n";
		cin >> x;
		cin.ignore(); // убираем \n из потока
		switch (x)
		{
		case 1:
		{
			cout << "Enter exit or EXIT to return to the MAIN MENU\n";
			smtp.plug();
			cout << smtp.get(buffer) << endl;
			bool flag = true;
			while (flag)
			{
				getline(cin, msg);
				smtp.post(msg);
				cout << smtp.get(buffer) << endl;
				if (msg.find("DATA") != string::npos || msg.find("data") != string::npos)
				{
					string fullmsg;
					while (getline(cin, msg))
					{
						if (msg == ".")
						{
							break;
						}
						fullmsg += msg + "\r\n";
					}
					fullmsg += "\r\n.";
					smtp.post(fullmsg);
					cout << smtp.get(buffer) << endl;
				}
				if (msg == "exit" || msg == "EXIT")
				{
					flag = false;
				}
			}
			break;
		}
		case 2:
		{
			cout << "Enter exit or EXIT to return to the MAIN MENU\n";
			pop3.plug();
			cout << pop3.get(buffer) << endl;
			bool flag = true;
			while (flag)
			{
				getline(cin, msg);
				pop3.post(msg);
				cout << pop3.get(buffer) << endl;
				if (msg.find("retr ") != string::npos || msg.find("RETR ") != string::npos)
				{
					cout << pop3.get(buffer) << endl;
				}
				if (msg == "exit" || msg == "EXIT")
				{
					flag = false;
				}
			}
			break;
		}
		case 3:
		{
			pop3.plug();
			pop3.get(buffer);
			msg.clear();
			pop3.post("USER MrAbuser@Fedkin.ru");
			pop3.get(buffer);
			pop3.post("PASS 1111");
			pop3.get(buffer);
			pop3.post("STAT");
			string tmp;
			tmp = pop3.get(buffer);
			if (tmp[4] != '0')
			{
				string num{ tmp[4] };
				int count = stoi(num);
				vector<string> messages;
				for (int i{1}; i <= count; ++i)
				{
					string cmd{ "RETR " + to_string(i) };
					pop3.post(cmd);
					pop3.get(buffer);
					messages.push_back(pop3.get(buffer));
					//string cmd2{ "DELE " + to_string(i) };
					//pop3.post(cmd2);
				}
				pop3.post("QUIT");
				pop3.get(buffer);


				smtp.plug();
				smtp.get(buffer);
				smtp.post("HELO");
				smtp.get(buffer);
				smtp.post("MAIL FROM: MrAbuser@Fedkin.ru");
				smtp.get(buffer);

				for (int i{}; i < count; ++i)
				{
					smtp.post("RCPT TO: MrUser@Fedkin.ru");
					smtp.get(buffer);
					string cmd{ "DATA" };
					smtp.post(cmd);
					smtp.get(buffer);
					if (cmd.find("DATA") != string::npos || cmd.find("data") != string::npos)
					{
						string fullmsg;
						fullmsg += messages[i] + "\r\n.";
						smtp.post(fullmsg);
						cout << smtp.get(buffer) << endl;
					}					
				}
				smtp.post("QUIT");
				smtp.get(buffer);
			}
			else
			{
				cout << "Mailbox is empty\n";
			}

			break;
		}
		case 4:
		{
			cout << "\nGood bye...\n";
			Sleep(1000);
			quit = true;
			break;
		}
		default:
		{
			cout << "\nWrong input! Try again.\n";
			break;
		}
		}
	}

	WSACleanup();
}