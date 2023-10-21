
%{
    #include <sti/utils/BinaryData.h>
    #include <sti/utils/BinaryDataStream.h>
    using STI::Utils::BinaryDataStream;
    using STI::Utils::BinaryDataStreamTarget;
%}

// %include <carrays.i>
// %include <various.i>

// %array_class(unsigned char,ByteArr);

%shared_ptr(STI::Utils::BinaryData);
%shared_ptr(STI::Utils::BinaryDataStream);
%shared_ptr(STI::Utils::BinaryDataStreamTarget);

%template(BinaryDataVector) std::vector< std::shared_ptr < STI::Utils::BinaryData > >;

//////////////////

/// https://stackoverflow.com/questions/9934059/swig-technique-to-wrap-unsigned-binary-data/9979340#9979340

%typemap(jni) signed char* STI::Utils::BinaryData::get "jbyteArray"
%typemap(jtype) signed char* STI::Utils::BinaryData::get "byte[]"
%typemap(jstype) signed char* STI::Utils::BinaryData::get "byte[]"
%typemap(javaout) signed char* STI::Utils::BinaryData::get {
  return $jnicall;
}

%typemap(in, numinputs=0, noblock=1) size_t* len { 
  size_t length=0;
  $1 = &length;
}
//https://docs.oracle.com/javase/7/docs/api/java/nio/ByteBuffer.html  : Direct ByteBuffer allocation (allocated and managed by c++)
//https://stackoverflow.com/questions/34460860/returning-bytebuffer-from-jni-is-copy-or-reference

//GetDirectBufferAddress:  from Java ByteBuffer to c++ (allocated in java)

%typemap(out) signed char* STI::Utils::BinaryData::get {
  size_t length = 0;
  if (arg1 != 0) {  //arg1 is this, BinaryData*
    length = arg1->length();
  }

  $result = JCALL1(NewByteArray, jenv, length);
  JCALL4(SetByteArrayRegion, jenv, $result, 0, length, $1);

  //test
  // void* DataPointer;
  // $result = JCALL2(NewDirectByteBuffer, jenv, DataPointer, length);
}



////////// getBytes()  ///////////////

// %apply unsigned char *NIOBUFFER { unsigned char* };
// unsigned char* STI::Utils::BinaryData::getBytes();

%typemap(jni) unsigned char *NIOBUFFER "jobject"  
%typemap(jtype) unsigned char *NIOBUFFER "java.nio.ByteBuffer"  
%typemap(jstype) unsigned char *NIOBUFFER "java.nio.ByteBuffer"  
%typemap(javaout) unsigned char* STI::Utils::BinaryData::getBytes {
  return $jnicall;
}
%typemap(out) unsigned char* STI::Utils::BinaryData::getBytes {
  size_t length = 0;
  unsigned char* dataTest;
  if (arg1 != 0 && arg1->get<unsigned char>(dataTest)) { 
    //arg1 is this, BinaryData*
    //calling get to ensure that BinaryData holds the correct type (length will stay zero if not)
    length = arg1->length();
  }
  void* dataPtr = static_cast<void*>(result);
  $result = JCALL2(NewDirectByteBuffer, jenv, dataPtr, length);
}
%apply unsigned char *NIOBUFFER { unsigned char* };
unsigned char* STI::Utils::BinaryData::getBytes();

/////////////////


///// getInts ////////////

/// IntBuffer:   https://stackoverflow.com/questions/35416978/jni-newdirectbytebuffer-for-java-nio-intbuffer-newdirectintbuffer


%typemap(jni) int* NIOBUFFER "jobject"  
%typemap(jtype) int* NIOBUFFER "java.nio.IntBuffer"  
%typemap(jstype) int* NIOBUFFER "java.nio.IntBuffer"  
%typemap(javaout) int* STI::Utils::BinaryData::getInts {
  return $jnicall;
}
%typemap(out) int* STI::Utils::BinaryData::getInts {
  size_t length = 0;
  int* dataTest;
  if (arg1 != 0 && arg1->get<int>(dataTest)) { 
    //arg1 is this, BinaryData*
    //calling get to ensure that BinaryData holds the correct type (length will stay zero if not)
    length = arg1->length();
  }
  void* dataPtr = static_cast<void*>(result);
  jobject byteBuf = JCALL2(NewDirectByteBuffer, jenv, dataPtr, length);


  jclass ByteBufferClass = jenv->FindClass("java/nio/ByteBuffer");
  jclass ByteOrderClass = jenv->FindClass("java/nio/ByteOrder");
  jclass IntBufferClass = jenv->FindClass("java/nio/IntBuffer");

  jmethodID asIntBuffer_methodID = jenv->GetMethodID(ByteBufferClass, "asIntBuffer", "()Ljava/nio/IntBuffer;");
  jmethodID order_methodID = jenv->GetMethodID(ByteBufferClass, "order", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;");
  jmethodID nativeOrder_methodID = jenv->GetStaticMethodID(ByteOrderClass, "nativeOrder", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteOrder;");

  jobject endian = jenv->CallStaticObjectMethod(ByteOrderClass, nativeOrder_methodID);

  jenv->CallObjectMethod(byteBuf, order_methodID, endian);
  $result = jenv->CallObjectMethod(byteBuf, asIntBuffer_methodID);

}
%apply int* NIOBUFFER { int* };
int* STI::Utils::BinaryData::getInts();




///// getShorts ////////////

%typemap(jni) short* NIOBUFFER "jobject"  
%typemap(jtype) short* NIOBUFFER "java.nio.ShortBuffer"  
%typemap(jstype) short* NIOBUFFER "java.nio.ShortBuffer"  
%typemap(javaout) short* STI::Utils::BinaryData::getShorts {
  return $jnicall;
}
%typemap(out) short* STI::Utils::BinaryData::getShorts {
  size_t length = 0;
  short* dataTest;
  if (arg1 != 0 && arg1->get<short>(dataTest)) { 
    //arg1 is this, BinaryData*
    //calling get to ensure that BinaryData holds the correct type (length will stay zero if not)
    length = arg1->length();
  }
  void* dataPtr = static_cast<void*>(result);
  jobject byteBuf = JCALL2(NewDirectByteBuffer, jenv, dataPtr, length);

  jclass ByteBufferClass = jenv->FindClass("java/nio/ByteBuffer");
  jclass ByteOrderClass = jenv->FindClass("java/nio/ByteOrder");
  jclass ShortBufferClass = jenv->FindClass("java/nio/ShortBuffer");

  jmethodID asShortBuffer_methodID = jenv->GetMethodID(ByteBufferClass, "asShortBuffer", "()Ljava/nio/ShortBuffer;");
  jmethodID order_methodID = jenv->GetMethodID(ByteBufferClass, "order", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;");
  jmethodID nativeOrder_methodID = jenv->GetStaticMethodID(ByteOrderClass, "nativeOrder", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteOrder;");

  jobject endian = jenv->CallStaticObjectMethod(ByteOrderClass, nativeOrder_methodID);

  jenv->CallObjectMethod(byteBuf, order_methodID, endian);
  $result = jenv->CallObjectMethod(byteBuf, asShortBuffer_methodID);

}
%apply short* NIOBUFFER { short* };
short* STI::Utils::BinaryData::getShorts();




/////// getDoubles() //////

%typemap(jni) double* NIOBUFFER "jobject"  
%typemap(jtype) double* NIOBUFFER "java.nio.DoubleBuffer"  
%typemap(jstype) double* NIOBUFFER "java.nio.DoubleBuffer"  
%typemap(javaout) double* STI::Utils::BinaryData::getDoubles {
  return $jnicall;
}
%typemap(out) double* STI::Utils::BinaryData::getDoubles {
  size_t length = 0;
  double* dataTest;
  if (arg1 != 0 && arg1->get<double>(dataTest)) { 
    //arg1 is this, BinaryData*
    //calling get to ensure that BinaryData holds the correct type (length will stay zero if not)
    length = arg1->length();
  }
  void* dataPtr = static_cast<void*>(result);
  jobject byteBuf = JCALL2(NewDirectByteBuffer, jenv, dataPtr, length);


  jclass ByteBufferClass = jenv->FindClass("java/nio/ByteBuffer");
  jclass ByteOrderClass = jenv->FindClass("java/nio/ByteOrder");
  jclass DoubleBufferClass = jenv->FindClass("java/nio/DoubleBuffer");

  jmethodID asDoubleBuffer_methodID = jenv->GetMethodID(ByteBufferClass, "asDoubleBuffer", "()Ljava/nio/DoubleBuffer;");
  jmethodID order_methodID = jenv->GetMethodID(ByteBufferClass, "order", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;");
  jmethodID nativeOrder_methodID = jenv->GetStaticMethodID(ByteOrderClass, "nativeOrder", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteOrder;");

  jobject endian = jenv->CallStaticObjectMethod(ByteOrderClass, nativeOrder_methodID);

  jenv->CallObjectMethod(byteBuf, order_methodID, endian);
  $result = jenv->CallObjectMethod(byteBuf, asDoubleBuffer_methodID);

}
%apply double* NIOBUFFER { double* };
double* STI::Utils::BinaryData::getDoubles();



/////// getFloats() //////

%typemap(jni) float* NIOBUFFER "jobject"  
%typemap(jtype) float* NIOBUFFER "java.nio.FloatBuffer"  
%typemap(jstype) float* NIOBUFFER "java.nio.FloatBuffer"  
%typemap(javaout) float* STI::Utils::BinaryData::getFloats {
  return $jnicall;
}
%typemap(out) float* STI::Utils::BinaryData::getFloats {
  size_t length = 0;
  float* dataTest;
  if (arg1 != 0 && arg1->get<float>(dataTest)) { 
    //arg1 is this, BinaryData*
    //calling get to ensure that BinaryData holds the correct type (length will stay zero if not)
    length = arg1->length();
  }
  void* dataPtr = static_cast<void*>(result);
  jobject byteBuf = JCALL2(NewDirectByteBuffer, jenv, dataPtr, length);


  jclass ByteBufferClass = jenv->FindClass("java/nio/ByteBuffer");
  jclass ByteOrderClass = jenv->FindClass("java/nio/ByteOrder");
  jclass FloatBufferClass = jenv->FindClass("java/nio/FloatBuffer");

  jmethodID asFloatBuffer_methodID = jenv->GetMethodID(ByteBufferClass, "asFloatBuffer", "()Ljava/nio/FloatBuffer;");
  jmethodID order_methodID = jenv->GetMethodID(ByteBufferClass, "order", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;");
  jmethodID nativeOrder_methodID = jenv->GetStaticMethodID(ByteOrderClass, "nativeOrder", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteOrder;");

  jobject endian = jenv->CallStaticObjectMethod(ByteOrderClass, nativeOrder_methodID);

  jenv->CallObjectMethod(byteBuf, order_methodID, endian);
  $result = jenv->CallObjectMethod(byteBuf, asFloatBuffer_methodID);

}
%apply float* NIOBUFFER { float* };
float* STI::Utils::BinaryData::getFloats();




%include "sti/utils/BinaryData.h"

%extend STI::Utils::BinaryData
{
    signed char* STI::Utils::BinaryData::get()
    {
        signed char* data;
        if (self->get<signed char>(data)) {
            return data;
        }

        return data;
    }
  
    unsigned char* STI::Utils::BinaryData::getBytes()
    {
        unsigned char* uData;
        char* cData;

        if (self->get<unsigned char>(uData)) {
            return uData;
        }
        else if (self->get<char>(cData)) {
            return reinterpret_cast<unsigned char*>(cData);
        }

        return uData;
    }

    int* STI::Utils::BinaryData::getInts()
    {
        int* data;
        unsigned int* uData;
        if (self->get<int>(data)) {
            return data;
        }
        else if (self->get<unsigned int>(uData)) {
            return reinterpret_cast<int*>(uData);
        }

        return data;
    }

    short* STI::Utils::BinaryData::getShorts()
    {
        short* data;
        unsigned short* uData;
        if (self->get<short>(data)) {
            return data;
        }
        else if (self->get<unsigned short>(uData)) {
            return reinterpret_cast<short*>(uData);
        }

        return data;
    }

    double* STI::Utils::BinaryData::getDoubles()
    {
        double* data;
        if (self->get<double>(data)) {
            return data;
        }

        return data;
    }
  
    float* STI::Utils::BinaryData::getFloats()
    {
        float* data;
        if (self->get<float>(data)) {
            return data;
        }

        return data;
    }
  
}

////////////// BinaryDataStream //////////////

%include "sti/utils/BinaryDataStream.h"


