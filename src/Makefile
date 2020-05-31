dependencies: nng libbq27441

nng:
	@cd nng
	@mkdir build; cd build
	cmake -G Ninja ..
	ninja
	ninja test

libbq27441:
	@cd libbq27441
	make	

clean:
	rm -rf nng/build
	make -C libbq27441 clean
