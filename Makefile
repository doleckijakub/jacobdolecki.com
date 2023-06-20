CXX ?= g++

default: server

CPPFLAGS := -I http-server/src
CXXFLAGS := -Wall -Wextra -Wswitch-enum -pedantic -O2
LDFLAGS :=

server: main.cpp build/http.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

build:
	mkdir -p build

build/%.o: %.cpp | build
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

build/http.o:
	make -f http-server/Makefile $@ SRCDIR=http-server/src

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