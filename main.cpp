#include "4over6_util.h"
#include "test_server_nat.h"

 int main(int argc, char** argv) {
    if(strcmp(argv[1],"1") == 0) {
        do_server();
    }
    else if(strcmp(argv[1],"2") == 0){
        do_client();
    }
    else {
        do_test_server();
    }
    return 0;
}


