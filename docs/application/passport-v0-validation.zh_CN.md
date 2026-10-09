<p align="right"><a href="passport-v0-validation.md">English</a> · <strong>简体中文</strong></p>

# Milestone A 视觉修订与状态页验证

2026-09-30 的 Cottage Milestone A 视觉修订历史验证记录。以下结果仅适用于此处记录的确切固件摘要，不自动适用于后续构建。上游基线：`0b9e4c81ee4421c0bac39ca3561d65a8285acd4a`。

| 检查 | 结果 | 证据 |
| --- | --- | --- |
| Build | PASS | 完整 `tools/validate.sh`，ESP-IDF v5.5.3，返回 0 |
| Host tests | PASS | 完整仓库检查及按键/世界测试，含状态页进入、返回、待机 |
| LVGL 渲染与字体 | PASS | LVGL 9.5.0、24 KB 图形池、九张画面；实际中文字形与缺字负例检查 |
| 镜像与归档 | PASS | 合并镜像及分区校验；按内容摘要归档并独立复核 |
| Simulator | PASS（已观察项） | 新固件启动、欢迎对白、焦点、状态页、双向转场、长按释放不附带短按、首次唤醒吞键 |
| 烧录与启动 | PASS | 烧录通过数据哈希回读；重启后观察 15 秒，ELF 相符且进入应用 |
| Device tests | NOT RUN | 完整视觉、按键、电量验收仍等待用户实际观察 |

Unverified：新版实体屏颜色、可读性、流畅度、按键时序、真实电量计、后台任务启动后的内存。模拟器时序和功耗不是硬件测量。闲置后第一次 OK 保留场景且未打开状态页，第二次 OK 才打开；模拟器没有明显显示背光变暗，实体背光关闭仍未验证。

## 确切固件身份

- 合并镜像：1,131,488 bytes，偏移 `0x0`；ESP32-C3、8 MB Flash。
- 应用：1,065,952 bytes；factory 分区 8,323,072 bytes。
- 固件 SHA-256：`831f7d1d22a0f48093ee56fe51ecea4356b5559a55790b0640f91d19697ab4ad`。
- ELF SHA-256：`131707f865bf41374e95d9f0e7c2d5656da45f29002ddf46fdbc680d2d720d0d`。
- 内嵌版本：`0b9e4c8-dirty`；SDK `v5.5.3`。
- 归档：`build/firmware/831f7d1d22a0f48093ee56fe51ecea4356b5559a55790b0640f91d19697ab4ad/`。

此确切版本于 2026-09-30 从 `0x0` 完成烧录，仅擦除覆盖扇区 `0x0` 至 `0x114fff`。重启后观察 15 秒，未发现崩溃或反复重启；确认检测到 CW2017 且原电池 profile 匹配。电量任务初始化前可用 heap 为 246,164 bytes，最大空闲块为 114,688 bytes。实际电量显示仍未验证。合并镜像会替换固件，并可能重置 NVS，见[烧录政策](../development/engineering/firmware-layout.zh_CN.md#烧录与已存数据)。

## 视觉修改与状态页

木地板延伸到角色脚下。角色基于原立绘，保留自然站姿，增大头身比、简化表情和柔化头发；橙色郁金香与珊瑚红水壶改为卡通轮廓。脚下小光斑替代大选择框，“状态”用文字下划线提示。

小屋第二个焦点选中“状态”后按 OK 打开，再按 OK 返回。显示电量、开机运行时长和沫纯当前状态；后台任务每 10 秒通过现有 BSP 读取电量，未知显示 --%。主机渲染使用测试电量和时长；模拟器因没有电量计模型而显示不可用。

## 模拟器流程

来源：[VOID001/FoloToy-Passport-Simulator](https://github.com/VOID001/FoloToy-Passport-Simulator)。下载的 main 归档 SHA-256：`60f507b6721381bb80abe02c82d8e412380b7fb5f3caa1c904b29786c2a894b1`。本地目录为已忽略的 `work/simulator-source/FoloToy-Passport-Simulator-main`，开发板运行时校验通过。运行 `node server.mjs --allow-local-firmware-upload`，打开 `http://127.0.0.1:4190`，选择上述确切合并镜像。刷新会恢复模拟器默认 Demo，需重新选择本地固件。

模拟器通过 WASM/QEMU 运行实际固件，仅作为开发工具，不扩展产品 Web 前端或后端。添加的两个本地测试按钮通过模拟器现有 runtime API 持续按住/释放 UP、DOWN 电平；没有为模拟器改动固件或 BSP。其余操作使用模拟器普通短按及 OK。启动 UART 的 ELF 前缀 `131707f86` 与归档相符；UART 捕获止于应用日志之前，因此不作为完整应用日志检查。

已观察到正确转场终点及经过门口的中间画面，不据此承诺真实帧率。电量计、BLE 和准确低功耗行为受模拟器限制，真实电量及实体屏仍留待真机确认。

## 上一版反馈与未完成范围

上一版 `e7e41ffa53a6965c82154b7dd162a4c64e74ee53d132a104f5bb88807e81dbe7` 的已试功能获得了正面的真机反馈；这不代表此新版已通过真机验收。

完整 v0 的喝水/浇水事务、植物成长、NVS、夜景、睡姿和自主出门仍待后续实现；当前郁金香为固定视觉预览，尚未实现成长调度。


## 当前阶段百分比显示（2026-10-01）

植物详情和浇水完成显示当前阶段完成的整数百分比，替代累计 x/56；花园进度条使用相同百分比。种子 0–4、幼苗 4–20、花苞 20–56 各阶段从零开始，开花后保持 100%，整数百分比向下取整。存储格式与成长阈值不变。

- Build：PASS — 完整 `tools/validate.sh`，ESP-IDF 5.5.3，返回 0；归档已独立核验。
- Host tests：PASS — 完整测试集、阶段起点/中点/终点和开花百分比；32 张渲染画面、实际字形检查与静止刷屏回归。
- Device tests：NOT RUN — 运行验收未完成；分段烧录校验通过，但启动采集期间 USB 断开。
- Unverified：实体百分比/进度条可读性与交互。

合并镜像：1,840,176 bytes，SHA-256 `6b575e6c5705f4033be15325d4de4630e40afa4b716d3159ca9789a2be25fd33`。ELF SHA-256：`3b86971ffd041bfd6a1b9437b07d52dfba92c26f5f7e66fa91da3a5f4aabe11c`。内嵌版本：`feb6de2-dirty`；SDK `v5.5.3`。核验归档：`build/firmware/6b575e6c5705f4033be15325d4de4630e40afa4b716d3159ca9789a2be25fd33/`。经用户明确授权，将核验归档分段刷入 COM3 的 `0x0`、`0x8000` 和 `0x10000`；三个写入哈希均通过，擦除/写入范围未覆盖 NVS。主动重启后开始 45 秒采集，但 USB 串口连接在完成前消失。只读重新枚举未找到 Passport USB 设备或 COM3。采集在保存缓冲区前失败，因此本次没有可核验的启动日志证据；串口句柄已在异常时关闭。需将开机设备用数据线重新连接后，补做限时启动检查。未再次烧录或擦除。

模拟器：PARTIAL — 本地 VOID001 模拟器加载了未经改动的核验镜像并显示小屋，UART ELF 前缀 `3b86971ff` 匹配。UP/OK 和明确按住/松开的按键电平均未观察到界面变化。采样 PC 解码到 `esp_cpu_wait_for_intr` 中 WFI 之后的指令。另在被忽略的本地诊断副本中仅将该 WFI 替换为 NOP，修正镜像校验和/SHA，并注入花苞阶段 NVS 存档，仍未恢复交互。原因尚未解决，不将百分比详情和转场记为模拟器 PASS。正式归档和应用源码不包含 WFI 绕过修改，浏览器已恢复未经改动的镜像。百分比截图来自真实 LVGL 主机渲染，不是模拟固件画面。原始日志、截图与测试镜像仅保留在被忽略的本地 `work/`。

## 配网访问与静止重绘修复（2026-10-01）

上一版访问检查只接受 IPv4，而 HTTP 服务返回 IPv4 映射的 IPv6 套接字地址，误拒绝设置热点上的请求。现改用完整套接字地址缓冲区，并识别热点地址的两种形式，继续拒绝其他目标地址。主机用例覆盖两种允许地址、家庭网络目标、原生 IPv6 和错误长度。

重复设置属性使静止画面每 20 ms 失效重绘。上一版配套 ELF 的看门狗采样位于绘制调度和文字绘制。修复先比较可见状态，再调用 LVGL 属性设置，并仅在阶段变化时切换植物图片。渲染回归检查静止花园/状态页重复更新不增加刷屏次数，而杯数和状态页运行时长变化仍触发刷屏。

- Build：PASS — 完整 `tools/validate.sh`，ESP-IDF 5.5.3，返回 0；归档已独立核验。
- Host tests：PASS — 仓库测试集、29 张渲染画面、字形检查与静止刷屏回归。
- Device tests：PASS — 仅限分段烧录及 45 秒启动观察；未观察到看门狗、崩溃、断言或错误日志。
- Unverified：手机配网、正常界面交互、Wi-Fi/SNTP 和跨日。

经用户明确授权修复并烧录，将核验归档分段刷入 COM3 的 `0x0`、`0x8000` 和 `0x10000`；三个写入哈希均通过，未覆盖 NVS 范围。主动重启并采集 45 秒日志，确认 SDK `v5.5.3`、版本 `feb6de2-dirty` 以及匹配 ELF 前缀 `24fe0a87c`。此观察窗口内未再出现上一版重复的启动看门狗警告。串口已关闭。手机配网及实体界面验收留待用户，不代表完整功能验收。合并镜像：1,840,112 bytes，SHA-256 `fab28dc27ddd1981d906733ffc058d857c9880bba214bffc08ff01867c30b42e`。ELF SHA-256：`24fe0a87c9905a11fcca116fa1e8352e4cb16fbff1b21b2f19cbdae5ff642094`。核验归档：`build/firmware/fab28dc27ddd1981d906733ffc058d857c9880bba214bffc08ff01867c30b42e/`。修复版由尚未提交的源码构建，dirty 版本表示当时的这些修改。原始日志和生成文件仅保留在被忽略的本地目录。

## 每日成长与联网验证（2026-10-01）

- Build：PASS — 完整 `tools/validate.sh`，ESP-IDF 5.5.3，返回 0；合并镜像和调试归档已独立核验。
- Host tests：PASS — 完整测试集、每日上限/多余杯数、十四天最短周期、日历边界、开花后记录、迁移、时钟保护、跨午夜冻结快照重试和 Wi-Fi 表单解码。
- LVGL 渲染与字体：PASS — 29 张画面，包含全部植物阶段、每日界面、Wi-Fi 设置/状态/清除和等待校时；实际字形检查与缺字负例。
- Device tests：FAIL — COM3 分段烧录的三个写入哈希均通过，但 15 秒启动采集发现重复的任务看门狗警告：taskLVGL 运行期间 CPU 0 的 IDLE 任务未及时运行。
- Unverified：实际配网/重连/清除凭证、SNTP/跨日、屏幕/按键、NVS 迁移/重启保留，以及联网期间内部内存余量。

合并镜像：1,839,472 bytes，SHA-256 `c2582a4634740dcf8e9484026000e18c65c9a9895142153a423930802612670f`。配套 ELF SHA-256：`2f257dc98151ea5ceb0c42814e58bc239c0e6919761eb4820cf736f204a82e8b`。内嵌版本：`4d29fba-dirty`；SDK `v5.5.3`。归档：`build/firmware/c2582a4634740dcf8e9484026000e18c65c9a9895142153a423930802612670f/`，已使用 `tools/archive_firmware.py verify` 核验。应用源码已提交为 `4d29fba`；构建在提交前启动，保留配置时的版本后缀。生成固件和调试文件不纳入 Git。

已检查联网参考：上游 `demo/blufi-provisioning` 的 `9c039cc5127f22072afa83bedb7fa3d8efe635ad`。仅参考协议栈/生命周期模式，本应用使用独立 WPA2 热点和本地网页配网，不启用蓝牙。

经用户授权，将归档 `c2582a4634740dcf8e9484026000e18c65c9a9895142153a423930802612670f` 分段刷入 COM3 的 `0x0`、`0x8000` 和 `0x10000`，保留 NVS 区域。主动重启并限时采集启动日志，确认版本 `4d29fba-dirty`、SDK `v5.5.3` 和 ELF 前缀 `2f257dc98`。约 5.5 秒和 10.5 秒出现警告。配套 ELF 解码将采样 PC 定位到 LVGL 绘制调度和文字绘制；这说明空闲任务得不到运行时正在绘图，尚不能确定根因。应用和 LVGL 初始化完成，但运行验收未通过。串口已关闭，原始日志仅保留在被忽略的本地 `work/`。尚未修复或刷入替代固件。

## 手动浇水验证（2026-09-30）

- Build：PASS — 完整 `tools/validate.sh`，ESP-IDF 5.5.3，返回 0。
- Host tests：PASS — 完整测试集、浇水时序与输入锁、重试和防重复计数、记录格式及实际 NVS 任务故障注入。
- LVGL 渲染与字体：PASS — 18 张画面，含飞行/出水/归位、保存中、失败、完成、最大计数及记录不可用；实际中文字形检查与缺字负例。
- Device tests：NOT RUN — 浇水交互验收等待用户实际观察；COM3 分段烧录和启动检查已通过。
- Unverified：实体动画、新文字可读性、真实 NVS 保存和重启恢复、写入时断电后的行为。

合并镜像：1,160,512 bytes，SHA-256 `a9ac0d58f4456be2dd8a606d2ad67f125908d1f838b73a29908b542515602c76`。配套 ELF SHA-256：`ab291f3774db0db4ae6195abe3b0cc7aee7f5e057dbe9a77c1ce258da7907f5b`。内嵌版本：`0125d4a-dirty`；SDK `v5.5.3`。归档：`build/firmware/a9ac0d58f4456be2dd8a606d2ad67f125908d1f838b73a29908b542515602c76/`，已使用 `tools/archive_firmware.py verify` 独立核验。生成固件和调试文件不纳入 Git。

经用户明确授权，已将核验归档中的组件分段刷入 COM3 的 `0x0`、`0x8000` 和 `0x10000`，三个写入部分的哈希校验均通过。擦除和写入范围未覆盖 NVS 区域（`0x9000`–`0xEFFF`）。主动重启后采集了 15 秒启动日志，确认 ESP-IDF 5.5.3、版本 `0125d4a-dirty`、匹配的内嵌 ELF 哈希前缀 `ab291f377`，应用和 LVGL 初始化完成，观察期间未出现崩溃、看门狗或存储错误。采集后已关闭串口。原始日志保留在被忽略的本地 `work/` 目录。

沿用现有 8 MB 分区布局。分段刷写可保留 NVS 分区；从 `0x0` 刷写合并镜像可能重置已保存的浇水次数。两种方式都应先确认设备并获得授权。

## 核心互动回归（2026-10-08）

转场阻塞期间已稳定松开，但若新按键落在第一轮非阻塞采样，残留输入保护可能吞掉新操作。针对用例在旧实现上失败，改为先根据已确认的松开解除保护，再更新输入候选值后通过。仍按住的键、ADC 故障及短于消抖时间的松开继续被抑制。

原始按键流程按应用的 10 ms 采样顺序覆盖：欢迎／关闭、往返镜头、水壶选择、日期缺失保护、浇水保存失败／重试且不重复记杯、焦点保留、小屋长按唤醒和花园 OK 唤醒不误操作。保留已有持久化故障注入测试。

- Build：PASS — 完整 `tools/validate.sh`，ESP-IDF 5.5.3，返回 0；合并镜像和配套调试归档已独立核验。
- Host tests：PASS — 完整测试集及原始按键／转场松开专项回归。
- LVGL 渲染与字体：PASS — 32 张画面、实际字形覆盖和静止重绘检查；已目视检查核心场景／弹窗／门口／浇水画面。
- Device tests：NOT RUN — Windows 只读枚举未识别到 Passport；没有打开串口或烧录。
- Simulator：PARTIAL — 未修改的归档 `fab28dc2...` 和 `a9ac0d58...` 启动显示小屋，但 OK 和明确持续按住 UP 未观察到响应。最初的 `831f7d1d...` 欢迎弹窗正常响应，内置 Demo 的 DOWN 也正常响应。
- Unverified：模拟器根因、确切新镜像交互、实体按键映射／转场、背光／唤醒和 NVS 恢复。

已独立核验 `fab28dc2...` 归档，启动 ELF 前缀 `24fe0a87c` 匹配；采样 PC `0x40385b9e` 用配套 ELF 解码到 `esp_cpu_wait_for_intr`。这仅说明采样时位于等待位置，不证明某个计时器故障。没有把仅供模拟器诊断的修改加入应用或 BSP。Wi-Fi 行为、字体、素材和存储格式均不变。

合并镜像：1,840,176 bytes，SHA-256 `dbcdec658617fd8275f653db23042d6082b4e7825483ea5a93a9baab0c968eb1`。配套 ELF：`d5dcb2a314ccf8927bdffa472af8cb20dacdfe08bce6dab7fdceb978a36d7172`。内嵌版本 `d849764-dirty`；SDK `v5.5.3`。归档：`build/firmware/dbcdec658617fd8275f653db23042d6082b4e7825483ea5a93a9baab0c968eb1/`。`0x0`、`0x8000`、`0x10000` 的组件保持现有分区布局；兼容的分段烧录可避开 NVS，从 `0x0` 烧录合并镜像可能重置 NVS。本轮未烧录。

新镜像模拟器检查：PARTIAL。未修改的 `dbcdec65...` 合并镜像显示 Cottage 小屋；UART 版本为 `d849764-dirty`，SDK `v5.5.3`，ELF 前缀 `d5dcb2a31` 匹配。OK 和明确按住／松开 UP 仍未观察到弹窗或转场。本轮修复独立复现的输入保护问题，不宣称已修复模拟器运行问题。最初 `831f7d1d...` 对照镜像也完成了向左进入花园的转场。

## 2026-10-08：校时前可用的 local-first 浇水

离线浇水不再等待 Wi-Fi／SNTP。累计运行满 24 小时刷新四次成长额度，进度持久化；更多杯数只记杯，不再成长，关机时间不计入。首次校时保留本轮杯数和已用额度，下一向前自然日再刷新。版本 1／2 记录迁移不擦除植物或杯数。界面显示本轮杯数、四个槽位和明确刷新规则；无可信日期时显示离线计时。

- Build：PASS — 完整 `tools/validate.sh`，ESP-IDF 5.5.3，返回 0；合并镜像及配套调试归档已独立核验。未烧录实体设备。
- Host tests：PASS — 成长／记录、世界状态／原始按键流程和真实 NVS 任务回归，覆盖 24 小时边界、多轮、重启进度／额度、日期回退、保守校时、纯计时校验、保存失败／重试和待机后保留重试。
- LVGL 渲染／字体：PASS — 36 张画面，含离线额度、植物详情、离线状态、额度刷新和自然日过渡；已核验实际字形描述和静止画面刷新回归。主机画面不等于真机渲染验收。
- Device tests：NOT RUN — 只读串口枚举仅发现电脑通信端口 COM1，没有检测或打开 Passport 设备。
- Unverified：实体离线冷启动、断电后额度／进度保留、真实 SNTP 衔接／跨午夜、屏幕／按键、联网和 UI 并行内存，以及已有模拟器互动限制。

60 秒检查点会等待当前操作或按住的键，存储失败需重试。突然断电只丢失上次成功检查点之后的计时，从而延后下一轮；已保存杯数、成长与额度保持原子性。版本 3 迁移不能由旧固件读取，回退时必须保留原记录，不得擦除。

已核验完整镜像：1,844,048 bytes；SHA-256 `1afdbbd42d5df6572e29b57b1cb7676d2bb8cbd69765a27da228e850efbdea44`。匹配 ELF SHA-256：`e4ecf46d9c229c1d9fdf5491baa74a8db50497c0f31857baecc3889d90b86ce8`。版本 `d849764-dirty`，SDK `v5.5.3`。归档：`build/firmware/1afdbbd42d5df6572e29b57b1cb7676d2bb8cbd69765a27da228e850efbdea44/`。应用 1,778,512 bytes，保留现有 8 MB 分区。兼容分段写入 `0x0`、`0x8000`、`0x10000` 避开 NVS；合并镜像从 `0x0` 写入可能重置 NVS。

Simulator：该确切镜像为 PARTIAL。启动信息确认 ESP-IDF `v5.5.3`、`d849764-dirty`、ELF 前缀 `e4ecf46d9`，小屋和角色可见。OK 单击与明确按住／松开 UP 产生事件记录，但未观察到弹窗或花园转场；复现此前限制，不能证明离线互动。正式固件未加入模拟器专用补丁。

本轮工作更新文件（包括前一批 Milestone A 输入保护修复）：

- 应用：`main/main.c`；`main/passport/input.c`；`main/passport/` 下的 `growth.c`、`growth.h`、`growth_record.c`、`growth_record.h`、`storage.c`、`storage.h`、`view.c`、`world.c`、`world.h`。
- 字体：`assets/fonts/passport/passport_font_18.c`、`tools/prepare_passport_font.ps1`。
- 测试：`tests/test_passport.c`、`tests/test_passport_growth.c`、`tests/test_passport_storage.c`、`tests/passport_render/render.c`。
- 文档入口：`README.md`、`README.zh_CN.md`、`docs/README.md`、`docs/README.zh_CN.md`。
- 产品文档：`docs/application/vision.md`、`vision.zh_CN.md`、`roadmap.md`、`roadmap.zh_CN.md`。
- 架构与证据：`docs/application/passport-v0.md`、`passport-v0.zh_CN.md`、`passport-v0-validation.md`、`passport-v0-validation.zh_CN.md`。

下一步：连接 Passport 后授权确切镜像的真机测试，验收离线冷启动、五杯、重启保留、额度边界、真实校时，以及原有转场／唤醒。完成这些检查前不扩展未来功能。未提交或推送代码。

## 2026-10-08：安静培养与顶部栏小水滴

花园顶部栏使用四滴 10×14 像素的小水滴，直接放在对象名称和 Wi-Fi 图标之间。已移除白色杯数面板、可见杯数、成长条、百分比及周期刷新说明。植物详情改为各阶段简短描述，浇水完成只给一句轻轻的回应。后台记杯、额度、离线计时、迁移和已保存成长保持原有规则。

- Build：PASS — 完整 `tools/validate.sh`，ESP-IDF 5.5.3，返回 0；合并镜像及配套调试归档已独立核验。
- Host tests：PASS — 仓库／静态及原有状态／存储测试。LVGL 渲染／字体：PASS，36 张当前画面；后台杯数和同阶段进度变化保持静止，保留的水滴与植物阶段变化仍更新。新增字形已实际覆盖。
- Device tests：NOT RUN — 尚无烧录授权，也未检测到 Passport。
- Unverified：实体屏幕上小水滴与文字可读性、实际互动及已有模拟器输入限制；本轮未做模拟器验收。

更新文件：`main/passport/view.c`、`tests/passport_render/render.c`、`tools/prepare_passport_font.ps1`、`assets/fonts/passport/passport_font_18.c`，以及 README、愿景、路线图、架构和验证记录的中英配对文件。下一步：核验确切新镜像，再提供真机显示测试，保持 local-first 规则。

已核验完整镜像：1,844,400 bytes；SHA-256 `25185440ab08d0089cdfa4ec98a0a7435ee7d27edfb5c72dcad146f64fd8e2bf`。匹配 ELF SHA-256：`941399ae44e042a78c51fb659937c70016e58d47f57b01448bc3ebbd9e6e8b5a`。版本 `d849764-dirty`，SDK `v5.5.3`。归档：`build/firmware/25185440ab08d0089cdfa4ec98a0a7435ee7d27edfb5c72dcad146f64fd8e2bf/`。应用：1,778,864 bytes。现有分区兼容分段写入 `0x0`、`0x8000`、`0x10000` 并避开 NVS；合并镜像从 `0x0` 写入可能重置 NVS。未烧录设备，未提交／推送代码。只读串口枚举仍仅发现 COM1。

## 模拟器联网配置 — 2026-10-09

模拟器配置自动连接开放的 `Emulator Host Bridge`，使用电脑当前网络。
Wi-Fi 驱动的凭据 NVS 已关闭，配置只放在 RAM，不加载或覆盖真机凭据。
应用浇水 NVS 保持原有规则。状态页清除停止重连，设置恢复虚拟连接。
只有模拟器 defaults 启用 UART0 应用日志；普通固件保留 USB 控制台和真机
配网行为。模拟器配置不能烧录到真机。

- Build：PASS — 普通版完整验证和模拟器版完整验证，ESP-IDF 5.5.3；
  两份合并镜像／调试归档独立核验，导出文件哈希一致。
- Host tests：PASS — 完整静态验证；真实网络 worker 的连接、SNTP、忘记、
  恢复、已连接时重复设置、瞬时配置错误重试及失败清理测试。
  模拟器网络桥 12 项测试 PASS；电脑直接收到有效的 48-byte NTP 响应。
- 模拟器联网：PASS，使用下方最终镜像。UART 显示 ELF 前缀 `80374595f`、
  Wi-Fi 凭据 NVS 关闭、连接虚拟热点、IP `192.168.4.2`，并在启动后
  6071 ms 打印 `Network time synchronized`。此前仅联网版 `d245cd33...`
  已显示 DNS、NTP 流量和应用的绿色 Wi-Fi 图标，但 USB 应用日志无法进入
  UART 检查器；已由最终 UART 配置替代。
- Device tests：NOT RUN — 没有烧录授权，只发现 COM1。
- Unverified：模拟器 UI 按键／状态页清除与恢复、未校时离线冷启动、
  真机 Wi-Fi 配网／重连及显示／输入。联网通过不代表 Milestone A 输入验收完成。

最终镜像上，DOWN 单击和按电脑时间控制的 300 ms DOWN 按下／释放都产生了
检查器输入事件，但未观察到焦点变化。按键已释放。UI 验收仍为 PARTIAL，
尚未确定原因。

模拟器完整镜像：1,797,024 bytes，SHA-256
`a209707affb7ad9d6ef80456b9d00a65b16b74d8db029ec6fe396f1c7c33a681`；
应用 1,731,488 bytes；匹配 ELF SHA-256
`80374595f52f8c5bffafa52a2d6a70e55ae4b5f47beb8cd270d70e0dbabe2374`。
归档：`build/firmware/a209707affb7ad9d6ef80456b9d00a65b16b74d8db029ec6fe396f1c7c33a681/`。
普通完整镜像：1,844,672 bytes，SHA-256
`3fd7ee7d3e98edd81b6b8b0db425100f6eb52b1f863452a26ac9194ef392bcf0`；
应用 1,779,136 bytes；匹配 ELF SHA-256
`b1f680fb4c5b098910cdf01bc7cdc7698c6bffe99246ab20bd5337bda266b27d`。
归档：`build/firmware/3fd7ee7d3e98edd81b6b8b0db425100f6eb52b1f863452a26ac9194ef392bcf0/`。
两版均为 `d849764-dirty`、SDK `v5.5.3`。未烧录、提交或推送。

更新文件：`main/passport/network.c`、`main/Kconfig.projbuild`、
`sdkconfig.simulator.defaults`、`tools/validate.sh`、`tests/test_passport_network.c`、
`tests/passport_network_stubs/`，以及 README、架构、路线图、构建／测试与验证
记录的中英配对文件。下一步：在这份确切镜像上复现模拟器输入，再验证状态页
断网／恢复，然后在启动前阻断网络，验证未校时的离线路径。真机验收独立进行，
只在明确获得烧录授权后使用普通固件。

## 本地 MVP 补齐与精确镜像验收：2026-10-09

用户授权在次日早晨验收前补齐现有 Cottage 本地 MVP。未新建项目、加入云端功能、烧录、提交或推送。在 `feature/cottage-watering`、HEAD `d849764` 上继续工作，保留原有修改。

已实现：月光连续场景、坐睡角色图、07:00／19:00 日夜边界、22:00–07:00 睡眠及晚安弹窗、每天一次可重现的半小时散步、OUT 时隐藏角色／地面标记、仍可使用魔法水壶，以及记忆生活时间后的离线运行延续。独立版本化、带校验的生活存档通过已有 worker 保存；浇水 v3 记录和安静的四滴界面保留。生成的恒定图片数据共 906,840 字节，存于 Flash；这不是实测真机内存结果。

- Build：PASS — ESP-IDF 5.5.3 普通与模拟器完整门禁均退出 0，两份完整镜像／调试归档独立校验通过。构建使用原工作目录与原工具链；中止的临时原生目录副本未用于交付。
- Host tests：PASS — 完整静态门禁、纯日夜／睡眠／散步边界、storage worker 生活时间持久化／失败重试／未知记录保留，以及 OUT 浇水测试。实际 LVGL 渲染／字形检查通过，共 42 帧，包含新睡眠字形与闲置刷新回归。网络桥 12 项回归全部通过。
- Simulator：下述有限流程 PASS；Milestone A 真机验收仍待完成。模拟器结果不代表硬件结果。
- Device tests：NOT RUN — 用户安排次日早晨验收后再进行真机测试；未烧录任何固件。
- Unverified：实体屏幕颜色／中文字形可读性、睡姿尺寸、按键／ADC 时序、背光、堆栈余量、真实配网／BLE／Wi-Fi、突然断电行为与完整设备验收。精确时间／额度／散步边界使用主机测试覆盖，未进行 24 小时模拟器持续运行。门口中间帧有主机渲染截图，本次未在模拟器独立捕获该帧。

### 最终镜像身份

| 配置 | 完整镜像 SHA-256 | 完整／应用字节数 | 匹配 ELF SHA-256 |
| --- | --- | --- | --- |
| 模拟器 | `c02ff6266eb4f74fd69d67d40391cd480b275c9d47895c403b7f65917c7a7ac9` | 2227616 / 2162080 | `348384846c16cceb1f563ed8011fbba32c77c38b9c6dd979fb50e8cf2e85614b` |
| 真机 | `98a46c48653bc0833e65caac53d96d318158dc151c9baf2a5a33cd2d952c5bcd` | 2274944 / 2209408 | `7a7ddf8f34fb756c0a4bda11733f2f9b53075e1a31f524224385bbb73e7c22c2` |

两者版本均为 `d849764-dirty`，SDK `v5.5.3`。匹配归档位于 `build/firmware/<完整镜像sha256>/`。模拟器使用 RAM Wi-Fi 与 UART0，禁止将此配置烧录真机；设备验收使用普通固件。NVS／PHY／factory 分区保持不变。真机归档的 `0x0`、`0x8000`、`0x10000` 分段写入不包含 NVS；从 `0x0` 写完整合并镜像可能重置存档。不需要整片擦除。

### 实际模拟器流程

未修改的最终模拟器镜像启动日志匹配 ELF 前缀 `348384846`，获得 IP 并接受 SNTP。真实网络时间自动显示夜景与 HOME_SLEEP，睡眠弹窗可读。状态页显示日期，确认清除后断开连接。离线花园浇水完成、消耗顶部一滴；硬重启保留三滴。重启也按开发配置恢复虚拟网络。

只暂停主机网络桥后冷启动同一未修改镜像，没有接受 SNTP，也没有已记忆的生活时间：仍可使用白天 HOME_IDLE 并完成浇水。状态页显示离线计时。恢复网络桥，并在清除后选择状态页“设置 Wi-Fi”，重新获得 IP 和 SNTP（该次运行 `717664` ms），状态页随后显示 2026-10-09 和睡眠。已观察欢迎弹窗开关、小屋／花园焦点、往返终点、首次唤醒消耗。使用本地输入适配器并等待模拟器 350 ms 单／双击分派窗口后，正常屏幕 OK／DOWN 按钮可用；明确的 ADC 电平短按也可用。交付时所有按住的键均已松开。

在未修改的旧网络归档镜像复现此前互动限制后，明确电平短按／长按可以响应。不能据此判定固件普遍存在输入故障。`tools/cottage_simulator_input.patch` 提供本地墙钟节奏 ADC 短按适配，`tools/cottage_simulator_offline.patch` 增加网络传输测试开关；二者都不改变固件。暂停互联网传输时虚拟关联／DHCP 仍可能保持，绿色连接图标不能作为互联网可达证据。

另用合成 NVS 存档实际检查：19:01 夜景清醒及发芽、23:00 睡眠及花蕾、当日散步及开花。OUT 隐藏角色和地面标记，仍可完成浇水并消耗一滴。夹具只修改最终模拟器基底的 `0x9000–0xEFFF`，应用、引导与分区字节完全一致。三份 SHA-256 依次为 `c65b310f1887d00292dc6f3005b94f3869f0a54b2ac45fdcdf9013dad1c9ab21`、`e1992e5ebfec9992162a9c0b68acf9840e6f1b22d230773e496bf3bdad897af1`、`4a080f9d793601b89ef37046fd9b1508fcca5e06d59b6f60a646db857c289c8a`。它们是测试夹具，不是交付固件。早期夹具 CSV 错将文件路径写成 `data,binary`，导致 `ESP_ERR_NVS_INVALID_LENGTH`；改为 `file,binary` 后读取恢复。失败夹具不计入生产验收。普通配置夹具互动未验收通过。最终浏览器已恢复未修改模拟器镜像和在线网络桥。

### 更新文件与下一步验收

- 应用：`main/main.c`、`main/CMakeLists.txt`、`main/passport/life.c`、`life.h`、`storage.c`、`storage.h`、`network.c`、`network.h`、`world.c`、`world.h`、`view.c`。
- 素材／工具：夜景与睡姿原图／目标 PNG、`assets/images/passport/passport_images.c`、`assets/fonts/passport/passport_font_18.c`、`tools/prepare_passport_images.py`、`tools/prepare_passport_font.ps1`、两份模拟器补丁。
- 检查：`tests/test_passport_life.c`、`tests/test_passport_storage.c`、`tests/test_passport.c`、`tests/passport_render/render.c`、`tools/validate.sh`。
- 中英成对文档：根 README、素材 README、vision、roadmap、架构及此验证记录。之前离线／网络批次文件仍属于累计工作区修改。

下一步：早晨模拟器验收，然后获得明确授权后使用普通镜像做真机冷启动、三键／唤醒、往返、离线五杯／重启、真实 Wi-Fi 配网／重连、生活场景和存档保留测试。云同步、Journal、明信片、图鉴、报表及自定义角色继续后置。
