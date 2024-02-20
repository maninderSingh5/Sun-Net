#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "common.hpp"

namespace sun
{
	namespace net{
		template<typename T>
		struct mesg_header
		{
			T id;
			uint32_t size;
		};
		
		template <typename T>
		class message
		{
		public:
			mesg_header<T> header;
			std::vector<uint8_t> body;
		public:
			friend std::ostream& operator << (std::ostream& out, const message<T> mesg)
			{
				out << "ID : " << mesg.header.id << " Size : " << mesg.body.size();
				return out;
			}
			
			
			// serialize mesg
			template <typename DataType>
			friend message<T>& operator <<(message<T>& mesg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value,"message assert :datatype too complex to copy");
				uint64_t i = mesg.body.size();
				mesg.body.resize(i + sizeof(data));
				
				std::memcpy(mesg.body.data()+ i,&data,sizeof(data));
				mesg.header.size = mesg.body.size();
				return mesg;
			}
			
			template <typename DataType>
			void SerializeArray(DataType* data, uint64_t bytes)
			{
				uint64_t i = body.size();
				body.resize(i + bytes);
				
				std::memcpy(body.data()+ i,data,bytes);
				header.size = body.size();
			}
			
			
			//de-serialize mesg
			template <typename DataType>
			friend message<T>& operator >>(message<T>& mesg, DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value,"message assert :datatype too complex to copy");
				
				uint64_t i = mesg.body.size() - sizeof(data);
								
				std::memcpy(&data,mesg.body.data()+i,sizeof(data));
				
				mesg.body.resize(i);	
				mesg.header.size = mesg.body.size();	
				return mesg;
			}
			
			template <typename DataType>
			void DeSerializeArray(DataType* data, uint64_t bytes)
			{
				
				uint64_t i = body.size() - bytes;
								
				std::memcpy(data,body.data()+i,bytes);
				
				body.resize(i);	
				header.size = body.size();	
				
			}
		};
		
		template <typename T>
		class connection;
		
		template <typename T>
		struct owned_mesg
		{
			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> mesg;
			
friend std::ostream& operator <<(std::ostream& out, const owned_mesg<T>& msg)
			{
				out << msg.mesg;
				return out;
			}
		};
	}
}

#endif