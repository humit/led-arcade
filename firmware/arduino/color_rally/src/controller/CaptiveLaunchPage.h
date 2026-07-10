#pragma once
#include <Arduino.h>

const char CAPTIVE_LAUNCH_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="tr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>LED Arcade</title>
<style>
html,body{margin:0;min-height:100%;background:#080b12;color:#fff;font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}
body{display:grid;place-items:center;padding:18px;box-sizing:border-box}.card{width:min(92vw,520px);background:#151b2a;border:1px solid #2b3448;border-radius:28px;padding:26px;text-align:center;box-sizing:border-box}h1{margin:0 0 8px;font-size:34px}.lead{color:#b8c2d6;line-height:1.5}.button{display:flex;align-items:center;justify-content:center;width:100%;min-height:68px;margin-top:22px;border-radius:18px;background:linear-gradient(180deg,#2687ff,#123a77);color:#fff;text-decoration:none;font-size:20px;font-weight:900}.tiny{margin-top:18px;color:#7e8aa1;font-size:13px;line-height:1.5;overflow-wrap:anywhere}
</style>
</head>
<body>
<main class="card">
<h1>LED Arcade</h1>
<p class="lead">Wi-Fi bağlantısı hazır. Hareket sensörünü kullanabilmek için güvenli controller ekranını Safari'de aç.</p>
<a class="button" href="https://play.nevershire.net:444/" target="_blank" rel="noopener">CONTROLLER'I AÇ</a>
<p class="tiny">Açılmazsa Safari adres çubuğuna<br><strong>https://play.nevershire.net:444/</strong><br>yaz.</p>
</main>
</body>
</html>
)HTML";
