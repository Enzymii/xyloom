<p align="right">
  <a href="README.md">English</a> · <strong>简体中文</strong>
</p>


产品方向与下一批小任务：[愿景](docs/application/vision.zh_CN.md) · [路线图](docs/application/roadmap.zh_CN.md)。

# Xyloom · 梦隅

*A little world for someone to live in.*

梦隅，一隅小世界。当前应用 **Cottage** 是运行在 FoloToy AI Passport 上的陪伴原型，让沫纯住进小屋与花园组成的小天地。

## 命名说明

| 名称 | 层级与含义 |
| --- | --- |
| **Xyloom / 梦隅** | 总项目与产品名。梦隅意为“梦里的一个小角落 / 一隅小世界”。 |
| **Cottage** | Passport 当前 app。 |
| **Nookling** | 角色系统的预留术语，尚未作为功能落地。 |
| **matsujun / 沫纯** | 默认角色；英文场合使用日语罗马音拼写 matsujun。 |

## 当前范围

Milestone A 原型包含小屋与花园、角色显示、对话框、按键输入、镜头转场、背光待机，以及显示电量、开机时长和角色状态的状态页。实现边界与后续工作见[应用架构](docs/application/passport-v0.zh_CN.md)，验证状态见[验证记录](docs/application/passport-v0-validation.zh_CN.md)。

现在也可以在花园选中水壶、按 OK 手动浇水：水壶飞到花旁出水，再返回原位。每次完成浇水记录一杯；每轮前四杯推进成长：0 点种子、4 点幼苗、20 点花苞、56 点开花。今日杯数、每日额度、成长和累计杯数断电后保留，开花后继续记录喝水。状态页可开启临时热点，用手机设置 Wi-Fi；设备记住一个网络并自动校时，离线也立即可用；无可信日期时，累计运行满 24 小时刷新四次成长额度，重启保留本轮进度和额度，关机时间不计入。首次校时保留已用额度，下一可信日期再按自然日刷新。电量旁常驻 Wi-Fi 图标。花园顶部栏保留四滴小水滴，不加底框；通过植物外观与轻柔的查看文案感受成长，不展示百分比、成长条、杯数或计时规则。

## 项目结构

| 路径 | 内容 |
| --- | --- |
| `main/main.c` | Cottage 启动与硬件适配。 |
| `main/passport/` | Cottage 的输入、世界状态与画面。 |
| `assets/images/passport/`、`assets/fonts/passport/` | 应用图像与字体。 |
| `components/bsp/` | FoloToy AI Passport 板级支持。 |
| `docs/application/` | 原型架构与验证记录。 |
| `tests/`、`tools/` | 测试、素材处理与验证工具。 |

固件沿用现有 `FoloToy-AI-Passport` CMake 项目标识和目录布局。硬件与开发指南见[上游文档索引](docs/README.zh_CN.md)和[构建说明](docs/development/engineering/build-and-test.zh_CN.md)。

## 模拟器联网配置

运行 `./tools/validate.sh --simulator`，将 `build/Cottage-simulator-full.bin`
上传到本地 FoloToy 模拟器。Cottage 会通过电脑当前的网络联网，不需要家庭
Wi-Fi 凭据。在状态页选择清除以断网，选择设置以恢复连接。这个只使用 RAM
网络配置的开发版本不能刷到真机。参见[配置说明](docs/application/passport-v0.zh_CN.md#模拟器联网)。

## 许可

代码沿用现有 [MIT 许可](LICENSE)，字体保留 SIL Open Font License。应用插画不适用代码的 MIT 许可，具体见[素材来源与权限说明](assets/README.zh_CN.md#passport-v0-素材)。

## MVP 生活节奏

Cottage 已加入月光花园与暖灯小屋，沫纯在 22:00–07:00 坐睡，并每天散步半小时。记住的生活时间在离线开机期间继续流动，重启可恢复；从未校时的首次启动显示白天。睡眠和出门期间仍可浇水。真机验收前请查看[生活规则](docs/application/passport-v0.zh_CN.md#本地生活节奏2026-10-09)和对应[验证记录](docs/application/passport-v0-validation.zh_CN.md)。
