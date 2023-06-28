#pragma once

#include <http.hpp>
#include <html-builder.hpp>

using namespace std::string_literals;

struct endpointDispatchResult {
	bool dispatched;
	bool succeeded;
};

std::string exec(std::string cmd);
bool sendHTML(http::response &res, html::element &el);
std::string escapeHtmlString(const std::string &code);