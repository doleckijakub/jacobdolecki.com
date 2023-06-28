#include "endpoint-base.hpp"

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