#ifndef AUTHMANAGER_HPP
#define AUTHMANAGER_HPP

#include "sun_net.hpp"

namespace sun
{	
	template <typename Key, class... Value>
	class Authorizer {

	private:
		Database<Key, Value...> m_userDB;
	};
}

#endif