#pragma once
#ifndef NETATTRIBUTES_HPP
#define NETATTRIBUTES_HPP

#include "sun_net.hpp"
#include "connection.hpp"

namespace sun
{
	namespace net
	{
		struct UserConnection_ptr : std::shared_ptr<connection<Header> >
		{

		};

		struct UserDataQueue_ptr :std::shared_ptr<ThreadSafeQueue<message<Header> > >
		{

		};

	}

}


#endif
