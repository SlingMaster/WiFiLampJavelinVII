/* SSDP.ino */
// Допоміжна функція: видаляє пробіли та обрізає рядок
String cleanName(const String& name) {
    String result = name;
    result.replace(" ", "");  // видаляємо всі пробіли
    result.trim();            // видаляємо пробіли на початку і в кінці
    return result;
}
String onlyName(const String& name) {
    String result = name;
    result.replace("WiFiLamp", "");  // видаляємо всі пробіли
    result.trim();            // видаляємо пробіли на початку і в кінці
    return result;
}
void SSDP_init() {
    // Генеруємо унікальний ID на основі чіпа
    String chipID = String(ESP.getChipId()) + "-" + String(ESP.getFlashChipId());

    // Отримуємо ім'я пристрою та очищаємо його
    String IOT_NAME = getNameIOT(IOT_TYPE); 
    SSDP.setName(IOT_NAME);

    // ------------------- Маршрути HTTP -------------------
    HTTP.on("/description.xml", HTTP_GET, []() {
        SSDP.schema(HTTP.client());
    });

    HTTP.on("/ssdp", HTTP_GET, []() {
        String ssdpName = HTTP.arg("ssdp");
        if (ssdpName.length() > 0) {
            String cleanSSDP = cleanName(ssdpName);
            jsonWrite(configSetup, "SSDP", cleanSSDP);
            SSDP.setName(cleanSSDP);
            saveConfig();
            HTTP.send(200, "text/plain", "OK");
        } else {
            HTTP.send(400, "text/plain", "ERROR: No SSDP name provided");
        }
    });

    // ------------------- Налаштування SSDP -------------------
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);

    String ssdpNameFromConfig = cleanName(jsonRead(configSetup, "SSDP"));
    SSDP.setName(ssdpNameFromConfig);
    SSDP.setSerialNumber(chipID);
    SSDP.setURL("/");
    SSDP.setModelName("Firmware | " + IOT_NAME + " | ver." + VERSION);
    SSDP.setModelNumber(chipID + " | " + onlyName(ssdpNameFromConfig));
    SSDP.setModelURL("https://github.com/SlingMaster/" + cleanName(IOT_NAME));
    SSDP.setManufacturer("© Jeneral Samopal Company");
    SSDP.setManufacturerURL("http://winecard.ltd.ua/Portfolio/portfolio/");

    SSDP.begin();
}
