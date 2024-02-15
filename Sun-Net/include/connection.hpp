#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "common.hpp"
#include "ThreadSafeQueue.hpp"
#include "message.hpp"

namespace sun
{
	namespace net
	{
		template <typename T>
		class server_interface;
		
		template <typename T>
		class connection :public std::enable_shared_from_this<connection<T>>
		{
		public:
			enum owner
			{
				client,
				server
			};
			connection(owner parent,
						asio::io_context& context,
						asio::ip::tcp::socket&& socket,
						ThreadSafeQueue<owned_mesg<T>>& incoming):
							m_socket(std::move(socket)),
							m_context(context), 
							m_qMesgIn(incoming)
			{
				m_ownerType = parent;
				
				if(m_ownerType == owner::server)
				{
					m_handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
					m_handshakeCheck = scramble(m_handshakeOut);
				}
				
			}
			virtual ~connection()
			{
				
			}
			
			void ConnectToClient(server_interface<T>* server,uint32_t uid = 0)
			{
				if(m_ownerType == owner::server && m_socket.is_open())
				{
					m_id = uid;
					WriteValidation();
					ReadValidation(server);
				}
			}
		
			void ConnectToServer(asio::ip::tcp::resolver::results_type& endpoints)
			{
				asio::async_connect(m_socket,endpoints,
				[this](asio::error_code ec,asio::ip::tcp::endpoint endpoints )
				{
					if(!ec)
					{
						ReadValidation();
					}
					else
					{
						std::cout<<"[Client] Connection To Server Failed : "<<ec.message()<<"\n";
					}
				});
			}
			
			void Disconnect()
			{
				if(IsConnected())
					asio::post(
					[this]()
					{
						m_socket.close();
					});
			}
			bool IsConnected() const
			{
				return m_socket.is_open();
			}
			
			uint32_t GetID()
			{
				return m_id;
			}
			
			void Send(const message<T>& mesg)
			{
				asio::post(
				[this, mesg]()
				{
					bool qOutEmpty = m_qMesgOut.empty();
					m_qMesgOut.push(mesg);
					if(qOutEmpty)
					{
						WriteHead();
					}
				});
			}
			
			
			void ReadHead()
			{
				asio::async_read(m_socket,asio::buffer(&m_tempMesgIn.header,sizeof(mesg_header<T>)),
				[this](asio::error_code ec,uint32_t Readlenght)
				{
					if(!ec)
					{
						if(m_tempMesgIn.header.size > 0)
						{
							m_tempMesgIn.body.resize(m_tempMesgIn.header.size);
							ReadBody();
						}else
						{
							PushIncomingQMesg();
						}
					}
					else
					{
						std::cout<<"[Connection] Async ReadHead Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void ReadBody()
			{
				asio::async_read(m_socket,asio::buffer(m_tempMesgIn.body.data(),m_tempMesgIn.body.size()),
				[this](asio::error_code ec,uint32_t Readlenght)
				{
					if(!ec)
					{
						PushIncomingQMesg();
					}
					else
					{
						std::cout<<"[Connection] Async ReadBody Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void WriteHead()
			{
				auto tempMesgOut = m_qMesgOut.wait_and_pop();
				if(!tempMesgOut)	return;
				asio::async_write(m_socket,asio::buffer(&tempMesgOut->header,sizeof(mesg_header<T>)),
				[this,tempMesgOut](asio::error_code ec,uint32_t writelenght)
				{
					if(!ec)
					{
						if(tempMesgOut->header.size > 0)
						{
							WriteBody(tempMesgOut);
						}
						
					}
					else
					{
						std::cout<<"[Connection] Async WriteHead Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void WriteBody(std::shared_ptr<message<T>> MesgOut)
			{
				asio::async_write(m_socket,asio::buffer(MesgOut->body.data(),MesgOut->body.size()),
				[this](asio::error_code ec,uint32_t writelenght)
				{
					if(!ec)
					{
						if(!m_qMesgOut.empty())	WriteHead();
					}
					else
					{
						std::cout<<"[Connection] Async WriteBody Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void PushIncomingQMesg()
			{
				if(m_ownerType == owner::server)
				{
					m_qMesgIn.push({this->shared_from_this(),m_tempMesgIn});
				}
				else
				{
					m_qMesgIn.push({nullptr,m_tempMesgIn});
				}
				ReadHead();
			}
			
			void ReadValidation(server_interface<T>*server = nullptr)
			{
				asio::async_read(m_socket,asio::buffer(&m_handshakeIn,sizeof(uint64_t)),
				[this,server](asio::error_code ec, uint64_t lenght){
					if(!ec)
					{
						if(m_ownerType == owner::server)
						{
							if(m_handshakeIn == m_handshakeCheck)
							{
								std::cout<<"["<<GetID()<<"] Cleint Valdated\n";
								server->OnClientValidation(this->shared_from_this());
								ReadHead();
							}
							else
							{
								std::cout<<"["<<GetID()<<"] Handshake check Failed\n";
								m_socket.close();
							}
						}
						else
						{
							//client
							m_handshakeOut = scramble(m_handshakeIn);
							WriteValidation();
						}
					}
					else
					{
						std::cout<<"["<<GetID()<<"] ReadValidation Failed : "<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void WriteValidation()
			{
				asio::async_write(m_socket,asio::buffer(&m_handshakeOut,sizeof(uint64_t)),
				[this](asio::error_code ec, uint64_t lenght)
				{
					if(!ec)
					{
						if(m_ownerType == owner::client)
						{
							ReadHead();
						}
					}
					else
					{
						std::cout<<"["<<GetID()<<"] WriteValidation Failed : "<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			uint64_t scramble(uint64_t input)
			{
				uint64_t out = input;
				return out;
			}
			
		protected:
			asio::ip::tcp::socket m_socket;
			asio::io_context& m_context;
			ThreadSafeQueue<message<T>> m_qMesgOut;
			ThreadSafeQueue<owned_mesg<T>>& m_qMesgIn;
			
			message<T> m_tempMesgIn;
			
			owner m_ownerType;
			uint32_t m_id = 0;
			
			uint64_t m_handshakeIn = 0;
			uint64_t m_handshakeOut = 0;
			uint64_t m_handshakeCheck = 0;
			
		};
	}
}

#endif