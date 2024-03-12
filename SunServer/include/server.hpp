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
			
			void SystemMessageHandler(std::shared_ptr<connection<Header>> connection,std::shared_ptr<message<Header>> mesg)
			{
				switch(mesg->header.id)
				{
					case Header::LOGIN :
						UserLogin(connection,mesg);
						break;
					case Header::SIGNUP :
						UserSignUp(connection,mesg);
						break;
					case Header::ROOM_REQUEST :
						HandleRoomRequest(connection,mesg);
						break;
					case Header::QUERY :
						
						break;
					default :
						
						break;
				}
			}
			
			void HandleRoomRequest(std::shared_ptr<connection<Header>> connection,std::shared_ptr<message<Header>> mesg)
			{
				uint32_t requestTypeFlag;
				*mesg >> requestTypeFlag;
				
				uint32_t size;
				std::string receiver_id;
				*mesg >> size;
				receiver_id.resize(size);
				mesg->DeSerializeArray(receiver_id.data(),size);
				
				auto senderConnection = m_validConnections.find(connection->GetID());
				if(senderConnection == m_validConnections.end())
				{
					// invalid user's request packet discarded
					return;
				}
				
				auto senderDetail = m_userDetails.find(senderConnection->second);
				auto receiverDetail = m_userDetails.find(receiver_id);
				
				if(receiverDetail == m_userDetails.end())
				{
					std::cout<<receiver_id<<" Receiver NotFound\n";
					return;
				}
				if(std::get<2>(senderDetail->second)->IsFriendOf(receiver_id))
				{
				// sender is already a friend of receiver then request packet is discarded
					return;
				}
				
				
				switch(requestTypeFlag)
				{
					case RoomRequest::FRIEND_REQUEST :
						
						mesg->SerializeArray(senderConnection->second.data(),senderConnection->second.size());
						*mesg << senderDetail->first.size() << RoomRequest::FRIEND_REQUEST;
						std::get<2>(senderDetail->second)->StoreSentFriendRequest(receiver_id);
						std::get<2>(receiverDetail->second)->StoreRevcFriendRequest(senderDetail->first);
						break;
					
					case RoomRequest::FRIEND_REQ_ACCEPT:
						mesg->SerializeArray(senderConnection->second.data(),senderConnection->second.size());
						*mesg << senderDetail->first.size() << RoomRequest::FRIEND_REQ_ACCEPT;
						std::get<2>(senderDetail->second)->AddFriend(receiver_id);
						std::get<2>(receiverDetail->second)->AddFriend(senderDetail->first);
						break;
					
					case RoomRequest::FRIEND_REQ_REJECT:
						mesg->SerializeArray(senderConnection->second.data(),senderConnection->second.size());
						*mesg << senderDetail->first.size() << RoomRequest::FRIEND_REQ_REJECT;
						std::get<2>(senderDetail->second)->StoreFriendRequestReject(receiver_id);
						std::get<2>(receiverDetail->second)->StoreFriendRequestReject(senderDetail->first);
						break;
					default :
						return;
				}
				
				SendOrStore(receiver_id,mesg);
			}
			
			void TextMessageHandler(std::shared_ptr<connection<Header>> connection,std::shared_ptr<message<Header>> mesg)
			{
				std::string receiver_id, sender_id;
				uint32_t size = 0;
				*mesg >> size;
				receiver_id.resize(size);
				
				mesg->DeSerializeArray(receiver_id.data(),size);
				
				if(auto d=m_validConnections.find(connection->GetID()); d!=m_validConnections.end())
				{
					sender_id = d->second;
					mesg->header.id = Header::TEXT_MESSAGE;
					mesg->SerializeArray(sender_id.data(),sender_id.size());
					*mesg << sender_id.size();
				}
				else
				{
					std::cout<<"["<<connection->GetID()<<"] TEXT_MESSAGE discarded : Reason-> Inavlid Connection\n";
					return;
				}
				
				if(auto d=m_userDetails.find(receiver_id); d != m_userDetails.end())
				{
					if(!std::get<2>(d->second)->IsFriendOf(sender_id))
					{
						std::cout<<"["<<connection->GetID()<<"] TEXT_MESSAGE discarded : Reason-> Clients Aren't Friends\n";
						return;
					}
					SendOrStore(receiver_id, mesg);
				}
				else
				{
					std::cout<<"["<<connection->GetID()<<"] TEXT_MESSAGE discarded : Reason-> Clients doesn't exist\n";
					return;
				}
			}
			
			void UserLogin(std::shared_ptr<connection<Header>> connection,std::shared_ptr<message<Header>> mesg)
			{
				message<Header> ackPacket;
				ackPacket.header.id = Header(Header::SYS_MESSAGE | Header::LOG_NACK);
				
				std::string user_id, password;
				uint32_t size = 0;
				std::string error_mesg("User_id Not Found!!");
				
				*mesg >> size;
				if(size)
				{
					user_id.resize(size);
					mesg->DeSerializeArray(user_id.data(),size);
				}
				*mesg >> size;
				if(size)
				{
					password.resize(size);
					mesg->DeSerializeArray(password.data(),size);
				}
				
				//user_id should exist in the map and password should match with the stored password
				
				if(auto d = m_userDetails.find(user_id); d != m_userDetails.end())
				{
					if(password == std::get<3>(d->second))
					{
						ackPacket.header.id = Header(Header::SYS_MESSAGE | Header::LOG_ACK);
						std::get<2>(d->second)->Serialize(ackPacket);
						std::get<0>(d->second) = connection;
						m_validConnections[connection->GetID()] = d->first;
						connection->SetOutQueue(std::get<1>(d->second));
						std::cout<<"User Login - "<<user_id<<"\n";
					}
					else
					{
						error_mesg = "Wrong Password!! Try Again!";
					}
				}
				else
				{
					ackPacket.SerializeArray(error_mesg.data(),error_mesg.size());
				}
				
				connection->SendPriorityMesg(ackPacket);
			}
			
			void UserSignUp(std::shared_ptr<connection<Header>> connection,std::shared_ptr<message<Header>> mesg)
			{
				message<Header> ackPacket;
				std::string user_id, user_name, password;
				uint32_t size = 0;
				
				*mesg >> size;
				if(size)
				{
					user_id.resize(size);
					mesg->DeSerializeArray(user_id.data(),size);
				}
				*mesg >> size;
				if(size)
				{
					user_name.resize(size);
					mesg->DeSerializeArray(user_name.data(),size);
				}
				
				*mesg >> size;
				if(size)
				{
					password.resize(size);
					mesg->DeSerializeArray(password.data(),size);
				}
					
				if(auto d = m_userDetails.find(user_id); d == m_userDetails.end())
				{
					std::shared_ptr<user_info<Header>> user_data;
					user_data = std::make_shared<user_info<Header>>(user_id,user_name);
					
					std::shared_ptr<ThreadSafeQueue<message<Header>>> mesg_out_queue;
					mesg_out_queue = std::make_shared<ThreadSafeQueue<message<Header>>>();
					
					auto tuple = std::make_tuple(connection,mesg_out_queue,user_data,password);
					
					m_userDetails.emplace(user_id,std::move(tuple));
					m_validConnections.emplace(connection->GetID(),user_id);
					connection->SetOutQueue(std::get<1>(tuple));
					
					ackPacket.header.id = Header(Header::SYS_MESSAGE | Header::LOG_ACK);
					user_data->Serialize(ackPacket);
					std::cout<<"New Sign up - "<<user_id<<", "<<std::get<3>(m_userDetails[user_id])<<"\n";
				}
				else
				{
					std::string error_mesg("user_id already exists!");
					ackPacket.header.id = Header(Header::SYS_MESSAGE | Header::SIGN_NACK);
					ackPacket.SerializeArray(error_mesg.data(),error_mesg.size());
				}
				
				connection->SendPriorityMesg(ackPacket);
			}
			
			void SendOrStore(std::string user_id, std::shared_ptr<message<Header>> mesg)
			{
				auto userDetail = m_userDetails.find(user_id);
				
				if(userDetail == m_userDetails.end())
					return;
				
				if(!MessageClient(std::get<0>(userDetail->second),*mesg))
				{
					std::cout<<"store -";
					std::get<1>(userDetail->second)->push(*mesg);
				}
				
			}
			

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
				
				if(auto d=m_validConnections.find(client->GetID()); d!=m_validConnections.end())
				{
					std::get<0>(m_userDetails.find(d->second)->second).reset();
					m_validConnections.erase(client->GetID());
				}
			}
	
			virtual void OnMessage(std::shared_ptr<connection<Header>> connection,std::shared_ptr<sun::net::message<Header>> mesg)
			{
				
				asio::post(
					[this,connection,mesg]()
					{
						
						uint32_t flag = mesg->header.id;
						uint32_t workCategoryFlag = uint32_t(1<<(std::bit_width(flag) - 1));
						//uint32_t workCategoryFlag = uint32_t(1<<(31 - __builtin_clz(flag)));
						mesg->header.id = Header(~workCategoryFlag & mesg->header.id);
						
						switch(workCategoryFlag)
						{
							case Header::SYS_MESSAGE:
								std::cout << "Incoming Sys_Message\n";
								SystemMessageHandler(connection, mesg);
								break;
						
							case Header::FILE_MESSAGE:
								// implementation pending
								// m_fileManager.Handler(connection, mesg);
								break;
						
							case Header::TEXT_MESSAGE:
								TextMessageHandler(connection, mesg);
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
			//	client->Send(acknowledgementPacket);
			}
			
		public:
			// key			value
			
			// user_id , < (0)currentConnectionFromWhereUserIsConnected, (1)Queue for user to revc messages, (2)user_info, (3)password - collected data while signing in and updated according to user requests>
			std::unordered_map<std::string,std::tuple<std::shared_ptr<connection<Header>>,std::shared_ptr<ThreadSafeQueue<message<Header>>>,std::shared_ptr<user_info<Header>>,std::string>> m_userDetails;
			// connection_id, < user_id>  -- this vatiant is used to tell whether the connected client is valid, So that message packets can be sent
			std::unordered_map<uint32_t,std::string> m_validConnections;
		 //  FileManager m_fileManager ;
		};
	}	
}
#endif 
