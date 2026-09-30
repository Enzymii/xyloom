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
