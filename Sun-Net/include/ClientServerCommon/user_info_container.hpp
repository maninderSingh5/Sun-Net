#ifndef USER_INFO_CONTAINER_HPP
#define USER_INFO_CONTAINER_HPP

#include "sun_net.hpp"

namespace sun
{
	namespace net
	{
		template <typename T>
		class user_info
		{
		public:
			user_info()
			{}
			
			user_info(std::string id, std::string name)
			: m_user_id(id), m_userName(name)
			{}
			
			~user_info()
			{
				std::cout<<"userdied ->"<<m_user_id<<"\n";
			}
			
			void Serialize(message<T>& mesg)
			{
				//implementation pending
			}
			
			void DeSerilaize(message<T>& mesg)
			{
				//implementation pending
			}
			
			bool isFriendOf(std::string other_user)
			{
				auto d = m_rooms.find(other_user);
				return (d != m_rooms.end());
			}
			
			void AddFriend(std::string new_friend)
			{
				m_roomRequestIn.erase(new_friend);
				m_roomRequestOut.erase(new_friend);
				m_rooms.insert(new_friend);
			}
			
			void RemoveFriend(std::string friend_id)
			{
				m_rooms.erase(friend_id);
			}
			
			void StoreRevcFriendRequest(std::string request_revc)
			{
				m_roomRequestIn.insert(request_revc);
			}
			void StoreSentFriendRequest(std::string request_sent)
			{
				m_roomRequestOut.insert(request_sent);
			}
			
			void StoreFriendRequestReject(std::string user_id)
			{
				m_roomRequestIn.erase(user_id);
				m_roomRequestOut.erase(user_id);
			}
			
			bool isIncomingRequest(std::string uid)
			{
				return m_roomRequestIn.contains(uid);
			}
			
		private:
			std::string m_user_id;
			std::string m_userName;
			std::set<std::string> m_rooms;
			std::set<std::string> m_roomRequestIn;
			std::set<std::string> m_roomRequestOut;
			
		};
	}
}

#endif 
