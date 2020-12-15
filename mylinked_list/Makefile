all: release

release:
	rm -rf build &&\
	mkdir build &&\
	cd build &&\
	conan install -s build_type=Release .. --build=missing &&\
	cmake .. -DCMAKE_BUILD_TYPE=Release &&\
	cmake --build .;


xcode:
	mkdir -p xcode_project\
	&& cd xcode_project\
	&& conan install .. --build=missing\
	&& cmake .. -G Xcode;

clean:
	rm -rf build
	rm -rf xcode_project

run:
	cd build/bin\
	&& ./main