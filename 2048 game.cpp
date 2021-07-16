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
#define SCORE 5                     //�÷��Ǻϳ����ֵ��屶
#pragma comment(lib, "MSIMG32.LIB") //TransparentBlt()��ͼ

// �����״̬
enum State
{
    EXIST,  // ����
    DESTORY // ����
};

// ��ά���������ڱ�ʾλ�û��ߴ�С
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
    State currentState; // ��ǰ��״̬
    State targetState;  // �ƶ����״̬
    Vector2 size;
    Vector2 currentPos; // ��ǰλ��
    Vector2 targetPos;  // Ŀ��λ��
    IMAGE *img;
    IMAGE *newImg;
    float deltaPos;       // ÿ���ƶ�����λ��
    float deltaSize;      // ÿ�������
    float animationSpeed; // �����ٶ�

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
        // �ı䷽���С��ͼƬ������ʱ����С����Ķ�����
        if (size.x < img->getwidth())
        {
            size.x = size.y = size.x + deltaSize * deltaTime * animationSpeed / 2;
            if (size.x > img->getwidth())
            {
                size.x = size.y = (float)img->getwidth();
            }
        }

        // ���·���λ��
        if (currentPos.x != targetPos.x || currentPos.y != targetPos.y)
        {
            int directionX = sgn(targetPos.x - currentPos.x);
            int directionY = sgn(targetPos.y - currentPos.y);

            currentPos.x += deltaPos * directionX * deltaTime * animationSpeed;
            // ���С�� 5 ��Ϊ��ͬһλ��
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

    // �ѷ���ӵ�ǰλ���ƶ���Ŀ��λ�ã��ƶ���ı�״̬
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

int map[4][4];              // 4 * 4 ��ͼ
Block *blockMap[4][4];      // ��������
int score;                  // �÷�
int maxScore;               // ��ߵ÷�
int currentMaxBlock;        // ��ǰ��󷽿�
int maxBlock;               // ��ʷ��󷽿�
bool gameLoop;              // �Ƿ���Լ�����Ϸ
float keyTime = 0;          // �������
std::map<int, IMAGE> image; // �洢��������ͼ��
bool gameOver = false;      // ��Ϸ�Ƿ����

void lead()
{
    initgraph(WIDTH, HEIGHT);       //����
    setbkcolor(RGB(251, 248, 241)); //����
    settextcolor(RGB(150, 126, 104));
    mainInterface();
}
void mainInterface() //������
{
    cleardevice(); //����

    settextstyle(90, 0, _T("Tekton Pro")); //��������
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

    setfillcolor(RGB(244, 186, 1));                     //���õ�ǰ�������ɫ
    solidroundrect(145, 478, 295, 628, 25, 25);         //���ޱ߿�����Բ�Ǿ���
    settextstyle(75, 0, _T("Microsoft Yahei UI Bold")); //��������
    settextcolor(RGB(254, 253, 249));
    setbkcolor(RGB(244, 186, 1));    //����ɫ
    outtextxy(150, 510, _T("2048")); //����ַ���2048

    setbkcolor(RGB(251, 248, 241));
    settextcolor(RGB(150, 126, 104)); //������ɫ
    settextstyle(30, 0, _T("Microsoft Yahei Ul Boid"));

    MOUSEMSG m;
    while (1)
    {
        BeginBatchDraw();  //������ͼ
        m = GetMouseMsg(); //��ȡ�����Ϣ
        if (m.uMsg == WM_LBUTTONDOWN)
        {

            EndBatchDraw(); //����������ͼ
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
    cleardevice(); //����
    RECT C2 = {10, 60, 440, 650};
    drawtext(_T("����������Start Game������Ϸ��ͨ���������ҷ�������Ʒ�����ƶ���\n������ͬ�������ƶ�ʱ���ڣ����ǻ�ϳ�һ�����������ǵ�֮�͵ķ��飬\n�����ϳɳ�������Ϊ2048�ķ��飬��ϲ��ȡ��ʤ����\n�������δ�ϳɳ�2048֮ǰ���������Ѿ�û�п����ƶ��ķ��飬��ô�����Ϸʧ�ܣ�\n"), &C2, DT_WORDBREAK);

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
    cleardevice(); //����
    RECT C1 = {20, 60, 440, 650};
    drawtext(_T("Author of this version:\n\n ��Դ\n\n\nָ����ʦ���μ���"), &C1, DT_WORDBREAK);

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
    float deltaTime = 0; // ÿ֡��ʱ

    initgraph(440, 650); //��ʼ����ͼ����
    Load();              //����ͼ��
    BeginBatchDraw();    //��ʼ���������

    maxScore = 0;

    // ��ȡ��߷�
    maxScore = GetPrivateProfileInt(_T("2048"), _T("MaxScore"), 0, _T(".\\data.ini"));
    // ��ȡ��󷽿�
    maxBlock = GetPrivateProfileInt(_T("2048"), _T("MaxBlock"), 2, _T(".\\data.ini"));

    Init(); //��ʼ����Ϸ����

    while (gameLoop) //������Ϸѭ��
    {
        clock_t start = clock();

        cleardevice();     //����
        Update(deltaTime); //����
        Draw();            //��ͼ
        FlushBatchDraw();  //�������
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

// ���ź���
int sgn(float d)
{
    if (d < 0)
        return -1;
    if (d > 0)
        return 1;
    return 0;
}

//����Ƿ��������
bool canMoveUp()
{
    int i, j;
    //����
    //���μ��ÿһ��
    for (i = 0; i < 4; i++)
    {
        //�����ų���Զ�˵�һ����λ,ֱ�ӽ�jָ���һ������Ԫ��
        for (j = 3; j >= 0; j--)
            if (map[j][i])
                break;
        //j>0������һ�в���ȫ��Ϊ0
        if (j >= 0)
            //���μ��ÿһ��ʣ��Ԫ��,������λֱ�ӷ���true
            for (; j >= 0; j--)
                if (!map[j][i])
                    return true;
        //���μ�����ڵ�Ԫ���Ƿ������ͬ�ķ�������
        for (j = 3; j > 0; j--)
            if (map[j][i] && map[j][i] == map[j - 1][i])
                return true;
    }
    return false;
}

//����Ƿ��������
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

//����Ƿ��������
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

//����Ƿ��������
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

//����Ƿ�����ƶ�
bool Judge()
{
    if (canMoveUp() || canMoveDown() || canMoveLeft() || canMoveRight())
        return true;
    else
        return false;
}

//���λ�������������
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

//����
void Up()
{
    int flag = 0;
    int i, j;
    int value = 0;

    // ����ö��
    for (i = 0; i < 4; i++)
    {
        //����ÿһ�е�ÿһ��Ԫ��
        for (j = 0; j < 3; j++)
        {
            int k, l;
            for (k = j; k < 4; k++)
                //�ҵ���һ����Ϊ0��λ�ã�����λ
                if (map[k][i])
                    break;
            for (l = k + 1; l < 4; l++)
                //�ҵ���һ����Ϊ0��λ�ú���Ĳ�Ϊ0λ��
                if (map[l][i])
                    break;
            //��һ���ҵ��˷��㷽�飨�ϲ����ƶ��Ļ�����
            if (k < 4)
            {
                //���Ժϲ�
                if (l < 4 && map[k][i] == map[l][i])
                {
                    //�����һ����������0������������������ͬ����Ժϲ�
                    value = map[k][i] * 2;
                    map[k][i] = map[l][i] = 0;
                    map[j][i] = value;

                    //��������
                    Block *temp = blockMap[k][i];
                    blockMap[k][i] = NULL;
                    blockMap[j][i] = temp;
                    blockMap[j][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * j}, &image[map[j][i]]);
                    blockMap[l][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * (j + 1)}, &image[map[l][i]], DESTORY);

                    //���·�������󷽿�
                    //�÷ֵ������Ǻϲ��ķ�������*6
                    score += map[j][i] * SCORE;
                    if (score > maxScore)
                        maxScore = score;
                    if (map[j][i] > currentMaxBlock)
                        currentMaxBlock = map[j][i];
                    if (currentMaxBlock > maxBlock)
                        maxBlock = currentMaxBlock;

                    flag = 1;
                }

                //���ܺϲ�
                else
                {
                    value = map[k][i];
                    map[k][i] = 0;
                    map[j][i] = value;
                    //�����ǰλ������,��һ��Ԫ��Ϊ���㣬�������ƶ�
                    if (k != j)
                    {
                        flag = 1;

                        //��������
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
        //��j���ж���ϣ�������һ���ж�
    }
    //��i���ж���ϣ�������һ���ж�

    if (flag)     //�������ƶ���ϲ�
        putNew(); //�����������
}

//����
void Down()
{
    int flag = 0;
    int i, j;
    int value = 0;

    // ����ö��
    for (i = 0; i < 4; i++)
    {
        //����ÿһ�е�ÿһ��Ԫ��
        for (j = 3; j > 0; j--)
        {
            int k, l;
            for (k = j; k > -1; k--)
                //�ҵ���һ����Ϊ0��λ�ã�����λ
                if (map[k][i])
                    break;
            for (l = k - 1; l > -1; l--)
                //�ҵ���һ����Ϊ0��λ�ú���Ĳ�Ϊ0λ��
                if (map[l][i])
                    break;
            //��һ���ҵ��˷��㷽�飨�ϲ����ƶ��Ļ�����
            if (k > -1)
            {
                //���Ժϲ�
                if (l > -1 && map[k][i] == map[l][i])
                {
                    //�����һ����������0������������������ͬ����Ժϲ�
                    value = map[k][i] * 2;
                    map[k][i] = map[l][i] = 0;
                    map[j][i] = value;

                    //��������
                    Block *temp = blockMap[k][i];
                    blockMap[k][i] = NULL;
                    blockMap[j][i] = temp;
                    blockMap[j][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * j}, &image[map[j][i]]);
                    blockMap[l][i]->MoveTo({25.0f + 100 * i, 225.0f + 100 * (j + 1)}, &image[map[l][i]], DESTORY);

                    //���·�������󷽿�
                    //�÷ֵ������Ǻϲ��ķ�������*6
                    score += map[j][i] * SCORE;
                    if (score > maxScore)
                        maxScore = score;
                    if (map[j][i] > currentMaxBlock)
                        currentMaxBlock = map[j][i];
                    if (currentMaxBlock > maxBlock)
                        maxBlock = currentMaxBlock;

                    flag = 1;
                }

                //���ܺϲ�
                else
                {
                    value = map[k][i];
                    map[k][i] = 0;
                    map[j][i] = value;
                    //�����ǰλ������,��һ��Ԫ��Ϊ���㣬�������ƶ�
                    if (k != j)
                    {
                        flag = 1;

                        //��������
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
        //��j���ж���ϣ�������һ���ж�
    }
    //��i���ж���ϣ�������һ���ж�

    if (flag)     //�������ƶ���ϲ�
        putNew(); //�����������
}

//����
void Left()
{
    int flag = 0;
    int i, j;
    int value = 0;

    // ����ö��
    for (i = 0; i < 4; i++)
    {
        //����ÿһ�е�ÿһ��Ԫ��
        for (j = 0; j < 3; j++)
        {
            int k, l;
            for (k = j; k < 4; k++)
                //�ҵ���һ����Ϊ0��λ�ã�����λ
                if (map[i][k])
                    break;
            for (l = k + 1; l < 4; l++)
                //�ҵ���һ����Ϊ0��λ�ú���Ĳ�Ϊ0λ��
                if (map[i][l])
                    break;
            //��һ���ҵ��˷��㷽�飨�ϲ����ƶ��Ļ�����
            if (k < 4)
            {
                //���Ժϲ�
                if (l < 4 && map[i][k] == map[i][l])
                {
                    //�����һ����������0������������������ͬ����Ժϲ�
                    value = map[i][k] * 2;
                    map[i][k] = map[i][l] = 0;
                    map[i][j] = value;

                    //��������
                    Block *temp = blockMap[i][k];
                    blockMap[i][k] = NULL;
                    blockMap[i][j] = temp;
                    blockMap[i][j]->MoveTo({25.0f + 100 * j, 225.0f + 100 * i}, &image[map[i][j]]);
                    blockMap[i][l]->MoveTo({25.0f + 100 * (j + 1), 225.0f + 100 * i}, &image[map[i][l]], DESTORY);

                    //���·�������󷽿�
                    //�÷ֵ������Ǻϲ��ķ�������*6
                    score += map[i][j] * SCORE;
                    if (score > maxScore)
                        maxScore = score;
                    if (map[i][j] > currentMaxBlock)
                        currentMaxBlock = map[i][j];
                    if (currentMaxBlock > maxBlock)
                        maxBlock = currentMaxBlock;

                    flag = 1;
                }

                //���ܺϲ�
                else
                {
                    value = map[i][k];
                    map[i][k] = 0;
                    map[i][j] = value;
                    //�����ǰλ������,��һ��Ԫ��Ϊ���㣬�������ƶ�
                    if (k != j)
                    {
                        flag = 1;

                        //��������
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
        //��j���ж���ϣ�������һ���ж�
    }
    //��i���ж���ϣ�������һ���ж�

    if (flag)     //�������ƶ���ϲ�
        putNew(); //�����������
}

//����
void Right()
{
    int flag = 0;
    int i, j;
    int value = 0;

    // ����ö��
    for (i = 0; i < 4; i++)
    {
        //����ÿһ�е�ÿһ��Ԫ��
        for (j = 3; j > 0; j--)
        {
            int k, l;
            for (k = j; k > -1; k--)
                //�ҵ���һ����Ϊ0��λ�ã�����λ
                if (map[i][k])
                    break;
            for (l = k - 1; l > -1; l--)
                //�ҵ���һ����Ϊ0��λ�ú���Ĳ�Ϊ0λ��
                if (map[i][l])
                    break;
            //��һ���ҵ��˷��㷽�飨�ϲ����ƶ��Ļ�����
            if (k > -1)
            {
                //���Ժϲ�
                if (l > -1 && map[i][k] == map[i][l])
                {
                    //�����һ����������0������������������ͬ����Ժϲ�
                    value = map[i][k] * 2;
                    map[i][k] = map[i][l] = 0;
                    map[i][j] = value;

                    //��������
                    Block *temp = blockMap[i][k];
                    blockMap[i][k] = NULL;
                    blockMap[i][j] = temp;
                    blockMap[i][j]->MoveTo({25.0f + 100 * j, 225.0f + 100 * i}, &image[map[i][j]]);
                    blockMap[i][l]->MoveTo({25.0f + 100 * (j + 1), 225.0f + 100 * i}, &image[map[i][l]], DESTORY);

                    //���·�������󷽿�
                    //�÷ֵ������Ǻϲ��ķ�������*6
                    score += map[i][j] * SCORE;
                    if (score > maxScore)
                        maxScore = score;
                    if (map[i][j] > currentMaxBlock)
                        currentMaxBlock = map[i][j];
                    if (currentMaxBlock > maxBlock)
                        maxBlock = currentMaxBlock;

                    flag = 1;
                }

                //���ܺϲ�
                else
                {
                    value = map[i][k];
                    map[i][k] = 0;
                    map[i][j] = value;
                    //�����ǰλ������,��һ��Ԫ��Ϊ���㣬�������ƶ�
                    if (k != j)
                    {
                        flag = 1;

                        //��������
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
        //��j���ж���ϣ�������һ���ж�
    }
    //��i���ж���ϣ�������һ���ж�

    if (flag)     //�������ƶ���ϲ�
        putNew(); //�����������
}

void Update(float deltaTime)
{
    // ���·���
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
    // 0.2s ���԰���һ��
    if (keyTime < 0.2f || gameOver)
        return;

    if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState('W') & 0x8000)) // ��
    {
        Up();
        if (!Judge())
        {
            gameOver = true;
        }
        keyTime = 0;
    }
    else if ((GetAsyncKeyState(VK_DOWN) & 0x8000) || (GetAsyncKeyState('S') & 0x8000)) // ��
    {
        Down();
        if (!Judge())
        {
            gameOver = true;
        }
        keyTime = 0;
    }
    else if ((GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000)) // ��
    {
        Left();
        if (!Judge())
        {
            gameOver = true;
        }
        keyTime = 0;
    }
    else if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000)) // ��
    {
        Right();
        if (!Judge())
        {
            gameOver = true;
        }
        keyTime = 0;
    }
}

// ����������ʽ����ɫ
void settext(int height, int weight, UINT color)
{
    settextstyle(height, 0, _T("Arial"), 0, 0, weight, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                 CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH);
    settextcolor(color);
}

// ��ָ�����������ھ�������ַ���
void printtext(LPCTSTR s, int left, int top, int right, int width)
{
    RECT r = {left, top, right, width};
    drawtext(s, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// ���ƽ���
void Draw()
{
    // ��ʷ��󷽿�
    TransparentBlt(GetImageHDC(NULL), 12, 30, 90, 90, GetImageHDC(&image[maxBlock]), 0, 0, 90, 90, 0x9eaebb);

    setfillcolor(0x9eaebb);
    // ���Ƶ�ǰ����
    solidroundrect(112, 30, 264, 119, 10, 10);
    settext(28, 800, 0xdbe6ee);
    printtext(_T("SCORE"), 112, 40, 264, 69);
    std::wstringstream ss;
    ss << score;
    settext(44, 800, WHITE);
    printtext(ss.str().c_str(), 112, 70, 264, 114);
    ss.str(_T(""));

    // ������߷���
    solidroundrect(275, 30, 427, 119, 10, 10);
    settext(28, 800, 0xdbe6ee);
    printtext(_T("BEST"), 275, 40, 427, 69);
    ss << maxScore;
    settext(44, 800, WHITE);
    printtext(ss.str().c_str(), 275, 70, 427, 114);
    ss.str(_T(""));

    // ������ʾ��Ϣ
    settextcolor(BLACK);
    ss << " Current most cube number is " << currentMaxBlock << " !";
    settext(24, 800, 0x707b83);
    printtext(ss.str().c_str(), 0, 120, 439, 211);

    // ���Ʒ���װ�
    solidroundrect(12, 212, 427, 627, 10, 10);

    // ���Ʒ���
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

//��ʼ��ר�ã����λ����������2
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

// ��ʼ����Ϸ
void Init()
{
    srand((unsigned int)time(NULL)); // ��ʼ�����������

    memset(map, 0, 4 * 4 * sizeof(int)); // �ѵ�ͼ��ʼ��Ϊ 0
    //�����ڴ��ʼ����ͼ�ϵķ���
    //��ָ�����map��ָ���ǰ16��Block *��С���ڴ浥Ԫ��0�滻
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

// ��Ϸ�������� ���� 1 ��ʾ���������� ���� 0 ��ʾ������Ϸ
int OverInterface()
{
    // ������߼�¼
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

    // ������󷽿�
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

// �ͷ��ڴ�
void FreeMem()
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blockMap[i][j] != NULL)
                delete blockMap[i][j];
}

// �������ɷ���ͼƬ
//		img:		����ͼƬָ��
//		num:		�����ϵ�����
//		imgColor:	������ɫ
//		fontSize:	�����С
//		fontColor:	������ɫ
void CreateImage(IMAGE *img, LPCTSTR num, COLORREF imgColor, int fontSize, COLORREF fontColor)
{
    SetWorkingImage(img);               //�趨��ͼ�豸
    setbkmode(TRANSPARENT);             //�趨����ģʽ
    setbkcolor(0x9eaebb);               //�趨������ɫ
    settext(fontSize, 1000, fontColor); //�趨�����С��ɫ
    setfillcolor(imgColor);             //�趨������ɫ
    settextcolor(fontColor);            //�趨������ɫ

    cleardevice();

    solidroundrect(0, 0, img->getwidth() - 1, img->getheight() - 1, 10, 10); //����Բ�Ǿ���

    RECT r = {0, 0, img->getwidth() - 1, img->getheight() - 1};
    drawtext(num, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE); //��������
}

// ����ͼƬ����
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