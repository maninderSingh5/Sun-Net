#ifndef FLAGS_HPP
#define FLAGS_HPP

#include "common.hpp"

namespace sun
{
	enum Header : uint32_t
	{
		// • Systen headers -- set, reset or update user data
		
		//server send it to client after successful connection implementation
		CONNECTION_ACK		 = 1<<0,
		//login message contains user_id and password
		LOGIN					 = 1<<1,
		// Login Ack send by server with body contains user_info
		LOGACK					= 1<<2,
		//signing in packet contains user_id, user_name,password
		SIGNIN					= 1<<3,
		//Sign Not Ack will return with an error message which points out what went wrong while creating mew account
		SIGN_NACK				= 1<<4,
		//Query used to search for friends user_id in the server
		QUERY					 = 1<<5,
		//by string matching server will send ACK with matched user_id (message body may contain more than 1 user_ids) which further can be used to Request Rooms to interact with other people
		QACK					  = 1<<6,
		
		//ROOM formed by two friends for chatting
		ROOM_REQUEST			= 1<<7,
		ROOM_ACCEPT			 = 1<<8,
		ROOM_REJECT			 = 1<<9,
		
		// • Text headers -- involves sending messages to clients 
		
		
		
		// • File headers -- involves operations releated to files
		
		
		
		SYS_HANDLER			 = 1<<29,
		FILE_HANDLER			= 1<<30,
		TEXT_HANDLER			= (uint32_t)1<<31
	};
	
	
}

#endif
