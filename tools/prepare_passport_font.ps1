$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    python -m fontTools.varLib.instancer assets/fonts/passport/NotoSansSC.ttf wght=500 --output assets/fonts/passport/NotoSansSC-Medium.ttf
    if ($LASTEXITCODE -ne 0) { throw 'Font instancing failed' }
    npx --yes lv_font_conv@1.5.3 --font assets/fonts/passport/NotoSansSC-Medium.ttf --range 0x20-0x7e --symbols '·。不中主了人今保储先出分取可右后启啦器回在壶备失子存完家密小屋左已幼开录待态悠成手打择按接日时期未机来杯查校检橙正水沫浇浏消清满点热状用电着睡码确种等累纯网置联色花苗苞行览计认记设试请读败运返连选郁配重量金长门闲除香？～' --size 18 --bpp 4 --format lvgl --no-compress --lv-font-name passport_font_18 --lv-include lvgl.h --output assets/fonts/passport/passport_font_18.c
    if ($LASTEXITCODE -ne 0) { throw 'Font conversion failed' }
} finally {
    Pop-Location
}
