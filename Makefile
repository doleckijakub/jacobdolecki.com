CXX ?= g++

default: server

CPPFLAGS := -I http-server/src
CXXFLAGS := -Wall -Wextra -Wswitch-enum -pedantic -O2
LDFLAGS :=

server: main.cpp http-libs
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< build/*.o $(LDFLAGS)

build:
	mkdir -p build

build/%.o: %.cpp | build
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: http-libs
http-libs:
	make -f http-server/Makefile all SRCDIR=http-server/src

.PHONY: test
test: server
	./server 8080

.PHONY: clean
clean:
	@FILES=$$(git clean -ndX); \
	if [ -z "$$FILES" ]; then \
		echo "No deletable files, skipping"; \
	else \
		echo "These files will be deleted:"; \
		echo "$$FILES"; \
		read -p "Do you want to delete them? [N/y] " yn; \
		if [ "$$yn" = "y" ] || [ "$$yn" = "Y" ]; then \
			git clean -fdX; \
		else \
			echo "Deletion cancelled."; \
		fi \
	fi