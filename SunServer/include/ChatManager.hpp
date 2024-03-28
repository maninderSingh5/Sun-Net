#ifndef CHATMANAGER_HPP
#define CHATMANAGER_HPP

#include "sun_net.hpp"
#include "AuthManager.hpp"

namespace sun
{
	class ChatHandler
	{
	public:
		ChatHandler(Authorizer& auth)
		:authorizer(auth)
		{}
		
		void HandleTextPacket(UserConnection_ptr connection, std::shared_ptr<Message> mesg)
		{
			if(!authorizer.isValidConnection(connection->GetID()))
			{
				std::cout<<"Invalid Connection packet discarded\n";
				return;
			}
			
			
			ChatMesg flag;
			*mesg >> flag;
			
			switch(flag)
			{
				case ChatMesg::TEXT :
					HandleTextMesg(connection, mesg);
					break;
				
				case ChatMesg::FILE_MESG :
					break;
					
				default :
					break;
			};
		}
		
		void HandleRoomRequestPacket(UserConnection_ptr connection, std::shared_ptr<Message> mesg)
		{
			
			if(!authorizer.isValidConnection(connection->GetID()))
			{
				std::cout<<"Invalid Connection packet discarded\n";
				return;
			}
			
			
			RoomRequest flag;
			*mesg >> flag;
			
			switch(flag)
			{
				case RoomRequest::FRIEND_REQUEST :
					HandleFriendRequest(connection, mesg);
					break;
				case RoomRequest::FRIEND_REQ_ACCEPT:
					HandleReqAccepted(connection, mesg);
					break;
				case RoomRequest::FRIEND_REQ_REJECT:
					HandleReqRejected(connection, mesg);
					break;
					
				default :
					break;
			};
			
		}
		
		
	private:
		void HandleTextMesg(UserConnection_ptr connection, std::shared_ptr<Message> mesg)
		{
			
			UserID const sender_id = authorizer.get<UserID>(connection->GetID());
			UserID receiver_id;
			
			mesg->GetData(&receiver_id);
			
			if(!authorizer.isAuthenticated(receiver_id))
			{
				std::cout << "mesg to unkown user - packet discarded\n";
				return;
			}
			
			UserInfo_ptr receiver_info = authorizer.template get<UserInfo_ptr>(receiver_id);
			
			if(!receiver_info->isFriendOf(sender_id))
			{
				std::cout << "sender is not a friend of receiver - packet discarded\n";
				return;
			}
			
			mesg->SerializeArray(sender_id.data(),sender_id.size());
			*mesg << sender_id.size() << ChatMesg::TEXT;
			
			authorizer.template get<UserDataQueue_ptr>(receiver_id)->push(*mesg);
			
		}
		
		void HandleFriendRequest(UserConnection_ptr connection, std::shared_ptr<Message> mesg)
		{
			UserID const sender_id = authorizer.get<UserID>(connection->GetID());
			UserID receiver_id;
			mesg->GetData(&receiver_id);
			
			if(!authorizer.isAuthenticated(receiver_id))
			{
				std::cout << "Friend_Req to unknown user - packet discarded\n";
				return;
			}
			
			mesg->SerializeArray(sender_id.data(),sender_id.size());
			*mesg << sender_id.size() ;
			
			if(authorizer.template get<UserInfo_ptr>(sender_id)->isIncomingRequest(receiver_id))
			{	
				
				
				*mesg << RoomRequest::FRIEND_REQ_ACCEPT;
		
				Message senders_mesg;
				senders_mesg.header.id = Header::ROOM_REQUEST;
				senders_mesg.SerializeArray(receiver_id.data(), receiver_id.size());
				senders_mesg << RoomRequest::FRIEND_REQ_ACCEPT;
				
				authorizer.template get<UserInfo_ptr>(sender_id)->AddFriend(receiver_id);
				authorizer.template get<UserDataQueue_ptr>(sender_id)->push(senders_mesg);
				
				HandleReqAccepted(connection, mesg);
				return;
			}
			
			
			
			*mesg << RoomRequest::FRIEND_REQUEST; 
			authorizer.template get<UserInfo_ptr>(receiver_id)->StoreRevcFriendRequest(sender_id);
			authorizer.template get<UserInfo_ptr>(sender_id)->StoreSentFriendRequest(receiver_id);
			authorizer.template get<UserDataQueue_ptr>(receiver_id)->push(*mesg);
		}
		
		void HandleReqAccepted(UserConnection_ptr connection, std::shared_ptr<Message> mesg)
		{
			UserID const sender_id = authorizer.get<UserID>(connection->GetID());
			UserID receiver_id;
			mesg->GetData(&receiver_id);
			
			if(!authorizer.isAuthenticated(receiver_id))
			{
				return;
			}
			
			*mesg << RoomRequest::FRIEND_REQ_ACCEPT;
			authorizer.template get<UserInfo_ptr>(receiver_id)->AddFriend(sender_id);
			authorizer.template get<UserDataQueue_ptr>(receiver_id)->push(*mesg);
		}
		
		void HandleReqRejected(UserConnection_ptr connection, std::shared_ptr<Message> mesg)
		{
			
		}
		
	private:
		Authorizer& authorizer;
	};
}

#endif 
