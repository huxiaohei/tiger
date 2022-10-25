.PHONY: xx

"":
	cd src && ragel uri.rl -o uri.cc; \
	cd ..; \
	cd src/servers/http && ragel http_response_parser.rl -o http_response_parser.cc; \
	ragel http_request_parser.rl -o http_request_parser.cc; \
	cd ../../..; \
	if [ -d "build" ]; then \
		cd build && make -j4; \
	else \
		mkdir build; \
		cd build && cmake -DCMAKE_CXX_COMPILER:FILEPATH=$(shell which g++) -DCMAKE_C_COMPILER:FILEPATH=$(shell which gcc) ..; \
		make -j4; \
	fi

%:
	if [ -d "build" ]; then \
		cd build && make $@; \
	else \
		mkdir build; \
		cd build && cmake -DCMAKE_CXX_COMPILER:FILEPATH=$(shell which g++) -DCMAKE_C_COMPILER:FILEPATH=$(shell which gcc) $@..; \
		make $@; \
	fi