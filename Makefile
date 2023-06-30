CXX ?= g++

default: server

CPPFLAGS := -I . -I http-server/src -I html-builder/src
CXXFLAGS := -Wall -Wextra -Wswitch-enum -pedantic -O2
LDFLAGS :=

server: main.cpp libs endpoints
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< `find build -type f -name "*.o"` $(LDFLAGS)

build:
	mkdir -p build

build/%.o: %.cpp | build
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

ENDPOINTS := $(shell find endpoints -type f)
build/endpoint-dispatcher.o: endpoint-dispatcher.cpp $(ENDPOINTS) | build
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: list-endpoints
list-endpoints:
	echo $(ENDPOINTS)

.PHONY: endpoints
endpoints: build/endpoint-base.o build/endpoint-dispatcher.o

.PHONY: libs
libs: http-libs html-builder-libs

.PHONY: http-libs
http-libs:
	make -f http-server/Makefile all SRCDIR=http-server/src

.PHONY: html-builder-libs
html-builder-libs:
	make -f html-builder/Makefile all SRCDIR=html-builder/src

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