#ifndef FILETRANSFERDEVICE_H
#define FILETRANSFERDEVICE_H

#include <sti/LocalDevice.h>

#include <memory>
#include <vector>

class FileTransferDevice : public STI::Device::LocalDevice
{
public:
    FileTransferDevice(const STI::Utils::Configuration& config);

    bool writeChannel(short channel, const STI::Utils::MixedValue& value) override;
    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) override;

private:
    static constexpr unsigned imageWidth = 10;
    static constexpr unsigned imageHeight = 10;
    static constexpr unsigned imageBytesPerPixel = 1;
    static constexpr unsigned tifImageWidth = 100;
    static constexpr unsigned tifImageHeight = 100;
    static constexpr unsigned tifPaneCount = 3;

    std::vector<unsigned char> randomImageBytes(unsigned width = imageWidth,
                                                unsigned height = imageHeight,
                                                unsigned bytesPerPixel = imageBytesPerPixel);
    std::vector<unsigned char> randomTifImageBytes();
    std::shared_ptr<STI::Utils::Image> binaryDataBackedImage();
    std::shared_ptr<STI::Utils::Image> rawFileHolderBackedImage();
    std::shared_ptr<STI::Utils::Image> tifFileHolderBackedImage();
    STI::Utils::FileID textFileHolderMeasurement();
    std::shared_ptr<STI::Utils::BinaryData> plainBinaryDataMeasurement();
    STI::Utils::FileID virtualTextFileMeasurement();
    std::shared_ptr<STI::Utils::Image> invertImageMeasurement(const std::shared_ptr<STI::Utils::Image>& image);
    int importedFileSize(const STI::Utils::FileID& fileID);
    void imageVectorMeasurement(STI::Utils::MixedValue& data);

    std::shared_ptr<STI::Utils::Image> fileHolderBackedImage(const std::vector<unsigned char>& payload,
                                                             const std::string& extension,
                                                             unsigned width = imageWidth,
                                                             unsigned height = imageHeight);

    unsigned imageMeasurementIndex = 0;
    unsigned fileMeasurementIndex = 0;
    unsigned virtualFileMeasurementIndex = 0;
    STI::Utils::FileID lastImportedFileID;
    int lastImportedFileSize = 0;
};

#endif
