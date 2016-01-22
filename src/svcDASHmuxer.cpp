#include <iostream>
#include <fstream>
#include <cstring>
#include "types.h"
#include "muxsvc/muxsvc.h"
#include "file_exists.h"

void printPreamble() {

	std::cout << "\nsvcDASHmuxer - Muxes SVC DASH segments back together \n\n"
			  << "Usage: svcDASHmuxer [Output file][Base Layer] [Base Layer + 1] [...]" << std::endl;
}

DataLenPair readToMemory(std::string file)
{
	char*    buf = 0;
	int32_t size = 0;

	std::ifstream ifs(file.c_str(), std::ios::binary);

	if (!ifs.is_open())
		return DataLenPair(boost::shared_array<const char>(), 0);

	std::vector<char> data;
	data.assign(std::istreambuf_iterator<char>(ifs),
					  std::istreambuf_iterator<char>());

	buf = new char[data.size()];
	size = data.size();

	memcpy(buf, &data[0], size * sizeof(char));

	return DataLenPair(boost::shared_array<const char>(buf), size);
}

int main(int argc, char** argv)
{
	// Check if the user provided the two files
	if (argc < 4) {

		printPreamble();
		std::cerr << "\nYou have to specify at least 2 files to be muxed + 1 output file!" << std::endl;
		return 1;
	}

	std::vector<DataLenPair> svc_mux_data;

	/*
	 * Open the output file
	 */
	if (file_exists(std::string(argv[1]))) {

		std::cerr << "Output file '" << argv[1] << "' already exists! Please choose another output file." << std::endl;
		return 1;
	}

	std::ofstream muxed_data_fs(argv[1]);

	if (!muxed_data_fs.is_open()) {

		std::cerr << "Couldn't open file '" << argv[1] << "' for writing!" << std::endl;
		return 1;
	}

	/*
	 * Read the segment files into memory
	 */
	std::cout << "Opening & reading base layer file '" << argv[2] << "'...";

	DataLenPair base_layer = readToMemory(std::string(argv[2]));

	if (base_layer.get<1>() <= 0) {

		std::cerr << "Failed to read base layer file!" << std::endl;
		return 1;
	}

	svc_mux_data.push_back(base_layer);

	std::cout << "DONE" << std::endl;

	for (int i = 3; i < argc; i++) {

		std::cout << "Opening & reading "<< i - 2 <<"nd enh. layer '" << argv[1] << "'...";

		DataLenPair enh_layer = readToMemory(std::string(argv[i]));

		if (enh_layer.get<1>() <= 0) {

			std::cerr << "Failed to read enh. layer file!" << std::endl;
			return 1;
		}

		std::cout << "DONE" << std::endl;

		svc_mux_data.push_back(enh_layer);
	}

	/*
	 * Mux the layer files together
	 */
	std::cout << "Muxing the files together...";

	DataLenPair muxed_data = muxsvc::muxsvc_vec(svc_mux_data);

	if (muxed_data.get<1>() <= 0) {

		std::cerr << "Failed to mux the segment files together!" << std::endl;
		return 1;
	}

	muxed_data_fs.write(muxed_data.get<0>().get(), muxed_data.get<1>());
	muxed_data_fs.close();

	std::cout << "DONE" << std::endl;

	return 0;
}
