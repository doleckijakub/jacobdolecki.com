#include "endpoint-base.hpp"

#include ".secrets/endpoints/services.secret"

#include <iterator>

#define IMPLEMENT_SIMPLE_HTML_ELEMENT(name)                                                                            \
	struct name : public element {                                                                                     \
		explicit name() : element(#name, "", true) {}                                                                  \
	}

namespace html {
IMPLEMENT_SIMPLE_HTML_ELEMENT(table);
IMPLEMENT_SIMPLE_HTML_ELEMENT(tr);
IMPLEMENT_SIMPLE_HTML_ELEMENT(td);
} // namespace html

namespace endpoint {
namespace services {

void authorize(http::request &req) {
	const std::string authorization = req.getHeader("Authorization");
	if (authorization.length()) {
		if (authorization.find("Basic ") == 0) {
			const std::string credentialsBase64 = authorization.substr(6);
			if (credentialsBase64 == BASE64_CREDENTIALS) {
				return;
			} else {
				throw http::exception(401, "Unauthorized, wrong username or password");
			}
		}
	}
	req.response().setHeader("WWW-Authenticate", "Basic realm=\"Access to the /services\"");
	throw http::exception(401, "Unauthorized, log in to authenticate");
}

endpointDispatchResult serve(http::request &req) {
	switch (req.url.path.size()) {
		case 1: {
			authorize(req);

			auto html = html::html();

			auto head = html::head();
			head << html::link().addAttribute("rel", "stylesheet").addAttribute("href", "/index.css");
			html << head;

			auto body = html::body();
			auto table = html::table();
			{
				std::string systemctlstatus = exec("systemctl --type=service --state=running");
				std::vector<std::string> lines;
				std::stringstream ss(systemctlstatus);
				std::string line;

				std::getline(ss, line, '\n'); // TODO: heading

				while (std::getline(ss, line, '\n')) {
					if (!line.length())
						break;

					auto tr = html::tr();

					{
						std::istringstream iss(line);
						std::vector<std::string> columns(std::istream_iterator<std::string>{iss},
														 std::istream_iterator<std::string>());

						tr << (html::td()
							   << (html::a().addAttribute("href", req.url.path[0] + "/"s + columns[0]) << columns[0]));
						tr << (html::td() << columns[1]);
						tr << (html::td() << columns[2]);
						tr << (html::td() << columns[3]);
						std::string description;
						{
							for (size_t i = 4; i < columns.size(); ++i) {
								description += columns[i] + " "s;
							}
							description.pop_back();
						}
						tr << (html::td() << description);
					}

					table << tr;
				}
			}
			body << (html::main().addAttribute("style", "width: auto") << table);
			html << body;

			return {true, sendHTML(req.response(), html)};
		} break;
		case 2: {
			authorize(req);

			std::string rows = "100";
			{
				const auto it = req.url.searchParams.find("r");
				if (it != req.url.searchParams.end()) {
					try {
						std::stoi(it->second);
						rows = it->second;
					} catch (const std::exception &e) {
						
					}

					if (it->second == "")
						rows = "all";
				}
			}

			const auto service = req.url.path[1];
			const auto serviceFile = std::filesystem::path("/tmp/"s + service + ".log"s);

			const auto journalctlcmd = "journalctl -u "s + service + ((rows == "all") ? "" : (" | tail -n "s + rows)) +
									   " > "s + serviceFile.string();

			int journalctl = system(journalctlcmd.c_str());
			if (journalctl)
				return {true, false};

			bool sendOk =
				req.response().sendFile(serviceFile, http::content_type::TEXT_PLAIN, "/tmp/"s + service + ".log"s);

			std::filesystem::remove(serviceFile);

			return {true, sendOk};
		} break;
		default: {
			return {false, false};
		}
	}
}

} // namespace services
} // namespace endpoint
