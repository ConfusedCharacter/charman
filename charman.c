#include<ncurses.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<time.h> 
#include<math.h>

// game configuration
#define EXIT_KEY 'q'
#define GAME_SPEED 10
#define FREE_SPACE ' '
#define PLAYER_CHR 'x'
#define PLAYER_LIFE 4
#define WALL_CHR '.'
#define WALL_RANDOM 0.03
#define FRUIT_CHR '$'
#define FRUIT_BONUS 10
#define FRUITS_COUNT 15
#define FRUITS_LIFETIME 700
#define ENEMY_CHR '@'
#define ENEMY_COUNTS 5
#define ENEMY_SPEED 0.97
#define ENEMY_BOOST_TIME 60 // every 1 minutes
#define ENEMY_BOOST_SIZE 0.22

int     MAX_LINES;
int     MAX_COLS;
int     **WORLD;
int     SCORE;
int     PLAYING;
int     PLAYER_LINE;
int     PLAYER_COLUMN;
int     Player_Life;
int     ENEMY_RUNNING;
int     ENEMY_BOOSTED;
int     keyhandler();
int     rand_num(int min, int max);
int     in_range(int i,int min, int max);
int**   create2DArray();
void    draw();
void    init_game();
void    init_screen();
void    move_enemy();
void    move_player(char key);
void    game_info();
void    boost_warning();
double  random_dobule();
long long ENEMY_LAST_BOOST;

typedef struct EnemyS
{
    int l;
    int c;
} EnemyS;

typedef struct FruitS
{
    int l;
    int c;
    int a;
} FruitS;

FruitS FRUIT[FRUITS_COUNT];
EnemyS ENEMY[ENEMY_COUNTS];

int main(void)
{
    srand(time(NULL));
    init_screen();
    init_game();

    PLAYING = 1;
    while (PLAYING)
    {
        boost_warning();
        move_enemy();
        draw();
        game_info();
        if (keyhandler())
        {
            char key_received = getch();
            const char *set = "wsadq";
            if (strchr(set, key_received) != NULL){
                if (key_received == EXIT_KEY)
                {
                    // stop playing
                    PLAYING = 0;
                    // end screen
                    endwin();
                }
                move_player(key_received);
            }
        }
        napms(GAME_SPEED);
    }
    
    return EXIT_SUCCESS;
}

void init_screen()
{
    // init screen 
    initscr();
    MAX_COLS = COLS - 1;
    MAX_LINES = LINES - 1;
    
    // handle input from all screens
    cbreak();
    noecho();

    // // hide curses
    curs_set(0);

    // never wait for keys
    nodelay(stdscr, TRUE);

    // lock scroll to prevent scroll keys
    scrollok(stdscr, TRUE);

    // clear screen
    clear();
}

void init_game()
{
    time_t timestamp;
    time(&timestamp);

    // allocate memory for the 2D array world
    WORLD = create2DArray();
    Player_Life = PLAYER_LIFE;
    ENEMY_RUNNING = 1;
    ENEMY_BOOSTED = 0;
    ENEMY_LAST_BOOST = (long long)timestamp;

    // making all screen to ' ' and build walls
    for (size_t l = 0; l < MAX_LINES; l++)
    {
        for (size_t c = 0; c < MAX_COLS; c++)
        {
            if (random_dobule() > WALL_RANDOM)
            {
                WORLD[l][c] = FREE_SPACE;
            }else{
                WORLD[l][c] = WALL_CHR;
            }
            
        }
    }

    // place player 
    PLAYER_LINE = rand_num(1,MAX_LINES-1);
    PLAYER_COLUMN = rand_num(0,MAX_COLS-1);

    // Check if the player is positioned on a wall or not
    while (1)
    {
        if (WORLD[PLAYER_LINE][PLAYER_COLUMN] == WALL_CHR)
        {
            PLAYER_LINE = rand_num(1,MAX_LINES-1);
            PLAYER_COLUMN = rand_num(0,MAX_COLS-1);
        }
        else
        {
            break;
        }
    }

    // place fruits
    for (size_t i = 0; i < FRUITS_COUNT; i++)
    {
        FRUIT[i].l = rand_num(1,MAX_LINES-1);
        FRUIT[i].c = rand_num(0,MAX_COLS-1);
        FRUIT[i].a = rand_num(FRUITS_LIFETIME, FRUITS_LIFETIME*5);
    }

    // place enemies
    for (size_t i = 0; i < ENEMY_COUNTS; i++)
    {
        ENEMY[i].l = rand_num(0,MAX_LINES-1);
        ENEMY[i].c = rand_num(0,MAX_COLS-1);
    }
}

void game_info(){
    char score_result[20];
    sprintf(score_result,"Score: %d, Life: %d", SCORE, Player_Life);

    mvaddstr(1,1,score_result);
    mvaddstr(MAX_LINES,floor(MAX_COLS/2.0),"Dev: ConfusedCharacter");
    refresh();
}

void boost_warning()
{
    time_t timestamp;
    time(&timestamp);
    long long new_time = (long long)timestamp;

    if (new_time - ENEMY_LAST_BOOST >= ENEMY_BOOST_TIME / 2)
    {
        ENEMY_BOOSTED = 0;
    }

    if (new_time - ENEMY_LAST_BOOST >= ENEMY_BOOST_TIME * 2)
    {
        ENEMY_RUNNING = 0;
        ENEMY_BOOSTED = 1;
        mvaddstr(MAX_LINES/2, (MAX_COLS/2) - 10, "Warn!!! Enemy Boost Start in 5s");
        refresh();
        napms(5000);
        ENEMY_LAST_BOOST = new_time;
        ENEMY_RUNNING = 1;
    }
}

// draw walls, player, fruit
void draw(){
    // walls
    for (size_t l = 0; l < MAX_LINES; l++)
    {
        for (size_t c = 0; c < MAX_COLS; c++)
        {
            mvaddch(l, c, WORLD[l][c]);
        }
    }

    // fruit
    for (size_t i = 0; i < FRUITS_COUNT; i++)
    {
        // collect fruits by players
        if (FRUIT[i].c == PLAYER_COLUMN && FRUIT[i].l == PLAYER_LINE){
            SCORE += FRUIT_BONUS;
            FRUIT[i].l = rand_num(1,MAX_LINES-1);
            FRUIT[i].c = rand_num(0,MAX_COLS-1);
            FRUIT[i].a = rand_num(FRUITS_LIFETIME, FRUITS_LIFETIME*5);
        }
        // decrease lifetime
        FRUIT[i].a -= 1;
        if (FRUIT[i].a <= 0){
            FRUIT[i].l = rand_num(1,MAX_LINES-1);
            FRUIT[i].c = rand_num(0,MAX_COLS-1);
            FRUIT[i].a = rand_num(FRUITS_LIFETIME, FRUITS_LIFETIME*5);
        }
        mvaddch(FRUIT[i].l, FRUIT[i].c, FRUIT_CHR);
    }

    // enemies
    for (size_t i = 0; i < FRUITS_COUNT; i++)
    {
        mvaddch(ENEMY[i].l, ENEMY[i].c, ENEMY_CHR);
    }

    // player
    mvaddch(PLAYER_LINE,PLAYER_COLUMN,PLAYER_CHR);
    refresh();
}

// move player to line and columns
void move_player(char keyPressed)
{
    switch (keyPressed)
    {
    case 'w':
        if (WORLD[PLAYER_LINE-1][PLAYER_COLUMN] != WALL_CHR){
            PLAYER_LINE -= 1;
        }
        break;
    case 's':
        if (WORLD[PLAYER_LINE+1][PLAYER_COLUMN] != WALL_CHR){
            PLAYER_LINE += 1;
        }
        break;
    case 'a':
        if (WORLD[PLAYER_LINE][PLAYER_COLUMN-1] != WALL_CHR){
            PLAYER_COLUMN -= 1;
        }
        break;
    case 'd':
        if (WORLD[PLAYER_LINE][PLAYER_COLUMN+1] != WALL_CHR){
            PLAYER_COLUMN += 1;
        }
        break;
    default:
        break;
    }

    PLAYER_COLUMN = in_range(PLAYER_COLUMN, 0, MAX_COLS-1);
    PLAYER_LINE = in_range(PLAYER_LINE, 1, MAX_LINES -2);
}

// enemy smart move (AI)
void move_enemy()
{
    float enemy_pr_speed;

    for (size_t i = 0; i < ENEMY_COUNTS; i++)
    {
        if (ENEMY[i].l == PLAYER_LINE && ENEMY[i].c == PLAYER_COLUMN)
        {
            if (Player_Life <= 0)
            {
                mvaddstr(MAX_LINES/2,MAX_COLS/2,"GAME OVER!");
                refresh();
                napms(4000);
                PLAYING = 0;
                endwin();
                exit;
            }
            Player_Life -= 1;
            ENEMY[i].l = rand_num(0,MAX_LINES-1);
            ENEMY[i].c = rand_num(0,MAX_COLS-1);
        }
        if (ENEMY_BOOSTED) 
        { enemy_pr_speed = ENEMY_SPEED - ENEMY_BOOST_SIZE; }
        else 
        { enemy_pr_speed = ENEMY_SPEED; }

        if (ENEMY_RUNNING)
        {
            if (PLAYER_LINE > ENEMY[i].l)
            {
                if (random_dobule() > enemy_pr_speed)
                {
                    ENEMY[i].l += 1;
                }
            }

            if (PLAYER_LINE < ENEMY[i].l)
            {
                if (random_dobule() > enemy_pr_speed)
                {
                    ENEMY[i].l -= 1;
                }
            }

            if (PLAYER_COLUMN < ENEMY[i].c)
            {
                if (random_dobule() > enemy_pr_speed)
                {
                    ENEMY[i].c -= 1;
                }
            }

            if (PLAYER_COLUMN > ENEMY[i].c)
            {
                if (random_dobule() > enemy_pr_speed)
                {
                    ENEMY[i].c += 1;
                }
            }
        }
    }
}

int in_range(int i,int min, int max)
{
    if (i < min) { return min; }
    else if (i > max) { return max; }
    else { return i; }
}

int keyhandler(void)
{
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

// get random number between two number
int rand_num(int min, int max)
{
    return rand() % (max - min + 1 ) + min;
}

// get random number between 0,1
double random_dobule()
{
    return (double)rand() / (double)RAND_MAX;
}


int** create2DArray()
{
    int **array = (int **)malloc(MAX_LINES * sizeof(int *));
    for (int i = 0; i < MAX_LINES; i++)
    {   
        array[i] = (int *)malloc(MAX_COLS * sizeof(int));
    }
    return array;
} 

