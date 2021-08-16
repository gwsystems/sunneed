.ONESHELL:
dependencies: nng libbq27441

nng:
	@cd nng
	@[ ! -d build ] && mkdir build; cd build
	cmake -G Ninja ..
	if [ ! -f libnng.a ]; then
		ninja
	fi

libbq27441:
	make -C libbq27441

clean:
	rm -rf nng/build
	make -C libbq27441 clean

.PHONY: nng libbq27441 dependencies clean
