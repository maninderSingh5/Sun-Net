#ifndef USER_INFO_CONTAINER_HPP
#define USER_INFO_CONTAINER_HPP

#include "sun_net.hpp"

namespace sun
{
	namespace net
	{
		template <typename T>
		struct user_info
		{
			std::string m_user_id;
			std::string m_userName;
			std::vector<std::string> m_rooms;
			std::vector<std::string> m_roomRequestsIn;
			std::vector<std::string> m_roomsRequestOut;
			
			user_info(std::string id, std::string name)
			: m_user_id(id), m_userName(name)
			{}
			
			void Serialize(message<T>& mesg)
			{
				//implementation pending
			}
			
			void DeSerilaize(message<T>& mesg)
			{
				//implementation pending
			}
		};
	}
}

#endif 
