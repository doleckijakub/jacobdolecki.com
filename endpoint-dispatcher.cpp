#include "endpoint-dispatcher.hpp"

#include <unordered_map>
#include <string>
#include <functional>

// endpoints
#include "endpoints/index.cpp"

static std::unordered_map<std::string, std::function<endpointDispatchResult(http::request &)>> endpoints;

static void bakeEndpoints() { endpoints.insert_or_assign("/", endpoint::index::serve); }

endpointDispatchResult tryDispatchEndpoint(http::request &req) {
	if (!endpoints.size())
		bakeEndpoints();

	endpointDispatchResult dispatchingResult = {false, false};

	std::string absolutePathname; // different from req.url.pathname, because this is parsed from req.url.path and thus
								  // asserted to not contain multiple leading slashes
	{							  // construct absolutePathname
		for (const auto &segment : req.url.path) {
			absolutePathname += "/"s + segment;
		}
	}

	{ // try to dispatch an endpoint by repeadedly chopping of segments from absolutePathname, untill it's just "/"
		auto dispatchIfRegistered = [&]() {
			if (absolutePathname.empty()) {
				absolutePathname = "/";
			}

			auto foundEndpoint = endpoints.find(absolutePathname);
			if (foundEndpoint != endpoints.end()) {
				dispatchingResult = foundEndpoint->second(req);
			}
		};

		dispatchIfRegistered();

		if (dispatchingResult.dispatched)
			goto dispatched;

		size_t pos = absolutePathname.find_last_of('/', absolutePathname.length() - 2);

		while (!dispatchingResult.dispatched && pos != std::string::npos && absolutePathname.length() > 1) {
			absolutePathname = absolutePathname.substr(0, pos);

			dispatchIfRegistered();

			if (dispatchingResult.dispatched)
				goto dispatched;

			pos = absolutePathname.find_last_of('/', absolutePathname.length() - 2);
		}
	}

dispatched:
	return dispatchingResult;
}