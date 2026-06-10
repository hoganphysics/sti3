
#include "TestDevice.h"

#include <sti/utils/BinaryData.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/Image.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace
{
constexpr unsigned kImageWidth = 10;
constexpr unsigned kImageHeight = 10;
constexpr unsigned kImageBytesPerPixel = 1;

std::vector<unsigned char> makeRandomImageBytes()
{
	std::vector<unsigned char> bytes(kImageWidth * kImageHeight * kImageBytesPerPixel);

	static thread_local std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> byteDistribution(0, 255);

	std::generate(bytes.begin(), bytes.end(), [&byteDistribution]() {
		return static_cast<unsigned char>(byteDistribution(generator));
	});

	return bytes;
}

std::shared_ptr<STI::Utils::BinaryData> makeRandomImageBinaryData()
{
	auto bytes = makeRandomImageBytes();
	auto binaryData = std::make_shared<STI::Utils::BinaryData>();
	auto* imageBytes = binaryData->allocate<unsigned char>(bytes.size());

	std::copy(bytes.begin(), bytes.end(), imageBytes);

	return binaryData;
}

std::shared_ptr<STI::Utils::Image> makeBinaryDataBackedImage()
{
	auto image = std::make_shared<STI::Utils::Image>();
	image->setWidth(kImageWidth).setHeight(kImageHeight);
	image->setImageData(makeRandomImageBinaryData());
	return image;
}

std::string nextImageFilename()
{
	static std::atomic<unsigned> counter{0};
	return "readWrite-random-image-" + std::to_string(counter.fetch_add(1)) + ".raw";
}

bool writeImageFile(const std::shared_ptr<STI::Utils::FileHolder>& file, const std::vector<unsigned char>& bytes)
{
	if (file == nullptr || !file->openFile()) {
		return false;
	}

	bool success = bytes.empty()
		|| file->write(reinterpret_cast<const char*>(bytes.data()), static_cast<unsigned>(bytes.size()));
	file->closeFile();
	return success;
}

std::shared_ptr<STI::Utils::Image> makeFileHolderBackedImage(STI::Device::LocalDevice& device)
{
	auto bytes = makeRandomImageBytes();
	auto file = device.makeFileHolder("", nextImageFilename());

	if (!writeImageFile(file, bytes)) {
		return {};
	}

	auto image = std::make_shared<STI::Utils::Image>();
	image->setWidth(kImageWidth).setHeight(kImageHeight);
	image->setImageData(file);
	image->setFileID(file->getID());
	return image;
}
}


TestDevice::TestDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	// *** Define channels *** //

	// Output channels (the device's output actuators)
	addOutputChannel(0, MixedValueType::Double, "coil current");		//channel 0, must be a double
	addOutputChannel(1, MixedValueType::Int, "temperature setpoint");	//channel 1, must be an integer
	addOutputChannel(2, MixedValueType::Number, "supply voltage");		//channel 2, any numeric type
	addOutputChannel(3, MixedValueType::Vector, "list output");			//channel 3, vector tuple of outputs, format checked by device
	addOutputChannel(4, MixedValueType::String, "string output");

	// Input channels (make measurements that are recorded by the device)
	addInputChannel(10, MixedValueType::Number, "thermocouple voltage");		// measures a number (input)
	addInputChannel(13, MixedValueType::Image, "random image (BinaryData)");	// measures an Image backed by BinaryData
	addInputChannel(14, MixedValueType::Image, "random image (FileHolder)");	// measures an Image backed by FileHolder

	// Input/Output channel
	addInputChannel(11, MixedValueType::Number, MixedValueType::Vector, "vector args");	//measures a number (input); accepts a vector argument (output)
	addInputChannel(12, MixedValueType::Vector, MixedValueType::Number, "vector measurement");	//measures a vector (input), accepts a number argument (output)

}

bool TestDevice::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
	bool success = false;

	switch (channel)
	{
	case 0:
		//coil current (Double)
		std::cout << "Ch:" << channel << ", " << "coil current: " << value.getDouble() << std::endl;
		success = true;
		break;
	case 1:
		//temperature setpoint (Int)
		std::cout << "Ch:" << channel << ", " << "temperature setpoint: " << value.getInt() << std::endl;
		success = true;
		break;
	case 2:
		//supply voltage
		std::cout << "Ch:" << channel << ", " << "supply voltage: " << value.getNumber() << std::endl;
		success = true;
		break;
	case 3:
		//list output
		std::cout << "Ch:" << channel << ", " << "list output: ";
		
		//Example of vector type checking
		if (value.isType({ MixedValueType::Number, MixedValueType::String, MixedValueType::Boolean })) {
			//do something...
		}

		//print args
		{
			auto& tuple = value.getVector();
			for (auto& arg : tuple) {
				std::cout << "  " << arg.print() << std::endl;
			}
			std::cout << std::endl;
			success = true;
		}
		break;
	case 4:
		//string output
		std::cout << "Ch:" << channel << ", " << "string output: " << value.getString() << std::endl;
		success = true;
		break;
	default:
		break;
	}

	return success;
}

bool TestDevice::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	bool success = false;

	switch (channel) {
	case 10:
		//thermocouple voltage
		data.setValue(34.5);
		success = true;
		break;
	case 11:
		//Input/Output
		//vector arguments (output)
		if (value.isType({ MixedValueType::Number, MixedValueType::String })) {
			data.setValue(12.2 * value.getVector().at(0).getNumber());	//double measurement (input)
			success = true;	
		}
		break;
	case 12:
		//Input/Output
		{
			auto arg = value.getNumber();	//number argument (output)
			
			data.addValue(3.2 * arg);	//vector measurement (input)
			data.addValue("example string result");
			data.addValue(true);

			success = true;
		}
		break;
	case 13:
		//Image backed by BinaryData
		data.setValue(makeBinaryDataBackedImage());
		success = true;
		break;
	case 14:
		//Image backed by FileHolder
		{
			auto image = makeFileHolderBackedImage(*this);
			if (image != nullptr) {
				data.setValue(image);
				success = true;
			}
		}
		break;
	default:
		break;
	}
	return success;
}
