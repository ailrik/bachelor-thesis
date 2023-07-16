#include "dlink.h"

int main() {
    dlink_t* header = dlinkEmpty();

    //It just works man
    dlinkKill(header);

    return 0;
}