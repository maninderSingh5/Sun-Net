#include "sun_net.hpp"

enum HeadFlags: uint32_t{
	text,
	file
};

class client :public sun::net::client_interface<HeadFlags>
{
public:
	void SendText(std::string text){
		sun::net::message<HeadFlags> mesg;
		mesg.header.id = HeadFlags::text;	
		mesg.SerializeArray(text.data(),text.size());
		Send(mesg);
	}
	void printInMesg()
	{
		auto d = IncomingQ().wait_and_pop();
		
		for(auto& byte : d->mesg.body)
			std::cout<<byte;
		std::cout<<"\n";
	}
};

int main(int argc, char *argv[])
{
	client c;
	c.connect("127.0.0.1",60000);
	int size = 1000;
	std::string input;
	input.resize(size);
	std::thread t1(&client::printInMesg,&c);
	
	while(1)
	{
		std::cin >> input;
	
	c.SendText(input.data());
	
	}
	
	t1.join();
	return 0;
}