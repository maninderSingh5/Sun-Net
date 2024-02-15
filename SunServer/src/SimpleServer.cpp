#include <iostream>
#include "sun_net.hpp"

enum HeadFlags: uint32_t{
	text,
	file
};

class server :public sun::net::server_interface<HeadFlags>
{
	std::shared_ptr<sun::net::connection<HeadFlags>> tempClient = nullptr;
public:
	server(uint16_t port) :sun::net::server_interface<HeadFlags>(port)
	{}
	void getInput()
	{
		std::string input;
		input.resize(1024);
		while(1)
		{
		std::cin.getline(input.data(),1024);
		
		sun::net::message<HeadFlags> msg;
		msg.header.id = HeadFlags::text;
		msg.SerializeArray(input.c_str(),input.size());
		if(tempClient)
		tempClient->Send(msg);
		}
	}
	
	
	
	
protected:	
	virtual bool OnClientConnect(std::shared_ptr<sun::net::connection<HeadFlags>> client)
	{
		
		return true;
	}
	virtual void OnClientDisconnect(std::shared_ptr<sun::net::connection<HeadFlags>> client)
	{
				
	}
	
	virtual void OnMessage(std::shared_ptr<sun::net::connection<HeadFlags>> con,sun::net::message<HeadFlags> mesg)
	{
		std::cout<<mesg<<"->";
		std::cout<<"["<<con->GetID()<<"] ";
		for(uint i = 0 ;i<mesg.body.size();i++)
			std::cout<<mesg.body[i];
		std::cout<<"\n";
	}
public:
	virtual void OnClientValidation(std::shared_ptr<sun::net::connection<HeadFlags>> client)
	{
		std::string firstMesg = "Connected To Server\n";
		
		sun::net::message<HeadFlags> mesg;
		mesg.header.id = HeadFlags::text;
		mesg.SerializeArray(firstMesg.c_str(),firstMesg.size());
		client->Send(mesg);
		
		tempClient = client;
	}
};





int main(int argc, char *argv[]){
	 server sv(60000);
	 sv.Start();
	 std::thread t1(&server::getInput,&sv);
	 
	 while(1)
	 {
	 	sv.Update();
	 }
	 if(t1.joinable())
	 	t1.join();
	return 0;
}