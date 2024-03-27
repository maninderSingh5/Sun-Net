#ifndef SYSMANAGER_HPP
#define SYSMANAGER_HPP

#include "sun_net.hpp"
#include "AuthManager.hpp"

namespace sun
{
	class SysHandler
	{
	public:
		SysHandler(Authorizer& auth)
		:authorizer(auth)
		{}
		
		void HandlePacket(UserConnection_ptr connection,std::shared_ptr<Message> mesg)
		{
			SysRequest flag ;
			*mesg >> flag;
			
			switch(flag)
			{
				case SysRequest::LOGIN :
					std::cout<<"New Login\n";
					LoginUser(connection, mesg);
					break;
				
				case SysRequest::SIGNUP :
					std::cout<<"New SignUp\n";
					SignupUser(connection, mesg);
					break;
					
				default :
					break;
			};
		}
		
		void LoginUser(UserConnection_ptr connection,std::shared_ptr<Message> mesg)
		{
			UserID uid;
			Password pass;
			try
			{
				mesg->GetData(&pass, &uid);
				std::cout<<"new login attempt uid-"<<uid<<"-\n";
			}
			catch(std::exception& e)
			{
				std::cout<<"SysMesg - Login GetData failed packet discarded\n";
				SysRequestNotAck(connection, "log-in failed for unknown reasons", SysRequest::LOG_NACK);
				return;
			}
			
			if(!authorizer.isAuthenticated(uid))
			{
				//send login_not_ack with error message
				SysRequestNotAck(connection, "mistake in user_id", SysRequest::LOG_NACK);
				return;
			}
			
			auto& tuple_data  = authorizer.GetUserDetails<UserConnection_ptr, UserDataQueue_ptr, UserInfo_ptr, Password>(uid);
			
			if(std::get<Password>(tuple_data) != pass)
			{
				//send login_not_ack with message wrong password
				SysRequestNotAck(connection, "wrong password", SysRequest::LOG_NACK);
				return;
			}
			
			//send login_ack with account data
			std::get<UserConnection_ptr>(tuple_data) = connection;
			connection->SetOutQueue(std::get<UserDataQueue_ptr>(tuple_data));
			authorizer.ValidateConnection(connection->GetID(),uid);
			
			net::message<Header> log_ack_packet;
			log_ack_packet.header.id = Header::SYS_MESSAGE;
			std::get<UserInfo_ptr>(tuple_data)->Serialize(log_ack_packet);
			log_ack_packet << SysRequest::LOG_ACK;
			
			connection->SendPriorityMesg(log_ack_packet);
			connection->SendPendingMesg();
		}
		
		void SysRequestNotAck(UserConnection_ptr connection, std::string error_mesg, SysRequest nack_flag)
		{
			net::message<Header> not_ack_packet;
			not_ack_packet.header.id = Header::SYS_MESSAGE;
			
			not_ack_packet.SerializeArray(error_mesg.data(), error_mesg.size());
			not_ack_packet << error_mesg.size() << nack_flag;
			
			connection->SendIndependentMesg(not_ack_packet);
		}
		
		void SignupUser(UserConnection_ptr connection, std::shared_ptr<Message> mesg)
		{
			UserID uid;
			Password pass;
			std::string user_name;
			
			try
			{
				mesg->GetData(&uid, &pass, &user_name);
				std::cout<<"new sign up-"<<uid<<"- pass-"<<pass<<"-\n";
			}
			catch(...)
			{
				SysRequestNotAck(connection, "sign-up failed for unknown reasons", SysRequest::SIGN_NACK);
				return;
			}
			
			if(authorizer.isAuthenticated(uid))
			{
				SysRequestNotAck(connection, "try different user_id", SysRequest::SIGN_NACK);
				return;
			}
			
			UserDataQueue_ptr new_queue(new UserDataQueue());
			UserInfo_ptr user_details(new UserInfo(uid, user_name));
			
			authorizer.AuthenticateUser(uid, connection, new_queue, user_details, pass);
			
			net::message<Header> log_ack_packet;
			log_ack_packet.header.id = Header::SYS_MESSAGE;
			log_ack_packet << SysRequest::LOG_ACK;
			connection->SetOutQueue(new_queue);
			connection->SendPriorityMesg(log_ack_packet);
		}
		
	private:
		Authorizer& authorizer;
	};  
}

#endif