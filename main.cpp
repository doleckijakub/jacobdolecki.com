#include <iostream>
#include <cassert>
#include <csignal>

#include <filesystem>
namespace fs = std::filesystem;
static fs::path baseDirectory = ".";
const static fs::path staticResourcesFolder = "static";

#include <http.hpp>

#include "endpoint-dispatcher.hpp"

bool serve_GET(http::request &req) {
	{ // if an endpoint exists, serve it
		endpointDispatchResult dispatchingResult = tryDispatchEndpoint(req);
		if (dispatchingResult.dispatched) {
			return dispatchingResult.succeeded;
		}
	}

	{ // else if a file exists in ./static, send it
		size_t i = 0;
		const auto &path = req.url.path;
		auto filepath = baseDirectory / staticResourcesFolder;
		while (i < path.size()) {
			filepath /= path[i++];
		}
		return req.response().sendFile(filepath, req.url.pathname); // otherwise respond with 404
	}
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

	try {
		baseDirectory = fs::path(programName).parent_path();
	} catch (std::exception &err) {
		std::cerr << "Warning: Failed to retrieve base directory of the executable, defaulting to: " << baseDirectory
				  << std::endl;
	}

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