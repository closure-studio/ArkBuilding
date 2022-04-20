#include "c_albcexample.h"
#include "albc/calbc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef ERROR
#endif

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#define ALBC_CHECK(lbl, stm, e)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        e = NULL;                                                                                                      \
        stm;                                                                                                           \
        if (!call_success(e, __FILE__ ":" S__LINE__ ": " #stm))                                                        \
            goto lbl;                                                                                                  \
    } while (0)

#define ALBC_COLLECTION_TO_ARRAY(type, collection, name, len_name)                                                     \
    size_t len_name = AlbcCollectionGetCount(collection);                                                              \
    type *name = malloc(len_name * sizeof(type));                                                                      \
    AlbcCollectionForEach(collection, (AlbcForEachCallback)foreach_callback_impl, name);

#define ARRAY_ELEMS(x) (sizeof(x) / sizeof(x[0]))

const char *names[] = {
    u8"能天使", u8"德克萨斯", u8"拉普兰德",
    u8"孑", u8"雪雉", u8"空弦",
    u8"石棉", u8"红云", u8"刻俄柏",
    u8"泡泡", u8"豆苗", u8"火神",
    u8"清流"};

const char *skill_lists[][2] = {
    {u8"物流专家", u8""}, {u8"恩怨", u8"默契"},   {u8"醉翁之意·β", u8""},
    {u8"摊贩经济", u8""}, {u8"天道酬勤·β", u8""}, {u8"虔诚筹款·β", u8""},
    {u8"特立独行", u8"探险者"}, {u8"拾荒者", u8"回收利用"}, {u8"“都想要”", u8"“等不及”"},
    {u8"囤积者", u8"大就是好！"}, {u8"磐蟹·豆豆", u8"磐蟹·阿盘"}, {u8"工匠精神·β", u8""},
    {u8"清洁能源", u8"再生能源"}
};

const char *room_names[] = {
    u8"yi_lou_mao_yi_zhan",
    u8"2F贸易站",
    u8"三楼贸易站",
    u8"this is a long name",
    u8"this is a really long name",
    u8"this is a really really long name",
};

const AlbcRoomType room_types[] = {ALBC_ROOM_TRADING,     ALBC_ROOM_TRADING,     ALBC_ROOM_TRADING,
                                   ALBC_ROOM_MANUFACTURE, ALBC_ROOM_MANUFACTURE, ALBC_ROOM_MANUFACTURE};

const int room_prod_or_order_types[] = {
    ALBC_ROOM_ORDER_GOLD, ALBC_ROOM_ORDER_GOLD, ALBC_ROOM_ORDER_ORUNDUM,
    ALBC_ROOM_PROD_GOLD,  ALBC_ROOM_PROD_GOLD,  ALBC_ROOM_PROD_ORIGINIUM_SHARD_ORIROCK,
};

const int room_slot_counts[] = {3, 3, 3, 3, 3, 3};

bool call_success(AlbcException *e, const char *error_msg)
{
    if (e)
    {
        printf("%s: %s\n", error_msg, e->what);
        AlbcFreeException(e);
        return false;
    }
    return true;
}

typedef void *AlbcObjectHandle;
typedef void (*AlbcForEachCallback)(int, const void *, void *);
void foreach_callback_impl(int i, const AlbcObjectHandle *item, AlbcObjectHandle arr[])
{
    arr[i] = *item;
}

void c_albc_example_main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    AlbcModel *model = NULL;
    AlbcResult *result = NULL;
    AlbcException *e = NULL;

    AlbcDoLog(ALBC_LOG_LEVEL_INFO, "Starting ALBC C example", NULL);
    AlbcSetLogLevel(ALBC_LOG_LEVEL_DEBUG, NULL);

    ALBC_CHECK(assets_fail, AlbcInitCharacterTableFromFile("../test/character_table.json", &e), e);
    ALBC_CHECK(assets_fail, AlbcInitBuildingDataFromFile("../test/building_data.json", &e), e);
    ALBC_CHECK(model_fail, model = AlbcModelFromEmpty(&e), e);

    const int char_cnt = ARRAY_ELEMS(names);
    printf("adding %d characters\n", char_cnt);
    for (int i = 0; i < char_cnt; i++)
    {
        AlbcCharacter *c = AlbcCharacterFromName(names[i]);
        ALBC_CHECK(char_fail, AlbcCharacterSetMorale(c, 24, &e), e);
        bool prepare_ok = false;
        // 加入技能
        for (int j = 0; j < 2; j++)
        {
            if (skill_lists[i][j][0] == '\0')
                break;

            ALBC_CHECK(char_fail, AlbcCharacterAddSkillByName(c, skill_lists[i][j], &e), e);
        }
        ALBC_CHECK(char_fail, prepare_ok = AlbcCharacterPrepare(c, &e), e);
        if (!prepare_ok)
            goto char_fail;

        printf("%s: prepared\n", names[i]);
        ALBC_CHECK(char_fail, AlbcModelAddChar(model, c, &e), e);
        printf("%s: added\n", names[i]);
    char_fail:
        AlbcCharacterDestroy(c); // 对象所有权已移交给model，释放对象引用。
    }

    const int room_cnt = ARRAY_ELEMS(room_names);
    for (int i = 0; i < room_cnt; i++)
    {
        AlbcRoom *room = AlbcRoomFromType(room_names[i], room_types[i]);
        bool prepare_ok = false;
        AlbcRoomParamType type =
            room_types[i] == ALBC_ROOM_MANUFACTURE ? ALBC_ROOM_PARAM_PRODUCT_TYPE : ALBC_ROOM_PARAM_ORDER_TYPE;

        ALBC_CHECK(room_fail, AlbcRoomSetDblParam(room, type, room_prod_or_order_types[i], &e), e);
        ALBC_CHECK(room_fail, AlbcRoomSetDblParam(room, ALBC_ROOM_PARAM_SLOT_COUNT, room_slot_counts[i], &e), e);
        ALBC_CHECK(room_fail, prepare_ok = AlbcRoomPrepare(room, &e), e);
        if (!prepare_ok)
            goto room_fail;

        printf("%s: prepared\n", room_names[i]);
        ALBC_CHECK(room_fail, AlbcModelAddRoom(model, room, &e), e);
        printf("%s: added\n", room_names[i]);
    room_fail:
        AlbcRoomDestroy(room); // 同上
    }

    ALBC_CHECK(model_fail, AlbcModelSetDblParam(model, ALBC_MODEL_PARAM_DURATION, 3600 * 16, &e), e);
    ALBC_CHECK(model_fail, AlbcModelSetDblParam(model, ALBC_MODEL_PARAM_SOLVE_TIME_LIMIT, 60, &e), e);
    ALBC_CHECK(result_fail, result = AlbcModelGetResult(model, &e), e);

    printf("Done!\n");
    AlbcCollection *room_results = AlbcResultGetRooms(result);
    ALBC_COLLECTION_TO_ARRAY(AlbcRoomResult *, room_results, room_result_arr, room_result_cnt)
    printf("%zu room_results\n", room_result_cnt);
    for (int i = 0; i < room_result_cnt; i++)
    {
        AlbcRoomResult *room_res = room_result_arr[i];
        AlbcString *room_id = AlbcRoomResultGetIdentifier(room_res);
        printf("%s: %.2f in %.2fs\n",
               AlbcStringGetContent(room_id),
               AlbcRoomResultGetScore(room_res),
               AlbcRoomResultGetDuration(room_res));
        AlbcStringDestroy(room_id);

        AlbcCollection *char_ids = AlbcRoomResultGetCharacterIdentifiers(room_res);
        ALBC_COLLECTION_TO_ARRAY(AlbcString *, char_ids, char_ids_array, char_id_cnt)
        for (int j = 0; j < char_id_cnt; j++)
        {
            AlbcString *char_id = char_ids_array[j];
            printf("\t-%s\n", AlbcStringGetContent(char_id));
        }

        free(char_ids_array);
        AlbcCollectionDestroy(char_ids);
    }
    free(room_result_arr);
    AlbcCollectionDestroy(room_results);

result_fail:
    AlbcResultDestroy(result);

model_fail:
    AlbcModelDestroy(model);

assets_fail:
    AlbcFlushLog(NULL);
}