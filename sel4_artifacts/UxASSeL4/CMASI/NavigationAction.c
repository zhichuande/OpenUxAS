
#include <stdlib.h>
#include <inttypes.h>
#include "common/struct_defines.h"
#include "common/conv.h"
#include "NavigationAction.h"
#include "VehicleAction.h"
void lmcp_pp_NavigationAction(NavigationAction* s) {
    printf("NavigationAction{");
    printf("Inherited from VehicleAction:\n");
    lmcp_pp_VehicleAction(&(s->super));
    printf("}");
}
size_t lmcp_packsize_NavigationAction (NavigationAction* i) {
    size_t out = 0;
    out += lmcp_packsize_VehicleAction(&(i->super));
    return out;
}
size_t lmcp_pack_NavigationAction_header(uint8_t* buf, NavigationAction* i) {
    uint8_t* outb = buf;
    if (i == NULL) {
        lmcp_pack_uint8_t(outb, 0);
        return 1;
    }
    outb += lmcp_pack_uint8_t(outb, 1);
    memcpy(outb, "CMASI", 5);
    outb += 5;
    for (size_t j = 5; j < 8; j++, outb++)
        *outb = 0;
    outb += lmcp_pack_uint32_t(outb, 32);
    outb += lmcp_pack_uint16_t(outb, 3);
    return 15;
}
void lmcp_free_NavigationAction(NavigationAction* out, int out_malloced) {
    if (out == NULL)
        return;
    lmcp_free_VehicleAction(&(out->super), 0);
    if (out_malloced == 1) {
        free(out);
    }
}
void lmcp_init_NavigationAction (NavigationAction** i) {
    if (i == NULL) return;
    (*i) = malloc(sizeof(NavigationAction));
    *(*i) = (const NavigationAction) {
        0
    };
    ((lmcp_object*)(*i)) -> type = 32;
}
int lmcp_unpack_NavigationAction(uint8_t** inb, size_t *size_remain, NavigationAction* outp) {
    if (inb == NULL || *inb == NULL) {
        return -1;
    }
    if (size_remain == NULL || *size_remain == 0) {
        return -1;
    }
    NavigationAction* out = outp;
    uint32_t tmp;
    uint16_t tmp16;
    uint8_t isnull;
    uint32_t objtype;
    uint16_t objseries;
    char seriesname[8];
    CHECK(lmcp_unpack_VehicleAction(inb, size_remain, &(out->super)))
    return 0;
}
size_t lmcp_pack_NavigationAction(uint8_t* buf, NavigationAction* i) {
    if (i == NULL) return 0;
    uint8_t* outb = buf;
    outb += lmcp_pack_VehicleAction(outb, &(i->super));
    return (outb - buf);
}
