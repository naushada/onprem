#ifndef __uniimage__hpp__
#define __uniimage__hpp__

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <cerrno>
#include <clocale>
#include <cmath>
#include <cstring>

#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <sstream>
#include <unordered_map>
#include <map>
#include <tuple>
#include <getopt.h>
#include <atomic>
#include <future>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "http.hpp"
#include "mongodbc.h"


namespace noor {

    class Service;
    class Uniimage;
    class Tls;
    class  RestClient;

    enum client_connection: std::uint16_t {
        Disconnected = 0,
        Inprogress,
        Connected
    };

    enum ServiceType: std::uint32_t {
        Tcp_Web_Server_Service = 10,
        Tcp_Web_Client_Connected_Service,
            
        Tcp_Server_Service = 20,
        Tcp_Client_Connected_Service,

        Tcp_Client_Service_Async,
        Tcp_Client_Service_Sync,
            
        Tls_Tcp_Server_Service = 30,
        Tls_Tcp_Client_Connected_Service,

        Tls_Tcp_Client_Service_Async = 40,
        Tls_Tcp_Client_Service_Sync,

        Tls_Tcp_Restclient_Service_Async  = 50,
        Tls_Tcp_Restclient_Service_Sync,

        Tls_Tcp_Geolocation_Service_Async  = 60,
        Tls_Tcp_Geolocation_Service_Sync,

        Tls_Udp_Email_Service_Async = 70,
        Tls_Udp_Email_Service_Sync,

    };
}


class noor::Uniimage {

    public:
        noor::Service* GetService(noor::ServiceType serviceType);
        noor::Service* GetService(noor::ServiceType serviceType, const std::int32_t& channel);
        void DeleteService(noor::ServiceType serviceType, const std::int32_t& channel);
        void DeleteService(noor::ServiceType serviceType);
        std::int32_t CreateServiceAndRegisterToEPoll(noor::ServiceType serviceType, const std::string& IP, const std::uint16_t& PORT, bool syncOrAsync=false, std::int32_t channel= -1);
        std::int32_t RegisterToEPoll(noor::ServiceType serviceType, std::int32_t channel);
        std::int32_t DeRegisterFromEPoll(std::int32_t fd);
        std::int32_t start(std::int32_t to);
        std::int32_t stop(std::int32_t in);
        std::int32_t init();

        auto& getResponseCache() {
            return(m_cache);
        }

        Uniimage() : m_epollFd(-1), m_evts(), m_services(), m_cache() {}
        ~Uniimage() {
            ::close(m_epollFd);
            m_services.clear();
            m_cache.clear();
            m_evts.clear();
            m_config.clear();
        }

        std::unordered_map<std::string, std::string> get_config() const {return(m_config);}
        void set_config(std::unordered_map<std::string, std::string> cfg) {m_config = cfg;}
        

    private:
        std::int32_t m_epollFd;
        std::vector<struct epoll_event> m_evts;
        std::multimap<noor::ServiceType, std::unique_ptr<noor::Service>> m_services;
        //The key is serial number of device. and value is json object.
        std::unordered_map<std::string, std::string> m_cache;
        std::unordered_map<std::string, std::string> m_config;
        
};

/**
 * @brief 
 * 
 */
class noor::Tls {
    public:
        /**
         * @brief Construct a new Tls object
         * 
         */
        Tls(): m_method(nullptr), m_ssl_ctx(nullptr, SSL_CTX_free), m_ssl(nullptr, SSL_free) {
        }

        Tls(bool role): m_method(SSLv23_server_method()), 
                        m_ssl_ctx(SSL_CTX_new(m_method), SSL_CTX_free),
                        m_ssl(SSL_new(m_ssl_ctx.get()), SSL_free) {

            OpenSSL_add_all_algorithms();
            SSL_load_error_strings();
            /* ---------------------------------------------------------- *
             * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
             * ---------------------------------------------------------- */
            SSL_CTX_set_options(m_ssl_ctx.get(), SSL_OP_NO_SSLv2);
        }

        ~Tls() {
            
        }

        /**
         * @brief 
         * 
         * @param fd 
         * @param cert 
         * @param pkey 
         * @return std::int32_t 
         */
        std::int32_t init(std::int32_t fd) {
            //m_method = TLSv1_2_client_method();
            m_method = TLS_client_method();

            m_ssl_ctx = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>(SSL_CTX_new(m_method), SSL_CTX_free);
            //m_ssl_ctx.reset(SSL_CTX_new(m_method));

            m_ssl = std::unique_ptr<SSL, decltype(&SSL_free)>(nullptr, SSL_free);
            m_ssl.reset(SSL_new(m_ssl_ctx.get()));

            OpenSSL_add_all_algorithms();
            SSL_load_error_strings();
            /* ---------------------------------------------------------- *
             * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
             * ---------------------------------------------------------- */
            SSL_CTX_set_options(m_ssl_ctx.get(), SSL_OP_NO_SSLv2);

            std::int32_t rc = SSL_set_fd(m_ssl.get(), fd);
            
            return(rc);
        }

        std::int32_t init(const std::string &cert="../cert/cert.pem", const std::string& pkey="../cert/pkey.pem") {
            std::int32_t ret = -1;
            m_method = TLS_server_method();
            //m_method = TLS_method();
            m_ssl_ctx = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>(SSL_CTX_new(m_method), SSL_CTX_free);
            //m_ssl_ctx.reset(SSL_CTX_new(m_method));

            m_ssl = std::unique_ptr<SSL, decltype(&SSL_free)>(nullptr, SSL_free);
            m_ssl.reset(SSL_new(m_ssl_ctx.get()));

#if 0
            SSL_CTX_set_options(m_ssl_ctx.get (), SSL_OP_ALL);
            // disallow compression.
            SSL_CTX_set_options(m_ssl_ctx.get (), SSL_OP_NO_COMPRESSION);
            // disallow usage of SSLv2, SSLv3, TLSv1 and TLSv1.1 which are considered insecure.
            SSL_CTX_set_options(m_ssl_ctx.get (), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

            // choose the cipher according to the server's preferences.
            SSL_CTX_set_options(m_ssl_ctx.get (), SSL_OP_CIPHER_SERVER_PREFERENCE);

            // setup write mode.
            SSL_CTX_set_mode(m_ssl_ctx.get (), SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

            // automatically renegotiates.
            SSL_CTX_set_mode(m_ssl_ctx.get (), SSL_MODE_AUTO_RETRY);

            // enable SSL session caching.
            //SSL_CTX_set_session_id_context(sslContext.get (), reinterpret_cast <const unsigned char *> (&sessionId), sizeof (sessionId));

            // no verification by default.
            SSL_CTX_set_verify(m_ssl_ctx.get (), SSL_VERIFY_NONE, nullptr);

            // set default TLSv1.2 and below cipher suites.
            SSL_CTX_set_cipher_list(m_ssl_ctx.get (), "EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EECDH+ECDSA+CHACHA20:EECDH+aRSA+CHACHA20:EECDH+ECDSA+AESCCM:"
                                          "EDH+DSS+AESGCM:EDH+aRSA+CHACHA20:EDH+aRSA+AESCCM:-AESCCM8:EECDH+ECDSA+AESCCM8:EDH+aRSA+AESCCM8");

            //  set default TLSv1.3 cipher suites.
            SSL_CTX_set_ciphersuites(m_ssl_ctx.get (), "TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:"
                                          "TLS_AES_128_CCM_SHA256:TLS_AES_128_CCM_8_SHA256");

            // disallow client-side renegotiation.
            SSL_CTX_set_options(m_ssl_ctx.get (), SSL_OP_NO_RENEGOTIATION);

            // Set Diffie-Hellman key.
            if(SSL_CTX_set_dh_auto(m_ssl_ctx.get (), 1) != 1) {
                throw std::runtime_error ("SSL_CTX_set_dh_auto() failed");
            }

            // set groups/curves
            int curves [] = {
                NID_X9_62_prime256v1
            };
            if(SSL_CTX_set1_curves(m_ssl_ctx.get (), curves, sizeof(curves) / sizeof (curves [0])) != 1) {
                throw std::runtime_error ("SSL_CTX_set1_curves() failed");
            }

            // OpenSSL 3.0 has curve selection set to 'auto' by default
            // SSL_CTX_set_ecdh_auto (sslContext.get (), 1);

            // minimum protocol version TLS 1.2
            SSL_CTX_set_min_proto_version(m_ssl_ctx.get (), TLS1_2_VERSION);
#endif
            OpenSSL_add_all_algorithms();
            // set default TLSv1.2 and below cipher suites.
            SSL_CTX_set_cipher_list(m_ssl_ctx.get (), "EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EECDH+ECDSA+CHACHA20:EECDH+aRSA+CHACHA20:EECDH+ECDSA+AESCCM:"
                                          "EDH+DSS+AESGCM:EDH+aRSA+CHACHA20:EDH+aRSA+AESCCM:-AESCCM8:EECDH+ECDSA+AESCCM8:EDH+aRSA+AESCCM8");

            //  set default TLSv1.3 cipher suites.
            SSL_CTX_set_ciphersuites(m_ssl_ctx.get (), "TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:"
                                          "TLS_AES_128_CCM_SHA256:TLS_AES_128_CCM_8_SHA256");
                                          
            //For tls server
            if(cert.length() && pkey.length()) {

                if((ret = SSL_CTX_use_certificate_file(m_ssl_ctx.get(), cert.c_str(), SSL_FILETYPE_PEM)) <= 0) {
                    ERR_print_errors_fp(stderr);
                    return(ret);
                }

                if((ret = SSL_CTX_use_PrivateKey_file(m_ssl_ctx.get(), pkey.c_str(), SSL_FILETYPE_PEM)) <= 0 ) {
                    ERR_print_errors_fp(stderr);
                    return(ret);
                }
                if(!SSL_CTX_check_private_key(m_ssl_ctx.get())) {
                    perror("private key check");
                    exit(1);
                }
            }
            
            SSL_set_accept_state(m_ssl.get());

            return(ret);
        }
        /**
         * @brief 
         * 
         * @return std::int32_t 
         */
        std::int32_t client() {
            std::int32_t rc = SSL_connect(m_ssl.get());
            return(rc);
        }

        /**
         * @brief 
         * 
         * @return std::int32_t 
         */
        std::int32_t server(std::int32_t fd) {
            std::int32_t rc = -1;
            rc = SSL_set_fd(m_ssl.get(), fd);
            // prepare the object to work in server mode.
            SSL_set_accept_state (m_ssl.get ());
            
            rc = SSL_accept(m_ssl.get());
            std::cout << __TIMESTAMP__ << " line: " << __LINE__ << " SSL_accept return: " << rc << std::endl;
            ERR_print_errors_fp(stderr);
            return(rc);
        }

        /**
         * @brief 
         * 
         * @param out 
         * @param len 
         * @return std::int32_t 
         */
        std::int32_t peek(std::string& out, std::uint32_t len = 2048) {
            int rc = -1;
            std::array<char, 2048> ss;
            ss.fill(0);

            rc = SSL_peek(m_ssl.get(), ss.data(), len);

            if(rc > 0) {
                out.assign(ss.data(), rc);
            }
            return(rc);

        }/*peek*/

        /**
         * @brief 
         * 
         * @param out 
         * @param len 
         * @return std::int32_t 
         */
        std::int32_t read(std::string& out, std::uint32_t len = 2048) {
            std::int32_t rc = -1;
            std::array<char, 2048> in;
            in.fill(0);

            if(len == 2048) {
                rc = SSL_read(m_ssl.get(), in.data(), len);
                if(rc < 0) {
                    return(rc);
                }
                out.assign(in.data(), rc);
                return(rc);
            }

            std::stringstream ss;
            std::uint32_t offset = 0;
            std::string tmp;

            if(len < 2048) {
                do {
                    in.fill(0);
                    rc = SSL_read(m_ssl.get(), in.data(), len - offset);

                    if(rc < 0) {
                        return(rc);
                    }

                    offset += rc;
                    tmp.assign(in.data(), rc);
                    ss << tmp;

                }while(len != offset);
            } else {
                do {
                    in.fill(0);
                    rc = SSL_read(m_ssl.get(), in.data(), in.size());

                    if(rc < 0) {
                        return(rc);
                    }

                    offset += rc;
                    tmp.assign(in.data(), rc);
                    ss << tmp;

                }while(len != offset);
            }
            out.assign(ss.str());
            return(offset);

        }/*read*/

        /**
         * @brief 
         * 
         * @param out 
         * @return std::int32_t 
         */
        std::int32_t write(const std::string& out) {
            std::int32_t rc = -1;
            size_t offset = 0;
            auto len = out.length();

            do {
                rc = SSL_write(m_ssl.get(), out.data() + offset, len - offset);

                if(rc < 0) {
                    return(rc);
                }

                offset += rc;
            } while(len != offset);

            return(offset);

        }/*write*/

        /**
         * @brief 
         * 
         * @return auto& 
         */
        auto& ssl_ctx() {
            return(*(m_ssl_ctx.get()));
        }

        /**
         * @brief 
         * 
         * @return auto& 
         */
        auto& ssl() {
            return(*(m_ssl.get()));
        }

    private:
        const SSL_METHOD *m_method;
        std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> m_ssl_ctx;
        std::unique_ptr<SSL, decltype(&SSL_free)> m_ssl;
};

/**
 * @brief 
 * 
 */
class noor::RestClient {
    public:
        RestClient() : m_cookies(""), m_uri(""), m_deviceName(""), m_pending_request(false), m_promise() {}
        ~RestClient() {}
        std::string getToken(const std::string& in);
        std::string authorizeToken(const std::string& in, const std::string& user);
        std::string registerDatapoints(const std::vector<std::string>& dps);
        std::string buildRequest(const std::string& in, std::vector<std::string> param = {});
        std::string processResponse(const std::string& http_header, const std::string& http_body, std::string &svc);
        std::string getEvents(std::string uri="/api/v1/events");
        std::string uri() const {return(m_uri);}
        void uri(std::string path) { m_uri = path;}
        std::string cookies() const { return(m_cookies);}
        void cookies(std::string token) {m_cookies = token;}
        
        std::string status_code() {return m_status_code;}
        void status_code(std::string code) {m_status_code = code;}
        bool pending_request() const {return m_pending_request;}
        void pending_request(bool status) { m_pending_request = status;}
        auto& promise() {return m_promise;}
        void promise(const auto& pr) {m_promise = pr;}
        void deviceName(std::string  name) { m_deviceName.assign(name);}
        std::string deviceName() const { return(m_deviceName);}

    private:
        std::string m_cookies;
        std::string m_uri;
        std::string m_deviceName;
        std::string m_status_code;
        bool m_pending_request;
        std::promise<void> m_promise;
};

class noor::Service {
    public:

        Service() {
            m_handle = -1; 
            m_connected_clients.clear();
            m_dbinst.reset(nullptr);
        }

        Service(std::unordered_map<std::string, std::string> config) {
            if(config.empty()) {
                std::cout << "line: " << __LINE__ << " config is empty" << std::endl;
            }

            m_handle = -1; 
            m_connected_clients.clear();
        }

        virtual ~Service() {close(m_handle);}
        std::int32_t tcp_client(const std::string& IP, std::uint16_t PORT, std::int32_t& channel, bool isAsync=false);
        std::int32_t tcp_server(const std::string& IP, std::uint16_t PORT, std::int32_t& channel);
        std::int32_t tcp_rx(std::string& data);
        std::int32_t tcp_rx(std::int32_t channel, std::string& data, std::int32_t len=2048);
        std::int32_t tcp_peek(std::int32_t channel, std::string& data, std::int32_t len=2048);
        std::int32_t tcp_tx(const std::string& data);
        std::int32_t tcp_tx(std::int32_t channel, const std::string& data);
        std::int32_t web_rx(std::int32_t channel, std::string& data);
        std::int32_t web_rx(std::string& data);

        std::string build_web_response(Http& http);
        std::string process_web_request(const std::string& req, auto& dbinst);
        std::string handleGetMethod(Http& http, auto& dbinst);
        std::string handlePostMethod(Http& http, auto& dbinst);
        std::string handlePutMethod(Http& http, auto& dbinst);
        std::string handleDeleteMethod(Http& http);
        std::string handleOptionsMethod(Http& http);
        
        std::string buildHttpResponse(Http& http, const std::string& rsp_body);
        
        std::string buildHttpRedirectResponse(Http& http, std::string rsp_body = "");
        std::string buildHttpResponseOK(Http& http, std::string body, std::string contentType);
        std::string get_contentType(std::string ext);
        std::string get_geolocation(const std::string& IP, const std::string& access_token);

        virtual std::string onReceive(std::string in) {
            std::cout << "line: " << __LINE__ << "Must be overriden " << std::endl;
            return(in);
        }

        virtual std::int32_t onClose(std::string in) {
            std::cout << "line: " << __LINE__ << "Must be overriden " << std::endl;
            return(in.length());
        }

        void ip(std::string IP) {
            m_ip = IP;
        }

        std::string ip() const {
            return(m_ip);
        }

        void port(std::uint16_t _p) {
            m_port = _p;
        }

        std::uint16_t port() const {
          return(m_port);
        }

        std::int32_t handle() const {
            return(m_handle);
        }

        void handle(std::int32_t fd) {
            m_handle = fd;
        }

        void connected_client(client_connection st) {
            //m_connected_clients.insert(std::make_pair(handle(), st));
            m_connected_clients[handle()] = st;
        }

        auto& connected_client() {
            return(m_connected_clients);
        }

        auto connected_client(std::int32_t channel) {
            return(m_connected_clients[channel]);
        }

        Tls& tls() {
            return(m_tls);
        }

        RestClient& restC() {
            return(m_restC);
        }
        auto addr() {
            return(m_addr);
        }

        MongodbClient& dbinst() { return (*m_dbinst.get());}
        void dbinst(std::unique_ptr<MongodbClient> inst) { m_dbinst = std::move(inst);}

    private:
        std::string m_ip;
        std::uint16_t m_port;
        std::int32_t m_handle;
        struct sockaddr_in m_addr;
        std::unordered_map<std::int32_t, client_connection> m_connected_clients;
        std::vector<struct epoll_event> m_epoll_evts;
        noor::Tls m_tls;
        noor::RestClient m_restC;
        std::unique_ptr<MongodbClient> m_dbinst;
};

class TcpClient: public noor::Service {
    public:
        
        TcpClient(const std::string& IP, const std::uint16_t& PORT, std::int32_t& channel, bool isAsync) {
            tcp_client(IP, PORT, channel, isAsync);
        }

        TcpClient(const std::int32_t& fd, const std::string& IP , const std::int32_t& PORT) {
            handle(fd);
            //learn them for future
            ip(IP);
            port(PORT);
            connected_client(noor::client_connection::Connected);
        }

        ~TcpClient() {}
        virtual std::string onReceive(std::string in) override;
        virtual std::int32_t onClose(std::string in) override;
};



class TcpServer: public noor::Service {
    public:


        TcpServer(const std::string& IP, const std::uint16_t& PORT, std::int32_t& channel) {
            tcp_server(IP, PORT, channel);
        }
        ~TcpServer() {}
        virtual std::string onReceive(std::string in) override;
        virtual std::int32_t onClose(std::string in) override;
};

class WebServer: public noor::Service {
    public:
        WebServer(auto config, noor::ServiceType svcType) : Service() {

            std::string sIP("127.0.0.1");
            auto it = std::find_if(config.begin(), config.end(), [] (const auto& ent) {return(!ent.first.compare("server-ip"));});

            if(it != config.end()) {
                sIP.assign(it->second);
            }
            if(svcType == noor::ServiceType::Tcp_Web_Server_Service) {
                web_server(sIP, std::stoi(config.at("web-port")));
            }
        }
        ~WebServer() {}

        virtual std::string onReceive(std::string in) override;
        virtual std::int32_t onClose(std::string in) override;
};


#endif /* __uniimage__hpp__ */
