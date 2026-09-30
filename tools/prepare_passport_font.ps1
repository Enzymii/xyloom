$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    python -m fontTools.varLib.instancer assets/fonts/passport/NotoSansSC.ttf wght=500 --output assets/fonts/passport/NotoSansSC-Medium.ttf
    if ($LASTEXITCODE -ne 0) { throw 'Font instancing failed' }
    npx --yes lv_font_conv@1.5.3 --font assets/fonts/passport/NotoSansSC-Medium.ttf --range 0x20-0x7e --symbols '沫纯主人回来啦～小屋花园房间郁金香水壶出门了屋里静悄悄的。这是种下橙色记录喝将在后续阶段开放睡着轻一点状态设备与电量运行小时分在家悠闲按返回' --size 18 --bpp 4 --format lvgl --no-compress --lv-font-name passport_font_18 --lv-include lvgl.h --output assets/fonts/passport/passport_font_18.c
    if ($LASTEXITCODE -ne 0) { throw 'Font conversion failed' }
} finally {
    Pop-Location
}
