#ifndef AUTHMANAGER_HPP
#define AUTHMANAGER_HPP

#include "sun_net.hpp"

namespace sun
{	
	template <typename Key, class... Value>
	class Authorizer 
	{
	public:
		Authorizer(Database<Key, Value...>& database)
			: m_userDB(database)
		{

		}

		bool isAuthenticate(Key key)
		{
			return m_userDB.contains(key);
		}

		bool isValidConnection(int connection_id)
		{
			return m_validConnection.contains(connection_id);
		}

		void AuthenticateUser(Key key, Value&... args)
		{
			m_userDB.set_new_value(key, args);
			int connection_id = m_userDB.get<UserConnection_ptr>(key)->Getid();
			m_validConnection.set_new_value(connection_id, key);
			
		}
		
		std::shared_ptr<Key> GetKey(int connection_id)
		{
			if (!m_validCOnnection.contains(connection_id))
				return nullptr;

			return std::make_shared<Key>(m_validConnection.get<Key>(connection_id));
		}

		void InvalidateConnection(int connection_id)
		{
			db_iterator it = m_validConnection.find(connection_id);

			if (it == m_validConnection.end())
				return;
			Key key = it->second.get<Key>();
			m_validConnection.erase(it);
			m_userDB.get<UserConnection_ptr>(key) = nullptr;

		}


	private:
		Database<Key, Value...>& m_userDB;
		Database<int, Key> m_validConnection;

	};
}

#endif