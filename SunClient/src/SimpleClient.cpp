#include "sun_net.hpp"

using namespace sun::net;
using namespace sun;
class client :public sun::net::client_interface<sun::Header>
{
public:
	void SendText(std::string user_id,std::string text){

		sun::net::message<sun::Header> mesg;
		mesg.header.id = sun::Header::TEXT_MESSAGE;	
		mesg.SerializeArray(text.data(),text.size());
		mesg.SerializeArray(user_id.data(),user_id.size());
		mesg << user_id.size();
		
		Send(mesg);
	}
	void printInMesg()
	{
		while(!IncomingQ().empty() || IsConnected())
		{
		auto d = IncomingQ().wait_and_pop();

		switch((uint32_t)d->mesg.header.id)
		{
			case Header::TEXT_MESSAGE:
				std::cout<<"TEXT - ";
				break;
			case (Header::ROOM_REQUEST):
				std::cout<<"ROOM_REQUEST - ";
				break;
			default:
				std::cout<<"DEFAULT - ";
				break;
		}
		for(auto& byte : d->mesg.body)
			std::cout<<byte;
		std::cout<<"\n";
		}
	}
	void SignUp(std::string id,std::string name,std::string pass)
	{
		sun::net::message<Header> login;
		login.header.id = Header(Header::SYS_MESSAGE | SIGNUP);
		
		login.SerializeArray(pass.data(),pass.size());
		login << pass.size();
		login.SerializeArray(name.data(),name.size());
		login << name.size();
		login.SerializeArray(id.data(),id.size());
		login << id.size();
		Send(login);
	}
	
	void Login(std::string id,std::string pass)
	{
		sun::net::message<Header> login;
		login.header.id = Header(Header::SYS_MESSAGE | LOGIN);
		
		login.SerializeArray(pass.data(),pass.size());
		login << pass.size();
		login.SerializeArray(id.data(),id.size());
		login << id.size();
		Send(login);
	}
	
	void SendRoomRequest(std::string user_id)
	{
		sun::net::message<Header> request;
		request.header.id = Header(Header::SYS_MESSAGE | ROOM_REQUEST);
		request.SerializeArray(user_id.data(),user_id.size());
		request << user_id.size() << RoomRequest::FRIEND_REQUEST;
		Send(request);
	}
	
	void AcceptRoomRequest(std::string user_id)
	{
		sun::net::message<Header> request;
		request.header.id = Header(Header::SYS_MESSAGE | ROOM_REQUEST);
		request.SerializeArray(user_id.data(),user_id.size());
		request << user_id.size() << RoomRequest::FRIEND_REQ_ACCEPT;
		Send(request);
	}
	
	void getLogin()
	{
		std::string user_id,password;
		std::cout << "[Server] Enter user_id and password - ";
		std::cin >> user_id >> password;
		Login(user_id, password);
	}
	
	void getSignUp()
	{
		std::string id, name, pass;
		uint32_t fullNameSize = 100;
		//name.resize(fullNameSize);
		std::cout<<"Signing Up.....\n\tuser_id - ";
		std::cin >> id;
		std::cout << "\n";
		std::cout<<"\tName - ";
		std::cin >> name;
		std::cout << "\n";
		std::cout << "\tPassword - ";
		std::cin >> pass;
		std::cout << "\n";
		SignUp(id,name,pass);
	}
	
	void getRequestSend()
	{
		std::string user_id;
		std::cout << "Enter friend's user_id - ";
		std::cin >> user_id;
		std::cout << "\n";
		SendRoomRequest(user_id);
	}
	
	void getAcceptRequest()
	{
		std::string id;
		std::cout<<"Accept Friend Request, user_id - ";
		std::cin >> id;
		std::cout << "\n";
		AcceptRoomRequest(id);
	}
	
	void getMessage()
	{
		std::string user_id, message;
		std::cout<< "User_id & Message -";
		std::cin >> user_id >> message;
//		std::cin.clear();
//		std::cout << "\n";
//		std::cout<<"Message - ";
//		std::cin >> message;
		std::cout << "\n";
		SendText(user_id,message);
	}
	
	void getInput()
	{
		int command = 0;
		std::cin >> command;
		switch(command)
		{
			case 1:
				getLogin();
				break;
			case 2:
				getSignUp();
				break;
			case 3:
				getRequestSend();
				break;
			case 4:
				getAcceptRequest();
				break;
			case 5:
				getMessage();
				break;
		}
	}
	
};

int main(int argc, char *argv[])
{
	client c;
	int attempts = 6;
	while(--attempts)
	{
		c.connect("192.168.1.43",60000);
//		std::this_thread::sleep_for(std::chrono::seconds(1));
		if(c.IsConnected())
			break;
	}
	std::thread t1(&client::printInMesg,&c);
	
	while(attempts && c.IsConnected())
	{
	
		c.getInput();
	
	}
	
	t1.join();
	return 0;
}
