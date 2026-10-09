<p align="right">
  <strong>简体中文</strong> · <a href="README.md">English</a>
</p>

# 资源目录（Assets）

本目录集中存放可复用的资源（字库、图片、音乐等），按资源类型分子目录管理。每个资源放在其类型对应的子目录，并记录放置路径、命名方式、集成方式与来源/许可。二进制资源（字体、图片、音频）不属于纯 markdown 文档，请勿与文档混放。涉及版权/授权的资源需注明来源与许可。

## 字库（fonts）

可复用的字库文件与生成的字库源码放在 `fonts/`。

- 命名要能反映字族、字重、字级与格式。
- 记录来源、许可、字符范围、转换命令与目标放置路径。
- 添加字库前评估 Flash 与内部 RAM 影响；ESP32-C3 无 PSRAM。
- 不提交许可不允许分发的字库。

## 图片（images）

可复用的源图与生成的显示资产放在 `images/`。

| 文件 | 尺寸与格式 | 用途与来源 |
| --- | --- | --- |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160，JPEG | 嵌入中英文上游文档首页的产品主图，突出 AI Passport 产品形象与开放、人人可创作的理念。 |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724，PNG RGBA | 保留为可选技术参考图，不再用于首页主视觉。于 2026-09-17 使用内置图像生成工具为本仓库生成；已根据文档中的硬件能力契约核对图中的六项标签与参数。 |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336，PNG RGBA | 从仓库原始 `images/logo.png` 中精确裁切并去除背景的黑色字标；用于中英文上游文档首页的浅色主题。 |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336，PNG RGBA | 提取字标的白色版本；上游文档首页使用 `<picture>` 在 GitHub 深色主题下显示。 |

- 使用描述性命名，并记录尺寸、像素格式、转换步骤与目标路径。
- 优先采用适合 240 × 320 RGB565 显示的格式，并纳入 Flash 与内部 RAM 考量。
- 许可允许时保留可编辑源文件，并记录来源与许可。
- 图片中不得包含设备二维码秘密、凭证或个人数据。

## 音乐与音效（music）

可复用的音乐与音效源码放在 `music/`。

- 记录来源、许可、采样率、位深、声道、转换命令与目标路径。
- 与当前 BSP 音频路径匹配时优先采用 16 kHz、16 位单声道 PCM。
- 嵌入音频前评估 Flash 与内部 RAM 成本；长录音应流式或分块。
- 无再分发许可不提交媒体文件。

## Passport v0 素材

- `images/passport/tulip-seed-v1.png`、`tulip-sprout-v1.png`（1100×1430 RGBA）及 `tulip-bud-v1.png`（1100×1429 RGBA）：2026-09-30 使用内置 ImageGen，参照 `tulip-v2.png` 风格生成。提示词概要和哈希见 `tulip-growth-sources.json`，沿用下述插画授权政策。转换增加三个共用底部锚点的 55×65 RGB565A8 画布，可见高度 14/28/50 px，增加 32,175 bytes 只读 Flash 数据。保留源图 alpha，机械转换以 alpha >= 8 裁切，忽略几乎不可见的边缘噪点。

- `images/passport/world-day-master-v2.png`、`mochun-idle-reference-v3.png`、`tulip-v2.png` 与 `watering-can-v2.png`（PNG 源图；角色及物件含 alpha）：2026-09-29 使用 ImageGen 生成；matsujun / 沫纯以维护者提供的立绘为身份参考。这些插画、缩放后的 target PNG 与图像数组应维护者要求随项目公开，但不适用代码的 MIT 许可。此处不另行授予素材复用或再分发权限，如需使用请联系维护者。历史 `mochun` / `momo` 文件名和图像标识保留以兼容现有引用。
- `tools/prepare_passport_images.py` 转换为 528×320 RGB565 背景、150×200 RGB565A8 角色、55×65 RGB565A8 郁金香及 60×45 RGB565A8 水壶，写入 `images/passport/passport_images.c` 常量数组并由 main CMake 编译。target PNG 保留缩放结果。图像有效载荷共 478,920 bytes。
- `fonts/passport/NotoSansSC.ttf`：来自 [Google Fonts Noto Sans SC](https://github.com/google/fonts/tree/main/ofl/notosanssc)，SIL Open Font License 1.1，附 `OFL.txt`。完整 TTF 不链接。`tools/prepare_passport_font.ps1` 记录确切文案和 ASCII 0x20–0x7E，使用 lv_font_conv 1.5.3 生成 weight 500 / 18 px / 4 bpp / 不压缩子集。`passport_font_18.c` 由应用编译并显式选用。`tests/passport_render/render.c` 检查实际字形描述符，含缺字负例。

### Cottage 生活素材（2026-10-09）

`images/passport/world-night-master-v1.png` 和 `matsujun-sleep-v1.png` 由内置 ImageGen 生成，分别以现有白天全景和待机角色为编辑参考，沿用现有插画授权政策。夜景提示词保留连续花园／小屋构图，改为月光室外与暖灯室内；睡姿提示词保留角色身份和服装，要求闭眼坐睡及真实透明背景。`tools/prepare_passport_images.py` 将它们转换为 `passport_world_night`（528×320 RGB565）和 `passport_momo_sleep`（150×200 RGB565A8，可见高度 155 px、底部对齐）。全部常量图片数据现在共 906,840 字节。保留原白天／待机源图，目标 PNG 和生成数组沿用既有显示格式。
