#ifndef GM_PARSER_TYPE_H
#define GM_PARSER_TYPE_H

typedef enum gm_type
{
    GM_TYPE_S32,

    GM_TYPE_GUARDIAN_ANGEL
} gm_type_e;

const char *gm_type_to_string(const gm_type_e type);

#endif // GM_PARSER_TYPE_H
