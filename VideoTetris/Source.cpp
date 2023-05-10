#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <Windows.h>

wstring tetromino[7];//俄罗斯方块的七个常见形状

int nFieldWidth = 12;// 场宽
int nFieldHeight = 18;// 场高

// 存储场上的元素为无符号字符数组
unsigned char* pField = nullptr;// 动态分配

// Defining the screen size(屏幕缓冲区)
int nScreenWidth = 80; // Console Screen Size X (columns) 屏幕宽度
int nScreenHeight = 30; // Console Screen Size Y (rows) 屏幕高度
//屏幕宽高要与命令行一致，否则输出会变形。

// 使用单维数组，根据需求更改数组，使用简单的数字
//    0  1  2  3		   0  1  2  3
// 0  0  1  2  3		0 12  8  4  0
// 1  4  5  6  7		1 13  9  5  1
// 2  8  9  10 11		2 14  10 6  2
// 3 12  13 14 15（0）	3 15  11 7  3 （90）
// 旋转函数 px--索引x py--索引y r--旋转的度数
int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return py * 4 + px;			// 0 degrees
	case 1: return 12 + py - (px * 4);  // 90 degrees(旋转90度)
	case 2: return 15 - (py * 4) - px;  // 180 degrees
	case 3: return 3 - py + (px * 4);    // 270 degrees
	}
	return 0;
}

//碰撞检测函数
bool DoesPieceFit(int nTetrimino, int nRotation, int nPosX, int nPosY)
{//四分虫ID(nTetrimino) 当前旋转(nRotation) 形状碎片在数组中的位置(nPosX,nPosY)

	// 检查新形状与形状碎片是否有重叠，
	// 只要有一个重叠就返回false
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			// 四元数组转换成字段数组
			// Get index into piece
			// 用旋转函数得到tetro数组索引
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			// 没有旋转将索引放入字段，根据当前位置，记住停顿
			// 错误代码：int fi = (nPosX + py) * nFieldWidth + (nPosY + px);
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);


			// 需要检查是否出界(c语言编程)
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					// 错误代码
					//if (tetromino[nTetrimino][pi] = L'X' && pField[fi] != 0)
					if (tetromino[nTetrimino][pi] == L'X' && pField[fi] != 0)
						// pi:旋转索引	
						// tetromino[nTetrimino][pi] = L'X':俄罗斯方块指数等于X
						// pField[fi] != 0:竞争环境不等于0
						// 满足条件说明发生了碰撞(场上已经有东西了)

						// 返回false，表示这块放不下
						return false; // fail on first hit
				}
			}

		}
	// 默认情况返回true
	return true;
}

int main() {

	// srand((unsigned)time(NULL));


	// Create assets(创建资产)
	// 游戏的资产：游戏的一部分以及要使用的东西
	// 四氨基块(俄罗斯方块)，存储成一根绳子(W字符串)
	// 形状构建为字符串（形状资产）
	tetromino[0].append(L"..X.");// 四乘四数组的四分体
	tetromino[0].append(L"..X.");// 句号表示空白
	tetromino[0].append(L"..X.");// 大写的X表示形状的一部分
	tetromino[0].append(L"..X.");

	// 其他形状
	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L".X..");
	tetromino[6].append(L".X..");

	// 初始化比赛场地的数组(场地资产)
	// 把数组中的所有东西都设置为0，除非它在数组的侧面或底部
	pField = new unsigned char[nFieldWidth * nFieldHeight]; // Creat play field buffer
	for (int x = 0; x < nFieldWidth; x++)// Board Boundary
		for (int y = 0; y < nFieldHeight; y++)
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
			// x == 0 表示x边界为0
			// x == nFieldWidth - 1 表示x边界为比赛场地的宽度
			// y == nFieldHeight - 1 表示y边界为比赛场地的高度
			// 用9（#）来表示边界，用0来表示空白

	// Create a screen buffer!
	// 有效地使用命令行作为屏幕缓冲区
	// 创建一个w chin数组,保存屏幕宽度和高度，
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	// 填满空白
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	// 抓住控制台缓冲区的句柄，将其设置为活动屏幕缓冲区
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	// 然后使用一个单独的命令来绘制缓冲区
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// 游戏循环（是游戏引擎最重要的部分）：现在发生的一切的顺序
	bool bGameOver = false;

	// 游戏状态
	int nCurrentPiece = 1; // 正在下降de当前碎片
	int nCurrentRotation = 0; // 是否旋转，旋转的角度
	// 当前碎片正在下降,在场地里的哪个位置(x,y)
	int nCurrentX = nFieldWidth / 2;// 设置为字段，宽度除以二
	int nCurrentY = 0;  

	// 存储当前的密钥
	bool bKey[4];// 左右下箭头，z旋转

	// 按住z键强制每50ms旋转一次，需要设置一把锁，让用户长按只旋转一次。
	// 一面旗帜，提示用户是否按住旋转按钮
	bool bRotateHold = false;

	// 游戏的难度（速度）
	int nSpeed = 20;
	// 计算游戏滴答声的计数器
	int nSpeedCounter = 0;
	// 是否压下新的一块
	bool bForceDown = false;

	// 要让线视觉上消失，我们得存储一些行存在的事实。（这里使用矢量）
	vector <int> vLines;

	while (!bGameOver)
	{
// 时机--GAME TIME(提升用户体验)==========================
		// 一个游戏滴答
		this_thread::sleep_for(50ms);
		// 速度计数器
		nSpeedCounter++;
		// 当游戏滴答声的数量等于速度时，压下新的一块
		bForceDown = (nSpeedCounter == nSpeed);

// 用户输入--INPUT========================================

		// 获取同步键状态
		for (int k = 0; k < 4; k++)                             // R    L  D  Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		// GetAsyncKeyState:Get异步键状态函数(获取用户是否按了那个键)
		// "\x27\x25\x28z"[k]:常量字符串表达式
		// 得到一个真或假的数组

// 游戏逻辑--GAME LOGIC:==================================

		// 用户按左键
		if (bKey[1])
		{
			// 检查，当前作品是否适合目前位置的左边
			// DIS拼接功能
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))
			{// DoesPieceFit:合适吗 nCurrentPiece:当前的作品 nCurrentRotation:当前的旋转 
			 // nCurrentX - 1:我们只想检查左边 nCurrentY:不检查垂直运动
				nCurrentX = nCurrentX - 1;
			}
		}

		// 用户按右键
		if (bKey[0])
		{
			// 检查，当前作品是否适合目前位置的左边
			// DIS拼接功能
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY))
			{ // nCurrentX + 1:检查右边 
				nCurrentX = nCurrentX + 1;
			}
		}

		// 用户按下键
		if (bKey[2])
		{
			// 检查，当前作品是否适合目前位置的左边
			// DIS拼接功能
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{ // nCurrentY + 1:检查下边 
				nCurrentY = nCurrentY + 1;
			}
		}

		// 代码优化:
		// nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		// nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		// nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		// 用户按z键（旋转）
		if (bKey[3])
		{
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			// 如果用户按了键，旋转一次
			bRotateHold = true;
		}
		else
			// 如果用户按住旋转键，没有松开，旋转将增加零
			bRotateHold = false;

		//形状下降
		if (bForceDown)
		{
			// 是否可以往下压
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;// It can, so do it!
			else// 不能往下压
			{
				// Lock the current piece in the field 锁定当前部分在场地中
				// 并成为背景的一部分 
				// 重新遍历tetromino阵列，并转换成字段
				for (int px = 0; px < 4; px++)// Board Boundary
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
						// 当那里有一个X
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
							// 用当前值加1更新我们的字段

				// Check have we got any lines 检查是否创造出完整的水平线
				// 不需要检查整个场地，只需要查最后一块下落的tetromino碎片
				// 遍历四元数组，每次检查一行
				for(int py = 0; py < 4; py++)
					// 检查是否超出场地边界（nFieldHeight - 1）
					if (nCurrentY + py < nFieldHeight - 1)// nCurrentY:作品当前位置 py:tetro的偏移量
					{  // nCurrentY + py：四元数组的第几行
						// 假设有一条线
						bool bLine = true;
						// 遍历这条线上的碎片（不包括边界，so 1=<px<nFieldWidth - 1）
						for (int px = 1; px < nFieldWidth - 1; px++)
							// 行中如果有空格（即索引到的场中元素为0），bLine = false
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0; // b &= a --> b = b & a(与)
							// pField[(nCurrentY + py) * nFieldWidth + px]
							// 对字段进行二维索引？
							// 与在 Draw Field 里的用法类似

						// 如果存在一条线（即这一行被填满，没有空格）
						if (bLine)
						{
							// Remove Line,set to =
							// 把他们替换为 =
							for (int px = 1; px < nFieldWidth - 1; px++)
								(pField[(nCurrentY + py) * nFieldWidth + px]) = 8;

							// 存储行存在的事实
							vLines.push_back(nCurrentY + py);
						}
					}

				// Choose next piece 选择下一件作品
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0; // 旋转设置为0
				nCurrentPiece = rand() % 7; // 伪随机取下一个

				// if piece	dose not fit 如果作品不能再下压，那么游戏结束
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}

			// 速度计数器再次调回零
			nSpeedCounter = 0;
		}

		//评分


// 渲染输出--RENDER OUTPUT================================

		// Draw Field
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
				// 0-- (用来填充空白) 1--A（形状） ...  8--=（用来画一条线） 9--#（边界） 

		// Draw Current Piece
			// 当前活动的部分，掉落并被玩家控制的piece需要单独绘制，因为他还不是领域的一部分
			// 迭代tetro的所有细胞
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					// nCurrentPiece:目前的四元数正在下降
					// Rotate(px, py, nCurrentRotation):得到旋转的索引
					// nCurrentRotation:当前旋转
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
		
																				// 0 + 65 --> A

		// 如果线条存在，更新缓冲区
		if (!vLines.empty())
		{
			// Display Frame(cheekily to draw lines)
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);// Delay a bit

			// Remove Lines（消除线条）
			// 去掉线条的同时，向下移动线条上方的所有部分
			for(auto &v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];// 向下移动
					pField[px] = 0;// 消除
				}

			vLines.clear();
		}

		// Display Frame
			// 控制台输出字符数组绘制屏幕，从位置零开始
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

	}
	// Game Over
	// CloseHandle(hConsole);
	return 0;
}

// 简单的游戏引擎，主游戏循环有以下部分组成：
// 时机--GAME TIME(提升用户体验)
// 用户输入--INPUT
// 游戏逻辑--GAME LOGIC:
	//形状下降，碰撞检测，评分
// 渲染输出--RENDER OUTPUT	