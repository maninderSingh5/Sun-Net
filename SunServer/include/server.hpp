#ifndef SERVER_HPP
	#define SERVER_HPP

// from Sun-Net Project
#include "sun_net.hpp"

#include "AuthManager.hpp"
#include "SysManager.hpp"
#include "ChatManager.hpp"

namespace sun
{
	namespace net
	{
		class server : public server_interface<Header>
		{
		public:
			server(uint16_t port)
			:server_interface<Header>(port), authorizer(), sys_handler(authorizer), chat_handler(authorizer)
			{}
			

		protected:	
			virtual bool OnClientConnect(std::shared_ptr<connection<Header>> client)
			{
		
				return true;
			}
			virtual void OnClientDisconnect(std::shared_ptr<connection<Header>> client)
			{
				//update maps
				if(!client)
				{
					return;
				}
				
			}
	
			virtual void OnMessage(UserConnection_ptr connection,std::shared_ptr<Message> mesg)
			{
				
				asio::post(
					[this,connection,mesg]()
					{
						
						switch(mesg->header.id)
						{
							case Header::SYS_MESSAGE:
								std::cout<<"SysMesg\n";
								sys_handler.HandlePacket(connection, mesg);
								break;
						
							case Header::FILE_MESSAGE:
								// implementation pending
								// m_fileManager.Handler(connection, mesg);
								break;
						
							case Header::CHAT_MESSAGE:
								chat_handler.HandleTextPacket(connection, mesg);
								break;
								
							case Header::ROOM_REQUEST:
								chat_handler.HandleRoomRequestPacket(connection,mesg);
								break;
							
							case Header::QUERY :
								
								break;
								
							default :
								break;
						}
					});
			}
		public:
			virtual void OnClientValidation(std::shared_ptr<connection<Header>> client)
			{
			//	send CONNECTION_ACK packet on connection validation
			//	message<Header> acknowledgementPacket;
			//	acknowledgementPacket.header.id = Header::CONNECTION_ACK;
			//	client->Send(acknowledgementPacket);
			}
			
		public:
			Authorizer authorizer;
			SysHandler sys_handler;
			ChatHandler chat_handler;
		 //  FileManager m_fileManager ;
		};
	}	
}
#endif 
