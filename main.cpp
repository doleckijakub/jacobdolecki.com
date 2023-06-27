#include <iostream>
#include <cassert>
#include <csignal>

#include <filesystem>
namespace fs = std::filesystem;
const static fs::path staticResourcesFolder = "static";

#include <http.hpp>
#include <html-builder.hpp>

using namespace std::string_literals;

std::string exec(std::string cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

bool sendHTML(http::response &res, html::element &el) {
	std::string content;
	el.dumpToString(content);

	res.setStatus(200);
	res.setContentType(http::content_type::TEXT_HTML);
	res.setContentString(content);
	return res.send();
}

std::string escapeHtmlString(const std::string &code) {
	std::string escaped;

	for (const char c : code) {
		switch (c) {
			case '\n': {
				escaped += "<br/>";
			} break;
			case ' ': {
				escaped += "&nbsp;";
			} break;
			default: {
				escaped += c;
			}
		}
	}

	return escaped;
}

html::div fortune() {
	auto div = html::div();

	div.addAttribute("id", "fortune");

	std::string cowfile = exec("cowsay -l | awk 'NR>1 {print $1}' | shuf -n 1");
	std::string cmd = "fortune | cowsay -f "s + cowfile;

	div << (html::span().addAttribute("class", "terminal") << cmd) << html::br();

	std::string cowspeach = exec("fortune | cowsay -f "s + cowfile);

	div << (html::div() << escapeHtmlString(cowspeach));

	return div;
}

bool serve_GET(http::request &req) {
	if (req.url.path.size() == 0) { // index
		auto html = html::html();

		auto head = html::head();
		head << html::link().addAttribute("rel", "stylesheet").addAttribute("href", "index.css");
		html << head;

		auto body = html::body();
		body << (html::main() /* <<  */);
		body << fortune();
		html << body;

		return sendHTML(req.response(), html);
	}

	size_t i = 0;
	const auto &path = req.url.path;

	auto filepath = staticResourcesFolder;

	while (i < path.size()) {
		filepath /= path[i++];
	}

	return req.response().sendFile(filepath);
}

bool requestListener(http::request &req) {
	switch (req.method) {
		case http::method::GET: {
			return serve_GET(req);
		} break;
		case http::method::UNKNOWN:
		case http::method::HEAD:
		case http::method::POST:
		case http::method::PUT:
		case http::method::DELETE:
		case http::method::CONNECT:
		case http::method::OPTIONS:
		case http::method::TRACE:
		case http::method::PATCH:
		default: {
			throw http::exception(405, "Method unsupported");
		}
	}
}

bool dispatchError(http::request &req, int code, const std::string error) {
	req.response().setStatus(code);
	req.response().setContentType(http::content_type::TEXT_HTML);

	req.response().setContentString("<h1>Error "s + std::to_string(code) + "</h1>"s + error);
	return req.response().send();
}

void usage(const std::string &programName) { std::cerr << "Usage: " << programName << " <port>" << std::endl; }

int main(int argc, char const *argv[]) {
	auto next_arg = [&]() -> std::string {
		assert(argc);
		const char *arg = *argv;
		--argc;
		++argv;
		return arg;
	};

	std::string programName = next_arg();

	uint16_t port;

	if (argc) {
		std::string portStr = next_arg();

		try {
			port = std::stol(portStr);
		} catch (const std::invalid_argument &err) {
			std::cerr << "Error: Not a valid port number: " << portStr << std::endl;
			return 1;
		} catch (const std::out_of_range &err) {
			std::cerr << "Error: Number: " << portStr << " out of availible port range" << std::endl;
			return 1;
		}
	} else {
		std::cerr << "Error: No port supplied" << std::endl;
		usage(programName);
		return 1;
	}

	http::host host = http::host::local;

	http::server server(requestListener, dispatchError);

	for (const auto &sig : {SIGINT, SIGTERM, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGSEGV, SIGBUS, SIGSYS, SIGPIPE})
		std::signal(sig, http::server::stopAllInstances);

	server.listen(
		host, port,
		[&]() {
			std::cerr << "Listening on " << host << ":" << port << std::endl;
		},
		[&](const std::string &error) {
			std::cerr << "Failed to listen: " << error << std::endl;
		});
}