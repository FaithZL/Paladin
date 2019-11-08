 #include <iostream>

// #include <glog/logging.h>

 int test_glog_main(const char* argv) {
     FLAGS_alsologtostderr = 1;
     google::InitGoogleLogging(argv);

     LOG(INFO) << "I am INFO!";
     LOG(WARNING) << "I am WARNING!";
     LOG(ERROR) << "I am ERROR!";
//     LOG(FATAL) << "I am FATAL!";
     VLOG(1) << "TEST";

     return 0;
 }
