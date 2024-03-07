#ifndef SERVER_HPP
	#define SERVER_HPP

// from Sun-Net Project
#include "sun_net.hpp"

namespace sun
{
	namespace net
	{
		class server : public server_interface<Header>
		{
		public:
			server(uint16_t port) :server_interface<Header>(port)
			{}
			
			void SystemMessageHandler(std::shared_ptr<connection<Header>> connection,sun::net::message<Header>& mesg)
			{
				switch(mesg.header.id)
				{
					case Header::LOGIN :
						UserLogin(connection,mesg);
						break;
					case Header::SIGNUP :
						UserSignUp(connection,mesg);
						break;
				}
			}
		
			void TextMessageHandler(std::shared_ptr<connection<Header>> connection,sun::net::message<Header>& mesg)
			{
				std::string userToSentMesg;
				uint32_t size = 0;
				mesg >> size;
				userToSentMesg.resize(size);
				mesg.DeSerializeArray(userToSentMesg.data(),size);
				
				if(auto d=m_validConnections.find(connection->GetID()); d!=m_validConnections.end())
				{
					mesg.SerializeArray(d->second.data(),d->second.size());
					mesg << size;
				}
				
				if(auto d=m_userDetails.find(userToSentMesg); d != m_userDetails.end())
				{
					std::get<0>(d->second)->Send(mesg);
				}
				
			}
			
			void UserLogin(std::shared_ptr<connection<Header>> connection,sun::net::message<Header>& mesg)
			{
				message<Header> ackPacket;
				std::string user_id, password;
				uint32_t size = 0;
				std::string error_mesg("User_id or Password is Wrong!!");
				
				mesg >> size;
				if(size)
				{
					user_id.resize(size);
					mesg.DeSerializeArray(user_id.data(),size);
				}
				mesg >> size;
				if(size)
				{
					password.resize(size);
					mesg.DeSerializeArray(password.data(),size);
				}
				
				//user_id should exist in the map and password should match with the stored password
				
				if(auto d = m_userDetails.find(user_id); d != m_userDetails.end() && password == std::get<3>(d->second))
				{
					ackPacket.header.id = Header(Header::SYS_MESSAGE | Header::LOG_ACK);
					std::get<2>(d->second)->Serialize(ackPacket);
					std::get<0>(d->second) = connection;
					m_validConnections[connection->GetID()] = d->first;
					connection->SetOutQueue(std::get<1>(d->second));
				}
				else
				{
					ackPacket.header.id = Header(Header::SYS_MESSAGE | Header::LOG_NACK);
					ackPacket.SerializeArray(error_mesg.data(),error_mesg.size());
				}
				
				
				connection->Send(ackPacket);
			}
			
			void UserSignUp(std::shared_ptr<connection<Header>> connection,sun::net::message<Header>& mesg)
			{
				message<Header> ackPacket;
				std::string user_id, user_name, password;
				uint32_t size = 0;
				
				mesg >> size;
				if(size)
				{
					user_id.resize(size);
					mesg.DeSerializeArray(user_id.data(),size);
				}
				mesg >> size;
				if(size)
				{
					user_name.resize(size);
					mesg.DeSerializeArray(user_name.data(),size);
				}
				
				mesg >> size;
				if(size)
				{
					password.resize(size);
					mesg.DeSerializeArray(password.data(),size);
				}
					
				if(auto d = m_userDetails.find(user_id); d == m_userDetails.end())
				{
					std::shared_ptr<user_info<Header>> user_data(new user_info<Header>(user_id,user_name));
					auto tuple = std::make_tuple(connection,std::make_shared<ThreadSafeQueue<message<Header>>>(),user_data,password);
					
					ackPacket.header.id = Header(Header::SYS_MESSAGE | Header::LOG_ACK);
					user_data->Serialize(ackPacket);
					
					std::get<1>(d->second) = std::make_shared<ThreadSafeQueue<message<Header>>>();
					
					std::get<3>(d->second) = password;
					
				}
				else
				{
					std::string error_mesg("user_id already exists!");
					ackPacket.header.id = Header(Header::SYS_MESSAGE | Header::SIGN_NACK);
					ackPacket.SerializeArray(error_mesg.data(),error_mesg.size());
				}
				connection->Send(ackPacket);
			}
			
			

		protected:	
			virtual bool OnClientConnect(std::shared_ptr<connection<Header>> client)
			{
		
				return true;
			}
			virtual void OnClientDisconnect(std::shared_ptr<connection<Header>> client)
			{
				//update maps
				if(auto d=m_validConnections.find(client->GetID()); d!=m_validConnections.end())
				{
					std::get<0>(m_userDetails.find(d->second)->second).reset();
					m_validConnections.erase(client->GetID());
				}
			}
	
			virtual void OnMessage(std::shared_ptr<connection<Header>> connection,sun::net::message<Header> mesg)
			{
				
				asio::post(
					[this,connection,&mesg]()
					{
						uint32_t flag = mesg.header.id;
						uint32_t workCategoryFlag = uint32_t(1<<(std::bit_width(flag)));
						mesg.header.id = Header(~workCategoryFlag & mesg.header.id);
				
						switch(workCategoryFlag)
						{
							case Header::SYS_MESSAGE:
								SystemMessageHandler(connection,mesg);
							break;
						
							case Header::FILE_MESSAGE:
								// implementation pending
								// m_fileManager.Handler(connection,mesg);
								break;
						
							case Header::TEXT_MESSAGE:
								TextMessageHandler(connection,mesg);
								break;
								
							default :
								break;
						}
					});
			}
		public:
			virtual void OnClientValidation(std::shared_ptr<connection<Header>> client)
			{
				//send CONNECTION_ACK packet on connection validation
				message<Header> acknowledgementPacket;
				acknowledgementPacket.header.id = Header::CONNECTION_ACK;
				client->Send(acknowledgementPacket);
			}
			
		public:
			// key			value
			
			// user_id , < (0)currentConnectionFromWhereUserIsConnected, (1)Queue for user to revc messages, (2)user_info, (3)password - collected data while signing in and updated according to user requests>
			std::unordered_map<std::string,std::tuple<std::shared_ptr<connection<Header>>,std::shared_ptr<ThreadSafeQueue<message<Header>>>,std::shared_ptr<user_info<Header>>,std::string>> m_userDetails;
			// connection_id, < user_id>  -- this invariant is used to tell whether the connected client is valid, So that message packets can be sent
			std::unordered_map<uint32_t,std::string> m_validConnections;
		 //  FileManager m_fileManager ;
		};
	}	
}
#endif 
