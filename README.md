# Travel-Simulation-System (基于 ImGui 的旅行模拟系统)

这是一个使用 C++ 开发的交互式旅行管理系统。项目展示了如何将后端业务逻辑与 DirectX 11 渲染引擎结合，通过 Dear ImGui 构建现代化的图形用户界面 (GUI)，并通过加载微软雅黑字体完美解决了中文字符显示问题。

## 📸 运行演示

> *(注：请确保你的 docs 文件夹下存有 screenshot.png 图片)*

## ✨ 核心特性

- **实时 GUI 交互**：基于 Dear ImGui 1.90 实时渲染，摆脱控制台黑窗口。

- **动态模拟逻辑**：一键生成/重置旅客行程数据，支持自动匹配旅行团。

- **中文完美支持**：集成微软雅黑 TrueType 字体，彻底解决 C++ GUI 开发中的中文乱码。

- **高性能架构**：利用 DirectX 11 硬件加速渲染后端。

## 🛠️ 环境依赖

- **编译器**: MinGW-w64 (GCC 11.0+)

- **图形 API**: DirectX 11

- **操作系统**: Windows 10/11

## 🚀 快速开始

### 1. 编译项目

在项目根目录下打开 PowerShell，执行以下标准编译命令：

PowerShell

```
g++ main.cpp imgui/imgui*.cpp -o TravelSim.exe -lgdi32 -ld3d11 -ld3dcompiler -ldwmapi
```

### 2. 运行程序

编译完成后，执行以下命令启动程序：

PowerShell

```
./TravelSim.exe
```

## 📅 开发计划

- [ ] 增加旅客数据的持久化存储（JSON/CSV 格式）

- [ ] 实现旅行线路的地图可视化显示

- [ ] 优化高 DPI 屏幕下的 UI 缩放表现

## ⚖️ 许可证

本项目采用 [MIT License](https://www.google.com/search?q=LICENSE) 授权。

---

## 

<details> <summary>🛠️ <b>维护者指南 (点击展开上传步骤)</b></summary> > **提示**：以下是推送到远程仓库的专业命令，仅供维护参考。 ```powershell
# 1. 初始化本地 Git 仓库
git init
# 2. 将所有整理后的文件加入暂存区
git add .
# 3. 提交更改
git commit -m "feat: complete project reconstruction with professional README"
# 4. 关联你的远程仓库
git remote add origin [https://github.com/你的名字/Travel-Simulation-System.git](https://github.com/你的名字/Travel-Simulation-System.git)
# 5. 强制推送以更新仓库结构
git push -u origin main -f 
