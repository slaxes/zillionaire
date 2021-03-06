#include "common.h"
#include "richman.h"
#include "display.h"
#include "housedeal.h"
#include "api.h"

int init_money;
GAME *game_state = NULL;
char role_name[4][16] = {"Q钱夫人", "A阿士伯", "J金贝贝", "S孙小美"};

int main()
{
    bool status;
    bool if_debug = false;

    game_state = (GAME *)malloc(sizeof(GAME));

    CLEANSCREEN();
    printf("\nWelcome to Rich Man!\n");

    while (1) {
        char return_back;
        printf("您是否要恢复之前游戏进度(y/n): ");
        return_back = UsFgetsChar();
        if (return_back == 'y' || return_back == 'Y') {
            // TODO: recovery progress have saved!
            game_load();
            goto RECOVER;
        }
        if(return_back == 'n' || return_back == 'N') {
            // player choose to begin a new game
            break;
        }
        if(return_back == 't' || return_back == 'T') {
            // can preset order
            if_debug = true;
            printf("\n进入debug模式\n");
            break;
        }
    }

    if (!if_debug) {
        SetInitMoney();

        status = ChooseRole(if_debug);
        if(!status){
            printf("exit game ..\n");
            goto Exit;
        }
    }

    InitMap();

    if (if_debug){
        printf("请输入预置指令\n");
        HandlePreset(game_state);
    }

RECOVER:
    DisplayMap(game_state);

    CLEAN_TIPS();
    GameStart();

    if(if_debug){
        Dump(game_state);
    }

    // TODO free memory;

    Exit:
    return 0;
}

void SetInitMoney()
{
    int money;
    while (1){
        printf("请设置玩家初始资金(1000~50000):");
        money = UsFgetsNum();
        if(money == 0){
            init_money = INITIAL_MONEY;
            return;
        }
        if(money >= 1000 && money <= 50000){
            init_money = money;
            return;
        }
    }
}

bool ChooseRole(int if_debug)
{
    int player_num;
    int exit_role[4] = {0,0,0,0};
    PLAYER *temp_player;
    bool ret = false;

    if (!if_debug){
        printf("请从以下角色中选择2~4位不重复的角色\n 1.钱夫人，2.阿土伯，3.金贝贝，4.孙小美:");
        while(!(player_num = UsFgetsRole(exit_role))){
            for(int i = 0; i< 4; ++i){
                exit_role[i] = 0;
            }
            printf("输入错误，请重新输入:");
        }
    }
    else {
        char temp[5] = {};
        scanf("%s", temp);
        player_num = (int)strlen(temp);
        for (int i = 0; i < strlen(temp); i++)
            exit_role[i] = Flag2Id(temp[player_num - 1 - i]);
    }

    game_state->player_num = player_num;
    // choose roles
    for(int i = 0; i < player_num; ++ i){

        temp_player = (PLAYER *)malloc(sizeof(PLAYER));
        INIT_PLAYER_HOUSE(temp_player);
        temp_player->player_id = exit_role[i];
        memcpy(temp_player->player_name, role_name[exit_role[i]-1], 16);
        temp_player->block_num = 0;
        temp_player->robot_num = 0;
        temp_player->bomb_num = 0;
        temp_player->cur_pos = 0;
        temp_player->money = init_money;
        temp_player->point = 0;
        temp_player->sleep_time = 0;
        temp_player->no_rent_time = 0;
        temp_player->next = game_state->player;
        game_state->player = temp_player;

    }

    game_state->current_player = game_state->player;
    ret = true;

    return ret;
}

void InitMap()
{
    int i;
    for(i = 1; i < 69; ++i){
        game_state->map[i].ToolType = T_NO;
        game_state->map[i].house_level = 0;
        game_state->map[i].house_owner_id = 0;
        game_state->map[i].house_flag = NULL;
        game_state->map[i].map_value = 0;
    }

    for(i = 1; i < 29; ++i) {
        game_state->map[i].map_value = 200;
    }

    for(; i < 35; ++i) {
        game_state->map[i].map_value = 500;
    }

    for (; i < 64; ++i) {
        game_state->map[i].map_value = 300;
    }

    for(; i < 70; ++i) {
        ADD_HOUSE_FLAG(i,MINERAL);
    }

    ADD_HOUSE_FLAG(0, START);
    ADD_HOUSE_FLAG(14, PARK);
    ADD_HOUSE_FLAG(28, TOOL);
    ADD_HOUSE_FLAG(35, GIFT);
    ADD_HOUSE_FLAG(49, PARK);
    ADD_HOUSE_FLAG(63, MAGIC);

}


void GameStart()
{
    char com_buf[BUF_SIZE];
    bool status;
    bool is_use_tool = false;
    bool house_op = false;
    while (1) {
        HouseShow();
        DisplayMap(game_state);

        // player play this round
        if (!game_state->current_player->sleep_time) {
            scanf("%s", com_buf);

            if(STR_EQU(COMMAND_ROLL, com_buf)){
                //Judge whether it is a god of wealth
                if(game_state->current_player->no_rent_time > 0){
                    game_state->current_player->no_rent_time -= 1;
                }
                getchar(); // clean input buffer.
                cmd_roll();

                status = AbleBuyHouse(game_state->current_player->cur_pos, game_state);
                if(status && !house_op){
                    cmd_buyhouse();
                }
                if(house_op){
                    printf("\n%s此轮已进行过房屋操作.", game_state->current_player->player_name);
                }

                // When the remaining player number is one, game over.
                if(GameEnd()){
                    goto Exit;
                }

                CHECK_OUT_PLAYER(game_state);
                is_use_tool = false;
                house_op = false;
            }
            else if (STR_EQU(COMMAND_QUWRY, com_buf)){
                cmd_query();
            }
            else if (STR_EQU(COMMAND_HELP, com_buf)) {
                cmd_help();
            }

            else if (STR_EQU(COMMAND_SELLHOUSE, com_buf)) {
                CLEAN_TIPS();
                if(house_op){
                    printf("%s:\n  此回合您已经进行过房屋操作.", game_state->current_player->player_name);
                }
                else{
                    house_op = cmd_sellhouse();
                }
            }
            else if (STR_EQU(COMMAND_STEP, com_buf)) {
                //Judge whether it is a god of wealth
                if(game_state->current_player->no_rent_time > 0){
                    game_state->current_player->no_rent_time -= 1;
                }
                getchar(); // clean input buffer.
                cmd_step();

                status = AbleBuyHouse(game_state->current_player->cur_pos, game_state);
                if(status && !house_op){
                    cmd_buyhouse();
                }
                if(house_op){
                    printf("\n%s此轮已进行过房屋操作.", game_state->current_player->player_name);
                }

                // When the remaining player number is one, game over.
                if(GameEnd()){
                    goto Exit;
                }

                CHECK_OUT_PLAYER(game_state);
                is_use_tool = false;
                house_op = false;
            }
            //use the bomb
//            else if(STR_EQU(COMMAND_BOMB, com_buf)){
//               if(is_use_tool){
//                   while(getchar() != '\n');
//
//                   CLEAN_TIPS();
//                   printf("\n 你此回合已经使用了道具\n");
//               }
//               else{
//                   getchar();
//                   is_use_tool = cmd_place_bomb();
//               }
//
//            }
            else if(STR_EQU(COMMAND_BLOCK,com_buf)){
                if(is_use_tool){
                    while(getchar() != '\n');

                    CLEAN_TIPS();
                    printf("\n 你此回合已经使用了道具\n");
                }
                else{
                    getchar();
                    is_use_tool = cmd_place_block();
               }
            }
            else if(STR_EQU(COMMAND_ROBOT,com_buf)){
                if(is_use_tool){
                    while(getchar() != '\n');

                    CLEAN_TIPS();
                    printf("\n 你此回合已经使用了道具\n");
                }
                else{
                    getchar();
                    is_use_tool = cmd_use_robot();

                }
            }
            else if (STR_EQU(COMMAND_QUIT, com_buf)) {
                cmd_quit();
                goto Exit;
            }
            else {
                cmd_help();
                continue;
            }
        }

        // player skip this round
        else {
            -- game_state->current_player->sleep_time;
            CLEAN_TIPS();
            printf("%s被人陷害,跳过此轮.",game_state->current_player->player_name);
            CHECK_OUT_PLAYER(game_state);
            house_op = false;
            is_use_tool = false;
        }
    }
    Exit:
    return;

}

bool GameEnd()
{
    bool ret = false;

    if(game_state->player_num == 1){
        printf("\n游戏结束.\n");
        printf("恭喜%s获得胜利\n\n\n\n\n\n\n\n\n\n\n\n\n",game_state->current_player->player_name);
        ret = true;
    }

    return ret;
}

void cmd_roll()
{
    short step;
    char flag;
    bool bomb_status = false;

    GET_STEP(step);

    bomb_status = BlockedOrBombed(step, game_state);

    if(bomb_status){
        goto Exit;
    }

    GET_PLAYER_FLAG(game_state, flag);

    // player leave current map block
    DEL_HOUSE_FLAG(game_state->current_player->cur_pos, flag);
    
    // player move
    game_state->current_player->cur_pos = (game_state->current_player->cur_pos + step) % 70;

    // player arrive new map block
    ADD_HOUSE_FLAG(game_state->current_player->cur_pos, flag);

    // update map
    DisplayMap(game_state);
    CLEAN_TIPS();
    printf("%s获得的步数为：%d\n",game_state->current_player->player_name, step);

    Exit:
    return;
}

bool cmd_place_block()
{
    bool ret = false;
    int target_block;

    CLEAN_TIPS();
    while(1){
        target_block = UsFgetsNum();

        if(target_block >= -10 && target_block <= 10){
            break;
        }
        printf("请输入正确的目标的距离(-10~10):");
    }

    if(game_state->current_player->block_num > 0){
        target_block = target_block + game_state->current_player->cur_pos;

        if(game_state->map[target_block].house_flag != NULL){
            printf("该地块不能放置道具.");
            return ret;
        }

        if(game_state->map[target_block].ToolType == T_NO){
            game_state->current_player->block_num --;
            game_state->map[target_block].ToolType = T_BLOCK;
            ADD_HOUSE_FLAG(target_block, ROADBLOCK);

            DisplayMap(game_state);

            CLEAN_TIPS();
            printf("\n%s在地图块 %d 放置了路障",game_state->current_player->player_name, target_block);
            ret = true;

            return ret;
        }
        else{
            printf("\n地图块 %d 已经放置了其他道具\n",target_block);
            return ret;
        }
        
    }
    else{
        printf("%s你没有路障了，想什么呢\n",game_state->current_player->player_name);
        return ret;
    }

}

bool cmd_place_bomb()
{
    int target_block;
    bool ret = false;

    CLEAN_TIPS();
    while(1){
        target_block = UsFgetsNum();

        if(target_block >= -10 && target_block <= 10){
            break;
        }
        printf("请输入正确的目标的距离(-10~10):");
    }

    if(game_state->current_player->bomb_num > 0){
        target_block=target_block + game_state->current_player->cur_pos;

        if(game_state->map[target_block].house_flag != NULL){
            printf("该地块不能放置道具.");
            return ret;
        }

        if(game_state->map[target_block].house_flag == T_NO){
            game_state->current_player->bomb_num--;
            game_state->map[target_block].ToolType = T_BOMB;
            ADD_HOUSE_FLAG(target_block, BOMB);

            DisplayMap(game_state);
            CLEAN_TIPS();
            printf("\n%s在地图块 %d 放置了炸弹",game_state->current_player->player_name,target_block);
            ret = true;

            return ret;
        }
        else{
            printf("\n地图块 %d 已经放置了其他道具\n",target_block);
            return ret;
        }
        
    }
    else{
        printf("%s你没有炸弹了，想什么呢",game_state->current_player->player_name);
        return ret;
    }
}
 
bool cmd_use_robot()
{
    bool ret = false;
    int pos_range = game_state->current_player->cur_pos;

    CLEAN_TIPS();
    if(game_state->current_player->robot_num <= 0){
        printf("%s你没有机器人了，想什么呢",game_state->current_player->player_name);
        goto Exit;
    }

    game_state->current_player->robot_num --;

    for(int i = 0; i <= 10; ++i){
        if(game_state->map[pos_range+i].house_flag != NULL){
            if(game_state->map[pos_range+i].house_flag->flag == BOMB){
                DEL_HOUSE_FLAG(pos_range + i, BOMB);
            }
            else if(game_state->map[pos_range +i].house_flag->flag == ROADBLOCK){
                DEL_HOUSE_FLAG(pos_range + i, ROADBLOCK);
            }
        }
    }
    DisplayMap(game_state);

    CLEAN_TIPS();
    printf("\n%s已经使用了机器娃娃.",game_state->current_player->player_name);
    ret = true;

    Exit:
    return  ret;
}

void cmd_step()
{
    int step;
    char flag;
    bool bomb_status;

    CLEAN_TIPS();
    while((step = UsFgetsNum()) == 0){
        printf("请输入距离!");
    }

    bomb_status = BlockedOrBombed(step, game_state);

    if(bomb_status){
        goto Exit;
    }

    GET_PLAYER_FLAG(game_state, flag);

    // player leave current map block
    DEL_HOUSE_FLAG(game_state->current_player->cur_pos, flag);

    // player move
    game_state->current_player->cur_pos = (game_state->current_player->cur_pos + step) % 70;

    // player arrive new map block
    ADD_HOUSE_FLAG(game_state->current_player->cur_pos, flag);

    // update map
    DisplayMap(game_state);
    CLEAN_TIPS();
    printf("\n%s获得的步数为：%d\n", game_state->current_player->player_name, step);

    Exit:
    return;
}

void cmd_query()
{
    PlayQuery(game_state);
}

void cmd_help()
{
    PlayHelp(game_state);
    CLEAN_TIPS();
    printf("\n\tThanks for playing.\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    game_save();
=======
    printf("\n Thanks for playing.");    
    // printf("\n This game status have been stored");
    // StoreTheGameState();
    printf("\n please press any key to continue.");
    PAUSE();
>>>>>>> 97edbcf3b5eed79acdd5d7b9e1baa5205aae22c8
}

void cmd_buyhouse()
{
    BuyHouse(game_state->current_player->player_id, game_state->current_player->cur_pos, game_state);
}

bool cmd_sellhouse()
{
    bool ret;
    int h_pos;
    getchar();
    h_pos = UsFgetsNum();
    ret = SellHouse(game_state, h_pos);
    return  ret;
}

void HouseShow()
{
    H_FLAG *first_pos_flag = game_state->map[game_state->current_player->cur_pos].house_flag;
    H_FLAG *flag = first_pos_flag;
    char player_flag ;
    GET_PLAYER_FLAG(game_state, player_flag);
    while(flag->flag != player_flag && flag->next) {
        flag = flag->next;
    }
    char current_flag = flag->flag;
    char first_flag = first_pos_flag->flag;
    flag->flag = first_flag;
    first_pos_flag ->flag = current_flag;
}

void game_save()
{
    FILE *fp;
    size_t fw;
    PLAYER *tmp_ply;

    fp = fopen("richman","wb");
    if(!fp){
        printf("Open file failed!\n");
        goto Exit;
    }

    fw = fwrite(game_state, sizeof(GAME), 1, fp);
    if(!fw){
        printf("Write map data error.\n");
        goto Exit;
    }

    tmp_ply = game_state->player;
    for (int i = 0; i < game_state->player_num; ++ i){
        fw = fwrite(tmp_ply, sizeof(PLAYER), 1, fp);
        if(!fw){
            printf("Write palyer data error.\n");
            goto Exit;
        }
        tmp_ply = tmp_ply->next;
    }

    fw = fwrite(game_state->current_player, sizeof(PLAYER), 1,fp);
    if(!fw){
        printf("Write current player failde.\n");
    }

    Exit:
    if(fp){
        fclose(fp);
    }
    return;
}

void game_load()
{
    FILE *fp;
    size_t fr;
    PLAYER *tmp_ply;
    PLAYER *cur_ply;

    fp = fopen("richman", "rb");
    if(!fp){
        printf("Open file failed.\n");
        goto Exit;
    }

    fr = fread(game_state, sizeof(GAME), 1, fp);
    if(!fr){
        printf("Read map failed\n");
        goto Exit;
    }
    game_state->player = NULL;
    game_state->current_player = NULL;

    for(int i = 0; i < 70; ++ i){
        game_state->map[i].house_flag = NULL;
    }

    for(int i = 64; i < 70; ++i) {
        ADD_HOUSE_FLAG(i,MINERAL);
    }

    ADD_HOUSE_FLAG(0, START);
    ADD_HOUSE_FLAG(14, PARK);
    ADD_HOUSE_FLAG(28, TOOL);
    ADD_HOUSE_FLAG(35, GIFT);
    ADD_HOUSE_FLAG(49, PARK);
    ADD_HOUSE_FLAG(63, MAGIC);


    for(int i = 0; i < game_state->player_num; ++i){
        tmp_ply = (PLAYER *)malloc(sizeof(PLAYER));
        fr = fread(tmp_ply, sizeof(PLAYER), 1, fp);
        if(!fr){
            printf("Read player failed.\n");
            goto Exit;
        }
        tmp_ply->next = NULL;
        INIT_PLAYER_HOUSE(tmp_ply);

        cur_ply = game_state->player;
        if(!cur_ply){
            game_state->player = tmp_ply;
        }
        else{
            while(cur_ply->next){
                cur_ply = cur_ply->next;
            }
            cur_ply->next = tmp_ply;
        }
        ADD_HOUSE_FLAG(tmp_ply->cur_pos, tmp_ply->player_name[0]);
    }

    // TODO: how to recover it
    game_state->current_player = game_state->player;

    for(int i = 0; i < 70; ++ i){
        if(game_state->map[i].ToolType){
            ADD_HOUSE_FLAG(i, T_BLOCK);
        }
        if(game_state->map[i].house_owner_id){
            tmp_ply = game_state->current_player;
            while (tmp_ply){
                if(tmp_ply->player_id == game_state->map[i].house_owner_id){
                    tmp_ply->house_id[i] = &(game_state->map[i]);
                }
                tmp_ply = tmp_ply->next;
            }
        }
    }

    cur_ply = (PLAYER *)malloc(sizeof(PLAYER));
    fr = fread(cur_ply, sizeof(PLAYER), 1, fp);
    if(!fr){
        printf("Read current player failed.");
        goto Exit;
    }

    tmp_ply = game_state->player;
    while(tmp_ply){
        if(tmp_ply->player_id == cur_ply->player_id){
            game_state->current_player = tmp_ply;
            break;
        }
        tmp_ply = tmp_ply->next;
    }

    Exit:
    if(fp){
        fclose(fp);
    }

    return;
}
