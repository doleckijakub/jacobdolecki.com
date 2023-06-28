#include "endpoint-base.hpp"

namespace endpoint {
namespace index {

static html::div fortune() {
	auto div = html::div();

	div.addAttribute("id", "fortune");

	std::string cowfile = exec("cowsay -l | awk 'NR>1 {print $1}' | shuf -n 1");
	std::string cmd = "fortune | cowsay -f "s + cowfile;

	div << (html::span().addAttribute("class", "terminal") << cmd) << html::br();

	std::string cowspeach = exec("fortune | cowsay -f "s + cowfile);

	div << (html::div() << escapeHtmlString(cowspeach));

	return div;
}

endpointDispatchResult serve(http::request &req) {
	if (req.url.path.size() != 0)
		return {false, false};

	auto html = html::html();

	auto head = html::head();
	head << html::link().addAttribute("rel", "stylesheet").addAttribute("href", "/index.css");
	html << head;

	auto body = html::body();
	body << (html::main() /* <<  */);
	body << fortune();
	html << body;

	return {true, sendHTML(req.response(), html)};
}

} // namespace index
} // namespace endpoint
