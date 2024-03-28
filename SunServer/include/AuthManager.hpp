#ifndef AUTHMANAGER_HPP
#define AUTHMANAGER_HPP

#include "sun_net.hpp"

namespace sun
{
	class Authorizer 
	{
	public:
		Authorizer()
			: m_userDB()
		{

		}

		bool isAuthenticated(UserID uid)
		{
			return m_userDB.contains(uid);
		}

		bool isValidConnection(int connection_id)
		{
			return m_validConnection.contains(connection_id);
		}
		
		template <typename Key, class... Value>
		void AuthenticateUser(Key key, Value&... args)
		{
			m_userDB.set_new_value(key, args...);
			int connection_id = m_userDB.template get<UserConnection_ptr>(key)->GetID();
			
			ValidateConnection(connection_id, key);
		}
		
		void ValidateConnection(int con_id, const UserID& key)
		{
			m_validConnection.set_new_value(con_id, key);
		}
		
		
		template <typename Value>
		Value& get(int connection_id)
		{
			return (m_validConnection.get<Value>(connection_id));
		}
		
		template <typename Value>
		void set(UserID uid, const Value& arg)
		{
			m_validConnection.set<Value>(uid, arg);
		}
		
		template <typename Value>
		Value& get(UserID uid)
		{
			return (m_userDB.get<Value>(uid));
		}
		
		void InvalidateConnection(int connection_id)
		{
			auto it = m_validConnection.find(connection_id);

			if (it == m_validConnection.end())
				return;
			UserID key = it->second.template get<UserID>();
			m_validConnection.erase(it);
			m_userDB.template set<UserConnection_ptr>(key,nullptr);

		}
		template <class... Value>
		std::tuple<Value...>& GetUserDetails(UserID uid)
		{
			return m_userDB.find(uid)->second.GetData();
		}
		
	private:
		Database<UserID, UserConnection_ptr, UserDataQueue_ptr, UserInfo_ptr,Password> m_userDB;
		Database<int, UserID> m_validConnection;

	};
}

#endif
