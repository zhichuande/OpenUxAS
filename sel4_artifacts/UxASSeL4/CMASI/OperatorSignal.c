
#include <stdlib.h>
#include <inttypes.h>
#include "common/struct_defines.h"
#include "common/conv.h"
#include "OperatorSignal.h"
#include "KeyValuePair.h"
void lmcp_pp_OperatorSignal(OperatorSignal* s) {
    printf("OperatorSignal{");
    printf("Signals: ");
    printf("[");
    for (uint32_t index = 0; index < s->Signals_ai.length; index++) {
        lmcp_pp_KeyValuePair((s->Signals[index]));
        printf(",");
    }
    printf("\n");
    printf("}");
}
size_t lmcp_packsize_OperatorSignal (OperatorSignal* i) {
    size_t out = 0;
    out += 2;
    for (uint32_t index = 0; index < i->Signals_ai.length; index++) {
        out += 15 + lmcp_packsize_KeyValuePair(i->Signals[index]);
    }
    return out;
}
size_t lmcp_pack_OperatorSignal_header(uint8_t* buf, OperatorSignal* i) {
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
    outb += lmcp_pack_uint32_t(outb, 38);
    outb += lmcp_pack_uint16_t(outb, 3);
    return 15;
}
void lmcp_free_OperatorSignal(OperatorSignal* out, int out_malloced) {
    if (out == NULL)
        return;
    if (out->Signals != NULL) {
        for (uint32_t index = 0; index < out->Signals_ai.length; index++) {
            if (out->Signals[index] != NULL) {
                lmcp_free_KeyValuePair(out->Signals[index], 1);
            }
        }
        free(out->Signals);
    }
    if (out_malloced == 1) {
        free(out);
    }
}
void lmcp_init_OperatorSignal (OperatorSignal** i) {
    if (i == NULL) return;
    (*i) = malloc(sizeof(OperatorSignal));
    *(*i) = (const OperatorSignal) {
        0
    };
    ((lmcp_object*)(*i)) -> type = 38;
}
int lmcp_unpack_OperatorSignal(uint8_t** inb, size_t *size_remain, OperatorSignal* outp) {
    if (inb == NULL || *inb == NULL) {
        return -1;
    }
    if (size_remain == NULL || *size_remain == 0) {
        return -1;
    }
    OperatorSignal* out = outp;
    uint32_t tmp;
    uint16_t tmp16;
    uint8_t isnull;
    uint32_t objtype;
    uint16_t objseries;
    char seriesname[8];
    CHECK(lmcp_unpack_uint16_t(inb, size_remain, &tmp16))
    tmp = tmp16;
    (out)->Signals = malloc(sizeof(KeyValuePair*) * tmp);
    if (out->Signals==0) {
        return -1;
    }
    out->Signals_ai.length = tmp;
    for (uint32_t index = 0; index < out->Signals_ai.length; index++) {
        CHECK(lmcp_unpack_uint8_t(inb, size_remain, &isnull))
        if (isnull == 0 && inb != NULL) {
            out->Signals[index] = NULL;
        } else if (inb != NULL) {
            CHECK(lmcp_unpack_8byte(inb, size_remain, seriesname))
            CHECK(lmcp_unpack_uint32_t(inb, size_remain, &objtype))
            CHECK(lmcp_unpack_uint16_t(inb, size_remain, &objseries))
            lmcp_init_KeyValuePair(&(out->Signals[index]));
            CHECK(lmcp_unpack_KeyValuePair(inb, size_remain, (out->Signals[index])))
        }
    }
    return 0;
}
size_t lmcp_pack_OperatorSignal(uint8_t* buf, OperatorSignal* i) {
    if (i == NULL) return 0;
    uint8_t* outb = buf;
    outb += lmcp_pack_uint16_t(outb, i->Signals_ai.length);
    for (uint32_t index = 0; index < i->Signals_ai.length; index++) {
        if (i->Signals[index]==NULL) {
            outb += lmcp_pack_uint8_t(outb, 0);
        } else {
            outb += lmcp_pack_uint8_t(outb, 1);
            memcpy(outb, "CMASI", 5);
            outb += 5;
            for (size_t j = 5; j < 8; j++, outb++)
                *outb = 0;
            outb += lmcp_pack_uint32_t(outb, 2);
            outb += lmcp_pack_uint16_t(outb, 3);
            outb += lmcp_pack_KeyValuePair(outb, i->Signals[index]);
        }
    }
    return (outb - buf);
}
