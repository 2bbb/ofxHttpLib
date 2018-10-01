//
//  ofxHttpLib.h
//
//  Created by ishii 2bit on 2018/09/25.
//

#ifndef ofxHttpLib_h
#define ofxHttpLib_h

#include <thread>
#include <future>
#include <regex>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_ZLIB_SUPPORT
#include "httplib.h"

namespace ofx {
    namespace HttpLib {
        struct Request {
            std::string host;
            std::uint16_t port;
            std::string endpoint;
            std::string basic_user;
            std::string basic_pass;
        };
        
        namespace Utils {
            bool parseURL(std::string url, Request &request) {
                std::smatch match;
                std::regex pattern{"^http(s?)://(([^:]+):([^@]+)@)?([^/:]+):?([0-9]*)(/?.*)$"};
                auto res = std::regex_match(url, match, pattern);
                if(res && match.size() == 8) {
                    bool is_https = 0 < match.str(1).length();
                    request.basic_user = match.str(3);
                    request.basic_pass = match.str(4);
                    request.host = match.str(5);
                    request.port = match.str(6) == ""
                                 ? (is_https ? 443 : 80)
                                 : std::stoi(match.str(6));
                    request.endpoint = match.str(7);
                    return true;
                }
                ofLogError("ofxHttpLib") << "can't parse url: " << url;
                return false;
            };
        };
        inline std::shared_ptr<httplib::Response> get(const std::string &host,
                                                      std::uint16_t port,
                                                      const std::string &endpoint)
        {
            httplib::Client client(host.c_str(), port);
            auto res = client.Get(endpoint.c_str());
            return res;
        }
        inline std::shared_ptr<httplib::Response> get(const Request &request)
        { return get(request.host.c_str(), request.port, request.endpoint.c_str()); };
        
        template <typename callback_t>
        void get(const std::string &host,
                std::uint16_t port,
                const std::string &endpoint,
                callback_t callback)
        {
            std::thread([callback, host, port, endpoint] {
                auto res = get(host, port, endpoint);
                callback(std::move(res));
            }).detach();
        }
        
        template <typename callback_t>
        void get(const Request &request,
                 callback_t callback)
        {
            get(request.host, request.port, request.endpoint, callback);
        }
    };
};

namespace ofxHttpLib = ofx::HttpLib;

#endif /* ofxHttpLib_h */
