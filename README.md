# simpleprotobuf
a simple data encoder support for Google's protocol buffers.
it's a new lib which can run without google protobuf.
importantly, it can encode or decode the datas from google protobuf.

**gpbencode.h** 
>类gpbdecoder和gpbencoder分别实现了数据的解码与编码，编码方式完全按照Google's protocol buffers源码实现，所以完全兼容现有的Google's protocol buffers版本。
支持google protobuf中类型sint32 int64 uint32 uint64 float double bool string及对应repeated map
不支持int32 sint64 fixed32 fixed64(也可以实现)

**gpbgenerator.h**
>类gpbgenerator的作用是把.proto文件编译成c++文件，该生成的c++文件依赖于上面的gpbdecoder和gpbencoder，从而脱离了对google protobuf的依赖。该类同样也不依赖于google protobuf库。

sampley.h中my.proto aa.proto是生成的示例。
