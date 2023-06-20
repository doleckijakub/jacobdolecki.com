#include <iostream>
#include <cassert>

#include <http.hpp>

bool requestListener(http::request &req) {
	return req.respond_string(200, http::content_type::TEXT_PLAIN, "Testing...");
}

bool dispatchInternalServerError(http::request &req) {
	return req.respond_string(500, http::content_type::TEXT_PLAIN, "500 Internal server error");
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

	http::server server(requestListener, dispatchInternalServerError);

	server.listen(
		host, port,
		[&]() {
			std::cerr << "Listening on " << host << ":" << port << std::endl;
		},
		[&](const std::string &error) {
			std::cerr << "Failed to listen: " << error << std::endl;
		});
}