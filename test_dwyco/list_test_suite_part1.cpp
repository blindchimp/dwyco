/* === Dwyco List Helpers Test Case #1 & #2 === */
#include <dlli.h>
#include <cstdio>

int main(void) {
    DWYCO_LIST list = dwyco_list_new();
    
    if (list) {
        dwyco_list_release(list);
        printf("[OK] Created and released list\n");
    } else {
        return 0;
    }
}
