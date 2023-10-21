
%{
    #include <sti/utils/Image.h>
    using STI::Utils::Image;

    // #include <sti/utils/ImageWriter.h>
    // using STI::Utils::ImageWriter;
%}

%shared_ptr(STI::Utils::Image);
// %shared_ptr(STI::Utils::ImageWriter);

// %include "sti/utils/ImageWriter.h"

//Image
%template(ImageVector) std::vector< std::shared_ptr< STI::Utils::Image > >;
// %ignore STI::Utils::Image::writeToFile(const std::shared_ptr<STI::Utils::ImageWriter>& writer, const std::string& targetDirectory);
%include "sti/utils/Image.h"
