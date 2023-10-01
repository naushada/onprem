#ifndef __uniimage__cc__
#define __uniimage__cc__

/**
 * @file uniimage.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-06
 * 
 * @copyright Copyright (c) 2023
 *
 _   _ _ __ (_|_)_ __ ___   __ _  __ _  ___ 
| | | | '_ \| | | '_ ` _ \ / _` |/ _` |/ _ \
| |_| | | | | | | | | | | | (_| | (_| |  __/
 \__,_|_| |_|_|_|_| |_| |_|\__,_|\__, |\___|
                                 |___/     
 */

#include "uniimage.hpp"
#include "http.hpp"

using json = nlohmann::json;

/**
 * @brief 
 * 
 * @return std::int32_t 
 */
std::int32_t noor::Uniimage::init() {
    m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);

    if(m_epollFd < 0) {
        // Creation of epoll instance is failed.
        if(EMFILE == errno) {
            std::cout << "line: " << __LINE__ << " Unable to create epoll instance errno: EMFILE open file descriptor limits for a process is reached" << std::endl;
        } else if(ENFILE == errno) {
            std::cout << "line: " << __LINE__ << " Unable to create epoll instance errno: ENFILE open files limit is reached" << std::endl;

        } else if(ENOMEM == errno) {
            std::cout << "line: " << __LINE__ << " Unable to create epoll instance errno: ENOMEM no memory to create kernel object" << std::endl;
        } else {
            std::cout << "line: " << __LINE__ << " epoll_create1 is failed" << std::endl;
        }
    }
    return(m_epollFd);
}

std::int32_t noor::Uniimage::CreateServiceAndRegisterToEPoll(noor::ServiceType serviceType, const std::string& IP, const std::uint16_t& PORT, 
                                                             bool isAsync,std::int32_t channel) {
    do {
        switch(serviceType) {
            //Add here any new client
            case noor::ServiceType::Tcp_Client_Service_Sync:
            case noor::ServiceType::Tcp_Client_Service_Async:
            case noor::ServiceType::Tls_Tcp_Client_Service_Sync:
            case noor::ServiceType::Tls_Tcp_Client_Service_Async:
            case noor::ServiceType::Tls_Tcp_Restclient_Service_Sync:
            case noor::ServiceType::Tls_Tcp_Restclient_Service_Async:
            case noor::ServiceType::Tls_Tcp_Client_Connected_Service:
            case noor::ServiceType::Tcp_Client_Connected_Service:
            case noor::ServiceType::Tcp_Web_Client_Connected_Service:
            case noor::ServiceType::Tls_Tcp_Geolocation_Service_Sync:
            case noor::ServiceType::Tls_Tcp_Geolocation_Service_Async:
            {
                auto inst = std::make_unique<TcpClient>(IP, PORT, channel, isAsync);
                m_services.insert(std::make_pair(serviceType, std::move(inst)));
                RegisterToEPoll(serviceType, channel);
            }
            break;

            default:
            {
                auto inst = std::make_unique<TcpServer>(IP, PORT, channel);
                m_services.insert(std::make_pair(serviceType, std::move(inst)));
                RegisterToEPoll(serviceType, channel);
            }
            break;
        }
    }while(0);

    return(0);
}

/**
 * @brief 
 * 
 * @param in 
 * @return std::int32_t 
 */
std::int32_t noor::Uniimage::stop(std::int32_t in) {
    return(0);
}

/**
 * @brief 
 * 
 * @param toInMilliSeconds 
 * @return std::int32_t 
 */
std::int32_t noor::Uniimage::start(std::int32_t toInMilliSeconds) {

    if(m_evts.empty()) {
        std::cout << "line: " << __LINE__ << " event list is empty, please create the events to monitor" << std::endl;
        return(-1);
    }

    if(toInMilliSeconds < 0) {
        std::cout << "line: " << __LINE__ << " timeout value is -ve hence resetting it to zero" << std::endl;
        toInMilliSeconds = -1;
    }

    std::vector<struct epoll_event> activeEvt(m_services.size());
    std::int32_t nReady = -1;

    while(true) {
        
        activeEvt.resize(m_evts.size());
        nReady = ::epoll_wait(m_epollFd, activeEvt.data(), activeEvt.size(), toInMilliSeconds);

        //Upon timeout nReady is ZERO and -1 Upon Failure.
        if(nReady > 0) {
            activeEvt.resize(nReady);
            std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " nReady: " << nReady << " m_services.size(): " << m_services.size() << std::endl;

        } else if(nReady < 0) {
            //Error is returned by epoll_wait
            if(EBADF == errno) {
                std::cout << "line: " << __LINE__ << " epfd is not a valid file descriptor" << std::endl;
            } else if(EFAULT == errno) {
                std::cout << "line: " << __LINE__ << " The memory area pointed to by events is not accessible with write permissions" << std::endl;
            } else if(EINTR == errno) {
                std::cout << "line: " << __LINE__ << " The call was interrupted by a signal handler before" << std::endl;
            } else if(EINVAL == errno) {
                std::cout << "line: " << __LINE__ << " epfd is not an epoll file descriptor, or maxevents is less than or equal to zero" << std::endl;
            }
            std::cout << "line: " << __LINE__ << "nReady: " << nReady << std::endl;
            continue;
        } else if(!nReady) {
            //Timeout happens -- for client role only.
            if(get_config()["role"].compare("server")) {
                if(!get_config()["protocol"].compare(0, 3, "tcp")) {
                    auto svc = GetService(noor::ServiceType::Tcp_Client_Service_Async);
                    if(svc != nullptr) {
                        auto channel = svc->handle();
                        if(noor::client_connection::Disconnected == svc->connected_client(channel)) {
                            auto IP = svc->ip();
                            auto PORT = svc->port();
                            DeRegisterFromEPoll(channel);
                            DeleteService(noor::ServiceType::Tcp_Client_Service_Async, channel);
                            CreateServiceAndRegisterToEPoll(noor::ServiceType::Tcp_Client_Service_Async, IP, PORT, true);
                        } else if(noor::client_connection::Disconnected == svc->connected_client(channel)) {
                            //Get the events from Rest Server.
                            if(!getResponseCache().empty()) {
                                auto len = svc->tcp_tx(channel, getResponseCache().begin()->second);
                                if(len > 0) {
                                    std::cout << "line: " << __LINE__ << " sent to Server over TCP len: " << len << std::endl;
                                    std::cout << "line: " << __LINE__ << " sent to Server over TCP : " << getResponseCache().begin()->second << std::endl;
                                }
                            }
                        }
                    }
                } else if(!get_config()["protocol"].compare(0, 3, "tls")) {
                    auto svc = GetService(noor::ServiceType::Tls_Tcp_Client_Service_Async);
                    if(svc != nullptr) {
                        auto channel = svc->handle();
                        if(noor::client_connection::Disconnected == svc->connected_client(channel)) {
                            auto IP = svc->ip();
                            auto PORT = svc->port();
                            DeRegisterFromEPoll(channel);
                            DeleteService(noor::ServiceType::Tls_Tcp_Client_Service_Async, channel);
                            CreateServiceAndRegisterToEPoll(noor::ServiceType::Tls_Tcp_Client_Service_Async, IP, PORT, true);
                        } else if(noor::client_connection::Disconnected == svc->connected_client(channel)) {
                            //Get the events from Rest Server.
                            if(!getResponseCache().empty()) {
                                std::int32_t req_len = getResponseCache().begin()->second.length();
                                auto payload_len = htonl(req_len);
                                std::stringstream data("");
                                data.write (reinterpret_cast <char *>(&payload_len), sizeof(std::int32_t));
                                data << getResponseCache().begin()->second;
                                std::cout << "line: " << __LINE__ << " sending to Server over TLS : " << data.str() << std::endl;
                                auto len = svc->tls().write(data.str());
                                if(len > 0) {
                                    std::cout << "line: " << __LINE__ << " sent to Server over TLS len: " << len << std::endl;
                                    std::cout << "line: " << __LINE__ << " sent to Server over TLS : " << getResponseCache().begin()->second << std::endl;
                                }
                            }
                        }
                    }
                } else if(!get_config()["protocol"].compare(0, 4, "dtls")) {

                }
            }
            continue;
        }

        for(auto it = activeEvt.begin(); it != activeEvt.end(); ++it) {
            auto ent = *it;
            std::uint32_t Fd = std::uint32_t((ent.data.u64 >> 32) & 0xFFFFFFFF);
            noor::ServiceType serviceType = noor::ServiceType(ent.data.u64 & 0xFFFFFFFF);

            if(ent.events & EPOLLOUT) {

                switch(serviceType) {
                    case noor::ServiceType::Tcp_Client_Service_Async:
                    {
                        do {
                            // check that there's no error for socket.
                            std::int32_t optval = -1;
                            socklen_t optlen = sizeof(optval);
                            if(!getsockopt(Fd, SOL_SOCKET, SO_ERROR, &optval, &optlen)) {
                                struct sockaddr_in peer;
                                socklen_t sock_len = sizeof(peer);
                                memset(&peer, 0, sizeof(peer));

                                auto ret = getpeername(Fd, (struct sockaddr *)&peer, &sock_len);
                                if(ret < 0 && errno == ENOTCONN) {
                                    //re-attemp connection now.
                                    std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " re-attempting the connection " << std::endl;
                                    auto inst = GetService(serviceType);
                                    if(inst == nullptr) break;
                                    auto IP = inst->ip();
                                    auto PORT = inst->port();
                                    DeRegisterFromEPoll(Fd);
                                    DeleteService(serviceType, Fd);
                                    CreateServiceAndRegisterToEPoll(serviceType, IP, PORT, true);
                                    break;
                                }
                            }

                            //There's no error on the socket
                            //struct epoll_event evt;
                            ent.events = EPOLLIN | EPOLLRDHUP;
                            auto svc = GetService(serviceType);
                            if(svc == nullptr) break;

                            svc->connected_client(noor::client_connection::Connected);
                            auto ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, Fd, &ent);
                            (void)ret;
                            
                            if(!getResponseCache().empty()) {
                                auto len = svc->tcp_tx(Fd, getResponseCache().begin()->second);
                                if(len > 0) {
                                    std::cout << "line: " << __LINE__ << " sent to Server over TCP len: " << len << std::endl;
                                    std::cout << "line: " << __LINE__ << " sent to Server over TCP : " << getResponseCache().begin()->second << std::endl;
                                }
                                break;
                            }
                        } while(0);
                    }
                    break;

                    case noor::ServiceType::Tls_Tcp_Client_Service_Async:
                    {
                        do {
                            // check that there's no error for socket.
                            std::int32_t optval = -1;
                            socklen_t optlen = sizeof(optval);
                            if(!getsockopt(Fd, SOL_SOCKET, SO_ERROR, &optval, &optlen)) {
                                struct sockaddr_in peer;
                                socklen_t sock_len = sizeof(peer);
                                memset(&peer, 0, sizeof(peer));

                                auto ret = getpeername(Fd, (struct sockaddr *)&peer, &sock_len);
                                if(ret < 0 && errno == ENOTCONN) {
                                    //re-attemp connection now.
                                    std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " re-attempting the connection " << std::endl;
                                    auto inst = GetService(serviceType);
                                    if(inst == nullptr) break;
                                    auto IP = inst->ip();
                                    auto PORT = inst->port();
                                    DeRegisterFromEPoll(Fd);
                                    DeleteService(serviceType, Fd);
                                    CreateServiceAndRegisterToEPoll(serviceType, IP, PORT, true);
                                    break;
                                }
                            }
                            std::cout << "line: " << __LINE__ << " Tls Tcp client is connected to DMS successfully " << std::endl;
                            //There's no error on the socket
                            //struct epoll_event evt;
                            ent.events = EPOLLIN |EPOLLRDHUP;
                            auto ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, Fd, &ent);
                            (void)ret;
                            auto svc = GetService(serviceType);
                            if(svc == nullptr) break;
                            svc->connected_client(noor::client_connection::Connected);
                            //do a TLS Hand shake
                            svc->tls().init(Fd);
                            svc->tls().client();

                            if(!getResponseCache().empty()) {
                                std::int32_t req_len = getResponseCache().begin()->second.length();
                                //Encode First Payload len
                                auto payload_len = htonl(req_len);
                                std::stringstream data("");
                                data.write (reinterpret_cast <char *>(&payload_len), sizeof(std::int32_t));
                                data << getResponseCache().begin()->second;
                                auto len = svc->tls().write(data.str());

                                if(len > 0) {
                                    std::cout << "line: " << __LINE__ << " sent to Device Mgmt Server over TLS len: " << len << std::endl;
                                    break;
                                }
                                std::cout << "line: " << __LINE__ << " Failed to sent to Device Mgmt Server over TLS len: " << len << std::endl;
                            }
                        } while(0);
                    }
                    break;
                    case noor::ServiceType::Tls_Tcp_Restclient_Service_Async:
                    case noor::ServiceType::Tls_Tcp_Restclient_Service_Sync:
                    {
                        //tcp connection is established - do tls handshake
                        auto svc = GetService(serviceType);
                        if(svc == nullptr) break;

                        svc->tls().init(Fd);
                        svc->tls().client();

                        ent.events = EPOLLIN |EPOLLRDHUP;
                        auto ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, Fd, &ent);
                        (void)ret;

                        //Get Token for Rest Client
                        json Value = json::object();
                        Value["login"] = get_config()["userid"];
                        Value["password"] = get_config()["password"];

                        auto req = svc->restC().getToken(Value.dump());
                        std::cout << "line: " << __LINE__ << " request sent: " << std::endl << req << std::endl;
                        auto len = svc->tls().write(req);
                        (void)len;
                    }
                    break;
                    default:
                    {
                        std::cout << "line: " << __LINE__ << " default case for EPOLLOUT" << std::endl;
                    }
                    break;
                }
            } else if(ent.events & EPOLLIN) {

                //file descriptor is ready for read.
                switch(serviceType) {
                    case noor::ServiceType::Tcp_Web_Server_Service:
                    {
                        //New Web Connection
                        std::int32_t newFd = -1;
                        struct sockaddr_in addr;
                        socklen_t addr_len = sizeof(addr);
                        newFd = ::accept(Fd, (struct sockaddr *)&addr, &addr_len);

                        if(newFd > 0) {
                            std::uint16_t PORT = ntohs(addr.sin_port);
                            std::string IP(inet_ntoa(addr.sin_addr));
                            std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " new client connection from IP: " << IP << " PORT: " << PORT << std::endl;
                            /* tcp_peek on non-blocking socket will return zero, which is causing the socket tobe closed.
                             */
                            m_services.insert(std::make_pair(noor::ServiceType::Tcp_Web_Client_Connected_Service , std::make_unique<TcpClient>(newFd, IP, PORT)));
                            RegisterToEPoll(noor::ServiceType::Tcp_Web_Client_Connected_Service, newFd);
                        }
                    }
                    break;
                    case noor::ServiceType::Tcp_Server_Service:
                    {
                        std::int32_t newFd = -1;
                        struct sockaddr_in addr;
                        socklen_t addr_len = sizeof(addr);
                        newFd = ::accept(Fd, (struct sockaddr *)&addr, &addr_len);
                        
                        // new connection is accepted successfully.

                        if(newFd > 0) {
                            std::uint16_t PORT = ntohs(addr.sin_port);
                            std::string IP(inet_ntoa(addr.sin_addr));
                            std::cout<< "line: " << __LINE__ << " new client from device IP: " << IP <<" PORT: " << PORT << " FD: " << newFd << std::endl;
                            m_services.insert(std::make_pair(noor::ServiceType::Tcp_Client_Connected_Service , std::make_unique<TcpClient>(newFd, IP, PORT)));
                            RegisterToEPoll(noor::ServiceType::Tcp_Client_Connected_Service, newFd);
                        }
                    }
                    break;
                    case noor::ServiceType::Tls_Tcp_Server_Service:
                    {
                        std::int32_t newFd = -1;
                        struct sockaddr_in addr;
                        socklen_t addr_len = sizeof(addr);
                        newFd = ::accept(Fd, (struct sockaddr *)&addr, &addr_len);
                        std::cout << "line: " << __LINE__ << " value of newFd: " << newFd << std::endl;
                        // new connection is accepted successfully.

                        if(newFd > 0) {
                            std::uint16_t PORT = ntohs(addr.sin_port);
                            std::string IP(inet_ntoa(addr.sin_addr));
                            std::cout<< "line: " << __LINE__ << " new client for TLS server IP: " << IP <<" PORT: " << PORT << " FD: " << newFd << std::endl;
                        #if 0
                            //making socket non-blocking
                            auto flags = ::fcntl(newFd, F_GETFL);
                            if(::fcntl(newFd, F_SETFL, flags | O_NONBLOCK) < 0) {
                                std::cout << __TIMESTAMP__ << ": line: " << __LINE__ << " making socker non-blocking for fd: " << newFd << " failed" << std::endl;
                            }
                        #endif
                            m_services.insert(std::make_pair(noor::ServiceType::Tls_Tcp_Client_Connected_Service ,
                                              std::make_unique<TcpClient>(newFd, IP, PORT)));
                            auto svc = GetService(noor::ServiceType::Tls_Tcp_Client_Connected_Service, newFd);
                            if(svc == nullptr) break;

                            std::string cert("../cert/cert.pem"), pkey("../cert/pkey.pem");
                            svc->tls().init(cert, pkey);
                            if(svc->tls().server(newFd) < 0) {
                                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " SSL_accepts failed" << std::endl;
                                ERR_print_errors_fp(stderr);
                                DeleteService(noor::ServiceType::Tls_Tcp_Client_Connected_Service, newFd);
                                break;
                            }

                            RegisterToEPoll(noor::ServiceType::Tls_Tcp_Client_Connected_Service, newFd);
                        }
                    }
                    break;
                    case noor::ServiceType::Tcp_Client_Connected_Service:
                    {
                        do {
                            //Data is availabe for read. --- tcp_rx()
                            std::string request("");
                            auto svc = GetService(serviceType, Fd);
                            if(svc == nullptr) break;

                            auto result = svc->tcp_rx(Fd, request);
                            std::cout << "line: " << __LINE__ << " result: " << result << " connected client " << std::endl;

                            if(!result) {
                                auto serialnumber = get_serialnumber(Fd);
                                std::cout << "line: " << __LINE__ << " serialnumber: " << serialnumber << std::endl;
                                if(serialnumber.length()) {
                                    
                                    auto collection = "device";
                                    auto filter = json::object();
                                    filter["serialnumber"] = serialnumber;
                                    auto it = getResponseCache().find(serialnumber);
                                    if(it != getResponseCache().end()) {
                                        auto &dbinst = GetService(noor::ServiceType::Tcp_Web_Server_Service)->dbinst();
                                        auto result = it->second;
                                        auto Value = json::parse(result);
                                        Value["status"] = "offline";
                                        auto query = json::object();
                                        query["$set"] = Value;
                                        std::cout << "line: " << __LINE__ << " query: " << query << std::endl;
                                        dbinst.update_collectionEx(collection, filter.dump(), query.dump());
                                    }
                                    channeltoserialmap().erase(Fd);
                                    std::cout << "line: " << __LINE__ << " channeltoserialmap().size: " << channeltoserialmap().size() << std::endl;
                                }

                                //TCP Connection is closed.
                                std::cout << "line: " << __LINE__ << " closing the client connection for service: " << serviceType << std::endl;
                                DeRegisterFromEPoll(Fd);
                                DeleteService(serviceType, Fd);
                                break;
                            }
                            Http http(request);

                            if(!http.uri().compare(0, 27, "/api/v1/dms/device/register")) {
                                //Upon First connection of device to DMS.
                                json Value = json::parse(http.body());
                                //Write into DB now.
                                auto serialnumber = Value["serialnumber"].get<std::string>();
                                Value["status"]  = "online";
                                time_t now = time(0);
                                std::string strTime(ctime(&now));
                                //remember the Fd for this serialnumber.
                                channeltoserialmap(Fd, serialnumber);
                                Value["lastcommunicationdate"] = strTime.erase(strTime.find('\n', 0), 1);
                                auto &dbinst = GetService(noor::ServiceType::Tcp_Web_Server_Service)->dbinst();
                                auto collection = "device";
                                auto filter = json::object();
                                filter["serialnumber"] = serialnumber;
                                auto projection = json::object();
                                projection["_id"] = false;

                                std::cout << "line: " << __LINE__ << " filter: " << filter << std::endl;
                                std::cout << "line: " << __LINE__ << " payload: " << Value.dump() << std::endl;
                                auto response = dbinst.get_documentEx(collection, filter.dump(), projection.dump());

                                if(!response.length()) {
                                    std::cout << "line: " << __LINE__ << " creating a document" << std::endl;
                                    dbinst.create_documentEx(collection, Value.dump());
                                    break;
                                }

                                auto query = json::object();
                                query["$set"] = Value;
                                std::cout << "line: " << __LINE__ << " updatign a document query:" << query << std::endl;
                                dbinst.update_collectionEx(collection, filter.dump(), query.dump());
                                break;

                            } else {
                                //Find the connection for web_client_connected_service's channel number
                            }
                            
                            

                        }while(0);
                    }
                    break;
                    case noor::ServiceType::Tls_Tcp_Client_Connected_Service: //The Tls client on device is connectedto DMS  
                    {
                        do {
                            //Data is availabe for read. --- tls read()
                            auto svc = GetService(serviceType, Fd);
                            if(svc == nullptr) break;

                            std::string request("");
                            std::int32_t payload_len = 0;
                            std::int32_t len = -1;
                            // Read first 4 Bytes Header which is of length of payload
                            len = svc->tls().read(request, 4);

                            std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " len: " << len << std::endl;
                            std::cout << "line: " << __LINE__ << " while doing peek errno: "<< std::strerror(errno) << std::endl;

                            if(len > 0 && len == 4) {
                                std::istringstream istrstr;
                                istrstr.rdbuf()->pubsetbuf(request.data(), len);
                                istrstr.read(reinterpret_cast<char *>(&payload_len), sizeof(payload_len));
                                payload_len = ntohl(payload_len);
                                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " payload_len: " << payload_len << std::endl;
                            } else if(!len) {
                                //connection is closed.
                                std::cout << "line: " << __LINE__ << " closing the client connection for serviceType: " << serviceType << " len: " << len << std::endl;
                                DeRegisterFromEPoll(Fd);
                                DeleteService(serviceType, Fd);
                                
                                break;
                            }

                            if(!payload_len || payload_len < 0) {
                                std::cout << "line: " << __LINE__ << " closing the client connection for serviceType: " << serviceType << " len: " << len << std::endl;                                
                                DeRegisterFromEPoll(Fd);
                                DeleteService(serviceType, Fd);
                                break;
                            } else {
                                len = svc->tls().read(request, payload_len);
                                if(len < 0 || !len) {
                                    std::cout << "line: " << __LINE__ << " closing the client connection for serviceType: " << serviceType << " len: " << len << std::endl;
                                    DeRegisterFromEPoll(Fd);
                                    DeleteService(serviceType, Fd);
                                    break;
                                } else if(len > 0) {
                                    json jobj = json::parse(request);
                                    break;
                                }
                            }
                        }while(0);
                    }
                    break;
                    case noor::ServiceType::Tcp_Web_Client_Connected_Service:
                    {
                        do {

                            std::string request("");
                            auto svc = GetService(serviceType, Fd);
                            if(svc == nullptr) break;

                            auto result = svc->web_rx(Fd, request);

                            if(!result) {
                                //connection is closed
                                std::cout << "line: " << __LINE__ << " closing the client connection for serviceType: " << serviceType << std::endl;
                                DeRegisterFromEPoll(Fd);
                                DeleteService(serviceType, Fd);
                                break;
                            }

                            // Http http(request);
                            //if(!http.uri().compare(0, 11, "/api/v1/dms")) {
                                //Handle locally.
                            std::cout << "line: " << __LINE__  << " total length: " << result << std::endl;
                            auto rsp = svc->process_web_request(request, GetService(noor::ServiceType::Tcp_Web_Server_Service)->dbinst());
                            if(rsp.length()) {
                                auto ret = svc->tcp_tx(Fd, rsp);
                                break;
                            }
                            //} else if(!http.uri().compare(0, 14, "/api/v1/device")) {
                                //Pass to device.
                            //}
                            
                        }while(0);
                    }
                    break;
                    case noor::ServiceType::Tcp_Client_Service_Sync:
                    {
                        do {
                            //Data is availabe for read. --- tcp_rx()
                            std::string request("");
                            auto svc = GetService(serviceType);
                            if(svc == nullptr) break;

                            auto result = svc->tcp_rx(Fd, request);
                            if(!result) {
                                std::cout << "line: " << __LINE__ << " closing the connection for Service: " << serviceType << std::endl;
                                auto IP = svc->ip();
                                auto PORT = svc->port();
                                DeleteService(serviceType);
                                DeRegisterFromEPoll(Fd);
                                CreateServiceAndRegisterToEPoll(serviceType, IP, PORT,true);
                                break;
                            }
                        }while(0);
                    }
                    break;
                    case noor::ServiceType::Tcp_Client_Service_Async:
                    {
                        do {
                            //Data is availabe for read. --- tcp_rx()
                            std::string request("");
                            auto svc = GetService(serviceType);
                            if(svc == nullptr) break;

                            auto result = svc->tcp_rx(Fd, request);

                            if(!result) {
                                std::cout << "line: " << __LINE__ << " closing the connection for Service: " << serviceType << std::endl;
                                auto IP = svc->ip();
                                auto PORT = svc->port();
                                DeleteService(serviceType);
                                DeRegisterFromEPoll(Fd);
                                CreateServiceAndRegisterToEPoll(serviceType, IP, PORT,true);
                                break;
                            }
                            std::cout << "line: " << __LINE__ << " serviceType: " << serviceType << " received from DMS: " << request << std::endl;
                            {
                                //Pass on over TLS to Device
                                auto svc = GetService(noor::ServiceType::Tls_Tcp_Restclient_Service_Sync);
                                if(svc == nullptr) break;

                                svc->tls().write(request);
                            }
                            
                            break;
                        }while(0);
                    }
                    break;

                    case noor::ServiceType::Tls_Tcp_Restclient_Service_Sync:
                    {
                        auto svc = GetService(serviceType);
                        if(svc == nullptr) break;

                        std::string out;
                        std::size_t header_len = -1;
                        std::size_t payload_len = -1;
                        bool is_conn_closed = false;
                        std::string status;
                        do {

                            auto ret = svc->tls().peek(out);
                            if(ret > 0) {

                                Http http(out);
                                status.assign(http.status_code());
                                auto ct = http.value("Content-Length");
                                header_len = http.get_header(out).length();

                                if(ct.length() > 0) {
                                    //Content-Length is present
                                    payload_len = std::stoi(ct);
                                    std::cout << "line: " << __LINE__ << " value of content-length: " << std::stoi(ct) << std::endl;
                                }

                            } else if(!ret) {
                                //TLS connection is closed
                                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " tls connection is closed" << std::endl;
                                is_conn_closed = true;
                                break;
                            }
                        }while(header_len != out.length());

                        do {
                            if(is_conn_closed) {
                                //Peer is closed now 
                                DeRegisterFromEPoll(Fd);
                                DeleteService(serviceType);
                                break;
                            }

                            //Read HTTP Header first.
                            auto ret = svc->tls().read(out, header_len);
                            if(ret < 0) {
                                break;
                            }
                            //HTTP Body
                            if(payload_len < 0) {
                                break;
                            }

                            std::string body;
                            body.clear();
                            ret = svc->tls().read(body, payload_len);

                            //Process Response Now.
                            if(ret < 0) {
                                break;
                            }

                            if(!svc->restC().status_code().compare(0, 3, "401") || 
                               !svc->restC().status_code().compare(0, 3, "403")) {
                                //missing authentication or invalid authentication
                                //Get Token for Rest Client
                                json Value = json::object();
                                Value["login"] = get_config()["userid"];
                                Value["password"] = get_config()["password"];

                                auto req = svc->restC().getToken(Value.dump());
                                std::cout << "line: " << __LINE__ << " request sent: " << std::endl << req << std::endl;
                                auto len = svc->tls().write(req);
                                (void)len;
                                break;
                            }

                            std::string result("");
                            auto req = svc->restC().processResponse(out, body, result);
                            if(result.length()) {
                                //push result to remote server
                                json Value = json::parse(result);
                                
                                if(Value["serialnumber"] != nullptr) {
                                    auto srNumber = Value["serialnumber"].get<std::string>();
                                    //The key is the serial number
                                    getResponseCache().insert(std::pair(srNumber, result));
                                    std::cout << "line: " << __LINE__ << " serialNumber: " << srNumber << std::endl;
                                }
                                {
                                    auto svc = GetService(noor::ServiceType::Tcp_Client_Service_Async);
                                    if(svc == nullptr) break;

                                    auto channel = svc->handle();
                                    if(channel > 0 && noor::client_connection::Connected == svc->connected_client(channel)) {
                                        //Add HTTP Header to it.
                                        std::stringstream ss;
                                        ss << "POST /api/v1/dms/device/register HTTP/1.1\r\n"
                                           << "Connection: keep-aalive\r\n"
                                           << "Content-Length: " << result.length() << "\r\n"
                                           << "\r\n"
                                           << result;

                                        auto len = svc->tcp_tx(channel, ss.str());
                                        if(len > 0) {
                                            std::cout << "line: " << __LINE__ << " sent to TCP Server len: " << len << " for serviceType: " << noor::ServiceType::Tcp_Client_Service_Async << " result: " << result << std::endl; 
                                        }
                                    }
                                    break;
                                }
                            } else if(req.length()) {
                                ret = svc->tls().write(req);
                                if(ret < 0) {
                                    std::cout << "line: " << __LINE__ << " ssl write failed for serviceTYpe: " << serviceType << std::endl;
                                    break;
                                }
                                std::cout << "line: " << __LINE__ << " request sent to : " << std::endl << req << std::endl;
                            }

                        }while(0);
                    }
                    break;
                    case noor::ServiceType::Tls_Tcp_Client_Service_Async:
                    {
                        do {
                            //Data is availabe for read. --- tls read()
                            std::string request("");
                            auto svc = GetService(serviceType);
                            if(svc == nullptr) break;

                            auto result = svc->tls().read(request, 4);

                            if(!result || result < 0) {
                                std::cout << "line: " << __LINE__ << " closing the connection for Service: " << serviceType << " result: " << result << std::endl;
                                auto IP = svc->ip();
                                auto PORT = svc->port();
                                DeleteService(serviceType);
                                DeRegisterFromEPoll(Fd);
                                CreateServiceAndRegisterToEPoll(serviceType, IP, PORT,true);
                                break;
                            }

                            std::int32_t payload_len = -1;
                            std::istringstream istrstr;
                            istrstr.rdbuf()->pubsetbuf(request.data(), result);
                            istrstr.read(reinterpret_cast<char *>(&payload_len), sizeof(payload_len));
                            payload_len = ntohl(payload_len);
                            std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " payload_len: " << payload_len << std::endl;
                            if(payload_len > 0) {
                                result = svc->tls().read(request, payload_len);
                                if(!result || result < 0) {
                                    auto IP = svc->ip();
                                    auto PORT = svc->port();
                                    DeleteService(serviceType);
                                    DeRegisterFromEPoll(Fd);
                                    CreateServiceAndRegisterToEPoll(serviceType, IP, PORT,true);
                                    break;
                                }
                                std::cout << "line: " << __LINE__ << " serviceType: " << serviceType << " received from DMS: " << payload_len << std::endl;
                                {
                                    //Pass on over TLS to Device
                                    auto svc = GetService(noor::ServiceType::Tls_Tcp_Restclient_Service_Sync);
                                    if(svc == nullptr) break;
                                    svc->tls().write(request);
                                }
                            }
                            
                            break;
                        }while(0);
                    }
                    break;

                    default:
                    {
                        std::cout << "line: " << __LINE__ << " EPOLLIN default case" << std::endl;
                        break;

                    }
                }
                
            } else if(ent.events & EPOLLRDHUP) {
                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " events: " << ent.events << std::endl;
                //Connection is closed by other end
                switch(serviceType) {
                    //The Client of DeviceMgmtServer is closed/disconnected.
                    case noor::ServiceType::Tcp_Client_Service_Async:
                    case noor::ServiceType::Tcp_Client_Service_Sync:
                    case noor::ServiceType::Tls_Tcp_Client_Service_Async:
                    case noor::ServiceType::Tls_Tcp_Restclient_Service_Async:
                    case noor::ServiceType::Tls_Tcp_Restclient_Service_Sync:
                    {
                        //start attempting the connection...
                        auto inst = GetService(serviceType, Fd);
                        if(inst == nullptr) break;
                        
                        auto IP = inst->ip();
                        auto PORT = inst->port();
                        std::cout << "line: " << __LINE__ << " closing connection for service: " << serviceType << " Fd: " << Fd << std::endl;
                        DeleteService(serviceType);
                        DeRegisterFromEPoll(Fd);
                        CreateServiceAndRegisterToEPoll(serviceType, IP, PORT, true);
                    }
                    break;
                    default:
                    {
                        //Connection is closed.
                        std::cout << "line: " << __LINE__ << " default case: connection is closed for service: " << serviceType << std::endl;
                        DeleteService(serviceType, Fd);
                        DeRegisterFromEPoll(Fd);
                    }
                }
            } else if(ent.events & EPOLLERR) {
                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " events: " << ent.events << std::endl;
                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " epollerr events: " << ent.events  << " Fd:" << Fd << " serviceType: " << serviceType << std::endl;
            } else if(ent.events & EPOLLONESHOT) {
                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " events: " << ent.events << std::endl;
                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " oneshots events: " << ent.events << " Fd:" << Fd << " serviceType: " << serviceType << std::endl;
            } else {
                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " events: " << ent.events << std::endl;
                std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " unhandled events: " << ent.events << " Fd:" << Fd << " serviceType: " << serviceType << std::endl;
            }
        }
    }
    return(0);
}

/**
 * @brief 
 * 
 * @param fd 
 * @return std::int32_t 
 */
std::int32_t noor::Uniimage::DeRegisterFromEPoll(std::int32_t fd) {
    noor::ServiceType serviceType;
    auto it = std::find_if(m_evts.begin(), m_evts.end(), [&](const auto& ent) ->bool {
        auto evtFd = std::int32_t((ent.data.u64 >> 32) & 0xFFFFFFFF);
        serviceType = noor::ServiceType(ent.data.u64 & 0xFFFFFFFF);
        return(evtFd == fd);
    });

    if(it != m_evts.end()) {
        if(::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1)
        {
            std::cout << "line: " << __LINE__ << " Failed to delete Fd from epoll instance for fd: " << fd << std::endl;
        }

        ::close(fd);
        m_evts.erase(it);
        std::cout << "line: " << __LINE__ << " closed the Fd: " << fd << " m_evts.size(): " <<m_evts.size()  <<std::endl;
        return(0);
        
    }

    return(-1);
}

/**
 * @brief 
 * 
 * @param serviceType 
 * @return std::int32_t 
 */
std::int32_t noor::Uniimage::RegisterToEPoll(noor::ServiceType serviceType, std::int32_t channel) {
    
    struct epoll_event evt;

    std::cout << "line: " << __LINE__ << " handle: " << channel << " serviceType: " << serviceType  << " added to epoll" << std::endl;
    std::uint64_t dd = std::uint64_t(channel);
    evt.data.u64 = std::uint64_t(dd) << 32 | std::uint64_t(serviceType);

    if((serviceType == noor::ServiceType::Tcp_Client_Service_Async) ||
       (serviceType == noor::ServiceType::Tcp_Client_Service_Sync) ||
       (serviceType == noor::ServiceType::Tls_Tcp_Restclient_Service_Sync) ||
       (serviceType == noor::ServiceType::Tls_Tcp_Restclient_Service_Async) ||
       (serviceType == noor::ServiceType::Tls_Tcp_Client_Service_Async) ||
       (serviceType == noor::ServiceType::Tls_Tcp_Client_Service_Sync)) {
        evt.events = EPOLLOUT|EPOLLRDHUP;
        std::cout << "line: " << __LINE__ << " value of events: " << evt.events << " serviceType: " << serviceType << std::endl;

    } else {
        evt.events = EPOLLIN | EPOLLRDHUP;
        std::cout << "line: " << __LINE__ << " value of events: " << evt.events << " serviceType: " << serviceType << std::endl;
    }

    if(::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, channel, &evt) == -1)
    {
        std::cout << "line: " << __LINE__ << " Failed to add Fd to epoll instance for serviceType: " << serviceType << " Terminating the process"<< std::endl;
        exit(2);
    }

    m_evts.push_back(evt);
    return(0);
}

/**
 * @brief 
 * 
 * @param serviceType 
 * @return std::unique_ptr<noor::Service>& 
 */
noor::Service* noor::Uniimage::GetService(noor::ServiceType serviceType) {
    auto it = m_services.find(serviceType);
    //return(it->second);
    
    if(it != m_services.end()) {
        return(it->second.get());
    }
    
    return(nullptr);
}

noor::Service* noor::Uniimage::GetService(noor::ServiceType serviceType, const std::int32_t& channel) {
    auto it = m_services.equal_range(serviceType);
    for(auto &ent = it.first; ent != it.second; ++ent) {
        if(channel > 0 && channel == ent->second->handle()) {
            return(ent->second.get());
        }
    }
    return(nullptr);
}

void noor::Uniimage::DeleteService(noor::ServiceType serviceType, const std::int32_t& channel) {
    auto it = m_services.equal_range(serviceType);
    if(it.first != m_services.end()) {
        for(auto &ent = it.first; ent != it.second; ++ent) {
            if(channel > 0 && channel == ent->second->handle()) {
                //Erasing element from multimap now.
                m_services.erase(ent);
                std::cout << "line: " << __LINE__ << " m_services.size(): " << m_services.size() << std::endl;
                break;
            }
        }
    }
}

void noor::Uniimage::DeleteService(noor::ServiceType serviceType) {
    auto it = m_services.find(serviceType);
    //Erasing element from multimap now.
    if(it != m_services.end()) {
        it = m_services.erase(it);
    }
    std::cout << "line: " << __LINE__ << " m_services.size(): " << m_services.size() << std::endl;
}

/******************************************************************************
 _ __ ___  ___| |_ ___| (_) ___ _ __ | |_ 
| '__/ _ \/ __| __/ __| | |/ _ \ '_ \| __|
| | |  __/\__ \ || (__| | |  __/ | | | |_ 
|_|  \___||___/\__\___|_|_|\___|_| |_|\__|
                                          
*********************************************************************************/
std::string noor::RestClient::getToken(const std::string& in) {
    std::string host("192.168.1.1:443");
    std::stringstream ss("");
    m_uri.assign("/api/v1/auth/tokens");

    ss << "POST " << m_uri <<" HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Content-Type: application/vnd.api+json\r\n"
        << "Connection: keep-alive\r\n"
        << "Accept: application/vnd.api+json\r\n"
        << "Content-Length: " << in.length() << "\r\n"
        << "\r\n"
        << in;

    return(ss.str());
}

/**
 * @brief 
 * 
 * @param in 
 * @param user 
 * @return std::string 
 */
std::string noor::RestClient::authorizeToken(const std::string& in, const std::string& user) {
    std::string host("192.168.1.1:443");
    std::stringstream ss("");
    m_uri.assign("/api/v1/auth/authorization/");
    m_uri += user;

    ss << "GET " << m_uri <<" HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Content-Type: application/vnd.api+json\r\n"
        << "Connection: keep-alive\r\n"
        << "Accept: application/vnd.api+json\r\n"
        << "Authorization: Bearer " << m_cookies << "\r\n"
        << "Content-Length: 0" << "\r\n"
        << "\r\n";

    return(ss.str());
}

/**
 * @brief 
 * 
 * @param dps 
 * @return std::string 
 */
std::string noor::RestClient::registerDatapoints(const std::vector<std::string>& dps) {
    std::string host("192.168.1.1:443");
    std::stringstream ss("");
    uri("/api/v1/register/db?fetch=true");

    json jarray = json::array();
    for(const auto& ent: dps) {
        jarray.push_back(ent);
    }
    auto body = jarray.dump();
    json Value = json::object();
    Value["last"] = jarray;
    body = Value.dump();

    //clear the previous contents now.
    ss.str("");
    ss << "POST " << uri() <<" HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Content-Type: application/vnd.api+json\r\n"
        << "Connection: keep-alive\r\n"
        << "Accept: application/vnd.api+json\r\n"
        << "Authorization: Bearer " << cookies() << "\r\n"
        << "Content-Length: " << body.length() << "\r\n"
        << "\r\n"
        << body;

    return(ss.str());
}

std::string noor::RestClient::buildRequest(const std::string& in, std::vector<std::string> param) {
    return(std::string());
}

std::string noor::RestClient::getEvents(std::string uri) {
   std::string host("192.168.1.1:443");
    std::stringstream ss("");
    m_uri.assign("/api/v1/events?timeout=10");

    ss << "GET " << m_uri <<" HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Content-Type: application/vnd.api+json\r\n"
        << "Connection: keep-alive\r\n"
        << "Accept: application/vnd.api+json\r\n"
        << "Authorization: Bearer " << m_cookies << "\r\n"
        << "Content-Length: 0" << "\r\n"
        << "\r\n";

    return(ss.str());
 
}
/**
 * @brief 
 * 
 * @param http_header 
 * @param http_body 
 * @param result 
 * @return std::string 
 */
std::string noor::RestClient::processResponse(const std::string& http_header, const std::string& http_body, std::string& result) {
    std::cout << "line: " << __LINE__ << " header: " <<std::endl << http_header << " http_body: " << http_body << std::endl;
    
    if(!uri().compare(0, 19, "/api/v1/auth/tokens")) {
        json json_object = json::parse(http_body);
        m_cookies.assign(json_object["data"]["access_token"]);

        if(pending_request()) {
            //release the future now
            promise().set_value();
            return(std::string());
        }
        //return(authorizeToken(http_body, "test"));
        return(registerDatapoints({{"device"}}));

    } else if(!uri().compare(0, 26, "/api/v1/auth/authorization")) {
        std::cout << "line: " << __LINE__ << " http_body: " << http_body << std::endl;
        json json_object = json::parse(http_body);
        //device name is unknown
        return(registerDatapoints({{"device"}}));

    } else if(!uri().compare(0, 19, "/api/v1/register/db")) {
        std::unordered_map<std::string, std::string> cache;
        
        if(http_body.length()) {
            //Parse the json response
            json Value = json::parse(http_body);

            if(!deviceName().length() && Value["data"]["device.product"] != nullptr) {
                //remember the deviceName for subsequent response.
                deviceName(Value["data"]["device.product"].get<std::string>());
                if(!deviceName().compare(0, 4, "RX55")) {
                    return(registerDatapoints({
                            {"device"},
                            {"system.os"},
                            //Wan IP Address
                            {"net.interface.common[].ipv4.address"},
                            {"net.interface.cellular[c1]"},
                            //WiFi Mode
                            {"net.interface.wifi[w1].radio.mode"},
                            {"net.interface.wifi[w2].radio.mode"},
                            {"system.bootcheck.signature"}
                            }));
                }
                return(registerDatapoints({
                        {"device"},
                        {"system.os"},
                        //Wan IP Address
                        {"net.interface.common[].ipv4.address"},
                        {"net.interface.cellular[]"},
                        //WiFi Mode
                        {"net.interface.wifi[w1].radio.mode"},
                        {"net.interface.wifi[w2].radio.mode"},
                        {"net.interface.wifi[w3].radio.mode"},
                        {"system.bootcheck.signature"}}));
            }

            //We have received the Device Information Process it.
            auto ValueResponse = json::object();

            ValueResponse["model"] = Value["data"]["device.product"].get<std::string>();
            ValueResponse["serialnumber"] = Value["data"]["device.provisioning.serial"].get<std::string>();
            ValueResponse["osversion"] = Value["data"]["system.os.version"].get<std::string>();
            ValueResponse["osbuildnumber"] = Value["data"]["system.os.buildnumber"].get<std::string>();
            ValueResponse["firmwarename"] = Value["data"]["system.os.name"].get<std::string>();
            ValueResponse["status"] = "";
            ValueResponse["lastcommunicationdate"] = "";

            auto ValueResponseArr = json::array();
            auto ValueResponseCellular = json::object();

            if(!deviceName().compare(0, 4, "RX55")) {

                ValueResponseCellular["carrier"] = Value["data"]["net.interface.cellular[c1].operator"].get<std::string>();
                ValueResponseCellular["technology"] = Value["data"]["net.interface.cellular[c1].technology.current"].get<std::string>();
                ValueResponseCellular["apn"] = Value["data"]["net.interface.cellular[c1].apninuse"].get<std::string>();
                ValueResponseCellular["signalstrength"] = std::to_string(Value["data"]["net.interface.cellular[c1].rssi"].get<std::int32_t>());
                ValueResponseCellular["ipaddress"] = Value["data"]["net.interface.common[c1].ipv4.address"].get<std::string>();
                ValueResponseCellular["imei"] = Value["data"]["net.interface.cellular[c1].imei"].get<std::string>();

                ValueResponseArr.push_back(ValueResponseCellular);

            } else if(!deviceName().compare(0, 4, "XR80")) {

                if(Value["data"]["net.interface.cellular[c2].service"] != nullptr && 
                  !(Value["data"]["net.interface.cellular[c2].service"].get<std::string>()).compare(0, 9, "Available")) {

                    ValueResponseCellular["carrier"] = Value["data"]["net.interface.cellular[c2].operator"].get<std::string>();
                    ValueResponseCellular["technology"] = Value["data"]["net.interface.cellular[c2].technology.current"].get<std::string>();
                    ValueResponseCellular["apn"] = Value["data"]["net.interface.cellular[c2].apninuse"].get<std::string>();
                    ValueResponseCellular["signalstrength"] = std::to_string(Value["data"]["net.interface.cellular[c2].rssi"].get<std::int32_t>());
                    ValueResponseCellular["ipaddress"] = Value["data"]["net.interface.common[c2].ipv4.address"].get<std::string>();
                    ValueResponseCellular["imei"] = Value["data"]["net.interface.cellular[c2].imei"].get<std::string>();
                    ValueResponseArr.push_back(ValueResponseCellular);
                }

                if(Value["data"]["net.interface.cellular[c3].service"] != nullptr && 
                  !(Value["data"]["net.interface.cellular[c3].service"].get<std::string>()).compare(0, 9, "Available")) {

                    ValueResponseCellular["carrier"] = Value["data"]["net.interface.cellular[c3].operator"].get<std::string>();
                    ValueResponseCellular["technology"] = Value["data"]["net.interface.cellular[c3].technology.current"].get<std::string>();
                    ValueResponseCellular["apn"] = Value["data"]["net.interface.cellular[c3].apninuse"].get<std::string>();
                    ValueResponseCellular["signalstrength"] = std::to_string(Value["data"]["net.interface.cellular[c3].rssi"].get<std::int32_t>());
                    ValueResponseCellular["ipaddress"] = Value["data"]["net.interface.common[c3].ipv4.address"].get<std::string>();
                    ValueResponseCellular["imei"] = Value["data"]["net.interface.cellular[c3].imei"].get<std::string>();
                    ValueResponseArr.push_back(ValueResponseCellular);
                }

            } else if(!deviceName().compare(0, 4, "XR90")) {

                if(Value["data"]["net.interface.cellular[c4].service"] != nullptr && 
                  !(Value["data"]["net.interface.cellular[c4].service"].get<std::string>()).compare(0, 9, "Available")) {

                    ValueResponseCellular["carrier"] = Value["data"]["net.interface.cellular[c4].operator"].get<std::string>();
                    ValueResponseCellular["technology"] = Value["data"]["net.interface.cellular[c4].technology.current"].get<std::string>();
                    ValueResponseCellular["apn"] = Value["data"]["net.interface.cellular[c4].apninuse"].get<std::string>();
                    ValueResponseCellular["signalstrength"] = std::to_string(Value["data"]["net.interface.cellular[c4].rssi"].get<std::int32_t>());
                    ValueResponseCellular["ipaddress"] = Value["data"]["net.interface.common[c4].ipv4.address"].get<std::string>();
                    ValueResponseCellular["imei"] = Value["data"]["net.interface.cellular[c4].imei"].get<std::string>();
                    ValueResponseArr.push_back(ValueResponseCellular);
                }

                if(Value["data"]["net.interface.cellular[c5].service"] != nullptr && 
                  !(Value["data"]["net.interface.cellular[c5].service"].get<std::string>()).compare(0, 9, "Available")) {

                    ValueResponseCellular["carrier"] = Value["data"]["net.interface.cellular[c5].operator"].get<std::string>();
                    ValueResponseCellular["technology"] = Value["data"]["net.interface.cellular[c5].technology.current"].get<std::string>();
                    ValueResponseCellular["apn"] = Value["data"]["net.interface.cellular[c5].apninuse"].get<std::string>();
                    ValueResponseCellular["signalstrength"] = std::to_string(Value["data"]["net.interface.cellular[c5].rssi"].get<std::int32_t>());
                    ValueResponseCellular["ipaddress"] = Value["data"]["net.interface.common[c5].ipv4.address"].get<std::string>();
                    ValueResponseCellular["imei"] = Value["data"]["net.interface.cellular[c5].imei"].get<std::string>();
                    ValueResponseArr.push_back(ValueResponseCellular);
                }

            } else {

            }
            ValueResponse["cellular"] = ValueResponseArr;
            std::cout << "line: " << __LINE__ << " value: " << ValueResponse.dump() << std::endl;
            result.assign(ValueResponse.dump());
        }
    } else if(!uri().compare(0, 14, "/api/v1/events")) {
        
    }
    return(std::string());
}


std::vector<struct option> options = {
    {"role",                      required_argument, 0, 'r'},
    {"server-ip",                 required_argument, 0, 'i'},
    {"server-port",               required_argument, 0, 'p'},
    {"web-port",                  required_argument, 0, 'w'},
    {"wan-interface-instance",    required_argument, 0, 'a'},
    {"protocol",                  required_argument, 0, 't'},
    {"mongodb-uri",               required_argument, 0, 's'},
    {"self-port",                 required_argument, 0, 'e'},
    {"timeout",                   required_argument, 0, 'o'},
    {"machine",                   required_argument, 0, 'm'},
    {"config-json",               required_argument, 0, 'c'},
    {"userid",                    required_argument, 0, 'u'},
    {"password",                  required_argument, 0, 'd'},
    {"geolocation-access-token",  required_argument, 0, 'g'},
    {"emailid",                   required_argument, 0, 'l'},
    {"email-password",            required_argument, 0, 'q'}
};

/*
 _ __ ___   __ _(_)_ __  
| '_ ` _ \ / _` | | '_ \ 
| | | | | | (_| | | | | |
|_| |_| |_|\__,_|_|_| |_|
*/
/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(std::int32_t argc, char *argv[]) {
    std::int32_t c;
    std::int32_t option_index = 0;
    std::unordered_map<std::string, std::string> config;
    
    while ((c = getopt_long(argc, argv, "r:i:p:w:t:a:s:e:o:m:u:d:", options.data(), &option_index)) != -1) {
        switch(c) {
            case 'r':
            {
                std::string role("");
                role = optarg;
                if(role.compare("client") && (role.compare("server"))) {
                    std::cout << "Invalid value for --role, possible value is client or server "<< std::endl;
                    return(-1);
                }
                config.emplace(std::make_pair("role", optarg));
            }
            break;
            case 'i':
            {
                config.emplace(std::make_pair("server-ip", optarg));
            }
            break;
            case 'p':
            {
                config.emplace(std::make_pair("server-port", optarg));
            }
            break;
            case 'w':
            {
                config.emplace(std::make_pair("web-port", optarg));
            }
            break;
            case 'a':
            {
                config.emplace(std::make_pair("wan-interface-instance", optarg));
            }
            break;
            case 't':
            {
                config.emplace(std::make_pair("protocol", optarg));
            }
            break;
            case 's':
            {
                config.emplace(std::make_pair("mongodb-uri", optarg));
            }
            break;
            case 'e':
            {
                config.emplace(std::make_pair("self-port", optarg));
            }
            break;
            case 'o':
            {
                config.emplace(std::make_pair("timeout", optarg));
            }
            break;
            case 'm':
            {
                config.emplace(std::make_pair("machine", optarg));
            }
            break;
            case 'u':
            {
                config.emplace(std::make_pair("userid", optarg));
            }
            break;
            case 'd':
            {
                config.emplace(std::make_pair("password", optarg));
            }
            break;
            case 'g':
            {
                config.emplace(std::make_pair("geolocation-access-token", optarg));
            }
            break;
            case 'l':
            {
                config.emplace(std::make_pair("emailid", optarg));
            }
            break;
            case 'q':
            {
                config.emplace(std::make_pair("email-password", optarg));
            }
            break;

            default:
            {
                std::cout << "--role <client|server> " << std::endl
                          << "--server-ip <ip address of server> " << std::endl
                          << "--server-port <server port number> " << std::endl
                          << "--web-port  <server-web-port for http request> " << std::endl
                          << "--mongodb-uri   <uri for mongodb> " << std::endl
                          << "--self-port <self port for bind to receive request> " << std::endl
                          << "--protocol  <tcp|udp|unix/tls> " << std::endl
                          << "--wan-interface-instance <c1|c3|c4|c5|w1|w2|e1|e2|e3> " << std::endl
                          << "--timeout     <value in ms> " << std::endl
                          << "--machine     <host|> " << std::endl
                          << "--userid      <Rest Client User ID> " << std::endl
                          << "--password    <Rest Client Password> " << std::endl
                          << "--geolocation-access-token <access-token for getting geo location>" << std::endl
                          << "--emailid <email for login to smtp server>" << std::endl
                          << "--email-password < email password forlogin to smtp server>" << std::endl;
                          return(-1);
            }
        }
    }
    
    
    noor::Uniimage inst;
    
    auto ret = inst.init();
    inst.set_config(config);

    std::cout << "line: " << __LINE__ << " epoll_fd: " << ret << std::endl;

    if(!config["role"].compare("client")) {
        
        if(!config["protocol"].compare("tcp")) {
            inst.CreateServiceAndRegisterToEPoll(noor::ServiceType::Tcp_Client_Service_Async,
                                                 config["server-ip"], std::stoi(config["server-port"]),
                                                 true);
        } else if(!config["protocol"].compare("udp")) {

        } else if(!config["protocol"].compare("tls")) {
            inst.CreateServiceAndRegisterToEPoll(noor::ServiceType::Tls_Tcp_Client_Service_Async, 
                                                 config["server-ip"], std::stoi(config["server-port"]),
                                                 true);
        } else if(!config["protocol"].compare("dtls")) {

        } else {
            //Protocol not supported.
            std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " protocol is not supported " << std::endl;
            exit(0);
        }
        
        std::string bridgeIP("192.168.1.1");
        auto httpsPort = 443;
        inst.CreateServiceAndRegisterToEPoll(noor::ServiceType::Tls_Tcp_Restclient_Service_Sync, bridgeIP, httpsPort);
        
    } else if(!config["role"].compare("server")) {

        if(!config["server-ip"].length()) {
            config["server-ip"] = "127.0.0.1";
        }

        if(!config["protocol"].compare("tcp")) {
            inst.CreateServiceAndRegisterToEPoll(noor::ServiceType::Tcp_Server_Service, config["server-ip"], std::stoi(config["server-port"]));

        } else if(!config["protocol"].compare("tls")) {
            inst.CreateServiceAndRegisterToEPoll(noor::ServiceType::Tls_Tcp_Server_Service, config["server-ip"], std::stoi(config["server-port"]));
        } else if(!config["protocol"].compare("dtls")) {

        } else {
            //Protocol not supported.
            std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " protocol is not supported " << std::endl;
            exit(0);
        }

        std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " listening on PORT:" << config["web-port"] << std::endl;
        inst.CreateServiceAndRegisterToEPoll(noor::ServiceType::Tcp_Web_Server_Service, config["server-ip"], std::stoi(config["web-port"]));
        auto svc = inst.GetService(noor::ServiceType::Tcp_Web_Server_Service);
        if(svc != nullptr) {
            svc->dbinst(std::make_unique<MongodbClient>(config["mongodb-uri"]));
        }
    }

    //The Unit of timeout is in millisecond.
    auto timeout = 100;
    if(config["timeout"].length()) {
        timeout = std::stoi(config["timeout"]);
    }
    // timeout is in milli seconds
    inst.start(timeout);
}

std::string noor::Service::get_geolocation(const std::string& IP, const std::string& access_token) {
    #if 0
    //Do a TLS handshake now
    svc->tls().init(svc->handle());
    svc->tls().client();
    #endif

    //Prepare Request to get geolocation 
    std::stringstream ss;
    ss << "GET /" << IP
       << "?access_key="
       << access_token
       << " HTTP/1.1\r\n"
       << "Host: api.ipstack.com"
       <<"\r\n"
       << "Connection: keep-alive"
       << "\r\n"
       << "Content-Length: 0"
       << "\r\n"
       << "Accept: application/json, text/html"
       << "\r\n"
       << "User-Agent: Embedded Client"
       << "\r\n"
       << "\r\n";
    //std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " REQUEST: " << ss.str() << std::endl;

    if(tcp_tx(handle(), ss.str()) > 0) {
        //sent successfully.
        std::string response;
        if(tcp_rx(handle(), response) > 0) {
            return(response);
        }
    }
    return(std::string());
}

/**
 * @brief 
 * 
 * @param IP 
 * @param PORT 
 * @param isAsync 
 * @return std::int32_t 
 */
std::int32_t noor::Service::tcp_client(const std::string& IP, std::uint16_t PORT, std::int32_t &fd, bool isAsync) {
    struct addrinfo *result;
    /* Set up the address we're going to bind to. */
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(PORT);
    //m_addr.sin_addr.s_addr = inet_addr(IP.c_str());
    memset(m_addr.sin_zero, 0, sizeof(m_addr.sin_zero));

    auto s = getaddrinfo(IP.data(), nullptr, nullptr, &result);
    if (s != 0) {
        //Failed IP is an IP Address not Domain Name
        m_addr.sin_addr.s_addr = inet_addr(IP.c_str());
    } else {
        //Success
        m_addr.sin_addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr;
    }

    freeaddrinfo(result);
    auto len = sizeof(m_addr);
    std::int32_t channel = -1;
    //learn them for future
    ip(IP);
    port(PORT);

    if(isAsync) {
        channel = ::socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
        if(channel < 0) {
            std::cout << "line: " << __LINE__ <<" Creation of INET socket Failed" << std::endl;
            return(-1);
        }
    } else {
        channel = ::socket(AF_INET, SOCK_STREAM, 0);
        if(channel < 0) {
            std::cout << "line: " << __LINE__ <<" Creation of INET socket Failed" << std::endl;
            return(-1);
        }
    }

    handle(channel);
    connected_client(noor::client_connection::Disconnected);
    
    //let the caller see this file descriptor.
    fd = channel;
    
    /* set the reuse address flag so we don't get errors when restarting */
    auto flag = 1;
    if(::setsockopt(channel, SOL_SOCKET, SO_REUSEADDR, (std::int8_t *)&flag, sizeof(flag)) < 0 ) {
        std::cout << "line: " << __LINE__ << " Error: Could not set reuse address option on INET socket!" << std::endl;
        ::close(handle());
        connected_client().erase(handle());
        handle(-1);
        return(-1);
    }
    
    auto rc = ::connect(channel, (struct sockaddr *)&m_addr, len);
    if(rc < 0) {
        if(errno == EINPROGRESS) {    
            connected_client(noor::client_connection::Inprogress);
            std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " Async connection in progress IP: " << IP << " PORT: " << PORT << std::endl;
            return(0);

        } else if(errno == ECONNREFUSED) {
            //Server is not strated yet
            std::cout << "line: " << __LINE__ << " Connect is refused errno: "<< std::strerror(errno) << std::endl;
            ::close(handle());
            connected_client().erase(handle());
            handle(-1);
            return(-1);

        } else {
            std::cout << "line: " << __LINE__ << " Connect is failed errno: "<< std::strerror(errno) << std::endl;
            ::close(handle());
            connected_client().erase(handle());
            handle(-1);
            return(-1);
        }
    } else {
        connected_client(noor::client_connection::Connected);
        std::cout << "line: " << __LINE__ << " client is connected IP: " << IP << " PORT: " << PORT << std::endl;
    }

    return(0);
}

/**
 * @brief 
 * 
 * @param channel 
 * @param data 
 * @return std::int32_t 
 */
std::int32_t noor::Service::tcp_rx(std::int32_t channel, std::string& out, std::int32_t len) {
    std::int32_t rc = -1;
    std::array<char, 2048> in;
    in.fill(0);

    if(len == 2048) {
        rc = ::recv(channel, in.data(), in.size(), 0);
        if(rc < 0) {
            return(rc);
        }
        out.assign(in.data(), rc);

    } else {

        std::stringstream ss;
        std::int32_t offset = 0;
        std::string tmp;
        do {
            in.fill(0);
            rc = ::recv(channel, in.data(), in.size(), 0);

            if(rc < 0) {
                return(rc);
            }

            offset += rc;
            tmp.assign(in.data(), rc);
            ss << tmp;

        }while(len != offset);
        out.assign(ss.str());
    }

    return(out.length());
}

std::int32_t noor::Service::tcp_peek(std::int32_t channel, std::string& out, std::int32_t len)
{
    std::int32_t rc = -1;
    std::array<char, 2048> in;
    in.fill(0);

    if(len == 2048) {
        rc = ::recv(channel, in.data(), len, MSG_PEEK);
        if(rc <= 0) {
            return(rc);
        }
        out.assign(in.data(), rc);
    }
    return(rc);
}

std::int32_t noor::Service::tcp_rx(std::string& data) 
{
    return(tcp_rx(handle(),data));
}

std::string noor::Service::get_contentType(std::string ext)
{
    std::string cntType("");
    /* get the extension now for content-type */
    if(!ext.compare("woff")) {
      cntType = "font/woff";
    } else if(!ext.compare("woff2")) {
      cntType = "font/woff2";
    } else if(!ext.compare("ttf")) {
      cntType = "font/ttf";
    } else if(!ext.compare("otf")) {
      cntType = "font/otf";
    } else if(!ext.compare("css")) {
      cntType = "text/css";
    } else if(!ext.compare("js")) {
      cntType = "text/javascript";
    } else if(!ext.compare("eot")) {
      cntType = "application/vnd.ms-fontobject";
    } else if(!ext.compare("html")) {
      cntType = "text/html";
    } else if(!ext.compare("svg")) {
      cntType = "image/svg+xml";
    } else if(!ext.compare("gif")) {
      cntType ="image/gif";
    } else if(!ext.compare("png")) {
      cntType = "image/png";
    } else if(!ext.compare("ico")) {
      cntType = "image/vnd.microsoft.icon";
    } else if(!ext.compare("jpg")) {
      cntType = "image/jpeg";
    } else if(!ext.compare("json")) {
      cntType = "application/json";
    } else {
      cntType = "text/html";
    }
    return(cntType);
}


std::string noor::Service::buildHttpResponseOK(Http& http, std::string body, std::string contentType)
{
    std::stringstream ss("");

    ss << "HTTP/1.1 200 OK\r\n"
       << "Connection: keep-alive"
       //<< http.value("Connection")
       << "\r\n"
       << "Host: "
       << http.value("Host")
       << "\r\n";
       //<< "Access-Control-Allow-Origin: *\r\n";

    if(body.length()) {
        ss << "Content-Length: "
           << body.length()
           << "\r\n"
           << "Content-Type: "
           << contentType
           <<"\r\n"
           << "\r\n"
           << body;

    } else {
        ss << "Content-Length: 0\r\n";
    }
    return(ss.str());
}

std::string noor::Service::buildHttpRedirectResponse(Http& http, std::string rsp_body) {
    std::stringstream ss("");
    if(!rsp_body.length()) {
        rsp_body.assign("<html><title></title><head></head><body><h2>Redirecting to http://10.20.129.111</h2></body></html>");
    }

    ss << "HTTP/1.1 301 FOUND\r\n"
       << "Location: https://"
       << http.value("ipAddress")
       << ":443\r\n"
       << "Host: " << http.value("Host") << "\r\n"
       << "Connection: " << http.value("Connection") << "\r\n"
       << "Content-Type: text/html" << "\r\n"
       << "Content-Length: " << rsp_body.length() << "\r\n";
    
    if(!http.value("Origin").length()) {
        ss << "Access-Control-Allow-Origin: *\r\n";
    } else {
        ss << "Access-Control-Allow-Origin: "
           << http.value("Origin")
           << "\r\n";
    }

    ss << "\r\n"
       << rsp_body;

    return(ss.str());
}

std::string noor::Service::buildHttpResponse(Http& http, const std::string& rsp_body) {
    std::stringstream ss("");
    if(!rsp_body.length()) {
        ss << "HTTP/1.1 200 OK\r\n"
           << "Connection: close" 
           << "Content-Length: 0\r\n";
       return(ss.str());
    }

    ss << "HTTP/1.1 200 OK\r\n"
       << "Host: " << http.value("Host") << "\r\n"
       << "Connection: " << http.value("Connection") << "\r\n"
       << "Content-Type: application/vnd.api+json" << "\r\n";

    if(!http.value("Origin").length()) {
        ss << "Access-Control-Allow-Origin: *\r\n";
    } else {
        ss << "Access-Control-Allow-Origin: "
           << http.value("Origin")
           << "\r\n";
    }

    ss << "Content-Length: " << rsp_body.length() << "\r\n"
       << "\r\n"
       << rsp_body;

    return(ss.str());
}

std::string noor::Service::handleOptionsMethod(Http& http) {
    std::stringstream http_header("");
    http_header << "HTTP/1.1 200 OK\r\n";
    http_header << "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n";
    http_header << "Access-Control-Allow-Headers: DNT, User-Agent, X-Requested-With, If-Modified-Since, Cache-Control, Content-Type, Range\r\n";
    http_header << "Access-Control-Max-Age: 1728000\r\n";

    if(!http.value("Origin").length()) {
        http_header << "Access-Control-Allow-Origin: *\r\n";
    } else {
        http_header << "Access-Control-Allow-Origin: "
           << http.value("Origin")
           << "\r\n";
    }
    
    http_header << "Content-Type: text/plain; charset=utf-8\r\n";
    http_header << "Content-Length: 0\r\n";
    http_header << "\r\n";

    return(http_header.str());
}
std::string noor::Service::handleGetMethod(Http& http, auto& dbinst) {

    std::stringstream ss("");
    if(!http.uri().compare(0, 19, "/api/v1/dms/account")) {
        auto projection = json::object();
        projection["_id"] = false;
        auto collectionname = "account";
        auto filter = json::object();
        //QS value
        std::string querydocument = "";
        auto userid = http.value("userid");
        auto password = http.value("password");

        if(userid.length() > 0 && password.length() > 0) {
            filter["userid"] = userid;
            filter["password"] = password;
            auto response = dbinst.get_document(collectionname, filter.dump(), projection.dump());

            auto Value = json::object();
            Value["status"] = "success";
            Value["details"] = "";
            Value["response"] = response;
            if(!response.length()) {
                Value["status"] = "failure";
            }

            return(buildHttpResponseOK(http, Value.dump(), "application/json"));
        }

        userid = http.value("userid");
        std::string response("");

        if(userid.length()) {

            if(!userid.compare(0, 3, "all")) {
                response = dbinst.get_documents(collectionname, filter.dump(), projection.dump());
            } else {
                filter["userid"] = userid;
                response = dbinst.get_document(collectionname, filter.dump(), projection.dump());
            }

            auto Value = json::object();
            Value["status"] = "success";
            Value["details"] = "";
            Value["response"] = response;

            return(buildHttpResponseOK(http, Value.dump(), "application/json"));  
        }

    } else if(!http.uri().compare(0, 18, "/api/v1/dms/device")) {
        //Retrieve the Grievance Tickets 
        auto projection = json::object();
        projection["_id"] = false;
        auto collectionname = "device";
        auto filter = json::object();
        
        auto serialnumber = http.value("serialnumber");
        std::string response = "";

        if(serialnumber.length() > 0) {
            if(!serialnumber.compare(0, 3, "all")) {
                //Get All Account Documents fron grievance Collection
                response = dbinst.get_documents(collectionname, filter.dump(), projection.dump());
                //std::cout << "line: " << __LINE__ << " response: "  << response << std::endl;
            } else {
                filter["serialnumber"] = serialnumber;
                response = dbinst.get_document(collectionname, filter.dump(), projection.dump());
                //std::cout << "line: " << __LINE__ << " response: "  << response << std::endl;
            }

            auto Value = json::object();
            Value["status"] = "success";
            Value["details"] = "";
            Value["response"] = response;

            return(buildHttpResponseOK(http, Value.dump(), "application/json"));
        }

    } else if(!http.uri().compare(0, 21, "/api/v1/dms/swrelease")) {
        
    } else if(!http.uri().compare(0, 20, "/api/v1/dms/template")) {
        auto projection = json::object();

        projection["_id"] = false;
        projection["filename"] = true;
        projection["productmodel"] = true;
        projection["fwversion"] = true;
        projection["createdon"] = true;

        auto collectionname = "template";
        auto filter = json::object();
        std::string response = "";
        response = dbinst.get_documents(collectionname, filter.dump(), projection.dump());

        auto Value = json::object();
        Value["status"] = "success";
        if(!response.length()) {
            Value["status"] = "failure";
        }
        Value["details"] = "";
        Value["response"] = response;

        return(buildHttpResponseOK(http, Value.dump(), "application/json"));

    } else if((!http.uri().compare(0, 7, "/webui/"))) {
        /* build the file name now */
        std::string fileName("");
        std::string ext("");

        std::size_t found = http.uri().find_last_of(".");
        if(found != std::string::npos) {
          ext = http.uri().substr((found + 1), (http.uri().length() - found));
          fileName = http.uri().substr(6, (http.uri().length() - 6));
          std::string newFile = "../webgui/webui/" + fileName;
          /* Open the index.html file and send it to web browser. */
          std::ifstream ifs(newFile.c_str());
          std::stringstream ss("");

          if(ifs.is_open()) {
              std::string cntType("");
              cntType = get_contentType(ext); 

              ss << ifs.rdbuf();
              ifs.close();
              return(buildHttpResponseOK(http, ss.str(), cntType));
          } {
            std::cout << "line: " << __LINE__ << " couldn't open the file: " << newFile << std::endl; 
          }
        } else {
            std::cout <<"line: " << __LINE__ << " processing index.html file " << std::endl;
            std::string newFile = "../webgui/webui/index.html";
            /* Open the index.html file and send it to web browser. */
            std::ifstream ifs(newFile.c_str(), std::ios::binary);
            std::stringstream ss("");
            std::string cntType("");

            if(ifs.is_open()) {
                cntType = "text/html";
                ss << ifs.rdbuf();
                ifs.close();
                return(buildHttpResponseOK(http, ss.str(), cntType));
            } else {
                std::cout << "line: " << __LINE__ << " couldn't open the file: " << newFile << std::endl;
            }
        }
    } else if(!http.uri().compare(0, 1, "/")) {
        std::cout <<"line: " << __LINE__ << " processing index.html file " << std::endl;
        std::string newFile = "../webgui/webui/index.html";
        /* Open the index.html file and send it to web browser. */
        std::ifstream ifs(newFile.c_str(), std::ios::binary);
        std::stringstream ss("");
        std::string cntType("");

        if(ifs.is_open()) {
            cntType = "text/html";
            ss << ifs.rdbuf();
            ifs.close();

            return(buildHttpResponseOK(http, ss.str(), cntType));
        } else {
            std::cout << "line: " << __LINE__ << " couldn't open the file: " << newFile << std::endl;
        }
    }

    return(std::string());
}

std::string noor::Service::handlePostMethod(Http& http, auto& dbinst) {

    if(!http.uri().compare(0, 19, "/api/v1/dms/account")) {
        auto body = json::parse(http.body());
        auto collectionname = "account";
        std::cout << "line: " << __LINE__ << " json payload: " << body.dump() << std::endl;
        //Create a new document in account collection.
        auto response = dbinst.create_document(collectionname, http.body());
        std::cout << "line: " << __LINE__ << " response: " << response << std::endl;
        auto Value = json::object();
        Value["status"] = "success";
        Value["details"] = "";
        Value["response"] = response;

        if(!response.length()) {
            Value["status"] = "failure";
            Value["details"] = response;
        }

        return(buildHttpResponseOK(http, Value.dump(), "application/json"));

    } else if(!http.uri().compare(0, 20, "/api/v1/dms/register")) {
        
        auto body = json::parse(http.body());
        auto projection = json::object();

        projection["_id"] = false;
        //Retrieve the current value of grievanceid
        projection["serialnumber"] = true;

        auto collectionname = "register";
        auto filter = json::object();
        //QS value
        auto querydocument = json::object();
        auto response = dbinst.get_document(collectionname, filter.dump(), projection.dump());
        std::cout << "line: " << __LINE__ << " response: " << response << std::endl;
        auto Value = json::object();
        Value["status"] = "success";
        Value["details"] = "";

        if(!response.length()) {
            response = dbinst.create_document(collectionname, http.body());
            Value["response"] = "{\"oid\": " +  response + "}";
        } else  {
            Value["response"] = "{\"serialnumber\": " +  response + "}";
        }

        return(buildHttpResponseOK(http, Value.dump(), "application/json"));

    } else if(!http.uri().compare(0, 21, "/api/v1/dms/swrelease")) {
        
        auto body = json::parse(http.body());
        
        auto projection = json::object();
        projection["_id"] = false;
        auto collectionname = "swrelease";

        auto query = json::object();
        query["filename"] = body["filename"];
        
        auto Value = json::object();
        Value["status"] = "success";
        Value["details"] = "";
        //std::string content(http.body().c_str(), http.body().length());
        std::string content(body["content"].get<std::string>().c_str(), body["length"]);
        std::string filename(body["filename"]);
        auto response = dbinst.upload_file(filename, content, content.length());
        
        Value["response"] = response;

        if(!response.length()) {
            Value["status"] = "failure";
        } else {
            auto Value = json::object();
            Value["filename"] = filename;
            Value["name"] = body["name"];
            Value["revision"] = body["revision"];
            Value["createdon"] = body["createdon"];
            Value["uid"] = response;
            dbinst.create_document(collectionname, Value.dump());
        }

        return(buildHttpResponseOK(http, Value.dump(), "application/json"));

    } else if(!http.uri().compare(0, 20, "/api/v1/dms/template")) {
        auto body = json::parse(http.body());
        auto projection = json::object();

        projection["_id"] = false;
        auto collectionname = "template";
        auto query = json::object();
        query["filename"] = body["filename"];
        query["productmodel"] = body["productmodel"];

        auto Value = json::object();
        Value["status"] = "success";
        Value["details"] = "";

        std::string response = dbinst.get_document(collectionname, query.dump(), projection.dump());
        if(!response.length()) {
            response = dbinst.create_document(collectionname, http.body());
        } else {
            Value["details"] = "This file is already released";
        }
        
        Value["response"] = response;

        if(!response.length()) {

            Value["status"] = "failure";
        }
        return(buildHttpResponseOK(http, Value.dump(), "application/json"));
    } else {

    }
    return(std::string());
}

std::string noor::Service::handlePutMethod(Http& http, auto& dbinst) {
    std::stringstream ss("");
    
    if(!http.uri().compare(0, 19, "/api/v1/dms/account")) {
        auto body = json::parse(http.body());
        std::cout << "line: " << __LINE__ << " json payload: " << body.dump() << std::endl;
        auto collection = "account";
        auto filter = json::object();
        filter["userid"] = body["userid"];
        auto document = json::object();
        document["$set"] = body;

        auto response = dbinst.update_collection(collection, filter.dump(), document.dump());
        std::cout << "line: " << __LINE__ << " response: " << response << std::endl;
        auto Value = json::object();
        Value["status"] = "success";
        Value["details"] = "";
        Value["response"] = "";

        if(!response) {
            Value["status"] = "failure";
        }

        return(buildHttpResponseOK(http, Value.dump(), "application/json"));
    } else {

    }
    return(std::string());
}

std::string noor::Service::handleDeleteMethod(Http& http, auto& dbinst) {
    if(!http.uri().compare(0, 19, "/api/v1/dms/account")) {
        auto body = json::parse(http.body());
        std::cout << "line: " << __LINE__ << " json payload: " << body.dump() << std::endl;
        auto collection = "account";
        auto filter = json::object();
        filter["userid"] = http.value("userid");
        auto response = dbinst.delete_document(collection, filter.dump());
        std::cout << "line: " << __LINE__ << " response: " << response << std::endl;
        auto Value = json::object();
        Value["status"] = "success";
        Value["details"] = "";
        Value["response"] = "";

        if(!response) {
            Value["status"] = "failure";
        }

        return(buildHttpResponseOK(http, Value.dump(), "application/json"));
    }else if(!http.uri().compare(0, 20, "/api/v1/dms/template")) {
        auto body = json::parse(http.body());
        std::cout << "line: " << __LINE__ << " json payload: " << body.dump() << std::endl;
        auto collection = "template";
        auto filter = json::object();
        std::cout << "line: " << __LINE__ << " body:  " << body.dump()  << std::endl; 
        bool response = false;
        for(auto ent = body.begin(); ent != body.end(); ++ent) {
            response = dbinst.delete_document(collection, (*ent).dump());
        }
        std::cout << "line: " << __LINE__ << " response: " << response << std::endl;
        auto Value = json::object();
        Value["status"] = "success";
        Value["details"] = "";
        Value["response"] = "";

        if(!response) {
            Value["status"] = "failure";
        }

        return(buildHttpResponseOK(http, Value.dump(), "application/json"));
    }
}


std::string noor::Service::process_web_request(const std::string& req, auto& dbinst) {
    Http http(req);

    if(!http.method().compare("GET")) {
        return(handleGetMethod(http, dbinst));
    } else if(!http.method().compare("POST")) {
        return(handlePostMethod(http, dbinst));
    } else if(!http.method().compare("PUT")) {
        return(handlePutMethod(http, dbinst));
    } else if(!http.method().compare("OPTIONS")) {
        return(handleOptionsMethod(http));
    } else if(!http.method().compare("DELETE")) {
        return(handleDeleteMethod(http, dbinst));
    } else {
        //Error
    }
    return(std::string());
}

/**
 * @brief 
 * 
 * @param http 
 * @return std::string 
 */
std::string noor::Service::build_web_response(Http& http) {
    //Build HTTP Response
    std::cout << "URI: " << http.uri() << " method: " << http.method() << std::endl;
    std::stringstream ss("");
    std::string payload("<html><title></title><head></head><body><h2>Redirecting to http://10.20.129.111</h2></body></html>");
    ss << "HTTP/1.1 302 Found\r\n"
       //<< "Location: https://192.168.1.1:443\r\n"
       << "Location: http://10.20.129.111\r\n"
       << "Content-length: " << payload.length() << "\r\n"
       << "Connection: close\r\n"
       //<< "Cookie: unity_token=IC3wWl66tT3XrqO88iLBSxCYbuxhPvGz; unity_login=admin; last_connection={\"success_last\":\"Sat Apr  8 03:47:22 2023\",\"success_from\":\"192.168.1.100\",\"failures\":0}" 
       << "Cookie: " << http.value("Cookies")
       << "\r\n\r\n"
       << payload;

    std::cout << "The Web Response is " << ss.str() << std::endl;
    return(ss.str());
}

/**
 * @brief 
 * 
 * @param channel 
 * @param data 
 * @return std::int32_t 
 */
std::int32_t noor::Service::web_rx(std::int32_t channel, std::string& data) {
    std::string req;
    std::int32_t len = -1;
    std::int32_t payload_len = 0;

    len = tcp_peek(channel, req);
    if(!len) {
        std::cout << "function: "<<__FUNCTION__ << " line: " << __LINE__ << " channel: " << channel << " being closed" << std::endl;
        return(len);

    } else if(len > 0) {
        Http http(req);
        std::cout << "line: " << __LINE__ << " URI: "   << http.uri()    << std::endl;
        std::cout << "line: " << __LINE__ << " Header " << http.header() << std::endl;
        std::cout << "line: " << __LINE__ << " Body "   << http.body()   << std::endl;
        auto cl = http.value("Content-Length");

        if(!cl.length()) {
            std::cout << "line: " << __LINE__ << " Content-Length is not present" << std::endl;
            len = tcp_rx(channel, req, len);
            if(len > 0) {
                data.assign(req);
            }
            return(data.length());

        } else {
            std::cout << "function: "<< __FUNCTION__ << " line: " << __LINE__ <<" value of Content-Length " << cl << std::endl;
            payload_len = std::stoi(cl) + http.header().length();
            std::cout << "line: " << __LINE__ << " payload_len: " << payload_len << " len: " << len << " header_len: " << http.header().length() << std::endl;
            std::stringstream ss;
            size_t offset = 0;
            do {
                req.assign("");
                len = tcp_rx(channel, req, payload_len - offset);
                if(len <= 0) {
                    return(0);
                }

                ss << req;
                offset += len;
            } while(offset != payload_len);

            if(len == payload_len) {
                //We have received the full HTTP packet
                data.assign(ss.str());
            }
            return(data.length());
        }
    }
    return(0);
}

/**
 * @brief 
 * 
 * @param data 
 * @return std::int32_t 
 */
std::int32_t noor::Service::web_rx(std::string& data)
{
    return(web_rx(handle(), data));
}

/**
 * @brief 
 * 
 * @param IP 
 * @param PORT 
 * @return std::int32_t 
 */
std::int32_t noor::Service::tcp_server(const std::string& IP, std::uint16_t PORT, std::int32_t& fd) {
   /* Set up the address we're going to bind to. */
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(PORT);
    if(!IP.compare("127.0.0.1")) {
        m_addr.sin_addr.s_addr = INADDR_ANY;    
    } else {
        m_addr.sin_addr.s_addr = inet_addr(IP.c_str());
    }
    memset(m_addr.sin_zero, 0, sizeof(m_addr.sin_zero));

    std::int32_t channel = ::socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP);
    if(channel < 0) {
        std::cout << "line: " << __LINE__ << " Creation of INET socket Failed" << std::endl;
        return(-1);
    }

    handle(channel);
    /* set the reuse address flag so we don't get errors when restarting */
    auto flag = 1;
    if(::setsockopt(channel, SOL_SOCKET, SO_REUSEADDR, (std::int8_t *)&flag, sizeof(flag)) < 0 ) {
        std::cout << "line: " << __LINE__ << "Error: Could not set reuse address option on INET socket!" << std::endl;
        ::close(handle());
        handle(-1);
        return(-1);
    }
    auto ret = ::bind(channel, (struct sockaddr *)&m_addr, sizeof(m_addr));
    if(ret < 0) {
        std::cout <<"line: " << __LINE__ << " bind to IP: " << IP << " PORT: " << PORT << " Failed" <<std::endl;
        ::close(handle());
        handle(-1);
	    return(-1);
    }

    if(listen(channel, 10) < 0) {
        std::cout << "line: " << __LINE__ << " listen to channel: " << channel << " Failed" <<std::endl;
        ::close(handle());
        handle(-1);
	    return(-1);
    }

    //let the caller see this file descriptor.
    fd = channel;
    return(0); 
}



/**
 * @brief 
 * 
 * @param req 
 * @return std::int32_t 
 */
std::int32_t noor::Service::tcp_tx(const std::string& req) {
    return(tcp_tx(handle(),req));
}

std::int32_t noor::Service::tcp_tx(std::int32_t channel, const std::string& req) {
    std::int32_t offset = 0;
    std::int32_t req_len = req.length();
    std::int32_t len = -1;
    //auto payload_len = htonl(req_len);
    //std::stringstream data("");
    //data.write (reinterpret_cast <char *>(&payload_len), sizeof(std::int32_t));
    //data << req;

    //req_len = req.length();
    do {
        len = send(channel, (req.data() + offset), req_len - offset, 0);
        if(len < 0) {
            offset = len;
            break;
        }
        offset += len;
    } while(offset != req_len);

    if(offset == req_len) {
        std::cout <<"line: "<< __LINE__ << " Sent to TCP Client successfully: len:" << req_len << std::endl;
    }
    return(offset);
}

/**
 * @brief 
 * 
 * @param in 
 * @return std::int32_t 
 */
std::string TcpClient::onReceive(std::string in) {
    std::cout << "line: " << __LINE__ << " " << __PRETTY_FUNCTION__ << std::endl;
    return(std::string());
}

std::int32_t TcpClient::onClose(std::string in) {
    std::cout << "line: " << __LINE__ << " " << __PRETTY_FUNCTION__ << std::endl;
    return(0);
}

std::string TcpServer::onReceive(std::string in) {
    std::cout << "line: " << __LINE__ << " " << __PRETTY_FUNCTION__ << std::endl;
    return(std::string());
}

std::int32_t TcpServer::onClose(std::string in) {
    std::cout << "line: " << __LINE__ << " " << __PRETTY_FUNCTION__ << std::endl;
    return(0);
}




#endif /* __uniimage__cc__ */
