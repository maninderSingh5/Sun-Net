#pragma once
#ifndef NETATTRIBUTES_HPP
#define NETATTRIBUTES_HPP

#include "sun_net.hpp"

namespace sun
{
	namespace net
	{

		struct UserID : std::string
		{

		};

		struct Password : std::string
		{

		};

		struct UserInfo : user_info<Header>
		{

		};

		struct UserConnection_ptr : std::shared_ptr<connection<Header> >
		{

		};

		struct UserDataQueue_ptr :std::shared_ptr<ThreadSafeQueue<message<Header> > >
		{

		};


	}

}


#endif
