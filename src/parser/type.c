#include <gamma/parser/type.h>
#include <stdio.h>

const char *gm_type_to_string(const gm_type_e type)
{
    static_assert(GM_TYPE_GUARDIAN_ANGEL == 1, "gm_type_to_string(): type enum size changed");

    switch (type)
    {
    case GM_TYPE_S32:
        return "s32";
    default:
        fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
        return "<UNREACHABLE>";
    }
}
