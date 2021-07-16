#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <string.h>
#include <easyx.h>
#include <sstream>
#include <map>
#include <math.h>

#define WIDTH 440
#define HEIGHT 650
#define SCORE 5                     //得分是合成数字的五倍
#pragma comment(lib, "MSIMG32.LIB") //TransparentBlt()贴图

// 方块的状态
enum State
{
    EXIST,  // 存在
    DESTORY // 销毁
};

// 二维向量，用于表示位置或者大小
struct Vector2
{
    float x;
    float y;
};

void mainInterface();
void Introduction();
void Developers();
void start();

int sgn(float d);

bool canMoveUp();
bool canMoveDown();
bool canMoveLeft();
bool canMoveRight();
bool Judge();
void Up();
void Down();
void Left();
void Right();
void putNew();

void Update(float deltaTime);

void settext(int height, int weight, UINT color);
void printtext(LPCTSTR s, int left, int top, int right, int width);
void Draw();
void Init();
void initPutNew();
int OverInterface();
void FreeMem();
void CreateImage(IMAGE *img, LPCTSTR num, COLORREF imgColor, int fontSize, COLORREF fontColor);
void Load();

class Block
{
private:
    State currentState; // 当前的状态
    State targetState;  // 移动后的状态
    Vector2 size;
    Vector2 currentPos; // 当前位置
    Vector2 targetPos;  // 目标位置
    IMAGE *img;
    IMAGE *newImg;
    float deltaPos;       // 每秒移动多少位置
    float deltaSize;      // 每秒变大多少
    float animationSpeed; // 动画速度

public:
    Block(const Vector2 &pos, IMAGE *img)
    {
        currentPos = targetPos = pos;
        currentState = targetState = EXIST;
        size = {50, 50};
        this->img = this->newImg = img;

        deltaPos = 100;
        deltaSize = 40;
        animationSpeed = 20.0f;
    }

    void update(float deltaTime)
    {
        // 改变方块大小（图片刚生成时的由小到大的动画）
        if (size.x < img->getwidth())
        {
            size.x = size.y = size.x + deltaSize * deltaTime * animationSpeed / 2;
            if (size.x > img->getwidth())
            {
                size.x = size.y = (float)img->getwidth();
            }
        }

        // 更新方块位置
        if (currentPos.x != targetPos.x || currentPos.y != targetPos.y)
        {
            int directionX = sgn(targetPos.x - currentPos.x);
            int directionY = sgn(targetPos.y - currentPos.y);

            currentPos.x += deltaPos * directionX * deltaTime * animationSpeed;
            // 相距小于 5 视为在同一位置
            if (fabs(currentPos.x - targetPos.x) < 5)
            {
                currentPos.x = targetPos.x;
            }

            currentPos.y += deltaPos * directionY * deltaTime * animationSpeed;
            if (fabs(currentPos.y - targetPos.y) < 5)
            {
                currentPos.y = targetPos.y;
            }
        }
        if (currentPos.x == targetPos.x && currentPos.y == targetPos.y)
        {
            currentState = targetState;
            img = newImg;
        }
    }

    void draw()
    {
        TransparentBlt(GetImageHDC(NULL), int(currentPos.x + (90 - size.x) / 2), int(currentPos.y + (90 - size.y) / 2),
                       (int)size.x, (int)size.y, GetImageHDC(img), 0, 0, img->getwidth(), img->getheight(), BLACK);
    }

    // 把方块从当前位置移动到目标位置，移动后改变状态
    void MoveTo(const Vector2 &pos, IMAGE *newImg, State state = EXIST)
    {
        targetPos = pos;
        targetState = state;
        this->newImg = newImg;
    }

    State getState()
    {
        return currentState;
    }
};

int map[4][4];              // 4 * 4 地图
Block *blockMap[4][4];      // 方块索引
int score;                  // 得分
int maxScore;               // 最高得分
int currentMaxBlock;        // 当前最大方块
int maxBlock;               // 历史最大方块
bool gameLoop;              // 是否可以继续游戏
float keyTime = 0;          // 按键间隔
std::map<int, IMAGE> image; // 存储所有数字图像
bool gameOver = false;      // 游戏是否结束

void lead()
{
    initgraph(WIDTH, HEIGHT);       //长宽
    setbkcolor(RGB(251, 248, 241)); //背景
    settextcolor(RGB(150, 126, 104));
    mainInterface();
}
void mainInterface() //主界面
{
    cleardevice(); //清屏

    settextstyle(90, 0, _T("Tekton Pro")); //字体设置
    RECT r1 = {0, 0, WIDTH, HEIGHT / 3};
    drawtext(_T("2048"), &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    settextstyle(40, 0, _T("Segoe Script"));
    RECT r2 = {WIDTH / 2 - 95, HEIGHT / 3, WIDTH / 2 + 100, HEIGHT / 3 + 60};
    drawtext(_T("Start Game"), &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT r3 = {WIDTH / 2 - 95, HEIGHT / 3 + 60, WIDTH / 2 + 100, HEIGHT / 3 + 120};
    drawtext(_T("Introduction"), &r3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT r4 = {WIDTH / 2 - 90, HEIGHT / 3 + 120, WIDTH / 2 + 100, HEIGHT / 3 + 180};
    drawtext(_T("Developers"), &r4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT r5 = {WIDTH / 2 - 45, HEIGHT / 3 + 180, WIDTH / 2 + 45, HEIGHT / 3 + 240};
    drawtext(_T("Exit"), &r5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    setfillcolor(RGB(244, 186, 1));                     //设置当前的填充颜色
    solidroundrect(145, 478, 295, 628, 25, 25);         //画无边框的填充圆角矩形
    settextstyle(75, 0, _T("Microsoft Yahei UI Bold")); //设置字体
    settextcolor(RGB(254, 253, 249));
    setbkcolor(RGB(244, 186, 1));    //背景色
    outtextxy(150, 510, _T("2048")); //输出字符串2048

    setbkcolor(RGB(251, 248, 241));
    settextcolor(RGB(150, 126, 104)); //字体颜色
    settextstyle(30, 0, _T("Microsoft Yahei Ul Boid"));

    MOUSEMSG m;
    while (1)
    {
        BeginBatchDraw();  //批量绘图
        m = GetMouseMsg(); //获取鼠标信息
        if (m.uMsg == WM_LBUTTONDOWN)
        {

            EndBatchDraw(); //结束批量绘图
            if (m.x > WIDTH / 2 - 95 && m.x < WIDTH / 2 + 100 && m.y > HEIGHT / 3 && m.y < HEIGHT / 3 + 60)
                start();

            else if (m.x > WIDTH / 2 - 95 && m.x < WIDTH / 2 + 100 && m.y > HEIGHT / 3 + 60 && m.y < HEIGHT / 3 + 120)
                Introduction();

            else if (m.x > WIDTH / 2 - 90 && m.x < WIDTH / 2 + 100 && m.y > HEIGHT / 3 + 120 && m.y < HEIGHT / 3 + 180)
                Developers();

            else if (m.x > WIDTH / 2 - 45 && m.x < WIDTH / 2 + 45 && m.y > HEIGHT / 3 + 180 && m.y < HEIGHT / 3 + 240)
                exit(0);
        }
    }
}
void Introduction() //Introduction
{
    cleardevice(); //清屏
    RECT C2 = {10, 60, 440, 650};
    drawtext(_T("在主界面点击Start Game进入游戏，通过上下左右方向键控制方块的移动，\n当有相同方块在移动时相邻，它们会合成一个数字是它们的之和的方块，\n如果你合成出了数字为2048的方块，恭喜你取得胜利；\n如果你在未合成出2048之前，格子里已经没有可以移动的方块，那么你的游戏失败！\n"), &C2, DT_WORDBREAK);

    RECT R1 = {2, HEIGHT - 200, WIDTH - 2, HEIGHT - 2};
    drawtext(_T("BACK"), &R1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    MOUSEMSG m;
    while (1)
    {
        m = GetMouseMsg();
        if (m.uMsg == WM_LBUTTONDOWN)
        {
            if (m.x > 2 && m.x < WIDTH - 2 && m.y > HEIGHT - 200 && m.y < HEIGHT - 2)
                mainInterface();
        }
    }
}
void Developers() //Developers
{
    cleardevice(); //清屏
    RECT C1 = {20, 60, 440, 650};
    drawtext(_T("Author of this version:\n\n 王源\n\n\n指导老师：任纪生"), &C1, DT_WORDBREAK);

    RECT R1 = {2, HEIGHT - 200, WIDTH - 2, HEIGHT - 2};
    drawtext(_T("BACK"), &R1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    MOUSEMSG m;
    while (1)
    {
        m = GetMouseMsg();
        if (m.uMsg == WM_LBUTTONDOWN)
        {
            if (m.x > 2 && m.x < WIDTH - 2 && m.y > HEIGHT - 200 && m.y < HEIGHT - 2)
                mainInterface();
        }
    }
}
void start()
{
    float deltaTime = 0; // 每帧耗时

    initgraph(440, 650); //初始化绘图窗口
    Load();              //加载图像
    BeginBatchDraw();    //开始批量不输出

    maxScore = 0;

    // 读取最高分
    maxScore = GetPrivateProfileInt(_T("2048"), _T("MaxScore"), 0, _T(".\\data.ini"));
    // 读取最大方块
    maxBlock = GetPrivateProfileInt(_T("2048"), _T("MaxBlock"), 2, _T(".\\data.ini"));

    Init(); //初始化游戏界面

    while (gameLoop) //进入游戏循环
    {
        clock_t start = clock();

        cleardevice();     //清屏
        Update(deltaTime); //更新
        Draw();            //绘图
        FlushBatchDraw();  //批量输出
        //Sleep(1);

        clock_t end = clock();
        deltaTime = (end - start) / 1000.0f;
    }

    FreeMem();

    if (OverInterface() == 0)
        exit(0);
    else
    {

        mainInterface();
    }
}
int main(void)
{

    lead();
}

// 符号函数
int sgn(float d)
{
    if (d < 0)
        return -1;
    if (d > 0)
        return 1;
    return 0;
}

//检测是否可以上移
bool canMoveUp()
{
    int i, j;
    //上移
    //依次检查每一列
    for (i = 0; i < 4; i++)
    {
        //首先排除在远端的一串空位,直接将j指向第一个非零元素
        for (j = 3; j >= 0; j--)
            if (map[j][i])
                break;
        //j>0代表这一列并非全部为0
        if (j >= 0)
            //依次检查每一个剩余元素,遇见空位直接返回true
            for (; j >= 0; j--)
                if (!map[j][i])
                    return true;
        //依次检查相邻的元素是否存在相同的非零数字
        for (j = 3; j > 0; j--)
            if (map[j][i] && map[j][i] == map[j - 1][i])
                return true;
    }
    return false;
}

//检测是否可以下移
bool canMoveDown()
{
    int i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
            if (map[j][i])
                break;
        if (j < 4)
            for (; j < 4; j++)
                if (!map[j][i])
                    return true;
        for (j = 0; j < 3; j++)
            if (map[j][i] && map[j][i] == map[j + 1][i])
                return true;
    }
    return false;
}

//检测是否可以左移
bool canMoveLeft()
{
    int i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 3; j >= 0; j--)
            if (map[i][j])
                break;
        if (j >= 0)
            for (; j >= 0; j--)
                if (!map[i][j])
                    return true;
        for (j = 0; j < 3; j++)
            if (map[i][j] && map[i][j] == map[i][j + 1])
                return true;
    }
    return false;
}

//检测是否可以右移
bool canMoveRight()
{
    int i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
            if (map[i][j])
                break;
        if (j < 4)
            for (; j < 4; j++)
                if (!map[i][j])
                    return true;
        for (j = 0; j < 3; j++)
        {
            if (map[i][j] && map[i][j] == map[i][j + 1])
                return true;
        }
    }
    return false;
}

//检测是否可以移动
bool Judge()
{
    if (canMoveUp() || canMoveDown() || canMoveLeft() || canMoveRight())
        return true;
    else
        return false;
}

//随机位置生成随机数字
void putNew()
{
    int xi, yi;

    do
    {
        xi = rand() % 4;
        yi = rand() % 4;
    } while (map[xi][yi]);

    if (rand() % 10 < 8)
    {
        map[xi][yi] = 2;
        blockMap[xi][yi] = new Block({25.0f + 100 * yi, 225.0f + 100 * xi}, &image[2]);
    }
    else
    {
        map[xi][yi] = 4;
        blockMap[xi][yi] = new Block({25.0f + 100 * yi, 225.0f + 100 * xi}, &image[4]);
    }
}

//上移
void Up()
{
    int flag = 0;
    int i, j;
    int value = 0;

    // 按列枚举
    for (i = 0; i < 4; i++)
    {
        //对于每一列的每一个元素
        for (j = 0; j < 3; j++)
        {
            int k, l;
            for (k = j; k < 4; k++)
                //找到第一个不为0的位置，并定位
                if (map[k][i])
                    break;
            for (l = k + 1; l < 4; l++)
                //找到第一个不为0的位置后面的不为0位置
                if (map[l][i])
                    break;
            //这一列找到了非零方块（合并或移动的基础）
            if (k < 4)
            {
                //可以合并
                if (l < 4 && map[k][i] == map[l][i])
                {
                    //如果这一列有两个非0方块且这两个方块相同则可以合并
                    value = map[k][i] * 2;
                    map[k][i] = map[l][i] = 0;
                    map[j][i] = value;

                    //开启动画
                    Block *temp = blockMap[k][i];
                    blockMap[k][i] = NULL;
                    blockMap[j][i] = temp;
                    blockMap[j][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * j}, &image[map[j][i]]);
                    blockMap[l][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * (j + 1)}, &image[map[l][i]], DESTORY);

                    //更新分数和最大方块
                    //得分的增加是合并的方块数字*6
                    score += map[j][i] * SCORE;
                    if (score > maxScore)
                        maxScore = score;
                    if (map[j][i] > currentMaxBlock)
                        currentMaxBlock = map[j][i];
                    if (currentMaxBlock > maxBlock)
                        maxBlock = currentMaxBlock;

                    flag = 1;
                }

                //不能合并
                else
                {
                    value = map[k][i];
                    map[k][i] = 0;
                    map[j][i] = value;
                    //如果当前位置是零,下一个元素为非零，即可以移动
                    if (k != j)
                    {
                        flag = 1;

                        //开启动画
                        Block *temp = blockMap[k][i];
                        blockMap[k][i] = NULL;
                        blockMap[j][i] = temp;
                        blockMap[j][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * j}, &image[map[j][i]]);
                    }
                }
            }
            else
                break;
        }
        //第j行判断完毕，进行下一行判断
    }
    //第i列判断完毕，进行下一列判断

    if (flag)     //发生了移动或合并
        putNew(); //随机生成数字
}

//下移
void Down()
{
    int flag = 0;
    int i, j;
    int value = 0;

    // 按列枚举
    for (i = 0; i < 4; i++)
    {
        //对于每一列的每一个元素
        for (j = 3; j > 0; j--)
        {
            int k, l;
            for (k = j; k > -1; k--)
                //找到第一个不为0的位置，并定位
                if (map[k][i])
                    break;
            for (l = k - 1; l > -1; l--)
                //找到第一个不为0的位置后面的不为0位置
                if (map[l][i])
                    break;
            //这一列找到了非零方块（合并或移动的基础）
            if (k > -1)
            {
                //可以合并
                if (l > -1 && map[k][i] == map[l][i])
                {
                    //如果这一列有两个非0方块且这两个方块相同则可以合并
                    value = map[k][i] * 2;
                    map[k][i] = map[l][i] = 0;
                    map[j][i] = value;

                    //开启动画
                    Block *temp = blockMap[k][i];
                    blockMap[k][i] = NULL;
                    blockMap[j][i] = temp;
                    blockMap[j][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * j}, &image[map[j][i]]);
                    blockMap[l][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * (j + 1)}, &image[map[l][i]], DESTORY);

                    //更新分数和最大方块
                    //得分的增加是合并的方块数字*6
                    score += map[j][i] * SCORE;
                    if (score > maxScore)
                        maxScore = score;
                    if (map[j][i] > currentMaxBlock)
                        currentMaxBlock = map[j][i];
                    if (currentMaxBlock > maxBlock)
                        maxBlock = currentMaxBlock;

                    flag = 1;
                }

                //不能合并
                else
                {
                    value = map[k][i];
                    map[k][i] = 0;
                    map[j][i] = value;
                    //如果当前位置是零,下一个元素为非零，即可以移动
                    if (k != j)
                    {
                        flag = 1;

                        //开启动画
                        Block *temp = blockMap[k][i];
                        blockMap[k][i] = NULL;
                        blockMap[j][i] = temp;
                        blockMap[j][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * j}, &image[map[j][i]]);
                    }
                }
            }
            else
                break;
        }
        //第j行判断完毕，进行下一行判断
    }
    //第i列判断完毕，进行下一列判断

    if (flag)     //发生了移动或合并
        putNew(); //随机生成数字
}

//左移
void Left()
{
    int flag = 0;
    int i, j;
    int value = 0;

    // 按行枚举
    for (i = 0; i < 4; i++)
    {
        //对于每一行的每一个元素
        for (j = 0; j < 3; j++)
        {
            int k, l;
            for (k = j; k < 4; k++)
                //找到第一个不为0的位置，并定位
                if (map[i][k])
                    break;
            for (l = k + 1; l < 4; l++)
                //找到第一个不为0的位置后面的不为0位置
                if (map[i][l])
                    break;
            //这一行找到了非零方块（合并或移动的基础）
            if (k < 4)
            {
                //可以合并
                if (l < 4 && map[i][k] == map[i][l])
                {
                    //如果这一列有两个非0方块且这两个方块相同则可以合并
                    value = map[i][k] * 2;
                    map[i][k] = map[i][l] = 0;
                    map[i][j] = value;

                    //开启动画
                    Block *temp = blockMap[i][k];
                    blockMap[i][k] = NULL;
                    blockMap[i][j] = temp;
                    blockMap[i][j]->MoveTo({25.0f + 100 * j, 225.0f + 100 * i}, &image[map[i][j]]);
                    blockMap[i][l]->MoveTo({25.0f + 100 * (j + 1), 225.0f + 100 * i}, &image[map[i][l]], DESTORY);

                    //更新分数和最大方块
                    //得分的增加是合并的方块数字*6
                    score += map[i][j] * SCORE;
                    if (score > maxScore)
                        maxScore = score;
                    if (map[i][j] > currentMaxBlock)
                        currentMaxBlock = map[i][j];
                    if (currentMaxBlock > maxBlock)
                        maxBlock = currentMaxBlock;

                    flag = 1;
                }

                //不能合并
                else
                {
                    value = map[i][k];
                    map[i][k] = 0;
                    map[i][j] = value;
                    //如果当前位置是零,下一个元素为非零，即可以移动
                    if (k != j)
                    {
                        flag = 1;

                        //开启动画
                        Block *temp = blockMap[i][k];
                        blockMap[i][k] = NULL;
                        blockMap[i][j] = temp;
                        blockMap[i][j]->MoveTo({25.0f + 100 * j, 225.0f + 100 * i}, &image[map[i][j]]);
                    }
                }
            }
            else
                break;
        }
        //第j行判断完毕，进行下一行判断
    }
    //第i列判断完毕，进行下一列判断

    if (flag)     //发生了移动或合并
        putNew(); //随机生成数字
}

//右移
void Right()
{
    int flag = 0;
    int i, j;
    int value = 0;

    // 按行枚举
    for (i = 0; i < 4; i++)
    {
        //对于每一行的每一个元素
        for (j = 3; j > 0; j--)
        {
            int k, l;
            for (k = j; k > -1; k--)
                //找到第一个不为0的位置，并定位
                if (map[i][k])
                    break;
            for (l = k - 1; l > -1; l--)
                //找到第一个不为0的位置后面的不为0位置
                if (map[i][l])
                    break;
            //这一行找到了非零方块（合并或移动的基础）
            if (k > -1)
            {
                //可以合并
                if (l > -1 && map[i][k] == map[i][l])
                {
                    //如果这一行有两个非0方块且这两个方块相同则可以合并
                    value = map[i][k] * 2;
                    map[i][k] = map[i][l] = 0;
                    map[i][j] = value;

                    //开启动画
                    Block *temp = blockMap[i][k];
                    blockMap[i][k] = NULL;
                    blockMap[i][j] = temp;
                    blockMap[i][j]->MoveTo({25.0f + 100 * j, 225.0f + 100 * i}, &image[map[i][j]]);
                    blockMap[i][l]->MoveTo({25.0f + 100 * (j + 1), 225.0f + 100 * i}, &image[map[i][l]], DESTORY);

                    //更新分数和最大方块
                    //得分的增加是合并的方块数字*6
                    score += map[i][j] * SCORE;
                    if (score > maxScore)
                        maxScore = score;
                    if (map[i][j] > currentMaxBlock)
                        currentMaxBlock = map[i][j];
                    if (currentMaxBlock > maxBlock)
                        maxBlock = currentMaxBlock;

                    flag = 1;
                }

                //不能合并
                else
                {
                    value = map[i][k];
                    map[i][k] = 0;
                    map[i][j] = value;
                    //如果当前位置是零,下一个元素为非零，即可以移动
                    if (k != j)
                    {
                        flag = 1;

                        //开启动画
                        Block *temp = blockMap[i][k];
                        blockMap[i][k] = NULL;
                        blockMap[i][j] = temp;
                        blockMap[i][j]->MoveTo({25.0f + 100 * j, 225.0f + 100 * i}, &image[map[i][j]]);
                    }
                }
            }
            else
                break;
        }
        //第j行判断完毕，进行下一行判断
    }
    //第i列判断完毕，进行下一列判断

    if (flag)     //发生了移动或合并
        putNew(); //随机生成数字
}

void Update(float deltaTime)
{
    // 更新方块
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (blockMap[i][j] != NULL)
            {
                blockMap[i][j]->update(deltaTime);
                if (blockMap[i][j]->getState() == DESTORY)
                {
                    delete blockMap[i][j];
                    blockMap[i][j] = NULL;
                }
            }
        }
    }

    if (gameOver)
    {
        gameLoop = 0;
        return;
    }

    keyTime += deltaTime;
    // 0.2s 可以按键一次
    if (keyTime < 0.2f || gameOver)
        return;

    if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState('W') & 0x8000)) // 上
    {
        Up();
        if (!Judge())
        {
            gameOver = true;
        }
        keyTime = 0;
    }
    else if ((GetAsyncKeyState(VK_DOWN) & 0x8000) || (GetAsyncKeyState('S') & 0x8000)) // 下
    {
        Down();
        if (!Judge())
        {
            gameOver = true;
        }
        keyTime = 0;
    }
    else if ((GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000)) // 左
    {
        Left();
        if (!Judge())
        {
            gameOver = true;
        }
        keyTime = 0;
    }
    else if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000)) // 右
    {
        Right();
        if (!Judge())
        {
            gameOver = true;
        }
        keyTime = 0;
    }
}

// 设置文字样式和颜色
void settext(int height, int weight, UINT color)
{
    settextstyle(height, 0, _T("Arial"), 0, 0, weight, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                 CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH);
    settextcolor(color);
}

// 在指定矩形区域内居中输出字符串
void printtext(LPCTSTR s, int left, int top, int right, int width)
{
    RECT r = {left, top, right, width};
    drawtext(s, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 绘制界面
void Draw()
{
    // 历史最大方块
    TransparentBlt(GetImageHDC(NULL), 12, 30, 90, 90, GetImageHDC(&image[maxBlock]), 0, 0, 90, 90, 0x9eaebb);

    setfillcolor(0x9eaebb);
    // 绘制当前分数
    solidroundrect(112, 30, 264, 119, 10, 10);
    settext(28, 800, 0xdbe6ee);
    printtext(_T("SCORE"), 112, 40, 264, 69);
    std::wstringstream ss;
    ss << score;
    settext(44, 800, WHITE);
    printtext(ss.str().c_str(), 112, 70, 264, 114);
    ss.str(_T(""));

    // 绘制最高分数
    solidroundrect(275, 30, 427, 119, 10, 10);
    settext(28, 800, 0xdbe6ee);
    printtext(_T("BEST"), 275, 40, 427, 69);
    ss << maxScore;
    settext(44, 800, WHITE);
    printtext(ss.str().c_str(), 275, 70, 427, 114);
    ss.str(_T(""));

    // 绘制提示信息
    settextcolor(BLACK);
    ss << " Current most cube number is " << currentMaxBlock << " !";
    settext(24, 800, 0x707b83);
    printtext(ss.str().c_str(), 0, 120, 439, 211);

    // 绘制方块底板
    solidroundrect(12, 212, 427, 627, 10, 10);

    // 绘制方块
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            putimage(25 + 100 * j, 225 + 100 * i, &image[0]);
        }
    }
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (blockMap[i][j] != NULL)
                blockMap[i][j]->draw();
        }
    }
}

//初始化专用，随机位置生成数字2
void initPutNew()
{
    int xi, yi;
    do
    {
        xi = rand() % 4;
        yi = rand() % 4;
    } while (map[xi][yi]);
    map[xi][yi] = 2;
    blockMap[xi][yi] = new Block({25.0f + 100 * yi, 225.0f + 100 * xi}, &image[2]);
}

// 初始化游戏
void Init()
{
    srand((unsigned int)time(NULL)); // 初始化随机数种子

    memset(map, 0, 4 * 4 * sizeof(int)); // 把地图初始化为 0
    //申请内存初始化地图上的方块
    //将指针变量map所指向的前16个Block *大小的内存单元用0替换
    memset(blockMap, 0, 4 * 4 * sizeof(Block *));

    score = 0;
    gameLoop = 1;
    gameOver = false;
    currentMaxBlock = 2;
    initPutNew();
    initPutNew();
    setbkcolor(WHITE);
    setbkmode(TRANSPARENT);
}

// 游戏结束界面 返回 1 表示返回主界面 返回 0 表示结束游戏
int OverInterface()
{
    // 保存最高纪录
    std::wstringstream ss;
    ss << maxScore;
    WritePrivateProfileString(_T("2048"), _T("MaxScore"), ss.str().c_str(), _T(".\\data.ini"));
    ss.str(_T(""));
    ss << maxBlock;
    WritePrivateProfileString(_T("2048"), _T("MaxBlock"), ss.str().c_str(), _T(".\\data.ini"));

    setbkmode(TRANSPARENT);
    setbkcolor(0x8eecff);
    cleardevice();

    // Game Over
    settext(60, 1000, 0x696f78);
    printtext(_T("Game Over!"), 0, 0, 439, 199);

    // 绘制最大方块
    TransparentBlt(GetImageHDC(NULL), 175, 150, 90, 90, GetImageHDC(&image[currentMaxBlock]), 0, 0, 90, 90, 0x9eaebb);

    // Back
    setfillcolor(0x9dadba);
    solidroundrect(120, 310, 319, 389, 10, 10);
    settext(36, 1000, WHITE);
    printtext(_T("Back"), 120, 310, 319, 389);
    // Exit
    solidroundrect(120, 460, 319, 539, 10, 10);
    printtext(_T("Exit"), 120, 460, 319, 539);

    FlushBatchDraw();

    FlushMouseMsgBuffer();

    while (1)
    {
        while (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.mkLButton)
            {
                int x = msg.x;
                int y = msg.y;
                if (x >= 120 && x <= 319 && y >= 310 && y <= 389)
                    return 1;
                if (x >= 120 && x <= 319 && y >= 460 && y <= 539)
                    return 0;
            }
        }
        Sleep(100);
    }
    return 1;
}

// 释放内存
void FreeMem()
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blockMap[i][j] != NULL)
                delete blockMap[i][j];
}

// 用于生成方块图片
//		img:		方块图片指针
//		num:		方块上的数字
//		imgColor:	方块颜色
//		fontSize:	字体大小
//		fontColor:	字体颜色
void CreateImage(IMAGE *img, LPCTSTR num, COLORREF imgColor, int fontSize, COLORREF fontColor)
{
    SetWorkingImage(img);               //设定绘图设备
    setbkmode(TRANSPARENT);             //设定背景模式
    setbkcolor(0x9eaebb);               //设定背景颜色
    settext(fontSize, 1000, fontColor); //设定字体大小颜色
    setfillcolor(imgColor);             //设定方块颜色
    settextcolor(fontColor);            //设定字体颜色

    cleardevice();

    solidroundrect(0, 0, img->getwidth() - 1, img->getheight() - 1, 10, 10); //绘制圆角矩形

    RECT r = {0, 0, img->getwidth() - 1, img->getheight() - 1};
    drawtext(num, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE); //绘制数字
}

// 绘制图片缓存
void Load()
{
    IMAGE temp(90, 90);

    CreateImage(&temp, _T(""), 0xb5becc, 72, WHITE);
    image[0] = temp;
    CreateImage(&temp, _T("2"), 0xdbe6ee, 72, 0x707b83);
    image[2] = temp;
    CreateImage(&temp, _T("4"), 0xc7e1ed, 72, 0x707b83);
    image[4] = temp;
    CreateImage(&temp, _T("8"), 0x78b2f4, 72, WHITE);
    image[8] = temp;
    CreateImage(&temp, _T("16"), 0x538ded, 72, WHITE);
    image[16] = temp;
    CreateImage(&temp, _T("32"), 0x607df6, 72, WHITE);
    image[32] = temp;
    CreateImage(&temp, _T("64"), 0x3958e9, 72, WHITE);
    image[64] = temp;
    CreateImage(&temp, _T("128"), 0x6bd9f5, 56, WHITE);
    image[128] = temp;
    CreateImage(&temp, _T("256"), 0x4bd0f2, 56, WHITE);
    image[256] = temp;
    CreateImage(&temp, _T("512"), 0x2ac0e4, 56, WHITE);
    image[512] = temp;
    CreateImage(&temp, _T("1024"), 0x13b8e3, 40, WHITE);
    image[1024] = temp;
    CreateImage(&temp, _T("2048"), 0x00c5eb, 40, WHITE);
    image[2048] = temp;
    CreateImage(&temp, _T("4096"), 0x3958e9, 40, WHITE);
    image[4096] = temp;
    CreateImage(&temp, _T("8192"), 0x3958e9, 40, WHITE);
    image[8192] = temp;

    SetWorkingImage(NULL);
}