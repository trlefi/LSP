#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <malloc.h>
#include <math.h>
#include <thread>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <set>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <locale>
#include <string>
#include <codecvt>
#include <fstream>
#include "数组.h"
#include "跑图数组.h"
#include "nlohmann/json.hpp"
#include <MemoryTools.h>
#include <chrono>
#include <ctime>
#include <random>
#include <sstream>
#include <cmath>
#include <iomanip>



static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4],
    char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
            char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
            ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
        char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
        char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}


#define MAX_BUFFER_SIZE 4096
void error(const char *msg) {
    perror(msg);
    exit(1);
}

std::string getWebPageSource(const char *hostname, const char *path) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }

    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        error("No such host found");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(80); // Default HTTP port

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Error connecting");
    }

    char request[MAX_BUFFER_SIZE];
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\n\r\n", path,
        hostname);

    if (write(sockfd, request, strlen(request)) < 0) {
        error("Error writing to socket");
    }

    char buffer[MAX_BUFFER_SIZE];
    std::string result;

    // Flag to indicate when to start saving the response
    bool saveResponse = false;
    // Variable to store the expected content length
    ssize_t contentLength = -1;

    while (true) {
        ssize_t bytesRead = read(sockfd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';

            if (!saveResponse) {
                // Check for empty line, indicating end of headers
                if (strstr(buffer, "\r\n\r\n")) {
                    saveResponse = true;
                    // Skip the empty line
                    result += strstr(buffer, "\r\n\r\n") + 4;

                    // Extract content length
                    const char *contentLengthHeader = "Content-Length: ";
                    const char *contentLengthStart = strstr(buffer, contentLengthHeader);
                    if (contentLengthStart) {
                        contentLengthStart += strlen(contentLengthHeader);
                        contentLength = strtol(contentLengthStart, nullptr, 10);
                    }
                }
            } else
            {
                result += buffer;

                // Check if we've received the expected content length
                if (contentLength >= 0 && result.length() >= static_cast < size_t > (contentLength)) {
                    break;
                }
            }
        } else if (bytesRead == 0) {
            break; // Connection closed by remote end
        } else
        {
            error("Error reading from socket");
        }
    }

    close(sockfd);
    return result;
}




void deleteFolder(const char *folderPath) {
    DIR *dir = opendir(folderPath);
    struct dirent *entry;
    char filePath[256];

    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                sprintf(filePath, "%s/%s", folderPath, entry->d_name);

                if (entry->d_type == DT_DIR) {
                    deleteFolder(filePath); // 递归删除子文件夹
                } else
                {
                    remove(filePath); // 删除文件
                }
            }
        }
        closedir(dir);
    }

    rmdir(folderPath); // 删除空文件夹
}

double totalProgressPercent = 0;
using json = nlohmann::json;
using namespace std;
std::atomic < bool > keepWriting(false); //循环隐身
// 兼容了加密
// 改了包名
// 全渠道自适应
using namespace std;
// byte定义
typedef unsigned char byte;
// 任意门
int ball;
string call; // 定义文件通讯读取的变量
typedef char PACKAGENAME;

unsigned long long int str_length(const char s[]) {
    int len = 0;
    while (s[len])
    len++;
    return len;
}


std::string GetPathBetween(const std::string & fullPath, const std::string & startMarker,
    const std::string & endMarker) {
    size_t startIndex = fullPath.find(startMarker);
    if (startIndex == std::string::npos) {
        return ""; // 没有找到起始标记
    }

    startIndex += startMarker.length();
    size_t endIndex = fullPath.find(endMarker, startIndex);
    if (endIndex == std::string::npos) {
        return ""; // 没有找到结束标记
    }

    return fullPath.substr(startIndex, endIndex - startIndex);
}

std::string getExecutablePath() {
    // 在这里使用您选择的方法来获取可执行文件的路径
    // 示例代码中使用了读取 /proc/self/exe 的方法
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::string(buffer);
    } else {
        // 处理错误情况，这里简单地返回一个空字符串
        return "";
    }
}



std::string executablePath = getExecutablePath();
std::string extractedPath = GetPathBetween(executablePath, "/data/user/0/", "/");

string pkgName[11] = {
    "com.tencent.tmgp.eyou.eygy",
    "com.netease.sky",
    "com.netease.skz",
    "com.netease.sky.m4399",
    "com.netease.sky.vivo",
    "com.netease.sky.mi",
    "com.netease.sky.bilibili",
    "com.netease.sky.nearme.gamecenter",
    "com.netease.sky.huawei",
    "com.netease.sky.aligames",
    extractedPath
};

string qvdao[11] = {
    "应用宝",
    "官服",
    "官服共存",
    "4399",
    "vivo",
    "小米",
    "哔哩哔哩",
    "OPPO",
    "华为",
    "九游",
    "自定义"
};

const char* extractedPathCStr = extractedPath.c_str();
const char *baom[11] = {
    "com.tencent.tmgp.eyou.eygy",
    "com.netease.sky",
    "com.netease.skz",
    "com.netease.sky.m4399",
    "com.netease.sky.vivo",
    "com.netease.sky.mi",
    "com.netease.sky.bilibili",
    "com.netease.sky.nearme.gamecenter",
    "com.netease.sky.huawei",
    "com.netease.sky.aligames",
    extractedPathCStr
}; //包名数组

const char *jcbm;
string myPkgName;
string tempName;
string tempName1;
string tempNamea;



size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
    string *str = (string *) userdata;
    str->append(ptr, size * nmemb);
    return size * nmemb;
}


void getPackageName(void) {

    unsigned long long int id = -1;
    DIR *dir;
    FILE *fp;
    char filename[32];
    char cmdline[256];
    struct dirent *entry;
    dir = opendir("/proc");
    while ((entry = readdir(dir)) != NULL) {
        id = atoi(entry->d_name);
        if (id != 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);
                for (int i = 0; i < 11; i++) {
                    if (strcmp(pkgName[i].c_str(), cmdline) == 0) {
                        // printf("当前渠道为：%s\n",qvdao[i].c_str());
                        if (qvdao[i] == "华为") {
                            myPkgName = "com.sky.yhy";
                            // printf("\n%s\n",myPkgName);
                            return;
                        } else
                        {
                            myPkgName = pkgName[i].c_str();
                            return;
                        }

                    }
                }
            }
        }
    }
    // printf("运行的什么几把游戏\n");
    closedir(dir);
    return;

}

struct YdxgData
{
    std::string name;
    unsigned long long int values[13];
};



// 以下封装是用来根据包名获取进程pid的，看不懂不用管
unsigned long long int getPID(const char *packageName) {
    unsigned long long int id = -1;
    DIR *dir;
    FILE *fp;
    char filename[64];
    char cmdline[64];
    struct dirent *entry;
    dir = opendir("/proc");
    while ((entry = readdir(dir)) != NULL) {
        id = atoi(entry->d_name);
        if (id != 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);
                if (strcmp(packageName, cmdline) == 0) {
                    return id;
                }
            }
        }
    }
    closedir(dir);
    return -1;
}

typedef long int ADDRESS;

char *ztm;
char *jhm;
char *jqm;



unsigned long long int getso2(int pid, const char *module_name) {
    FILE *fp; // 声明文件指针变量
    long addr = 0; // 初始化模块地址为0
    char *pch; // 声明字符指针变量
    char filename[64]; // 声明字符串变量，用于存储文件名
    char line[1024]; // 声明字符串变量，用于存储文件内容
    int bl = 0; // 标识是否是第一行
    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid); // 构造maps文件名
    fp = fopen(filename, "r"); // 打开maps文件
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) // 循环读取maps文件每行内容
        {
            if (strstr(line, module_name)) // 判断当前行是否包含指定模块名称
            {
                if(bl == 0) // 如果是第一行，则忽略
                {
                    bl++;
                } else // 如果不是第一行，则提取模块地址并返回
                {
                    pch = strtok(line, "-"); // 以"-"为分隔符提取字符串
                    addr = strtoul(pch, NULL, 16);
                    if (addr == 0x8000)
                    addr = 0;
                    break;
                }
            }
        }
        fclose(fp);
    }
    return addr;
}

// 这个封装用于获取一个so的起始内存地址
// 也不用看懂 我直接从内存插件里拆来的代码
unsigned long long int get_appmodule_base(int pid, const char *module_name) {
    FILE *fp;
    long addr = 0;
    char *pch;
    char filename[64];
    char line[1024];
    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    fp = fopen(filename, "r");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                pch = strtok(line, "-");
                addr = strtoul(pch, NULL, 16);
                if (addr == 0x8000)
                addr = 0;
                break;
            }
        }
        fclose(fp);
    }
    return addr;
}

unsigned long long int apphandle;


// 获取64位指针
long int lsp64(long int addr) {
    unsigned long long int var = 0;
    pread64(apphandle, &var, 8, addr);
    return var;
}
//传入一个内存地址  读取16进制值然后以类型返回
float getFloat(long int addr) {
    float var = 0;
    pread64(apphandle, &var, 4, addr);
    return var;
}

int getDword(long int addr) {
    unsigned long long int var = 0;
    pread64(apphandle, &var, 4, addr);
    return var;
}

long getQword(long int addr) {
    long var = 0;
    pread64(apphandle, &var, 8, addr);
    return var;
}

int8_t getByte(long int addr) {
    unsigned long long int var = 0;
    pread64(apphandle, &var, 1, addr);
    return var;
}

//-----剪短的封装写入地址

//D类型
int WriteAddr_DWORD(long int addr, int value) {
    pwrite64(apphandle, &value, 4, addr);
    return 0;
}

//F类型
float WriteAddr_FLOAT(long int addr, float value) {
    pwrite64(apphandle, &value, 4, addr);
    return 0;
}
//B类型
int8_t WriteAddr_BYTE(long int addr, int value) {
    pwrite64(apphandle, &value, 1, addr);
    return 0;
}
//W类型
int16_t WriteW(long int addr, int value) {
    pwrite64(apphandle, &value, 2, addr);
    return 0;
}
//Q类型
int64_t WriteAddr_QWORD(long int addr, long int value) {
    pwrite64(apphandle, &value, 32, addr);
    return 0;
}
//E类型
double WriteE(long int addr, double value) {
    pwrite64(apphandle, &value, 64, addr);
    return 0;
}


float PTT(long int zbp, float x, float y, float z) {
    WriteAddr_FLOAT(zbp + 0x4, x);
    WriteAddr_FLOAT(zbp + 0x8, y);
    WriteAddr_FLOAT(zbp + 0xC, z);
    return 0;
}

float SYPTT(long int zbp, float x, float y, float z, const std::string & extractedPath) {
    std::string filePath =
    "/data/user/0/" + extractedPath + "/files/遇见配置/我的地址.txt";
    std::ifstream infile(filePath);
    float currentX,
    currentY,
    currentZ;

    if (infile.is_open()) {
        std::string line;
        std::getline(infile, line);
        std::istringstream iss(line);
        iss >> currentX >> currentY >> currentZ;
        infile.close();
    } else
    {
        WriteAddr_FLOAT(zbp + 0x4, x);
        WriteAddr_FLOAT(zbp + 0x8, y);
        WriteAddr_FLOAT(zbp + 0xC, z);
        std::ofstream outfile(filePath);
        if (outfile.is_open()) {
            outfile << x << ' ' << y << ' ' << z;
            outfile.close();
        }
        return 0;
    }

    float dx = x - currentX;
    float dy = y - currentY;
    float dz = z - currentZ;
    float distance = sqrt(dx * dx + dy * dy + dz * dz);
    dx /= distance; // 修正运算符
    dy /= distance; // 修正运算符
    dz /= distance; // 修正运算符

    float speed = std::min(distance / 500.0f, 0.01f); // Here we set speed
    // dynamically

    auto start_time = std::chrono::high_resolution_clock::now();
    float elapsed_time = 0;

    while (distance > 0) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time =
        std::chrono::duration < float,
        std::milli > (current_time - start_time).count();
        start_time = current_time;

        float step = speed * delta_time;
        if (step > distance) {
            step = distance;
        }

        currentX += dx * step;
        currentY += dy * step;
        currentZ += dz * step;

        WriteAddr_FLOAT(zbp + 0x4, currentX);
        WriteAddr_FLOAT(zbp + 0x8, currentY);
        WriteAddr_FLOAT(zbp + 0xC, currentZ);

        distance -= step;
        elapsed_time += delta_time;

        if (elapsed_time < 50) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 - (int)elapsed_time));
        }
        elapsed_time = 0;
    }

    WriteAddr_FLOAT(zbp + 0x4, x);
    WriteAddr_FLOAT(zbp + 0x8, y);
    WriteAddr_FLOAT(zbp + 0xC, z);

    std::ofstream outfile(filePath);
    if (outfile.is_open()) {
        outfile << x << ' ' << y << ' ' << z;
        outfile.close();
    }
    return 0;
}

// 两个封装获取全渠道基址
unsigned long long int jzz() {
    for (int u = 0; u < 11; u++) {
        // char *bbm=baom[u];
        unsigned long long int ipid = getPID(baom[u]); // 网易官服
        char lj[64];
        sprintf(lj, "/proc/%d/mem", ipid);
        apphandle = open(lj, O_RDWR);
        if (apphandle == 0) {
            puts("网易官服获取mem失败!");
            exit(1);
        }
        if (-1 != ipid) {
            return ipid;
        }
    }
}

unsigned long long int hqjz() {

    // printf("\nxaipid=%lx\n", ipid);
    char mname[] = "libBootloader.so"; // 基址入口模块
    unsigned long long int ipid = jzz();
    printf("\nipid=%lx\n", ipid);
    if (isapkrunning("com.netease.sky.bilibili") == 1) {
        //判断是否，true返回哔哩哔哩so
        unsigned long long int fool = getso2(ipid, mname);
        return fool;
    }
    unsigned long long int fool = get_appmodule_base(ipid, mname);
    return fool;
}

unsigned long long int pddz(string str, long int addr, int value, long int tzpy) {
    for (int i = 0; i <= 160000; i++) {
        if (getDword(addr + (i * 0x4) + tzpy) == value) {
            printf("%s 地址为%lX\n", str.c_str(), addr + (i * 0x4));
            return addr + (i * 0x4);
        } else if (getDword(addr - (i * 0x4) + tzpy) == value) {
            printf("%s 地址为%lX\n", str.c_str(), addr - (i * 0x4));
            return addr - (i * 0x4);
        }
    }
    printf("%s 没抓到\n", str.c_str());
}

unsigned long long int so = hqjz();


// 要去掉字符串的话，字符串和string str 和%s和
// str.c_str()都去掉
// 封装
unsigned long long int fbjz(string str, long int A, long int B) {
    unsigned long long int C = lsp64(A + B);
    // printf("%s 地址为:%lX\n",str.c_str(),C);
    return C;
}

// 封装
long long int lsp64_1(string str, long int A, long int B, long int C) {
    unsigned long long int E = lsp64(lsp64(so + A) + B) + C;
    // printf("%s 地址为:%lX\n",str.c_str(),E);
    return E;
}

long long int lsp64_2(string str, long int A, long int B, long int C, long int D) {
    unsigned long long int F = lsp64(lsp64(lsp64(so + A) + B) + C) + D;
    // printf("%s 地址为:%lX\n",str.c_str(),F);
    return F;
}

// 功能基址
unsigned long long int GFSDJZ = 0x361EB08;
unsigned long long int GFRWJZ = 0x33D0688;

// xa功能地址
unsigned long long int xadh = so + 0x13346CC; //点火   ✓
unsigned long long int xazh = so + 0x11F0D18; //炸花 ✓
unsigned long long int xaxh = so + 0x4AEDE4; //吸火


unsigned long long int DJMS = so + 0x1575288; // 单机模式 ✓
unsigned long long int wxnl = lsp64_1("无限能量", 0x36298C8, 0x30770, -0x1A8); //无限能量

// cb功能
unsigned long long int qyg = so + 0xC01533; //全衣柜 26 改 82
unsigned long long int TGDH = so + 0x36E31F8; //跳过动画 0改1
unsigned long long int youyi = so + 0x36AF04C; //友谊开关 0改1

// 速度地址
unsigned long long int jz1 = lsp64_1("速度地址", GFSDJZ, 0x1B8, 0x28);
// 人物地址
unsigned long long int zero = lsp64_2("人物地址", GFSDJZ, 0x228, 0x1BF8, -0x4);;
unsigned long long int yins = lsp64_2("隐身", GFRWJZ, 0xF0, 0x20, 0x42BC); //隐身
unsigned long long int hysl = lsp64_2("好友数量", GFRWJZ, 0x8, 0x618, 0x1B8); // 好友数量
unsigned long long int lazushuliang = lsp64_2("蜡烛数量", GFRWJZ, 0x10, 0xE0, 0x80); //蜡烛数量
unsigned long long int gysl = lsp64_2("光翼数量", GFRWJZ, 0x8, 0x718, 0x61C0); // 光翼数量
// 速度功能地址
unsigned long long int rym = lsp64_1("任意门", GFSDJZ, 0x4E8, 0x7C); // 任意门
unsigned long long int ydpy = lsp64_1("原地跑图", GFSDJZ, 0x770, 0x428); // 原地跑图
unsigned long long int ydrw = lsp64_1("原地任务", GFSDJZ, 0x2B0, 0x20); //原地任务
unsigned long long int zhahua = lsp64_1("炸花", GFSDJZ, 0x7F8, 0x934); // 炸花起始
unsigned long long int YCLZ = lsp64_1("隐藏蜡烛", GFSDJZ, 0x650, 0x5B0); // 隐藏蜡烛
unsigned long long int yxhz = lsp64_1("高清画质", GFSDJZ, 0x88, 0xAC0); // 游戏画质
unsigned long long int DTGY = lsp64_1("原地光翼", GFSDJZ, 0x630, 0xA0); //原地光翼

// 发包指针及提交
unsigned long long int FBZZ = lsp64_1("发包指针", GFSDJZ, 0x1E0, 0x10); // 发包指针
unsigned long long int Submission = pddz("提交", lsp64_1("提交(NO)", GFSDJZ, 0x1D0, -0x558C8), -1, 0x98); // 提交
unsigned long long int axid = pddz("爱心id", fbjz("爱心id", FBZZ, 0x6C0) - 0xB148, 2, 0x8); // 爱心id
unsigned long long int xhid = pddz("心火id", fbjz("心火id", FBZZ, 0x6C0) + 0xB1C0, 6, 0x8); //心火id

// 发包功能
unsigned long long int ydgy = fbjz("吸收光翼", FBZZ, 0x110); //原地光翼
unsigned long long int ydxgsd = fbjz("原地赛道", FBZZ, 0xA8); // 原地霞谷
unsigned long long int XZDZ = fbjz("全部动作", FBZZ, 0x70); // 全部动作
unsigned long long int YDSK = fbjz("原地神龛", FBZZ, 0xD0); // 开图
unsigned long long int giftt = fbjz("送心火", FBZZ, 0x458); //送心火
unsigned long long int shouxinh = fbjz("收心火", FBZZ, 0x460); //收心火
unsigned long long int lahei = fbjz("拉黑", FBZZ, 0x3D0); // 拉黑  上线在看看
unsigned long long int shouaxin = fbjz("收爱心", FBZZ, 0x430); // 收心
unsigned long long int huoqurw = fbjz("任务获取", FBZZ, 0x5F8); // 任务获取
unsigned long long int jieshourw = fbjz("接受任务", FBZZ, 0x88); // 接受任务
unsigned long long int xiugairw = fbjz("完成任务", FBZZ, 0xC8); // 完成任务
unsigned long long int tijiaorw = fbjz("提交任务", FBZZ, 0x90); // 提交任务
unsigned long long int dxbaddr = lsp64_2("自身身高", GFRWJZ, 0x8, 0x778, 0x50); // 身高
typedef unsigned char byte;


// 模拟人类(献祭)
void human_delay(int base_delay_ms, int variance_ms) {
    int random_delay = base_delay_ms + rand() % (variance_ms + 1);
    usleep(random_delay * 1000); // 将毫秒转换为微秒
}

// 光翼随机
int random_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

// 模拟人类(光翼)
void simulate_human_operation() {
    // 随机生成10到15秒之间的延迟
    int delay_time = random_int(15000, 22000);
    usleep(delay_time * 1000); // 将毫秒转换为微秒
}


unsigned long long int renyimen(int dt) {
    // 地图代码
    unsigned char doorx[25];
    memcpy(doorx, mapxp[dt][0], 25);
    WriteAddr_DWORD(rym + 0x60, 1);
    WriteAddr_QWORD(rym + 0x2C, 49);
    WriteAddr_QWORD(rym + 0x34, 24);
    WriteAddr_QWORD(rym + 0x3C, rym + 0x3FDC);
    WriteAddr_DWORD(rym + 0x409C, 1);
    /* WriteAddr_DWORD(rym + 0x40A0, 1); WriteAddr_DWORD(rym + 0x40A4, 1);
	   WriteAddr_DWORD(rym + 0x40A8, 1); */
    // 地图修改
    for (int m = 0; m < 24; m++) {
        WriteAddr_BYTE(rym + 0x3FDC + m, doorx[m]);
    }
    for (int m = 0; m < 16; m++) {
        WriteAddr_BYTE(rym + 0x2C + 24 + m, yanse[m]);
    }
    WriteAddr_FLOAT(rym - 0x14, 80000);
    WriteAddr_FLOAT(rym - 0x28, 80000);
    WriteAddr_FLOAT(rym - 0x3C, 80000);
    while (true) {
        if (atoi(mapxp[dt][1]) == getDword(ydpy)) {
            cout << "已到达" << endl;
            break;
        }
    }
}

void fb(long int addr) {
    // 发包提交
    WriteAddr_QWORD(Submission + 8, addr);
    WriteAddr_DWORD(addr + 0xC, 2);
    WriteAddr_DWORD(Submission, 1);
    WriteAddr_DWORD(Submission + 4, 0);
    time_t time1 = time(NULL);
    while (1) {
        time_t time2 = time(NULL);
        if (difftime(time2, time1) > 4.0) {
            WriteAddr_DWORD(Submission, 2);
            WriteAddr_DWORD(Submission + 0x4, 31);
            break;
        }
        if (getDword(Submission + 0x4) == 1) {
            puts("完成");
            break;
        }
    }
}

void szfb(std::vector < int > a, long int addr, long int JG) {
    // 数值发包
    for (int i = 0; i < a.size(); i++) {
        WriteAddr_DWORD(addr + JG + i * 4, a[i]);
    }
    // printf("地址为%lx\n",addr);
    fb(addr);
}

// 指针文本发包
void wbfb(int A, const char *a, long int addr, long int JG) {
    byte tmp[A];
    memset(tmp, 0, A);
    memcpy(tmp, a, str_length(a));
    for (int u = 0; u < A; u++) {
        WriteAddr_BYTE(addr + JG + u, tmp[u]);
    }
    fb(addr);
}

// int X;
unsigned long long int HQDS() {
    // 获取当前位置
    for (int i = 1; i < sizeof(mapxp) / sizeof(mapxp[0]); i++) {
        if (getDword(ydpy) == atoi(mapxp[i][1])) {
            // X = i;
            return i;
        }
    }
}




void WritePercentageToFile(double progress, double totalProgressPercent, double ditushu,
    const std::string & mingcheng, const std::string & extractedPath) {
    unsigned long long int progressInt = static_cast < int > (progress);
    unsigned long long int totalProgressPercentInt = static_cast < int > (totalProgressPercent);

    std::ofstream file("/data/data/" + extractedPath + "/files/遇见配置/" + mingcheng);
    if (file.is_open()) {
        file << "{" << std::endl;
        file << "\"地图进度\":\"" << progressInt << "\"," << std::endl;
        file << "\"总进度\":\"" << totalProgressPercentInt << "\"," << std::endl;
        file << "\"地图数量\":\"" << ditushu << "\"" << std::endl;
        file << "}" << std::endl;
        file.close();
    } else
    {
        std::cout << "无法打开文件。" << std::endl;
    }
}

void showcandle() {
    // 显示隐藏蜡烛
    for (int i = 0; i < 193; i++) {
        WriteAddr_DWORD(YCLZ + (i * 0x70), 28673);
    }
    puts("完成");
}

int TP(int A) {
    for (int i = 0; i < 79; i++) {
        // 你数组最大的地图数量
        if (A == atoi(mapxp[i][1])) {
            // 地图数组
            renyimen(i);
            return 0;
        }
    }
    puts("完成");
}

// 先祖白蜡
void XZBL() {
    for (int i = 0; i < 17; i++) {
        TP(atoi(XZLZR[i][0]));
        sleep(3);
        for (int u = 2; u < atoi(XZLZR[i][1]) + 2; u++) {
            wbfb(24, XZLZR[i][u], ydxgsd, 0x60);
            sleep(0.5);
        }
    }
    std::ifstream returnFile("/data/user/0/" + extractedPath + "/files/遇见配置/返回云巢.txt");
    if (returnFile.is_open()) {
        std::string returnLine;
        if (std::getline(returnFile, returnLine)) {
            if (returnLine == "开") {
                renyimen(75);
            } else {
                renyimen(1);
            }
        }
        returnFile.close();
    }
    puts("完成");
}

void rw(int x) {
    // 发包任务
    fb(huoqurw);
    sleep(0.5);
    std::vector < int > idk(4);
    for (int i = 0; i < 4; i++) {
        idk[i] = getDword(huoqurw + 0x1B8 + (i * 4));
    }
    szfb(idk, jieshourw, 0x60);
    sleep(0.5);
    unsigned long long int rows = sizeof(rwid) / sizeof(rwid[0]);
    for (int i = 0; i < rows; i++) {
        if (idk[x] == rwid[i][0]) {
            WriteAddr_DWORD(xiugairw + 0x60 + 8 + ((rwid[i][1] - 1) * 8), rwid[i][1]);
            WriteAddr_FLOAT(xiugairw + 0x60 + 8 + 4 + ((rwid[i][1] - 1) * 8),
                getFloat(ydrw + ((rwid[i][1] - 1) * 8) + 4) + rwid[i][2]);
            WriteAddr_FLOAT(ydrw + ((rwid[i][1] - 1) * 8) + 4,
                getFloat(ydrw + ((rwid[i][1] - 1) * 8) + 4) + 60);
        }
    }
    fb(xiugairw);
    sleep(0.5);
    WriteAddr_DWORD(tijiaorw + 0x78, idk[x]);
    fb(tijiaorw);
    sleep(0.5);
    puts("完成");
}

void js(int x) {
    WriteAddr_FLOAT(jz1, x);
}


int rwsl;
int HQrws() {
    //获取任务数量
    int t = 0;
    while (true) {
        if (getDword((ydrw + (t * 0x8) + 0x8)) < getDword((ydrw + (t * 0x8)))) {
            break;
        }
        t = t + 1;
    }
    rwsl = t + 1;
    return rwsl;
}

void yjxj() {
    srand(time(NULL)); // 初始化随机数种子

    WriteAddr_DWORD(TGDH, 1); // 开始操作

    js(10); // 执行加速10倍

    renyimen(35); // 任意门伊甸献祭

    human_delay(3000, 2000); // 休眠3到5秒之间的随机时间

    // 获取光翼数量
    int guangyi_count = getDword(gysl);

    // 定义完整的地址数组
    int addresses[] = {
        0xEF0,
        0x1010,
        0x1130,
        0x1250,
        0x1370,
        0x1490,
        0x15B0,
        0x16D0,
        0x17F0,
        0x1910,
        0x1A30,
        0x1B50,
        0x1C70,
        0x1D90,
        0x1EB0,
        0x1FD0,
        0x20F0,
        0x2210,
        0x2330,
        0x2450,
        0x2570,
        0x2690,
        0x27B0,
        0x28D0,
        0x29F0,
        0x2B10,
        0x2C30,
        0x2D50,
        0x2E70,
        0x2F90,
        0x30B0,
        0x31D0,
        0x32F0,
        0x3410,
        0x3530,
        0x3650,
        0x3770,
        0x3890,
        0x39B0,
        0x3AD0,
        0x3BF0,
        0x3D10,
        0x3E30,
        0x3F50,
        0x4070,
        0x4190,
        0x42B0,
        0x43D0,
        0x44F0,
        0x4610,
        0x4730,
        0x4850,
        0x4970,
        0x4A90,
        0x4BB0,
        0x4CD0,
        0x4DF0,
        0x4F10,
        0x5030,
        0x5150,
        0x5270,
        0x5390,
        0x54B0,
        0x55D0
    };

    // 修复黑门
    if (guangyi_count == 0) {
        human_delay(2000, 2000); // 休眠2到4秒之间的随机时间
        wbfb(24, "l_CandleSpace_0", ydgy, 0x60); //获取光翼1
        //现在应该在小黑屋  吸取这个光翼

        human_delay(2000, 2000); // 休眠2到4秒之间的随机时间

        renyimen(38); // 任意门 星光大道

        human_delay(2000, 2000); // 休眠2到4秒之间的随机时间

        renyimen(39); // 任意门 重生之路
        human_delay(2000, 2000); // 休眠2到4秒之间的随机时间

        PTT(zero, -0.12149596959352493f, 0.5504489541053772f, 233.96255493164062f); // 重生门

        human_delay(2000, 2000); // 休眠2到4秒之间的随机时间

        renyimen(1); // 任意门遇境

        human_delay(500, 500); // 休眠0.5到1秒之间的随机时间

        js(1); // 完成 加速改1

        WriteAddr_DWORD(TGDH, 0); // 结束操作
        puts("完成");
        return; // 结束函数

        //黑门已修复
    }

    // 确定需要献祭的次数
    int count = guangyi_count > 70 ? 63: guangyi_count - 2;

    // 献祭光翼
    for (int i = 0; i < count; i++) {
        WriteAddr_FLOAT(wxnl, 14);

        // 模拟5%的概率删除一个地址
        if (rand() % 100 < 5) {
            printf("删除地址 0x%X\n", addresses[i]);
            continue; // 跳过当前地址，不执行献祭操作
        }

        WriteAddr_DWORD(DTGY + addresses[i], 2); // 写入操作
        human_delay(1000, 1500); // 随机延迟1到1.5秒之间
    }

    puts("去掉翼");
    // 假设以下函数也是游戏操作的模拟
    PTT(zero, -12.857748985290527f, 83.97386932373047f, -250.93673706054688f); // 掉翼
    human_delay(2000, 4000); // 休眠2到6秒之间的随机时间
    while (1) {
        guangyi_count = getDword(gysl);
        if (guangyi_count == 0) {
            //检测是否自身光翼掉完
            break;
        }
        human_delay(1000, 1500); // 随机延迟1到1.5秒之间
    }
    renyimen(38); // 任意门 星光大道
    human_delay(1000, 4000); // 休眠1到5秒之间的随机时间
    PTT(zero, -0.12149596959352493f, 0.5504489541053772f, 233.96255493164062f); // 重生门
    human_delay(3000, 5000); // 休眠3到8/秒之间的随机时间
    renyimen(39); // 任意门 重生之路
    human_delay(2000, 6000); // 休眠2到8秒之间的随机时间
    renyimen(1);
    js(1); // 完成 加速改1

    puts("完成");
    WriteAddr_DWORD(TGDH, 0); // 结束操作
}


void hysg() {
    float sg,
    sg1,
    sgsj,
    sgsj1,
    sgsj2;
    unsigned long long int tmp[22];
    char name[99],
    input[256],
    files[256],
    formattedValue[256];
    sprintf(files, "/data/user/0/%s/files/遇见配置/好友/", jcbm);
    deleteFolder(files);
    mkdir(files, 0777);
    int friendsnum = getDword(hysl);

    for (int j = 0; j < friendsnum; j++) {
        memset(name, 0, sizeof(name));
        for (int i = 0; i < 22; i++) {
            tmp[i] = getByte(hysl + 0x28 + i + 1 + 0x2D0 * j);
            if (tmp[i] == 0) break;
            sprintf(input, "%c", tmp[i]);
            strcat(name, input);
        }

        sg = getFloat(hysl + 0x28 + 0x284 + 0x2D0 * j);
        sg1 = getFloat(hysl + 0x28 + 0x284 + 0x2D0 * j + 0x4);
        sgsj = 7.6 - 8.3 * sg - 3 * sg1;
        sgsj1 = 1.6 - 8.3 * sg;
        sgsj2 = 7.6 - 8.3 * sg - 3 * (-2);

        json hysgJson;
        hysgJson["好友名字"] = name;

        sprintf(formattedValue, "%.5f", sg);
        hysgJson["体型值"] = formattedValue;

        sprintf(formattedValue, "%.5f", sg1);
        hysgJson["身高值"] = formattedValue;

        sprintf(formattedValue, "%.5f", sgsj);
        hysgJson["目前身高"] = formattedValue;

        sprintf(formattedValue, "%.5f", sgsj1);
        hysgJson["最高可到"] = formattedValue;

        sprintf(formattedValue, "%.5f", sgsj2);
        hysgJson["最矮可到"] = formattedValue;

        std::string jsonString = hysgJson.dump();
        char hysgFilePath[256];
        sprintf(hysgFilePath, "/data/user/0/%s/files/遇见配置/好友/❤️%s", jcbm, name);
        FILE *fp = fopen(hysgFilePath, "w");
        if (fp != nullptr) {
            fprintf(fp, "%s", jsonString.c_str());
            fclose(fp);
        }
    }
    puts("完成");
}





unsigned long long int ydkt() {
    renyimen(75);
    sleep(3);
    // 发包开图
    for (int i = 0; i < 41; i++) {
        wbfb(40, kt[i], YDSK, 0x60);
    }
    sleep(3);
    renyimen(75);
    puts("完成");
}

void HQDZ(int sl) {
    // 全部动作
    for (int i = 0; i < sl; i++) {
        wbfb(24, AncestorID[i][0], XZDZ, 0x60);
    }
    puts("完成");
}



void DHJL1() {
    renyimen(3);
    sleep(2);
    int HBSJ[25] = {
        static_cast < int > (-1817621630),
        static_cast < int > (-11796233),
        static_cast < int > (-11796252),
        static_cast < int > (-11796259),
        static_cast < int > (-11796223),
        static_cast < int > (-11796195),
        static_cast < int > (-11796198),
        static_cast < int > (-11796193),
        static_cast < int > (-11796201),
        static_cast < int > (-11796194),
        static_cast < int > (-11796248),
        static_cast < int > (-11796435),
        static_cast < int > (-11796107),
        static_cast < int > (-11796199),
        static_cast < int > (-11796191),
        static_cast < int > (-11796229),
        static_cast < int > (-11796247),
        static_cast < int > (-11796102),
        static_cast < int > (-11796242),
        static_cast < int > (-11796100),
        static_cast < int > (-11796099),
        static_cast < int > (-11796101),
        static_cast < int > (-11796196),
        static_cast < int > (-11796200),
        static_cast < int > (-11796192)
    };

    for (int i = 0; i < 25; i++) {
        WriteAddr_DWORD(ydpy + 0x1C + 4 + (i * 4), HBSJ[i]);
    }
    WriteAddr_DWORD(ydpy + 0x1C + 33 * 4, 32);
    sleep(3);
    renyimen(1);
    puts("完成");
}

void DHJL2() {
    renyimen(9);
    sleep(2);
    int HBSJ[25] = {
        static_cast < int > (164626931),
        static_cast < int > (-11796107),
        static_cast < int > (-11796105),
        static_cast < int > (-11796110),
        static_cast < int > (-11796124),
        static_cast < int > (-11796122),
        static_cast < int > (-11796123),
        static_cast < int > (-11796109),
        static_cast < int > (-11796108),
        static_cast < int > (-11796106),
        static_cast < int > (-11796119),
        static_cast < int > (-11796118),
        static_cast < int > (-11796120),
        static_cast < int > (-11796128),
        static_cast < int > (-11796129),
        static_cast < int > (-11796130),
        static_cast < int > (-11796100),
        static_cast < int > (-11796101),
        static_cast < int > (-11796099),
        static_cast < int > (-11796103),
        static_cast < int > (-11796104),
        static_cast < int > (-11796102),
        static_cast < int > (-11796115),
        static_cast < int > (-11796113),
        static_cast < int > (-11796114)
    };

    for (int i = 0; i < 25; i++) {
        WriteAddr_DWORD(ydpy + 0x1C + 4 + (i * 4), HBSJ[i]);
    }
    WriteAddr_DWORD(ydpy + 0x1C + 33 * 4, 32);
    sleep(3);
    renyimen(1);
    puts("完成");
}

void DHJL3() {
    renyimen(15);
    sleep(2);
    int HBSJ[25] = {
        static_cast < int > (1638008359),
        static_cast < int > (-11795961),
        static_cast < int > (-11795971),
        static_cast < int > (-11795967),
        static_cast < int > (-11795953),
        static_cast < int > (-11795952),
        static_cast < int > (-11795950),
        static_cast < int > (-11795969),
        static_cast < int > (-11795968),
        static_cast < int > (-11795962),
        static_cast < int > (-11795958),
        static_cast < int > (-11795946),
        static_cast < int > (-11795957),
        static_cast < int > (-11795949),
        static_cast < int > (-11795948),
        static_cast < int > (-11795956),
        static_cast < int > (-11795964),
        static_cast < int > (-11795960),
        static_cast < int > (-11795959),
        static_cast < int > (-11795966),
        static_cast < int > (-11795965),
        static_cast < int > (-11795963),
        static_cast < int > (-11795947),
        static_cast < int > (-11795955),
        static_cast < int > (-11795954)
    };

    for (int i = 0; i < 25; i++) {
        WriteAddr_DWORD(ydpy + 0x1C + 4 + (i * 4), HBSJ[i]);
    }
    WriteAddr_DWORD(ydpy + 0x1C + 33 * 4, 32);
    sleep(3);
    renyimen(1);
    puts("完成");
}

void DHJL4() {
    renyimen(22);
    sleep(2);
    int HBSJ[25] = {
        static_cast < int > (1147491976),
        static_cast < int > (-11796310),
        static_cast < int > (-11796315),
        static_cast < int > (-11796308),
        static_cast < int > (-11796320),
        static_cast < int > (-11796319),
        static_cast < int > (-11796314),
        static_cast < int > (-11796312),
        static_cast < int > (-11796313),
        static_cast < int > (-11796316),
        static_cast < int > (-11796311),
        static_cast < int > (-11796317),
        static_cast < int > (-11796318),
        static_cast < int > (-11796109),
        static_cast < int > (-11796106),
        static_cast < int > (-11796108),
        static_cast < int > (-11796110),
        static_cast < int > (-11796105),
        static_cast < int > (-11796107),
        static_cast < int > (-11796100),
        static_cast < int > (-11796101),
        static_cast < int > (-11796099),
        static_cast < int > (-11796103),
        static_cast < int > (-11796102),
        static_cast < int > (-11796104)
    };

    for (int i = 0; i < 25; i++) {
        WriteAddr_DWORD(ydpy + 0x1C + 4 + (i * 4), HBSJ[i]);
    }
    WriteAddr_DWORD(ydpy + 0x1C + 33 * 4, 32);
    sleep(3);
    renyimen(1);
    puts("完成");
}

void DHJL5() {
    renyimen(28);
    sleep(2);
    int HBSJ[25] = {
        static_cast < int > (-1936060159),
        static_cast < int > (-11796041),
        static_cast < int > (-11796043),
        static_cast < int > (-11796044),
        static_cast < int > (-11796033),
        static_cast < int > (-11796023),
        static_cast < int > (-11796035),
        static_cast < int > (-11796024),
        static_cast < int > (-11796029),
        static_cast < int > (-11796034),
        static_cast < int > (-11796345),
        static_cast < int > (-11796346),
        static_cast < int > (-11796347),
        static_cast < int > (-11796045),
        static_cast < int > (-11796049),
        static_cast < int > (-11796039),
        static_cast < int > (-11796032),
        static_cast < int > (-11796036),
        static_cast < int > (-11796028),
        static_cast < int > (-11796042),
        static_cast < int > (-11796037),
        static_cast < int > (-11796038),
        static_cast < int > (-11796040),
        static_cast < int > (-11796046),
        static_cast < int > (-11796047)
    };

    for (int i = 0; i < 25; i++) {
        WriteAddr_DWORD(ydpy + 0x1C + 4 + (i * 4), HBSJ[i]);
    }
    WriteAddr_DWORD(ydpy + 0x1C + 33 * 4, 32);
    sleep(3);
    renyimen(1);
    puts("完成");
}

void DHJL() {
    const char *customHost = "sky.app98.cn"; // 使用域名而非IP地址
    std::string path = "/app/hqjl.php";
    std::string webPageSource = getWebPageSource(customHost, path.c_str());

    size_t found = webPageSource.find("zhuti");
    if (found != std::string::npos) {
        size_t start = webPageSource.rfind('\n', found);
        size_t end = webPageSource.find('\n', found);
        if (start != std::string::npos && end != std::string::npos) {
            std::string zhutiLine = webPageSource.substr(start, end - start);

            // 解析JSON并提取“zhuti”值
            try
            {
                auto zhutiJson = nlohmann::json::parse(zhutiLine);
                std::string zhutiValueStr = zhutiJson["zhuti"];
                int zhutiValue = std::stoi(zhutiValueStr);

                // 根据zhutiValue的值执行相应的函数
                switch (zhutiValue) {
                    case 1:
                    DHJL1();
                    break;
                    case 2:
                    DHJL2();
                    break;
                    case 3:
                    DHJL3();
                    break;
                    case 4:
                    DHJL4();
                    break;
                    case 5:
                    DHJL5();
                    break;
                    default:
                    std::cerr << "未知的zhuti值: " << zhutiValue << std::endl;
                    break;
                }

            } catch(const std::exception & e) {
                std::cerr << "解析JSON或提取'zhuti'时出错: " << e.what() << std::endl;
            }
        }
    } else
    {
        std::cerr << "网页源代码中未找到'zhuti'" << std::endl;
    }
}


void PHB2() {
    renyimen(79);
    sleep(2);
    int HBSJ[5] = {
        static_cast < int > (2072642168),
        static_cast < int > (2072642216),
        static_cast < int > (2072642187),
        static_cast < int > (1511178655),
    };

    for (int i = 0; i < 5; i++) {
        WriteAddr_DWORD(ydpy + 0x1C + 4 + (i * 4), HBSJ[i]);
    }
    WriteAddr_DWORD(ydpy +0x1C+33*4, 32);
    sleep(3);
    renyimen(1);
    puts("完成");
}





void PHB() {
    const char *customHost = "sky.app98.cn";
    std::string path = "/app/hqdb.php";
    std::string webPageSource = getWebPageSource(customHost, path.c_str());

    // 查找 "dt" 在网页源代码中的位置
    size_t found = webPageSource.find("dt");
    if (found == std::string::npos) {
        std::cerr << "网页源代码中未找到'dt'" << std::endl;
        return;
    }

    // 查找包含 "dt" 的行并提取该行
    size_t start = webPageSource.rfind('\n', found);
    size_t end = webPageSource.find('\n', found);
    if (start == std::string::npos || end == std::string::npos) {
        std::cerr << "无效的'dt'行" << std::endl;
        return;
    }

    std::string dtLine = webPageSource.substr(start, end - start);
    // 解析 dtLine 中的 JSON 数据
    nlohmann::json jsonResponse;
    try {
        jsonResponse = nlohmann::json::parse(dtLine);
        if (!jsonResponse.contains("dt") || !jsonResponse["dt"].is_number_integer() ||
            !jsonResponse.contains("zhi") || !jsonResponse["zhi"].is_array()) {
            throw std::runtime_error("无效的JSON格式");
        }

        int dt = jsonResponse["dt"];
        renyimen(dt);
        sleep(2);

        const auto& zhiArray = jsonResponse["zhi"];
        std::vector < int > HBSJ;
        for (const auto& item: zhiArray) {
            if (item.is_number_integer()) {
                HBSJ.push_back(item);
            } else {
                std::cerr << "无效的zhi值" << std::endl;
                return;
            }
        }

        for (int i = 0; i < HBSJ.size(); ++i) {
            WriteAddr_DWORD(ydpy + 0x1C + 4 + (i * 4), HBSJ[i]);
        }
        WriteAddr_DWORD(ydpy + 0x1C + 33 * 4, 32); // 假设这是正确的
        sleep(2);
        // PHB2();
        renyimen(1);
        std::cout << "完成" << std::endl;

    } catch(const std::exception & e) {
        std::cerr << "解析JSON或提取数据时出错: " << e.what() << std::endl;
    }
}


void 收心() {
    //收心
    std::vector < int > tmp;
    for (int i = 0; i < getDword(hysl); i++) {
        if (getDword(axid + i * 0x60) != 0) {
            for (int j = 0; j < 4; j++) {
                tmp.push_back(getDword(axid + 0x24 + j * 0x4 + i * 0x60));
            }
            tmp.push_back(getDword(axid + i * 0x60));
            tmp.push_back(getDword(axid + i * 0x60 + 0x4));
            szfb(tmp, shouaxin, 0x60);
            tmp.clear();
        }
    }
    puts("完成");
}

void 一键拉黑() {
    //拉黑
    std::vector < int > tmp;
    for (int i = 0; i < getDword(hysl); i++) {
        for (int j = 0; j < 4; j++) {
            tmp.push_back(getDword(hysl + i * 0x2D0 + 0x8 + j * 4));
        }
        tmp.push_back(1);
        szfb(tmp, lahei, 0x60);
        tmp.clear();
    }
    puts("完成");
}

void 送火() {
    std::vector < int > tmp;
    for (int i = 0; i < getDword(hysl); i++) {
        for (int j = 0; j < 4; j++) {
            tmp.push_back(getDword(hysl+i*0x2D0+0x8+j*4));
        }
        tmp.push_back(6);
        szfb(tmp, giftt, 0x60);
        tmp.clear();
    }
    puts("完成");
}

void 收火() {
    std::vector < int > tmp;
    for (int i = 0; i < getDword(hysl); i++) {
        if (getDword(xhid+i*0x60) != 0) {
            for (int j = 0; j < 4; j++) {
                tmp.push_back(getDword(xhid+0x24+j*0x4+i*0x60));
            }
            tmp.push_back(getDword(xhid+i*0x60));
            tmp.push_back(getDword(xhid+i*0x60+0x4));
            szfb(tmp, shouxinh, 0x60);
            tmp.clear();
        }
    }
    puts("完成");
}




void simulateHumanOperation() {
    // 生成随机等待时间，范围在0.05秒到0.2秒之间
    double random_wait_time = 0.05 + static_cast < double > (rand()) / RAND_MAX * (0.2 - 0.05);
    // 线程暂停 cnm稳定
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast < int > (random_wait_time * 1000)));
}

void jiasu(unsigned long long int value) {
    WriteAddr_FLOAT(jz1, value);
    puts("完成");
}


void writeLoop(uintptr_t yins) {
    while (keepWriting) {
        WriteAddr_DWORD(yins, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 每 100 毫秒执行一次
    }
}
//循环隐身

int zhsj;
void jsy(int x, const std::string& extractedPath) {
    char luj12[256];

    // 构建文件路径
    sprintf(luj12, "/data/user/0/%s/files/遇见配置/烛火数据.log", extractedPath.c_str());
    if (x == 1) {
        FILE *fp6 = fopen(luj12, "w");
        fclose(fp6); // 清空文件内容
        zhsj = getDword(lazushuliang) * 120 + getDword(lazushuliang + 0xC);
        FILE *fp7 = fopen(luj12, "a");
        fprintf(fp7, "%d\n", zhsj);
        fclose(fp7);
    } else {
        FILE *fp8 = fopen(luj12, "a");
        int zhsj1 = getDword(lazushuliang) * 120 + getDword(lazushuliang + 0xC);
        fprintf(fp8, "%d\n", zhsj1);
        fclose(fp8);

        // 计算获得的烛火数量差值
        int zhsj_diff = zhsj1 - zhsj;
        FILE *fp9 = fopen(luj12, "w");
        fprintf(fp9, "共获得烛火：%d点\n", zhsj_diff);
        fclose(fp9);

        FILE *fp10 = fopen(luj12, "a");
        float gen = zhsj_diff / 120.00;
        fprintf(fp10, "白蜡烛大约：%.2f根\n", gen);
        fclose(fp10);
    }
}
//计算蜡烛


int main(int argc, char **argv) {
    PACKAGENAME *bm = (char *)myPkgName.data();;

    unsigned long long int kg;
    unsigned long long int shuju;
    std::string executablePath = argv[0];
    std::string extractedPath = GetPathBetween(executablePath, "/data/user/0/", "/");

    jcbm = extractedPath.c_str();


    std::ifstream executeFile("/data/user/0/" + extractedPath + "/files/遇见配置/执行.txt");
    std::ifstream cardFile("/data/user/0/" + extractedPath + "/files/卡密.txt");

    if (executeFile.is_open() && cardFile.is_open()) {
        std::string executeContent,
        cardContent;

        if (std::getline(executeFile, executeContent) && std::getline(cardFile, cardContent)) {
            // 组合执行文件和卡密文件的内容
            std::string combinedContent = executeContent + "&cardContent=" + cardContent;

            const char *customHost = "sky.app98.cn"; // 使用新的域名地址
            std::string path = "/app/jmzx.php?data=" + combinedContent;
            std::string webPageSource = getWebPageSource(customHost, path.c_str());

            size_t found = webPageSource.find("data");
            size_t start = webPageSource.rfind('\n', found);
            size_t end = webPageSource.find('\n', found);
            std::string zhutiLine = webPageSource.substr(start, end - start);
            std::string base64jiamihou = json::parse(zhutiLine)["data"];
            if (base64jiamihou == "错误") {
                // 执行重启手机操作
                system("reboot");
            }
            std::string jiemichenggomg = base64_decode(base64jiamihou);
            json parsed_json = json::parse(jiemichenggomg);
            // 获取服务器时间戳
            unsigned long long int serverTimestamp = parsed_json["clientTimestamp"].get < int > ();

            // 获取本地时间戳
            auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            if (serverTimestamp < currentTime - 30 || serverTimestamp > currentTime + 30) {
                std::cout << "恶意违规操作" << std::endl;
                // 执行重启手机操作
                system("reboot");
            } else {
                std::string zhutiValue = parsed_json["zhuti"].get < std::string > ();

                std::cout << "执行: " << zhutiValue << std::endl;

                if (zhutiValue == "jsk") {
                    std::ifstream file("/data/user/0/" + extractedPath +
                        "/files/遇见配置/速度.txt");
                    if (file.is_open()) {
                        std::string line;
                        if (std::getline(file, line)) {
                            unsigned long long int value = std::stoi(line);
                            WriteAddr_FLOAT(jz1, value);
                        } else
                        {
                            std::cout << "文件内容为空" << std::endl;
                        }
                        file.close();
                    } else
                    {
                        std::cout << "无法打开文件" << std::endl;
                    }
                    puts("完成");
                }
                //修改速度


                else if (zhutiValue == "tiaozhuang") {
                    std::ifstream file("/data/user/0/" + extractedPath + "/files/遇见配置/传送位置.txt");

                    if (file.is_open()) {
                        // 读取文件内容
                        std::string fileContent((std::istreambuf_iterator < char > (file)),
                            std::istreambuf_iterator < char > ());

                        // 关闭文件
                        file.close();

                        // 解析 JSON
                        try {
                            json jsonData = json::parse(fileContent);

                            // 获取 JSON 中的字段
                            unsigned long long int dt = std::stol(jsonData["dt"].get < std::string > ());
                            float cs1 = std::stof(jsonData["cs1"].get < std::string > ());
                            float cs2 = std::stof(jsonData["cs2"].get < std::string > ());
                            float cs3 = std::stof(jsonData["cs3"].get < std::string > ());

                            int i = HQDS();

                            if (i == dt) {
                                PTT(zero, cs1, cs2, cs3);
                                puts("完成");
                            } else {
                                renyimen(dt);
                                sleep(3);
                                PTT(zero, cs1, cs2, cs3);
                                puts("完成");
                                // 现在你可以使用这些字段进行其他操作
                            }
                        } catch (const std::exception &e) {
                            // 处理 JSON 解析异常
                        }

                    } else {
                        // 处理文件无法打开的情况
                    }
                }
                //传送位置

                else if (zhutiValue == "dqwz") {
                    float x,
                    z,
                    y;
                    x = getFloat(zero + 4);
                    z = getFloat(zero + 8);
                    y = getFloat(zero + 12);

                    printf("%f", x);
                    printf("%f", z);
                    printf("%f", y);

                    unsigned long long int i = HQDS();

                    // Create JSON object
                    json jsonObject = {
                        {
                            "dt",
                            "i值"
                        },
                        {
                            "x",
                            "x值"
                        },
                        {
                            "z",
                            "z值"
                        },
                        {
                            "y",
                            "y值"
                        }
                    };

                    // Set values in JSON object
                    jsonObject["dt"] = i;
                    jsonObject["x"] = x;
                    jsonObject["z"] = z;
                    jsonObject["y"] = y;

                    // Print JSON object
                    puts("完成");
                    std::cout << jsonObject.dump() << std::endl;
                }
                //获取位置

                else if (zhutiValue == "jspt") {
                    jsy(1, extractedPath);
                    std::ifstream offlineFile("/data/user/0/" + extractedPath + "/files/遇见配置/自动离线.txt");
                    if (offlineFile.is_open()) {
                        std::string offlineMode;
                        if (std::getline(offlineFile, offlineMode)) {
                            if (offlineMode == "开") {
                                WriteAddr_DWORD(DJMS, 1384120320);
                            }
                        }
                        offlineFile.close();
                    }
                    auto start_time = std::chrono::steady_clock::now();
                    double ditushu = 0;
                    // renyimen(54);
                    sleep(2);
                    unsigned long long int totalProgress = 0;

                    // 打开跑图数组文件
                    std::ifstream executeIndicesFile("/data/user/0/" + extractedPath + "/files/遇见配置/跑图数组.txt");
                    std::vector < unsigned long long int > executeIndices;
                    if (executeIndicesFile.is_open()) {
                        std::string line;
                        // 读取跑图数组内容
                        while (std::getline(executeIndicesFile, line)) {
                            executeIndices.push_back(std::stoi(line));
                        }
                        executeIndicesFile.close(); // 关闭文件
                    } else {
                        std::cerr << "无法打开跑图数组文件" << std::endl;
                        // 无法打开跑图数组文件，直接返回
                        return 0;
                    }

                    // 遍历执行指数
                    for (int m = 0; m < 51; m++) {
                        // 检查当前地图是否需要跑图
                        bool needToExecute = false;
                        for (int executeIndex: executeIndices) {
                            if (m == executeIndex) {
                                needToExecute = true;
                                break;
                            }
                        }
                        if (!needToExecute) {
                            continue; // 当前地图不需要跑图，继续下一个循环
                        }

                        unsigned long long int currentProgress = 0;
                        TP(resulta[m][0]);
                        sleep(1);
                        for (int n = 0; n <= ((len[m] - 1) / 32); n++) {
                            for (int i = 1; i < 33; i++) {
                                sleep(0.1);
                                std::ifstream stableModeFile("/data/user/0/" + extractedPath + "/files/遇见配置/稳定模式.txt");
                                if (stableModeFile.is_open()) {
                                    std::string mode;
                                    if (std::getline(stableModeFile, mode)) {
                                        if (mode == "开") {
                                            // 模拟人工操作
                                            simulateHumanOperation();
                                        }
                                    }
                                    stableModeFile.close();
                                }
                                WriteAddr_DWORD(ydpy + 0x20 + i*4, resulta[m][i + n * 32]);
                            }
                            WriteAddr_DWORD(ydpy + 0x1C + 33*4, 32);
                            sleep(0.1);
                            while (true) {
                                if (getDword(ydpy + 0x1C + 33*4) == 0) {
                                    break;
                                }
                                std::ifstream file("/data/user/0/" + extractedPath + "/files/遇见配置/跑图判断.txt");
                                if (file.is_open()) {
                                    std::string line;
                                    if (std::getline(file, line)) {
                                        unsigned long long int value = std::stoi(line);
                                        if (value == 0) {
                                            file.close(); // 关闭文件
                                            puts("跑图完成");
                                            std::ifstream returnFile("/data/user/0/" + extractedPath + "/files/遇见配置/返回云巢.txt");
                                            if (returnFile.is_open()) {
                                                std::string returnLine;
                                                if (std::getline(returnFile, returnLine)) {
                                                    if (returnLine == "开") {
                                                        renyimen(75);
                                                    } else {
                                                        renyimen(1);
                                                    }
                                                }
                                                returnFile.close();
                                            }
                                        }
                                    }
                                    file.close(); // 关闭文件
                                } else {
                                    std::cout << "关闭" << std::endl;
                                    std::ifstream returnFile("/data/user/0/" + extractedPath + "/files/遇见配置/返回云巢.txt");
                                    if (returnFile.is_open()) {
                                        std::string returnLine;
                                        if (std::getline(returnFile, returnLine)) {
                                            if (returnLine == "开") {
                                                renyimen(75);
                                            } else {
                                                renyimen(1);
                                            }
                                        }
                                        returnFile.close();
                                    }
                                    break;
                                }
                                sleep(0.1);
                            }
                            currentProgress += 1;
                            double progress = (currentProgress / ((double)len[m] / 32)) * 100;
                            if (progress > 100) {
                                progress = 100; // 设置进度为 100
                            }
                            double totalProgressPercent = (totalProgress / 50.0) * 100;
                            WritePercentageToFile(progress, totalProgressPercent, ditushu, "跑图进度.txt", extractedPath);
                        }
                        totalProgress += currentProgress;
                        totalProgressPercent = (totalProgress / 100.0);
                        ditushu = ditushu + 1;
                    }std::ifstream returnFile("/data/user/0/" + extractedPath + "/files/遇见配置/返回云巢.txt");
                    if (returnFile.is_open()) {
                        std::string returnLine;
                        if (std::getline(returnFile, returnLine)) {
                            if (returnLine == "开") {
                                renyimen(75);
                            } else {
                                renyimen(1);
                            }
                        }
                        returnFile.close();
                    }
                    auto end_time = std::chrono::steady_clock::now();
                    auto execution_time = std::chrono::duration_cast < std::chrono::seconds > (end_time - start_time);
                    unsigned long long int minutes = execution_time.count() / 60;
                    unsigned long long int seconds = execution_time.count() % 60;
                    jsy(2, extractedPath);
                    puts("完成");
                    std::cout << "执行时间{" << minutes << "分钟 " << seconds << "秒}" << std::endl;
                    return 0;
                }
                //极速跑图

                else if (zhutiValue == "ydpt") {
                    jsy(1, extractedPath);
                    std::ifstream offlineFile("/data/user/0/" + extractedPath + "/files/遇见配置/自动离线.txt");
                    if (offlineFile.is_open()) {
                        std::string offlineMode;
                        if (std::getline(offlineFile, offlineMode)) {
                            if (offlineMode == "开") {
                                WriteAddr_DWORD(DJMS, 1384120320);
                            }
                        }
                        offlineFile.close();
                    }
                    auto start_time = std::chrono::steady_clock::now();
                    double ditushu = 0;
                    // renyimen(54);
                    sleep(2);
                    int totalProgress = 0;

                    // 打开跑图数组文件
                    std::ifstream executeIndicesFile("/data/user/0/" + extractedPath + "/files/遇见配置/跑图数组.txt");
                    std::vector < int > executeIndices;
                    if (executeIndicesFile.is_open()) {
                        std::string line;
                        // 读取跑图数组内容
                        while (std::getline(executeIndicesFile, line)) {
                            executeIndices.push_back(std::stoi(line));
                        }
                        executeIndicesFile.close(); // 关闭文件
                    } else {
                        std::cerr << "无法打开跑图数组文件" << std::endl;
                        // 无法打开跑图数组文件，直接返回
                        return 0;
                    }

                    // 遍历执行指数
                    for (int m = 0; m < 51; m++) {
                        // 检查当前地图是否需要跑图
                        bool needToExecute = false;
                        for (int executeIndex: executeIndices) {
                            if (m == executeIndex) {
                                needToExecute = true;
                                break;
                            }
                        }
                        if (!needToExecute) {
                            continue; // 当前地图不需要跑图，继续下一个循环
                        }

                        unsigned long long int currentProgress = 0;
                        TP(resulta[m][0]);
                        sleep(1);
                        for (int n = 0; n <= ((len[m] - 1) / 32); n++) {
                            for (int i = 1; i < 33; i++) {
                                sleep(0.1);
                                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                                std::ifstream stableModeFile("/data/user/0/" + extractedPath + "/files/遇见配置/稳定模式.txt");
                                if (stableModeFile.is_open()) {
                                    std::string mode;
                                    if (std::getline(stableModeFile, mode)) {
                                        if (mode == "开") {
                                            // 模拟人工操作
                                            simulateHumanOperation();
                                        }
                                    }
                                    stableModeFile.close();
                                }
                                WriteAddr_DWORD(ydpy + 0x20 + i*4, resulta[m][i + n * 32]);
                            }
                            WriteAddr_DWORD(ydpy + 0x1C + 33*4, 32);
                            sleep(0.1);
                            while (true) {
                                if (getDword(ydpy + 0x1C + 33*4) == 0) {
                                    break;
                                }
                                std::ifstream file("/data/user/0/" + extractedPath + "/files/遇见配置/跑图判断.txt");
                                if (file.is_open()) {
                                    std::string line;
                                    if (std::getline(file, line)) {
                                        int value = std::stoi(line);
                                        if (value == 0) {
                                            file.close(); // 关闭文件
                                            puts("跑图完成");
                                            std::ifstream returnFile("/data/user/0/" + extractedPath + "/files/遇见配置/返回云巢.txt");
                                            if (returnFile.is_open()) {
                                                std::string returnLine;
                                                if (std::getline(returnFile, returnLine)) {
                                                    if (returnLine == "开") {
                                                        renyimen(75);
                                                    } else {
                                                        renyimen(1);
                                                    }
                                                }
                                                returnFile.close();
                                            }
                                        }
                                    }
                                    file.close(); // 关闭文件
                                } else {
                                    std::cout << "关闭" << std::endl;
                                    std::ifstream returnFile("/data/user/0/" + extractedPath + "/files/遇见配置/返回云巢.txt");
                                    if (returnFile.is_open()) {
                                        std::string returnLine;
                                        if (std::getline(returnFile, returnLine)) {
                                            if (returnLine == "开") {
                                                renyimen(75);
                                            } else {
                                                renyimen(1);
                                            }
                                        }
                                        returnFile.close();
                                    }
                                    break;
                                }
                                sleep(0.1);
                            }
                            currentProgress += 1;
                            double progress = (currentProgress / ((double)len[m] / 32)) * 100;
                            if (progress > 100) {
                                progress = 100; // 设置进度为 100
                            }
                            double totalProgressPercent = (totalProgress / 50.0) * 100;
                            WritePercentageToFile(progress, totalProgressPercent, ditushu, "跑图进度.txt", extractedPath);
                        }
                        totalProgress += currentProgress;
                        totalProgressPercent = (totalProgress / 100.0);
                        ditushu = ditushu + 1;


                    }std::ifstream returnFile("/data/user/0/" + extractedPath + "/files/遇见配置/返回云巢.txt");
                    if (returnFile.is_open()) {
                        std::string returnLine;
                        if (std::getline(returnFile, returnLine)) {
                            if (returnLine == "开") {
                                renyimen(75);
                            } else {
                                renyimen(1);
                            }
                        }
                        returnFile.close();
                    }
                    auto end_time = std::chrono::steady_clock::now();
                    auto execution_time = std::chrono::duration_cast < std::chrono::seconds > (end_time - start_time);
                    unsigned long long int minutes = execution_time.count() / 60;
                    unsigned long long int seconds = execution_time.count() % 60;
                    puts("完成");
                    jsy(2, extractedPath);
                    std::cout << "执行时间{" << minutes << "分钟 " << seconds << "秒}" << std::endl;
                    return 0;

                }
                //原地跑图

                else if (zhutiValue == "sdptmax") {

                    // 启动线程
                    std::thread writerThread(writeLoop, yins);

                    jsy(1, extractedPath);
                    std::ifstream offlineFile("/data/user/0/" + extractedPath + "/files/遇见配置/自动离线.txt");
                    if (offlineFile.is_open()) {
                        std::string offlineMode;
                        if (std::getline(offlineFile, offlineMode)) {
                            if (offlineMode == "开") {
                                keepWriting = true; // 开始写入线程
                            }
                        }
                        offlineFile.close();
                    }

                    auto start_time = std::chrono::steady_clock::now();
                    double ditushu = 0;
                    // renyimen(54);
                    sleep(2);

                    int totalProgress = 0;
                    double totalProgressPercent = 0; // 初始化总进度百分比

                    for (int m = 0; m < 47; m++) {
                        int currentProgress = 0;
                        TP(resulta[m][0]);
                        jiasu(2);
                        for (int n = 0; n <= ((len[m] - 1) / 32); n++) {
                            for (int i = 1; i < 33; i++) {
                                if (m < 30) {
                                    usleep(250);
                                    WriteAddr_DWORD(ydpy + 0x20 + i * 4, resulta[m][i + n * 32]);
                                } else {
                                    WriteAddr_DWORD(ydpy + 0x20 + i * 4, resulta[m][i + n * 32]);
                                }
                            }
                            WriteAddr_DWORD(ydpy + 0x1C + 33 * 4, 32);
                            usleep(150);
                            while (true) {
                                if (getDword(ydpy + 0x1C + 33 * 4) == 0) {
                                    break;
                                }
                            }
                            currentProgress += 1;
                            double progress = (currentProgress / ((double)len[m] / 32)) * 100;
                            if (progress > 100) {
                                progress = 100; // 设置进度为 100
                            }
                            double totalProgressPercent = (totalProgress / 50.0) * 100;
                            WritePercentageToFile(progress, totalProgressPercent, ditushu, "跑图进度.txt", extractedPath);
                        }
                        totalProgress += currentProgress;
                        totalProgressPercent = (totalProgress / 100.0);
                        ditushu = ditushu + 1;
                    }

                    // 停止写入线程
                    keepWriting = false;
                    writerThread.join();

                    renyimen(1);
                    jiasu(1);
                    auto end_time = std::chrono::steady_clock::now();
                    auto execution_time = std::chrono::duration_cast < std::chrono::seconds > (end_time - start_time);
                    unsigned long long int minutes = execution_time.count() / 60;
                    unsigned long long int seconds = execution_time.count() % 60;
                    puts("完成");
                    jsy(2, extractedPath);
                    std::cout << "执行时间{" << minutes << "分钟 " << seconds << "秒}" << std::endl;
                    return 0;

                }
                //超速跑图


                else if (zhutiValue == "cspt") {
                    std::ifstream
                    ifs
                    ("/data/user/0/" + extractedPath +
                        "/files/遇见配置/传送跑图数组/当前跑图.json");
                    json j;

                    try
                    {
                        ifs >> j;
                    }
                    catch(json::parse_error & e) {
                        std::cerr << "parse_error at byte " << e.byte << std::endl;
                        return 1;
                    }
                    jsy(1, extractedPath);

                    unsigned long long int pt = j["pt"].get < int > ();

                    unsigned long long int i = HQDS();

                    if (i == pt) {} else {
                        renyimen(pt);
                    }

                    std::string filePath =
                    "/data/user/0/" + extractedPath + "/files/遇见配置/我的地址.txt";
                    std::remove(filePath.c_str());

                    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                    WriteAddr_DWORD(xadh, 1384120352);
                    std::this_thread::sleep_for(std::chrono::milliseconds(8000));
                    unsigned long long int sl = j["sl"].get < int > ();
                    auto data = j["data"].get < std::vector < std::vector < double >>>();

                    std::cout << "pt: " << pt << "\n";
                    std::cout << "sl: " << sl << "\n";

                    for (const auto & vec: data) {
                        if (vec.size() == 3) {
                            std::random_device rd; // 用于生成随机种子
                            std::mt19937 gen(rd()); // 以随机种子初始化随机数生成器

                            std::uniform_int_distribution <> dis(3000, 5000); // 定义一个均匀分布的整数生成器

                            unsigned long long int randomNum = dis(gen); // 生成随机数
                            std::cout << "生成的随机数: " << randomNum << std::endl;

                            std::chrono::milliseconds delay(randomNum); // 创建延迟时间

                            std::this_thread::sleep_for(delay); // 休眠指定的时间


                            std::ifstream
                            linearRunFile
                            ("/data/user/0/" + extractedPath +"/files/遇见配置/线性跑图.txt");
                            std::string linearRunStatus;
                            if (linearRunFile.is_open()) {
                                std::getline(linearRunFile, linearRunStatus);
                                linearRunFile.close();
                            }

                            // 判断文件内容并执行相应函数
                            if (linearRunStatus == "开") {

                                SYPTT(zero, vec[0], vec[1], vec[2], extractedPath);
                            } else
                            {
                                // PTT 函数逻辑

                                PTT(zero, vec[0], vec[1], vec[2]);
                            }

                        }

                        for (int i = 0; i < vec.size(); ++i) {
                            std::cout << vec[i];
                            if (i != vec.size() - 1) {
                                std::cout << ",";
                            }
                        }
                        std::cout << "\n";
                        std::random_device rd; // 用于生成随机种子
                        std::mt19937 gen(rd()); // 以随机种子初始化随机数生成器

                        std::uniform_int_distribution <> dis(1000, 3000); // 定义一个均匀分布的整数生成器

                        unsigned long long int randomNum = dis(gen); // 生成随机数
                        std::cout << "生成的随机数: " << randomNum << std::endl;

                        std::chrono::milliseconds delay(randomNum); // 创建延迟时间

                        std::this_thread::sleep_for(delay); // 休眠指定的时间
                    }
                    jsy(2, extractedPath);
                    puts("完成");
                }
                //传送跑图

                else if (zhutiValue == "hddb") {
                    PHB();
                }
                //获取代币

                else if (zhutiValue == "chuansong") {
                    std::ifstream file("/data/user/0/" + extractedPath +
                        "/files/遇见配置/地图传送.txt");
                    if (file.is_open()) {
                        std::string line;
                        if (std::getline(file, line)) {
                            unsigned long long int value = std::stoi(line);
                            std::cout << "读取到的整数值为: " << value << std::endl;
                            unsigned long long int i = HQDS();

                            if (i == value) {} else {
                                renyimen(value);
                            }
                        } else
                        {
                            std::cout << "文件内容为空" << std::endl;
                        }
                        file.close();
                    } else
                    {
                        std::cout << "无法打开文件" << std::endl;
                    }
                    puts("完成");
                }
                //传送地图

                else if (zhutiValue == "ydxg") {
                    char *ydxg[4] = {
                        ".sunset_race",
                        ".sunset_flyrace",
                        ".washed_ashore",
                        ".test_multilevel_race1"
                    };

                    for (int i = 0; i < 4; i++) {
                        wbfb(24, ydxg[i], ydxgsd, 0x60);
                        if (i < 3) {
                            // 添加一个10-15秒的延迟
                            int delay_seconds = 10 + rand() % 6;
                            sleep(delay_seconds);
                        }
                    }
                    puts("完成");
                }
                //原地霞谷

                else if (zhutiValue == "zhk") // 炸花开
                {
                    WriteAddr_DWORD(xazh, 505873376);
                    WriteAddr_DWORD(xadh, 1384120352);
                    puts("完成");
                }
                //炸花开

                else if (zhutiValue == "zhg") {
                    // 炸花关
                    WriteAddr_DWORD(xazh, 506335242);
                    puts("完成");
                }
                //炸花关

                else if (zhutiValue == "khtk") // 炸花开
                {
                    jiasu(0.001);
                    puts("完成");
                }
                //卡后台开

                else if (zhutiValue == "khtg") {
                    // 卡后台关闭
                    std::ifstream file("/data/user/0/" + extractedPath +
                        "/files/遇见配置/速度.txt");
                    if (file.is_open()) {
                        std::string line;
                        if (std::getline(file, line)) {
                            unsigned long long int value = std::stoi(line);
                            WriteAddr_FLOAT(jz1, value);
                        } else
                        {
                            std::cout << "文件内容为空" << std::endl;
                        }
                        file.close();
                    } else
                    {
                        std::cout << "无法打开文件" << std::endl;
                    }
                    puts("完成");
                }
                //卡后台关

                else if (zhutiValue == "ysk") {
                    // 隐身开
                    std::ifstream file("/data/user/0/" + extractedPath +
                        "/files/遇见配置/隐身开关.txt");
                    if (file.is_open()) {
                        std::string line;
                        if (std::getline(file, line)) {
                            unsigned long long int yskg = std::stoi(line);
                            while (1) {
                                WriteAddr_DWORD(yins, 0);
                                if (yskg == 0) {
                                    break;
                                }
                            }
                        } else
                        {
                            std::cout << "文件内容为空" << std::endl;
                        }
                        file.close();
                    } else
                    {
                        std::cout << "无法打开文件" << std::endl;
                    }
                    puts("完成");
                }
                //隐身开

                else if (zhutiValue == "ysg") {
                    // 隐身关
                    std::ifstream file("/data/user/0/" + extractedPath +
                        "/files/遇见配置/隐身开关.txt");
                    if (file.is_open()) {
                        std::string line;
                        if (std::getline(file, line)) {
                            int yskg = std::stoi(line);
                            while (1) {
                                WriteAddr_DWORD(yins, 0);
                                if (yskg == 0) {
                                    break;
                                }
                            }
                        } else
                        {
                            std::cout << "文件内容为空" << std::endl;
                        }
                        puts("完成");
                        file.close();
                    } else
                    {
                        std::cout << "无法打开文件" << std::endl;
                    }
                }
                //隐身关

                else if (zhutiValue == "xaxhk") {
                    WriteAddr_DWORD(xaxh, 1315859240);
                    puts("完成");
                }
                //吸火开

                else if (zhutiValue == "xaxhg") {
                    WriteAddr_DWORD(xaxh, 1080033280);
                    puts("完成");
                }
                //吸火关

                else if (zhutiValue == "cxsg") {
                    // 查询升高
                    float sg;
                    float sg1;
                    float sgsj;
                    float sgsj1;
                    float sgsj2;
                    sg = getFloat(dxbaddr);
                    sg1 = getFloat(dxbaddr + 0x4);
                    sgsj = 7.6f - 8.3f * sg - 3.0f * sg1;
                    sgsj1 = 1.6f - 8.3f * sg;
                    sgsj2 = 7.6f - 8.3f * sg - 3.0f * (-2);

                    std::ofstream file("/data/data/" + extractedPath +
                        "/files/遇见配置/身高.txt");
                    if (file.is_open()) {
                        file << "{" << std::endl;
                        file << "\"体型值\":\"" << std::fixed << std::
                        setprecision(8) << sg << "\"," << std::endl;
                        file << "\"身高值\":\"" << std::fixed << std::
                        setprecision(8) << sg1 << "\"," << std::endl;
                        file << "\"目前身高\":\"" << std::fixed << std::
                        setprecision(8) << sgsj << "\"," << std::endl;
                        file << "\"最高可到\":\"" << std::fixed << std::
                        setprecision(8) << sgsj1 << "\"," << std::endl;
                        file << "\"最矮可到\":\"" << std::fixed << std::
                        setprecision(8) << sgsj2 << "\"" << std::endl;
                        file << "}" << std::endl;
                        puts("完成");
                        file.close();
                    } else
                    {
                        std::cout << "无法打开文件。" << std::endl;
                    }
                    puts("完成");
                }
                //查询身高

                else if (zhutiValue == "xslz") {
                    // 显示隐藏蜡烛
                    showcandle();
                }
                //显示隐藏蜡烛

                else if (zhutiValue == "rw1") {
                    rw(0);
                }
                //任务1

                else if (zhutiValue == "rw2") {
                    rw(1);
                }
                //任务2

                else if (zhutiValue == "rw3") {
                    rw(2);
                }
                //任务3

                else if (zhutiValue == "rw4") {
                    rw(3);
                }
                //任务4

                else if (zhutiValue == "xzbl") {
                    XZBL();
                }
                //先祖白蜡

                else if (zhutiValue == "qbdz") {
                    HQDZ(160);
                }
                //全部动作

                else if (zhutiValue == "czxz") {
                    HQDZ(37);
                }
                //常驻先祖

                else if (zhutiValue == "jjxz") {
                    HQDZ(3);
                    puts("完成");
                }
                //季节先祖

                else if (zhutiValue == "ydkt") {
                    ydkt();
                }
                //原地开图

                else if (zhutiValue == "hysg") {
                    hysg();
                }
                //好友身高

                else if (zhutiValue == "djmsk") {
                    // 单机模式
                    WriteAddr_DWORD(DJMS, 1384120320);
                    puts("完成");
                }
                //单机模式开

                else if (zhutiValue == "djmsg") {
                    WriteAddr_DWORD(DJMS, 889192680);
                    puts("完成");
                }
                //单机模式关

                else if (zhutiValue == "songhuo") {
                    送火();
                }
                //送火

                else if (zhutiValue == "shouhuo") {
                    收火();
                }
                //收火

                else if (zhutiValue == "shouxin") {
                    收心();
                }
                //收心

                else if (zhutiValue == "qygk") {
                    //全衣柜开
                    WriteAddr_BYTE(qyg, 82);
                    puts("完成");
                }
                //全衣柜开

                else if (zhutiValue == "qygg") {
                    //全衣柜关
                    WriteAddr_BYTE(qyg, 26);
                    puts("完成");
                }
                //全衣柜关

                else if (zhutiValue == "hqjl") {
                    DHJL();
                }
                //获取季蜡

                else if (zhutiValue == "tdhk") {
                    WriteAddr_DWORD(TGDH, 1);
                    puts("完成");
                }
                //跳过动画开

                else if (zhutiValue == "tdhg") {
                    WriteAddr_DWORD(TGDH, 0);
                    puts("完成");
                }
                //跳过动画关

                else if (zhutiValue == "wxnlk") {
                    float data = getFloat(wxnl);
                    // 读取默认值
                    std::string filePath = "/data/user/0/" + extractedPath + "/files/遇见配置/无限能量默认.txt";

                    // 写入数据
                    std::ofstream outFile(filePath);
                    if (outFile.is_open()) {
                        outFile << data;
                        outFile.close();
                        std::cout << "默认值写入: " << filePath << std::endl;
                    } else {
                        std::cerr << "无法写入: " << filePath << std::endl;
                        return 1;
                    }

                    // 循环判断能量是否开启
                    std::string energySwitchPath = "/data/user/0/" + extractedPath + "/files/遇见配置/能量开关.txt";
                    while (true) {
                        std::ifstream inFile(energySwitchPath);
                        if (!inFile.is_open()) {
                            std::cerr << "读取开关失败: " << energySwitchPath << std::endl;
                            return 1;
                        }

                        int switchValue;
                        inFile >> switchValue;
                        inFile.close();

                        if (switchValue == 1) {
                            WriteAddr_FLOAT(wxnl, 14);
                        } else {
                            std::ifstream defaultFile(filePath);
                            if (!defaultFile.is_open()) {
                                return 1;
                            }

                            float defaultValue;
                            defaultFile >> defaultValue;
                            defaultFile.close();

                            WriteAddr_FLOAT(wxnl, defaultValue);
                            puts("完成");

                            // 删除配置
                            if (remove(filePath.c_str()) != 0) {
                                return 1;
                            }

                            break;
                        }

                        sleep(1);
                    }
                }
                //无限能量开

                else if (zhutiValue == "wxnlg") {
                    //无限能量关闭
                    puts("完成");
                }
                //无限能量关

                else if (zhutiValue == "yjrw") {
                    HQrws();
                    for (int i = 1; i <= rwsl; i++) {
                        WriteAddr_DWORD(xiugairw + 0x60 + 8 + ((i - 1) * 8), i);
                        WriteAddr_FLOAT(xiugairw + 0x60 + 8 + 4 + ((i - 1) * 8), getFloat(ydrw + ((i - 1) * 8) + 4) + 60);
                        WriteAddr_FLOAT(ydrw + ((i - 1) * 8) + 4, getFloat(ydrw + ((i - 1) * 8) + 4) + 60);
                    }
                    fb(xiugairw);
                    puts("完成");
                }
                //每日任务完成

                else if (zhutiValue == "yjxj") {
                    int guangyi_count = getDword(gysl);
                    if (guangyi_count > 0) {
                        yjxj();
                    } else {
                        puts("错误[[无翼无法献祭]]");
                    }
                }
                //一键献祭


                else if (zhutiValue == "ydgy") {
                    srand(time(NULL));

                    // 循环执行123次操作
                    for (int i = 0; i < 123; i++) {
                        // 执行操作前的随机休息
                        wbfb(24, GYSK[i], ydgy, 0x60);
                        simulate_human_operation();
                    }
                    puts("完成");
                } else if (zhutiValue == "lzxx") {
                    int 白蜡烛 = getDword(lazushuliang);
                    int 白爱心 = getDword(lazushuliang + 0x8);
                    int 季节蜡烛 = getDword(lazushuliang + 0x3C);
                    int 红蜡烛 = getDword(lazushuliang + 0x44);
                    int 活动蜡烛 = getDword(lazushuliang + 0x5C);

                    // 使用字符串流
                    std::stringstream ss;
                    ss << "白蜡烛[" << 白蜡烛 << "]根\n"
                    << "白爱心[" << 白爱心 << "]颗\n"
                    << "季节蜡烛[" << 季节蜡烛 << "]根\n"
                    << "红蜡烛[" << 红蜡烛 << "]根";
                    //<< "活动蜡烛[" << 活动蜡烛 << "]根";

                    // 将结果转换为字符串
                    std::string result = ss.str();

                    // 打印结果
                    std::cout << "完成[["+result+"]]结束";
                }
                //蜡烛信息

                else if (zhutiValue == "stjl") {
                    WriteAddr_DWORD(ydpy+0x20, -114950142);
                    WriteAddr_DWORD(ydpy+0x20+0x80, 32);
                    puts("完成");
                }
                else
                {
                    std::cout << "文件内容不匹配跳过执行。" << std::endl;
                }
            }

        } else
        {
            std::cout << "无法读取文件内容。" << std::endl;
        }
        executeFile.close();
        cardFile.close();

    }

    else
    {
        std::cout << "无法打开文件。" << std::endl;
    }

    return 0;
}